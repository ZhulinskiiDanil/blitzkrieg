#include "SessionAttemptCell.hpp"

#include <Geode/ui/TextArea.hpp>
#include <fmt/chrono.h>

namespace
{
  const ccColor4B CELL_COLOR{40, 40, 40, 255};
  const ccColor4B CELL_EXPANDED_COLOR{46, 46, 46, 255};
  const ccColor4B SELECTED_ROW_COLOR{255, 220, 90, 38};

  // Short enough for the one line head
  const char *getShortOutcomeName(AttemptOutcome outcome)
  {
    switch (outcome)
    {
    case AttemptOutcome::Counted:
      return "Counted";
    case AttemptOutcome::CountedChecked:
      return "Counted, done";
    case AttemptOutcome::RunPassed:
      return "Passed";
    case AttemptOutcome::StageClosed:
      return "Stage closed";
    case AttemptOutcome::Dropped:
      return "Dropped";
    default:
      return "Ignored";
    }
  }

  // Time of the day, with the date when it was not today
  std::string formatAttemptTime(std::time_t time)
  {
    if (time <= 0)
      return "-";

    const auto local = geode::localtime(time);
    const auto today = geode::localtime(std::time(nullptr));

    if (local.tm_year == today.tm_year && local.tm_yday == today.tm_yday)
      return fmt::format("{:%H:%M:%S}", local);

    return fmt::format("{:%b %d %H:%M}", local);
  }

  CCLabelBMFont *createLabel(
      std::string const &text,
      const char *font,
      float scale,
      float maxWidth,
      CCPoint anchor = {0.f, .5f})
  {
    auto label = CCLabelBMFont::create(text.c_str(), font);
    label->setAnchorPoint(anchor);
    label->limitLabelWidth(maxWidth, scale, scale * .4f);
    return label;
  }

  // Wrapped text, its top left corner goes to the position
  SimpleTextArea *createText(std::string const &text, float scale, float width, ccColor4B color)
  {
    auto area = SimpleTextArea::create(text, "chatFont.fnt", scale, width);
    area->setWrappingMode(WrappingMode::WORD_WRAP);
    area->setColor(color);
    area->setAnchorPoint({0.f, 1.f});
    return area;
  }
}

ccColor3B getAttemptOutcomeColor(AttemptOutcome outcome)
{
  switch (outcome)
  {
  case AttemptOutcome::Counted:
    return {255, 220, 90};
  case AttemptOutcome::CountedChecked:
    return {120, 180, 255};
  case AttemptOutcome::RunPassed:
    return {99, 224, 110};
  case AttemptOutcome::StageClosed:
    return {150, 255, 160};
  case AttemptOutcome::Dropped:
    return {253, 106, 106};
  default:
    return {130, 130, 130};
  }
}

