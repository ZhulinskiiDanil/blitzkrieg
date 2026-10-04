#include "StagesGraphLayer.hpp"

#include <algorithm>

#include "../../../../utils/formatTimePlayed.hpp"
#include "../../../../utils/getKeybindText.hpp"

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
    // Title and text of the info button in the corner
    std::string infoTitle;
    std::string info;
    // Stage the card selects on the chart, -1 when it is not clickable
    int stageIndex = -1;
  };

  const char *CARD_CLICK_INFO = "\n\nClick the card to select the stage on the chart, click again to open it.";

  // Captions are plain CCLabelBMFont, the <small> tags of UILabel would show up
  std::string stripSmallTags(std::string text)
  {
    for (std::string_view tag : {"<small>", "</small>"})
    {
      for (auto pos = text.find(tag); pos != std::string::npos; pos = text.find(tag))
        text.erase(pos, tag.size());
    }

    return text;
  }

  // Info button scaled to a given height
  InfoAlertButton *createInfoButton(std::string const &title, std::string const &text, float size)
  {
    auto button = InfoAlertButton::create(title, text, 1.f);
    button->setScale(size / button->getContentHeight());
    button->m_baseScale = button->getScale();
    button->setOpacity(150);
    return button;
  }
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

  const auto savedMetric = Mod::get()->getSavedValue<std::string>(METRIC_SAVE_KEY, "attempts");

  if (savedMetric == "playtime")
    m_metric = StageGraphMetric::Playtime;
  else if (savedMetric == "per-run")
    m_metric = StageGraphMetric::PerRun;
  else if (savedMetric == "timeline")
    m_metric = StageGraphMetric::Timeline;
  else
    m_metric = StageGraphMetric::Attempts;

  // ! --- Best run from 0% --- !
  const float bestPercent = m_level ? static_cast<float>(m_level->m_normalPercent.value()) : 0.f;
  const auto bestX = mapPercentFromZero(columns, bestPercent);

  // ! --- Layout, top to bottom --- !
  const float summaryTop = m_size.height - TOP_PADDING;
  const float controlsY = summaryTop - SUMMARY_HEIGHT - ROW_GAP - CONTROLS_HEIGHT / 2;
  const float chartTop = controlsY - CONTROLS_HEIGHT / 2 - ROW_GAP;

  drawSummary(columns, summaryTop);
  const float switchRight = drawMetricSwitch(controlsY);
  drawLegend(controlsY, switchRight + 10.f);

  // ! --- Charts, both in the same place, one is visible --- !
  const CCSize chartSize{m_size.width - SIDE_PADDING * 2, chartTop - BOTTOM_PADDING};

  auto openStage = [this](int stageIndex, std::string const &rangeId)
  {
    if (onOpenStage)
      onOpenStage(stageIndex, rangeId);
  };

  m_timeline = StageTimelineChart::create(chartSize);
  m_timeline->setPosition({SIDE_PADDING, BOTTOM_PADDING});
  m_timeline->onOpenStage = openStage;
  m_timeline->setData(columns);
  this->addChild(m_timeline);

  m_chart = StageBarChart::create(chartSize);
  m_chart->setPosition({SIDE_PADDING, BOTTOM_PADDING});
  m_chart->onOpenStage = openStage;
  this->addChild(m_chart);

  m_chart->setData(
      std::move(columns),
      m_metric == StageGraphMetric::Timeline ? StageGraphMetric::Attempts : m_metric);
  m_chart->setBestFromZero(bestX, bestPercent);

  updateChartVisibility();

  drawKeybindHint();
  listenToKeybinds();

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
  int currentStage = -1;
  StageGraphColumn const *hardest = nullptr;

  for (auto const &column : columns)
  {
    totalAttempts += column.attempts;
    totalTime += column.timePlayed;

    if (column.status == StageGraphStatus::Completed)
      completedStages++;

    if (column.status == StageGraphStatus::Current)
      currentStage = column.index;

    if (column.attempts > 0 && (!hardest || column.attempts > hardest->attempts))
      hardest = &column;
  }

  // ! The average per stage is a line on the chart, the card shows what is left
  const auto forecast = estimateRemaining(columns);
  SummaryCard forecastCard{
      "-",
      "Est. left",
      "Est. left",
      "How many <cy>attempts</c> and how much <cy>time</c> it may take to finish the stages that are left.\n"
      "It is the <cg>average of the completed stages</c> times the stages left, "
      "minus what those stages already took.\n"
      "Shown after the first stage is completed.",
      currentStage};

  if (forecast.done)
    forecastCard.value = "Done";
  else if (forecast.known)
  {
    forecastCard.value = "~" + formatCompactNumber(forecast.attempts);
    forecastCard.caption = fmt::format("Est. left, ~{}", stripSmallTags(formatTimePlayed(forecast.time)));
  }

  const std::vector<SummaryCard> cards = {
      {formatCompactNumber(static_cast<float>(totalAttempts)),
       "Attempts",
       "Attempts",
       "Total <cy>attempts</c> on all stages of the linked profile."},
      {formatTimePlayed(totalTime),
       "Playtime",
       "Playtime",
       "Total <cy>time played</c> on all stages of the linked profile."},
      {fmt::format("{}<small>/{}</small>", completedStages, columns.size()),
       "Stages done",
       "Stages done",
       "Stages where <cg>every run is checked</c>, out of all stages.",
       currentStage},
      forecastCard,
      {hardest
           ? fmt::format(
                 "{} <small>({})</small>",
                 hardest->index + 1,
                 // Already inside <small>, UILabel does not nest tags
                 stripSmallTags(formatCompactNumber(static_cast<float>(hardest->attempts))))
           : std::string("-"),
       "Hardest stage",
       "Hardest stage",
       "The stage that took the <cr>most attempts</c>, the attempts are in brackets.",
       hardest ? hardest->index : -1},
  };

  const float gap = 5.f;
  const float innerWidth = m_size.width - SIDE_PADDING * 2;
  const float cardWidth = (innerWidth - gap * (cards.size() - 1)) / cards.size();
  const float textPadding = 6.f;
  const float infoPadding = 3.f;
  // The top right corner belongs to the info button
  const float infoAreaWidth = INFO_SIZE + infoPadding * 2;

  for (std::size_t i = 0; i < cards.size(); ++i)
  {
    auto const &data = cards[i];
    const bool clickable = data.stageIndex >= 0;

    auto card = CCNode::create();
    card->setContentSize({cardWidth, SUMMARY_HEIGHT});
    card->setPosition({SIDE_PADDING + i * (cardWidth + gap), top - SUMMARY_HEIGHT});
    this->addChild(card);

    card->addChild(RectNode::create({cardWidth, SUMMARY_HEIGHT}, ccc4FFromccc4B(CARD_COLOR), 6));

    auto menu = CCMenu::create();
    menu->setContentSize({0.f, 0.f});
    menu->setPosition({0.f, 0.f});
    card->addChild(menu, 1);

    // ! --- Hit area left of the info button, the two never overlap --- !
    if (clickable)
    {
      const CCSize hitSize{cardWidth - infoAreaWidth, SUMMARY_HEIGHT};

      auto area = CCNode::create();
      area->setContentSize(hitSize);

      auto item = CCMenuItemSpriteExtra::create(area, this, menu_selector(StagesGraphLayer::onCard));
      item->setTag(data.stageIndex);
      item->m_scaleMultiplier = 1.f;
      item->setPosition(hitSize / 2);
      menu->addChild(item);
    }

    // ! --- Info button in the top right corner --- !
    auto infoButton = createInfoButton(
        data.infoTitle,
        clickable ? data.info + CARD_CLICK_INFO : data.info,
        INFO_SIZE);
    infoButton->setPosition({
        cardWidth - infoPadding - INFO_SIZE / 2,
        SUMMARY_HEIGHT - infoPadding - INFO_SIZE / 2,
    });
    menu->addChild(infoButton);

    auto value = UILabel::create(data.value, "bigFont.fnt", .35f);
    // Scales from the left edge
    value->setAnchorPoint({0.f, .5f});
    value->setPosition({textPadding, 19.f});

    // Long values shrink to fit the card and stay left of the info button
    const float maxWidth = cardWidth - textPadding * 2;
    const float maxValueWidth = maxWidth - INFO_SIZE - infoPadding;
    if (value->getContentWidth() > maxValueWidth)
      value->setScale(maxValueWidth / value->getContentWidth());

    card->addChild(value);

    auto caption = CCLabelBMFont::create(data.caption.c_str(), "bigFont.fnt");
    caption->setAnchorPoint({0.f, .5f});
    caption->setPosition({textPadding, 8.f});
    caption->setOpacity(130);
    caption->limitLabelWidth(maxWidth, .22f, .1f);
    card->addChild(caption);
  }
}

