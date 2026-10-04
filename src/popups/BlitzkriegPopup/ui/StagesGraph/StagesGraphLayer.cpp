#include "StagesGraphLayer.hpp"

#include <algorithm>

#include "../../../../utils/formatTimePlayed.hpp"

namespace
{
  const ccColor4B ACCENT_COLOR{255, 0, 82, 255};
  const ccColor4B BUTTON_COLOR{45, 45, 45, 255};
  const ccColor4B CARD_COLOR{30, 30, 30, 255};

  const char *METRIC_SAVE_KEY = "stage-graph-metric";

  struct SummaryCard
  {
    std::string value;
    std::string caption;
  };
}

StagesGraphLayer *StagesGraphLayer::create(GJGameLevel *level, const CCSize &contentSize)
{
  auto ret = new StagesGraphLayer();
  if (ret && ret->init(level, contentSize))
  {
    ret->autorelease();
    return ret;
  }

  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool StagesGraphLayer::init(GJGameLevel *level, const CCSize &contentSize)
{
  if (!CCLayer::init())
    return false;

  m_level = level;
  m_size = contentSize;
  this->setContentSize(m_size);

  auto *profile = GlobalStore::get()->getProfileByLevel(m_level);

  if (!profile || profile->data.stages.empty())
  {
    drawEmptyState("Attach your profile first", true);
    return true;
  }

  auto columns = buildStageGraphColumns(*profile);

  if (columns.empty())
  {
    drawEmptyState("No stages to show", false);
    return true;
  }

  m_metric = Mod::get()->getSavedValue<std::string>(METRIC_SAVE_KEY, "attempts") == "playtime"
                 ? StageGraphMetric::Playtime
                 : StageGraphMetric::Attempts;

  // ! --- Best run from 0% --- !
  const float bestPercent = m_level ? static_cast<float>(m_level->m_normalPercent.value()) : 0.f;
  const auto bestX = mapPercentFromZero(columns, bestPercent);

  // ! --- Layout, top to bottom --- !
  const float summaryTop = m_size.height - TOP_PADDING;
  const float controlsY = summaryTop - SUMMARY_HEIGHT - ROW_GAP - CONTROLS_HEIGHT / 2;
  const float chartTop = controlsY - CONTROLS_HEIGHT / 2 - ROW_GAP;

  drawSummary(columns, summaryTop);
  drawMetricSwitch(controlsY);
  drawLegend(controlsY, bestX.has_value());

  // ! --- Chart --- !
  m_chart = StageBarChart::create({m_size.width - SIDE_PADDING * 2, chartTop - BOTTOM_PADDING});
  m_chart->setPosition({SIDE_PADDING, BOTTOM_PADDING});
  m_chart->onOpenStage = [this](int stageIndex)
  {
    if (onOpenStage)
      onOpenStage(stageIndex);
  };
  this->addChild(m_chart);

  m_chart->setData(std::move(columns), m_metric);
  m_chart->setBestFromZero(bestX, bestPercent);

  return true;
}

// ! --- Empty state --- !

void StagesGraphLayer::drawEmptyState(const char *text, bool withProfilesButton)
{
  auto label = CCLabelBMFont::create(text, "bigFont.fnt");
  label->setScale(.75f);
  label->setOpacity(255 * .6f);
  label->setPosition(m_size / 2 + CCPoint{0.f, withProfilesButton ? 15.f : 0.f});
  this->addChild(label);

  if (!withProfilesButton)
    return;

  auto btn = CCMenuItemSpriteExtra::create(
      ButtonSprite::create("Open Profiles"),
      this,
      menu_selector(StagesGraphLayer::onOpenProfilesBtn));
  btn->setScale(.7f);
  btn->m_baseScale = .7f;

  auto menu = CCMenu::createWithItem(btn);
  menu->setPosition(m_size / 2 - CCPoint{0.f, 15.f});
  this->addChild(menu);
}

// ! --- Summary --- !

void StagesGraphLayer::drawSummary(std::vector<StageGraphColumn> const &columns, float top)
{
  int totalAttempts = 0;
  float totalTime = 0.f;
  int completedStages = 0;
  int completedAttempts = 0;
  StageGraphColumn const *hardest = nullptr;

  for (auto const &column : columns)
  {
    totalAttempts += column.attempts;
    totalTime += column.timePlayed;

    if (column.status == StageGraphStatus::Completed)
    {
      completedStages++;
      completedAttempts += column.attempts;
    }

    if (column.attempts > 0 && (!hardest || column.attempts > hardest->attempts))
      hardest = &column;
  }

  const std::vector<SummaryCard> cards = {
      {formatCompactNumber(static_cast<float>(totalAttempts)), "Attempts"},
      {formatTimePlayed(totalTime), "Playtime"},
      {fmt::format("{}<small>/{}</small>", completedStages, columns.size()), "Stages done"},
      {completedStages > 0
           ? formatCompactNumber(static_cast<float>(completedAttempts) / completedStages)
           : std::string("-"),
       "Avg per stage"},
      {hardest
           ? fmt::format("{} <small>({})</small>", hardest->index + 1, formatCompactNumber(static_cast<float>(hardest->attempts)))
           : std::string("-"),
       "Hardest stage"},
  };

  const float gap = 5.f;
  const float innerWidth = m_size.width - SIDE_PADDING * 2;
  const float cardWidth = (innerWidth - gap * (cards.size() - 1)) / cards.size();
  const float textPadding = 6.f;

  for (std::size_t i = 0; i < cards.size(); ++i)
  {
    auto card = CCNode::create();
    card->setContentSize({cardWidth, SUMMARY_HEIGHT});
    card->setPosition({SIDE_PADDING + i * (cardWidth + gap), top - SUMMARY_HEIGHT});
    this->addChild(card);

    card->addChild(RectNode::create({cardWidth, SUMMARY_HEIGHT}, ccc4FFromccc4B(CARD_COLOR), 6));

    auto value = UILabel::create(cards[i].value, "bigFont.fnt", .35f);
    // Scales from the left edge
    value->setAnchorPoint({0.f, .5f});
    value->setPosition({textPadding, 19.f});

    // Long values shrink to fit the card
    const float maxWidth = cardWidth - textPadding * 2;
    if (value->getContentWidth() > maxWidth)
      value->setScale(maxWidth / value->getContentWidth());

    card->addChild(value);

    auto caption = CCLabelBMFont::create(cards[i].caption.c_str(), "bigFont.fnt");
    caption->setAnchorPoint({0.f, .5f});
    caption->setPosition({textPadding, 8.f});
    caption->setOpacity(130);
    caption->limitLabelWidth(maxWidth, .22f, .1f);
    card->addChild(caption);
  }
}

// ! --- Metric switch --- !

void StagesGraphLayer::drawMetricSwitch(float y)
{
  const CCSize buttonSize{62.f, CONTROLS_HEIGHT};
  const float gap = 4.f;

  auto menu = CCMenu::create();
  menu->setPosition({SIDE_PADDING, y});
  this->addChild(menu);

  const std::pair<StageGraphMetric, const char *> metrics[] = {
      {StageGraphMetric::Attempts, "Attempts"},
      {StageGraphMetric::Playtime, "Playtime"},
  };

  float x = 0.f;

  for (auto const &[metric, text] : metrics)
  {
    auto content = CCNode::create();
    content->setContentSize(buttonSize);

    auto bg = RectNode::create(buttonSize, ccc4FFromccc4B(BUTTON_COLOR), buttonSize.height / 2);
    content->addChild(bg);

    auto label = CCLabelBMFont::create(text, "bigFont.fnt");
    label->setScale(.28f);
    label->setPosition(buttonSize / 2);
    content->addChild(label);

    auto item = CCMenuItemSpriteExtra::create(
        content, this, menu_selector(StagesGraphLayer::onMetric));
    item->setTag(static_cast<int>(metric));
    item->m_scaleMultiplier = 1.05f;
    item->setPosition({x + buttonSize.width / 2, 0.f});
    menu->addChild(item);

    m_metricButtons.push_back({metric, item, bg});
    x += buttonSize.width + gap;
  }

  updateMetricButtons();
}

void StagesGraphLayer::updateMetricButtons()
{
  for (auto &button : m_metricButtons)
  {
    button.bg->setColor(ccc4FFromccc4B(
        button.metric == m_metric ? ACCENT_COLOR : BUTTON_COLOR));
  }
}

void StagesGraphLayer::onMetric(CCObject *sender)
{
  auto *node = typeinfo_cast<CCNode *>(sender);

  if (!node || !m_chart)
    return;

  const auto metric = static_cast<StageGraphMetric>(node->getTag());

  if (metric == m_metric)
    return;

  m_metric = metric;
  updateMetricButtons();
  m_chart->setMetric(metric);

  Mod::get()->setSavedValue<std::string>(
      METRIC_SAVE_KEY,
      metric == StageGraphMetric::Playtime ? "playtime" : "attempts");
}

// ! --- Legend --- !

// Right-aligned on the controls row
void StagesGraphLayer::drawLegend(float y, bool withBestLine)
{
  struct Entry
  {
    const char *text;
    ccColor4B color;
    bool dashed;
  };

  std::vector<Entry> entries = {
      {"Completed", {99, 224, 110, 255}, false},
      {"Current", {255, 220, 90, 255}, false},
      {"Upcoming", {110, 110, 110, 153}, false},
  };

  if (withBestLine)
    entries.push_back({"Best from 0%", {100, 215, 255, 220}, true});

  const float swatchSize = 6.f;
  const float swatchGap = 3.f;
  const float entryGap = 8.f;

  float right = m_size.width - SIDE_PADDING;

  // From the right edge to the left, so the last entry ends at the edge
  for (auto it = entries.rbegin(); it != entries.rend(); ++it)
  {
    auto label = CCLabelBMFont::create(it->text, "bigFont.fnt");
    label->setScale(.22f);
    label->setOpacity(170);
    label->setAnchorPoint({1.f, .5f});
    label->setPosition({right, y});
    this->addChild(label);

    const float swatchRight = right - label->getScaledContentWidth() - swatchGap;

    if (it->dashed)
    {
      auto dash = CCDrawNode::create();

      for (int i = 0; i < 2; ++i)
      {
        const float x = swatchRight - swatchSize + i * 4.f;
        dash->drawSegment({x, y}, {x + 2.f, y}, .5f, premultiplyAlpha(ccc4FFromccc4B(it->color)));
      }

      this->addChild(dash);
    }
    else
    {
      auto swatch = RectNode::create({swatchSize, swatchSize}, premultiplyAlpha(ccc4FFromccc4B(it->color)), 1.5f);
      swatch->setPosition({swatchRight - swatchSize, y - swatchSize / 2});
      this->addChild(swatch);
    }

    right = swatchRight - swatchSize - entryGap;
  }
}

void StagesGraphLayer::onOpenProfilesBtn(CCObject *)
{
  if (onOpenProfiles)
    onOpenProfiles();
}