SessionAttemptCell *SessionAttemptCell::create(SessionAttempt const &attempt, float width, bool expanded)
{
  auto ret = new SessionAttemptCell();
  if (ret && ret->init(attempt, width, expanded))
  {
    ret->autorelease();
    return ret;
  }

  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool SessionAttemptCell::init(SessionAttempt const &attempt, float width, bool expanded)
{
  if (!CCNode::init())
    return false;

  m_id = attempt.id;
  m_expanded = expanded;

  // ! --- Details first, the height depends on them --- !
  CCNode *details = nullptr;
  float detailsHeight = 0.f;

  if (expanded)
  {
    details = CCNode::create();
    detailsHeight = buildDetails(details, attempt, width);
  }

  const float height = HEAD_HEIGHT + (expanded ? detailsHeight + PADDING : 0.f);
  this->setContentSize({width, height});

  this->addChild(RectNode::create(
      {width, height},
      ccc4FFromccc4B(expanded ? CELL_EXPANDED_COLOR : CELL_COLOR),
      5.f));

  buildHead(attempt, width, height);

  if (details)
  {
    details->setPosition({0.f, height - HEAD_HEIGHT});
    this->addChild(details);
  }

  return true;
}

// ! --- Head: one line --- !

void SessionAttemptCell::buildHead(SessionAttempt const &attempt, float width, float top)
{
  const float y = top - HEAD_HEIGHT / 2;
  const auto outcomeColor = getAttemptOutcomeColor(attempt.outcome);

  // ! Outcome dot
  const float dotSize = 5.f;
  auto dot = RectNode::create({dotSize, dotSize}, ccc4FFromccc3B(outcomeColor), dotSize / 2);
  dot->setPosition({8.f, y - dotSize / 2});
  this->addChild(dot);

  // ! Columns, left to right
  auto id = createLabel(fmt::format("#{}", attempt.id), "bigFont.fnt", .24f, 26.f);
  id->setOpacity(120);
  id->setPosition({17.f, y});
  this->addChild(id);

  auto range = createLabel(
      fmt::format("{:.2f} - {:.2f}%", attempt.from, attempt.to),
      "bigFont.fnt",
      .3f,
      78.f);
  range->setPosition({46.f, y});
  this->addChild(range);

  std::string targetText = "-";

  if (attempt.hasRange() && attempt.stageIndex >= 0)
    targetText = fmt::format("Stage {}   {:.2f}-{:.2f}%", attempt.stageIndex + 1, attempt.rangeFrom, attempt.rangeTo);
  else if (attempt.hasRange())
    targetText = fmt::format("{:.2f}-{:.2f}%", attempt.rangeFrom, attempt.rangeTo);
  else if (attempt.stageIndex >= 0)
    targetText = fmt::format("Stage {}", attempt.stageIndex + 1);

  auto target = createLabel(targetText, "bigFont.fnt", .26f, 100.f);
  target->setOpacity(200);
  target->setPosition({128.f, y});
  this->addChild(target);

  auto outcome = createLabel(getShortOutcomeName(attempt.outcome), "bigFont.fnt", .24f, 58.f);
  outcome->setColor(outcomeColor);
  outcome->setPosition({232.f, y});
  this->addChild(outcome);

  auto time = createLabel(
      fmt::format("{}  {:.1f}s", formatAttemptTime(attempt.startedAt), attempt.duration),
      "bigFont.fnt",
      .22f,
      std::max(20.f, width - 20.f - 296.f),
      {1.f, .5f});
  time->setOpacity(130);
  time->setPosition({width - 20.f, y});
  this->addChild(time);

  // ! Chevron, up when open
  if (auto chevron = CCSprite::createWithSpriteFrameName("purple-chevron-up-btn.png"_spr))
  {
    chevron->setScale(.35f);
    chevron->setRotation(m_expanded ? 0.f : 180.f);
    chevron->setOpacity(170);
    chevron->setPosition({width - 9.f, y});
    this->addChild(chevron);
  }
}

// ! --- Details: reason, facts and candidates --- !

float SessionAttemptCell::buildDetails(CCNode *details, SessionAttempt const &attempt, float width)
{
  const float innerWidth = width - PADDING * 2;
  float y = -DETAILS_GAP;

  // ! --- Reason --- !
  auto reason = createText(
      attempt.reason.empty() ? "No reason was recorded" : attempt.reason,
      .5f,
      innerWidth,
      {235, 235, 235, 255});
  reason->setPosition({PADDING, y});
  details->addChild(reason);
  y -= reason->getContentHeight() + DETAILS_GAP;

  // ! --- Facts, one topic per line --- !
  std::vector<std::vector<std::string>> lines;

  // The run
  std::vector<std::string> run;

  if (attempt.hasRange())
    run.push_back(fmt::format("Attempt #{} of the run", attempt.attemptNumber));
  if (attempt.newBest)
    run.push_back("New best of the run");

  lines.push_back(std::move(run));

  // How it was picked
  std::vector<std::string> selection;

  if (!attempt.pool.empty())
    selection.push_back(fmt::format("Picked from {}", attempt.pool));
  if (!attempt.rule.empty())
    selection.push_back(fmt::format("Rule: {}", attempt.rule));

  lines.push_back(std::move(selection));

  // When
  lines.push_back({
      fmt::format("Started {}", formatAttemptTime(attempt.startedAt)),
      fmt::format("Ended {}", formatAttemptTime(attempt.endedAt)),
      fmt::format("Took {:.2f}s", attempt.duration),
  });

  // Where
  std::vector<std::string> place;

  if (!attempt.levelName.empty())
    place.push_back(fmt::format("Level: {}", attempt.levelName));
  if (!attempt.profileName.empty())
    place.push_back(fmt::format("Profile: {}", attempt.profileName));

  lines.push_back(std::move(place));

  std::string factsText;

  for (auto const &line : lines)
  {
    if (line.empty())
      continue;

    if (!factsText.empty())
      factsText += '\n';

    for (std::size_t i = 0; i < line.size(); ++i)
      factsText += (i ? "   " : "") + line[i];
  }

  auto factsArea = createText(factsText, .42f, innerWidth, {170, 170, 170, 255});
  factsArea->setPosition({PADDING, y});
  details->addChild(factsArea);
  y -= factsArea->getContentHeight() + DETAILS_GAP;

  if (attempt.candidates.empty())
    return -y;

  // ! --- Candidates table --- !
  struct Column
  {
    const char *title;
    float x;
  };

  const Column columns[] = {
      {"Touched run", 0.f},
      {"Overlap", 110.f},
      {"Coverage", 170.f},
      {"Passes", 235.f},
      {"State", 295.f},
  };

  y -= DETAILS_GAP;

  for (auto const &column : columns)
  {
    auto title = createLabel(column.title, "chatFont.fnt", .42f, 70.f);
    title->setColor({255, 220, 90});
    title->setOpacity(200);
    title->setPosition({PADDING + column.x, y - TABLE_ROW_HEIGHT / 2});
    details->addChild(title);
  }

  y -= TABLE_ROW_HEIGHT;

  for (auto const &candidate : attempt.candidates)
  {
    const float rowY = y - TABLE_ROW_HEIGHT / 2;

    // The run that received the attempt
    if (candidate.selected)
    {
      auto highlight = RectNode::create(
          {innerWidth + 4.f, TABLE_ROW_HEIGHT},
          premultiplyAlpha(ccc4FFromccc4B(SELECTED_ROW_COLOR)),
          2.f);
      highlight->setPosition({PADDING - 2.f, y - TABLE_ROW_HEIGHT});
      details->addChild(highlight);
    }

    const std::string cells[] = {
        fmt::format("{}{:.2f} - {:.2f}%", candidate.selected ? "> " : "", candidate.from, candidate.to),
        fmt::format("{:.2f}%", candidate.overlap),
        fmt::format("{:.1f}%", candidate.coverage * 100.f),
        candidate.passable ? "yes" : "no",
        candidate.checked ? "done" : "open",
    };

    for (std::size_t i = 0; i < std::size(columns); ++i)
    {
      auto cell = createLabel(cells[i], "chatFont.fnt", .42f, 100.f);
      cell->setOpacity(candidate.selected ? 255 : 190);
      cell->setPosition({PADDING + columns[i].x, rowY});
      details->addChild(cell);
    }

    y -= TABLE_ROW_HEIGHT;
  }

  return -y;
}

// ! --- Touches: a tap on the head toggles, a drag stays a scroll --- !

void SessionAttemptCell::onEnter()
{
  CCNode::onEnter();

  // Does not swallow, the list still gets every touch and scrolls
  CCTouchDispatcher::get()->addTargetedDelegate(this, kCCMenuHandlerPriority, false);
}

void SessionAttemptCell::onExit()
{
  CCTouchDispatcher::get()->removeDelegate(this);
  CCNode::onExit();
}

bool SessionAttemptCell::canTakeTouch(CCTouch *touch)
{
  if (!nodeIsVisible(this))
    return false;

  const auto location = touch->getLocation();

  // ! Only the part of the cell that the list shows
  FLAlertLayer *popup = nullptr;

  for (auto *node = this->getParent(); node; node = node->getParent())
  {
    if (auto *scroll = typeinfo_cast<ScrollLayer *>(node))
    {
      const auto bottomLeft = scroll->convertToWorldSpace({0.f, 0.f});
      const auto topRight = scroll->convertToWorldSpace(scroll->getContentSize());

      if (location.x < bottomLeft.x || location.x > topRight.x ||
          location.y < bottomLeft.y || location.y > topRight.y)
        return false;
    }

    if (!popup)
      popup = typeinfo_cast<FLAlertLayer *>(node);
  }

  // ! Nothing is open over the popup, like the reset confirmation
  if (popup)
  {
    if (auto *scene = CCDirector::get()->getRunningScene())
    {
      FLAlertLayer *top = nullptr;

      for (auto *child : CCArrayExt<CCNode *>(scene->getChildren()))
      {
        if (auto *alert = typeinfo_cast<FLAlertLayer *>(child))
          top = alert;
      }

      if (top && top != popup)
        return false;
    }
  }

  // ! The head, in cell space
  const auto local = this->convertToNodeSpace(location);
  const auto size = this->getContentSize();

  return local.x >= 0.f && local.x <= size.width &&
         local.y >= size.height - HEAD_HEIGHT && local.y <= size.height;
}

bool SessionAttemptCell::ccTouchBegan(CCTouch *touch, CCEvent *)
{
  if (!canTakeTouch(touch))
    return false;

  m_touchStart = touch->getLocation();
  return true;
}

void SessionAttemptCell::ccTouchEnded(CCTouch *touch, CCEvent *)
{
  // A drag scrolled the list
  if (touch->getLocation().getDistance(m_touchStart) > TAP_DISTANCE)
    return;

  if (!canTakeTouch(touch))
    return;

  if (onExpandChanged)
    onExpandChanged(m_id, !m_expanded);
}