void StagesGraphLayer::onCard(CCObject *sender)
{
  auto *node = typeinfo_cast<CCNode *>(sender);

  if (!node || !m_chart)
    return;

  const int stageIndex = node->getTag();

  // The stage is shown on the bars
  if (m_metric == StageGraphMetric::Timeline)
    applyMetric(StageGraphMetric::Attempts);

  if (m_chart->isStageSelected(stageIndex))
  {
    if (onOpenStage)
      onOpenStage(stageIndex, {});

    return;
  }

  m_chart->selectStage(stageIndex);
}

// ! --- Metric switch --- !

float StagesGraphLayer::drawMetricSwitch(float y)
{
  const CCSize buttonSize{METRIC_BUTTON_WIDTH, CONTROLS_HEIGHT};

  auto menu = CCMenu::create();
  menu->setPosition({SIDE_PADDING, y});
  this->addChild(menu);

  const std::pair<StageGraphMetric, const char *> metrics[] = {
      {StageGraphMetric::Attempts, "Attempts"},
      {StageGraphMetric::Playtime, "Playtime"},
      {StageGraphMetric::PerRun, "Per run"},
      {StageGraphMetric::Timeline, "Timeline"},
  };

  float x = 0.f;

  for (auto const &[metric, text] : metrics)
  {
    auto content = CCNode::create();
    content->setContentSize(buttonSize);

    auto bg = RectNode::create(buttonSize, ccc4FFromccc4B(BUTTON_COLOR), buttonSize.height / 2);
    content->addChild(bg);

    auto label = CCLabelBMFont::create(text, "bigFont.fnt");
    label->limitLabelWidth(buttonSize.width - 10.f, .28f, .1f);
    label->setPosition(buttonSize / 2);
    content->addChild(label);

    auto item = CCMenuItemSpriteExtra::create(
        content, this, menu_selector(StagesGraphLayer::onMetric));
    item->setTag(static_cast<int>(metric));
    item->m_scaleMultiplier = 1.05f;
    item->setPosition({x + buttonSize.width / 2, 0.f});
    menu->addChild(item);

    m_metricButtons.push_back({metric, item, bg});
    x += buttonSize.width + METRIC_BUTTON_GAP;
  }

  updateMetricButtons();

  return SIDE_PADDING + x - METRIC_BUTTON_GAP;
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
  if (auto *node = typeinfo_cast<CCNode *>(sender))
    applyMetric(static_cast<StageGraphMetric>(node->getTag()));
}

