#include "StageHeader.hpp"

#include <algorithm>

#include "../../../../utils/formatTimePlayed.hpp"
#include "../../../../store/GlobalStore/RunWindow.hpp"

namespace
{
  struct StatusStyle
  {
    ccColor3B accent;
    ccColor3B fill;
    const char *text;
  };

  // The same colors as the run cells
  StatusStyle getStatusStyle(StageHeaderStatus status)
  {
    switch (status)
    {
    case StageHeaderStatus::Completed:
      return {{98, 240, 70}, {48, 66, 45}, "Completed"};
    case StageHeaderStatus::Locked:
      return {{218, 80, 80}, {56, 38, 38}, "Locked"};
    default:
      return {{255, 220, 90}, {61, 58, 42}, "In progress"};
    }
  }

  const ccColor3B RUN_CHECKED_COLOR{98, 240, 70};
  const ccColor3B RUN_CURRENT_COLOR{255, 220, 90};
  const ccColor3B RUN_CURRENT_TRACK_COLOR{97, 90, 52};
  const ccColor3B RUN_OPEN_COLOR{70, 70, 70};
  const ccColor3B RUN_PROGRESS_COLOR{150, 150, 150};

  // Share of the run covered by its best attempt, 0..1.
  // The best attempt may start before the run, only the overlap counts.
  float getBestRunCoverage(Range const &range)
  {
    const bool hasBestRun = range.bestRunFrom >= 0.f && range.bestRunTo > 0.f;

    if (!hasBestRun)
      return 0.f;

    const RunWindow bestRun{.start = range.bestRunFrom, .end = range.bestRunTo};
    return std::clamp(bestRun.coverage(&range), 0.f, 1.f);
  }
}