void StagesGraphLayer::applyMetric(StageGraphMetric metric)
{
  if (!m_chart || metric == m_metric)
    return;

  m_metric = metric;
  updateMetricButtons();
  updateChartVisibility();

  if (metric != StageGraphMetric::Timeline)
    m_chart->setMetric(metric);

  const char *saved = "attempts";

  switch (metric)
  {
  case StageGraphMetric::Playtime:
    saved = "playtime";
    break;
  case StageGraphMetric::PerRun:
    saved = "per-run";
    break;
  case StageGraphMetric::Timeline:
    saved = "timeline";
    break;
  default:
    break;
  }

  Mod::get()->setSavedValue<std::string>(METRIC_SAVE_KEY, saved);
}

void StagesGraphLayer::updateChartVisibility()
{
  const bool timeline = m_metric == StageGraphMetric::Timeline;

  // Hidden charts also stop taking touches, CCMenu checks the parents
  if (m_chart)
    m_chart->setVisible(!timeline);
  if (m_timeline)
    m_timeline->setVisible(timeline);
  if (m_legend)
    m_legend->setVisible(!timeline);
}

// ! --- Legend --- !

// Right-aligned on the controls row, the lines are labelled on the chart itself
void StagesGraphLayer::drawLegend(float y, float left)
{
  struct Entry
  {
    const char *text;
    ccColor4B color;
  };

  const Entry entries[] = {
      {"Done", {99, 224, 110, 255}},
      {"Current", {255, 220, 90, 255}},
      {"Upcoming", {110, 110, 110, 153}},
  };

  const float swatchSize = 6.f;
  const float swatchGap = 3.f;
  const float entryGap = 8.f;

  // Drawn left of the origin, so a scale keeps the right edge in place
  m_legend = CCNode::create();
  m_legend->setPosition({m_size.width - SIDE_PADDING, y});
  this->addChild(m_legend);

  // ! --- Info button at the right edge --- !
  auto infoButton = createInfoButton(
      "Stage Graph",
      "Every bar is a stage: <cg>green</c> ones are done, <cy>yellow</c> is the stage you are on "
      "and gray ones are upcoming.\n"
      "Thin lines split a bar into its runs, hover one to see it.\n"
      "The dashed <cl>blue</c> line is your best run from 0%, the dashed white line is the average stage.\n"
      "The arrow marks the stage you are on with its done runs.\n"
      "<cy>Per run</c> divides the attempts of a stage by its runs, so stages with more runs stay comparable.",
      INFO_SIZE);

  auto infoMenu = CCMenu::createWithItem(infoButton);
  infoMenu->setContentSize({0.f, 0.f});
  infoMenu->setPosition({-INFO_SIZE / 2, 0.f});
  m_legend->addChild(infoMenu);

  float right = -INFO_SIZE - entryGap;

  // From the right edge to the left, so the last entry ends at the edge
  for (auto it = std::rbegin(entries); it != std::rend(entries); ++it)
  {
    auto label = CCLabelBMFont::create(it->text, "bigFont.fnt");
    label->setScale(.22f);
    label->setOpacity(170);
    label->setAnchorPoint({1.f, .5f});
    label->setPosition({right, 0.f});
    m_legend->addChild(label);

    const float swatchRight = right - label->getScaledContentWidth() - swatchGap;

    auto swatch = RectNode::create({swatchSize, swatchSize}, premultiplyAlpha(ccc4FFromccc4B(it->color)), 1.5f);
    swatch->setPosition({swatchRight - swatchSize, -swatchSize / 2});
    m_legend->addChild(swatch);

    right = swatchRight - swatchSize - entryGap;
  }

  // ! --- Shrink when the switch leaves too little room --- !
  const float width = -(right + entryGap);
  const float available = m_legend->getPositionX() - left;

  if (width > available && available > 0.f)
    m_legend->setScale(available / width);
}

// ! --- Keyboard --- !

// Under the chart on the right, only where there is a keyboard
void StagesGraphLayer::drawKeybindHint()
{
#ifdef GEODE_IS_DESKTOP
  const auto prev = getKeybindText("prev-stage-keybind");
  const auto next = getKeybindText("next-stage-keybind");
  const auto open = getKeybindText("open-stage-keybind");

  std::vector<std::string> parts;

  if (!prev.empty() && !next.empty())
    parts.push_back(fmt::format("{} / {} to select", prev, next));
  if (!open.empty())
    parts.push_back(fmt::format("{} to open", open));

  if (parts.empty())
    return;

  std::string text = parts.front();

  for (std::size_t i = 1; i < parts.size(); ++i)
    text += "   " + parts[i];

  auto label = CCLabelBMFont::create(text.c_str(), "bigFont.fnt");
  label->setScale(.2f);
  label->setOpacity(100);
  label->setAnchorPoint({1.f, .5f});
  label->setPosition({m_size.width - SIDE_PADDING, 5.f});
  this->addChild(label);
#endif
}

void StagesGraphLayer::listenToKeybinds()
{
  // Holding a key keeps moving
  this->addEventListener(
      KeybindSettingPressedEventV3(Mod::get(), "prev-stage-keybind"),
      [this](Keybind const &keybind, bool down, bool repeat, double timestamp)
      {
        if (down)
          moveSelection(-1);
      });

  this->addEventListener(
      KeybindSettingPressedEventV3(Mod::get(), "next-stage-keybind"),
      [this](Keybind const &keybind, bool down, bool repeat, double timestamp)
      {
        if (down)
          moveSelection(1);
      });

  this->addEventListener(
      KeybindSettingPressedEventV3(Mod::get(), "open-stage-keybind"),
      [this](Keybind const &keybind, bool down, bool repeat, double timestamp)
      {
        if (down && !repeat)
          openSelected();
      });
}

void StagesGraphLayer::moveSelection(int delta)
{
  if (!nodeIsVisible(this))
    return;

  if (m_metric == StageGraphMetric::Timeline)
  {
    if (m_timeline)
      m_timeline->moveSelection(delta);
  }
  else if (m_chart)
  {
    m_chart->moveSelection(delta);
  }
}

void StagesGraphLayer::openSelected()
{
  if (!nodeIsVisible(this))
    return;

  if (m_metric == StageGraphMetric::Timeline)
  {
    if (m_timeline)
      m_timeline->openSelected();
  }
  else if (m_chart)
  {
    m_chart->openSelected();
  }
}

void StagesGraphLayer::onOpenProfilesBtn(CCObject *)
{
  if (onOpenProfiles)
    onOpenProfiles();
}