StageHeader *StageHeader::create(float width)
{
  auto ret = new StageHeader();
  if (ret && ret->init(width))
  {
    ret->autorelease();
    return ret;
  }

  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool StageHeader::init(float width)
{
  if (!CCNode::init())
    return false;

  m_width = width;
  this->setContentSize({width, HEIGHT});

  // ! --- Badge --- !
  m_badge = CCNode::create();
  m_badge->setContentSize({BADGE_SIZE, BADGE_SIZE});
  m_badge->setAnchorPoint({.5f, .5f});
  m_badge->setPosition({BADGE_SIZE / 2, HEIGHT / 2});
  this->addChild(m_badge);

  m_badgeBorder = RectNode::create({BADGE_SIZE, BADGE_SIZE}, {1, 1, 1, 1}, BADGE_RADIUS);
  m_badge->addChild(m_badgeBorder);

  m_badgeFill = RectNode::create(
      {BADGE_SIZE - BADGE_OUTLINE * 2, BADGE_SIZE - BADGE_OUTLINE * 2},
      {0, 0, 0, 1},
      BADGE_RADIUS - BADGE_OUTLINE);
  m_badgeFill->setPosition({BADGE_OUTLINE, BADGE_OUTLINE});
  m_badge->addChild(m_badgeFill);

  m_badgeLabel = CCLabelBMFont::create("", "bigFont.fnt");
  m_badgeLabel->setPosition({BADGE_SIZE / 2, BADGE_SIZE / 2 + 1.f});
  m_badge->addChild(m_badgeLabel);

  // ! --- Status chip --- !
  m_chip = CCNode::create();
  m_chip->setPosition({getContentX(), HEIGHT - 1.f - CHIP_HEIGHT});
  this->addChild(m_chip);

  // A colored dot carries the status, the text stays calm
  m_chipDot = RectNode::create({CHIP_DOT_SIZE, CHIP_DOT_SIZE}, {1, 1, 1, 1}, CHIP_DOT_SIZE / 2);
  m_chipDot->setPosition({0.f, (CHIP_HEIGHT - CHIP_DOT_SIZE) / 2});
  m_chip->addChild(m_chipDot);

  m_chipLabel = CCLabelBMFont::create("", "bigFont.fnt");
  m_chipLabel->setScale(.26f);
  m_chipLabel->setColor({200, 200, 200});
  m_chipLabel->setAnchorPoint({0.f, .5f});
  m_chipLabel->setPosition({CHIP_DOT_SIZE + CHIP_DOT_GAP, CHIP_HEIGHT / 2});
  m_chip->addChild(m_chipLabel);

  // ! --- Run segments and stats --- !
  m_segments = CCNode::create();
  m_segments->setContentSize({getContentWidth(), SEGMENT_HEIGHT});
  m_segments->setAnchorPoint({0.f, .5f});
  m_segments->setPosition({getContentX(), 17.5f});
  m_segments->setLayout(
      RowLayout::create()
          ->setGap(SEGMENT_GAP)
          ->setAutoScale(false)
          ->setGrowCrossAxis(false)
          ->setAxisAlignment(AxisAlignment::Start)
          ->setCrossAxisAlignment(AxisAlignment::Center));
  this->addChild(m_segments);

  m_stats = CCNode::create();
  m_stats->setPosition({getContentX(), 5.f});
  this->addChild(m_stats);

  return true;
}

void StageHeader::setStage(
    int stageIndex,
    Stage *stage,
    StageHeaderStatus status,
    std::string const &currentRangeId)
{
  this->setVisible(stage != nullptr);

  if (!stage)
    return;

  updateBadge(stageIndex, status);
  updateChip(status);
  updateSegments(stage, currentRangeId);
  updateStats(stage);
}

void StageHeader::updateBadge(int stageIndex, StageHeaderStatus status)
{
  const auto style = getStatusStyle(status);

  m_badgeBorder->setColor(ccc4FFromccc3B(style.accent));
  m_badgeFill->setColor(ccc4FFromccc3B(style.fill));

  m_badgeLabel->setString(std::to_string(stageIndex + 1).c_str());
  m_badgeLabel->setColor(style.accent);
  m_badgeLabel->limitLabelWidth(BADGE_SIZE - 10.f, .6f, .3f);

  // A small pop when another stage is shown
  if (m_shownIndex >= 0 && m_shownIndex != stageIndex)
  {
    m_badge->stopAllActions();
    m_badge->setScale(1.15f);
    m_badge->runAction(CCEaseBackOut::create(CCScaleTo::create(.25f, 1.f)));
  }

  m_shownIndex = stageIndex;
}

void StageHeader::updateChip(StageHeaderStatus status)
{
  const auto style = getStatusStyle(status);

  m_chipDot->setColor(ccc4FFromccc3B(style.accent));
  m_chipLabel->setString(style.text);
}

// One segment per run in the order of the level: done, current, open
void StageHeader::updateSegments(Stage *stage, std::string const &currentRangeId)
{
  m_segments->removeAllChildrenWithCleanup(true);

  std::vector<Range const *> runs;

  for (auto const &range : stage->ranges)
  {
    if (range.consider)
      runs.push_back(&range);
  }

  if (runs.empty())
    return;

  std::sort(runs.begin(), runs.end(), [](Range const *a, Range const *b)
            { return a->from < b->from; });

  const float width = getContentWidth();
  const int count = static_cast<int>(runs.size());
  const float segmentWidth =
      (width - SEGMENT_GAP * (count - 1)) / count * SEGMENT_SCALE;

  // ! --- Too many runs: one bar filled by the share of done runs --- !
  if (segmentWidth < MIN_SEGMENT_WIDTH)
  {
    const float barWidth = width * SEGMENT_SCALE;
    const int done = static_cast<int>(std::count_if(
        runs.begin(), runs.end(), [](Range const *r)
        { return r->checked; }));

    // Track and fill overlap, so they share one node in the layout
    auto bar = CCNode::create();
    bar->setContentSize({barWidth, SEGMENT_HEIGHT});

    bar->addChild(RectNode::create(
        {barWidth, SEGMENT_HEIGHT},
        ccc4FFromccc3B(RUN_OPEN_COLOR),
        SEGMENT_HEIGHT / 2));

    if (done > 0)
    {
      bar->addChild(RectNode::create(
          {barWidth * done / count, SEGMENT_HEIGHT},
          ccc4FFromccc3B(RUN_CHECKED_COLOR),
          SEGMENT_HEIGHT / 2));
    }

    m_segments->addChild(bar);
    m_segments->updateLayout();
    return;
  }

  // ! --- Segments: done runs are full, others are filled by their best attempt --- !
  const float radius = std::min(SEGMENT_HEIGHT / 2, segmentWidth / 2);

  for (int i = 0; i < count; ++i)
  {
    auto const *run = runs[i];
    const bool isCurrent = !currentRangeId.empty() && run->id == currentRangeId;

    // Track and fill overlap, so they share one node in the layout
    auto segment = CCNode::create();
    segment->setContentSize({segmentWidth, SEGMENT_HEIGHT});
    m_segments->addChild(segment);

    if (run->checked)
    {
      segment->addChild(RectNode::create(
          {segmentWidth, SEGMENT_HEIGHT},
          ccc4FFromccc3B(RUN_CHECKED_COLOR),
          radius));

      continue;
    }

    segment->addChild(RectNode::create(
        {segmentWidth, SEGMENT_HEIGHT},
        ccc4FFromccc3B(isCurrent ? RUN_CURRENT_TRACK_COLOR : RUN_OPEN_COLOR),
        radius));

    const float fillWidth = segmentWidth * getBestRunCoverage(*run);

    if (fillWidth > 0.f)
    {
      segment->addChild(RectNode::create(
          {fillWidth, SEGMENT_HEIGHT},
          ccc4FFromccc3B(isCurrent ? RUN_CURRENT_COLOR : RUN_PROGRESS_COLOR),
          std::min(radius, fillWidth / 2)));
    }
  }

  m_segments->updateLayout();
}

// Icons instead of captions: skull for attempts, clock for time
void StageHeader::updateStats(Stage *stage)
{
  m_stats->removeAllChildrenWithCleanup(true);

  int attempts = 0;
  float timePlayed = 0.f;

  for (auto const &range : stage->ranges)
  {
    if (!range.consider)
      continue;

    attempts += range.attempts;
    timePlayed += range.timePlayed;
  }

  const std::pair<const char *, std::string> items[] = {
      {"miniSkull_001.png", std::to_string(attempts)},
      {"GJ_timeIcon_001.png", formatTimePlayed(timePlayed)},
  };

  const float iconGap = 3.f;
  const float itemGap = 10.f;
  float x = 0.f;

  for (auto const &[frame, text] : items)
  {
    if (auto icon = CCSprite::createWithSpriteFrameName(frame))
    {
      icon->setScale(STAT_ICON_SIZE / std::max(icon->getContentWidth(), icon->getContentHeight()));
      icon->setOpacity(170);
      icon->setPosition({x + STAT_ICON_SIZE / 2, 0.f});
      m_stats->addChild(icon);

      x += STAT_ICON_SIZE + iconGap;
    }

    auto label = UILabel::create(text, "bigFont.fnt", .28f);
    label->setPosition({x, 0.f});
    label->setAnchorPoint({0, .5f});
    m_stats->addChild(label);

    x += label->getContentWidth() + itemGap;
  }
}
