#include "StageRangeCell.hpp"

StageRangeCell *StageRangeCell::create(Range *range, GJGameLevel *level, const CCSize &cellSize)
{
  StageRangeCell *ret = new StageRangeCell();
  if (ret && ret->init(range, level, cellSize))
  {
    ret->autorelease();
    return ret;
  }

  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool StageRangeCell::init(Range *range, GJGameLevel *level, const CCSize &cellSize)
{
  if (!CCLayer::init())
    return false;

  this->setContentSize(cellSize);
  m_range = range;
  m_id = range->id;
  m_checked = m_range->checked;
  m_level = level;
  Profile profile = GlobalStore::get()->getProfileByLevel(m_level);

  if (!profile.id.empty())
  {
    const auto currentRange = GlobalStore::get()->getCurrentRange(profile.id);

    if (!currentRange.id.empty())
      m_isCurrent = currentRange.id == m_range->id;
  }

  // ! --- Content --- !
  m_content = CCLayer::create();
  m_content->setZOrder(1);
  m_content->setContentSize(cellSize);
  m_content->setPosition({0, cellSize.height});
  m_content->setAnchorPoint({0, 1});
  m_content->setLayout(
      ColumnLayout::create()
          ->setGap(0)
          ->setAxisReverse(true)
          ->setAutoScale(false)
          ->setAutoGrowAxis(true));
  m_content->getLayout()->ignoreInvisibleChildren(true);
  m_content->setID("progress-cell-content"_spr);
  this->addChild(m_content);
  updateLayoutWrapper();

  // ! --- Head --- !
  m_head = CCLayer::create();
  m_head->setContentSize(cellSize);
  m_head->setID("progress-cell-head"_spr);
  m_content->addChild(m_head);
  updateLayoutWrapper();

  // ! --- Background --- !
  updateBackgroundTexture();

  // ! --- Range Label --- !
  const float diff = std::abs(m_range->from - m_range->to);

  std::string rangeText = fmt::format(
      "{:.0f}<small>.{:02.0f}%</small> - {:.0f}<small>.{:02.0f}%</small>",
      std::floor(m_range->from),
      std::round((m_range->from - std::floor(m_range->from)) * 100),
      std::floor(m_range->to),
      std::round((m_range->to - std::floor(m_range->to)) * 100));
  m_rangeLabel = Label::create(rangeText, "gjFont17.fnt", .4f);
  m_rangeLabel->setPosition({25.f, cellSize.height / 2});
  m_rangeLabel->setAnchorPoint({0.f, .5f});
  m_head->addChild(m_rangeLabel);
  updateTextColors();

  // ! --- Checkbox --- !
  auto toggleOn = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
  auto toggleOff = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");

  m_checkbox = CCMenuItemToggler::create(toggleOn, toggleOff, this, menu_selector(StageRangeCell::onToggle));
  m_checkbox->setAnchorPoint({0.5f, 0.5f});
  m_checkbox->setCascadeColorEnabled(true);
  m_checkbox->toggle(!m_checked);

  // ! --- Checkbox Menu --- !
  auto checkboxMenu = CCMenu::createWithItem(m_checkbox);
  checkboxMenu->setScale(0.5f);
  checkboxMenu->setAnchorPoint({0.5f, 0.f});
  checkboxMenu->setContentSize({32.f, 32.f});
  checkboxMenu->setPosition({5.f, cellSize.height / 2});
  m_head->addChild(checkboxMenu);

  // ! --- Expand Button --- !
  updateExpandButton();

  // ! --- Meta Info Content --- !
  bool isFirstRunExists = m_range->firstRunFrom >= 0 && m_range->firstRunTo > 0;
  bool isBestRunExists = m_range->bestRunFrom >= 0 && m_range->bestRunTo > 0;

  std::string attempts = fmt::format("{}", m_range->attempts);
  std::string attemptsToComplete = fmt::format("{} <small>attempt(s)</small>", m_range->attemptsToComplete);
  std::string completions = fmt::format("{} <small>times</small>", m_range->completionCounter);
  std::string firstRun = !isFirstRunExists
                             ? "<small>None</small>"
                             : fmt::format(
                                   "{}<small>.{:02d}%</small> - {}<small>.{:02d}%</small>",
                                   static_cast<int>(m_range->firstRunFrom),
                                   static_cast<int>(std::round((m_range->firstRunFrom - static_cast<int>(m_range->firstRunFrom)) * 100)),
                                   static_cast<int>(m_range->firstRunTo),
                                   static_cast<int>(std::round((m_range->firstRunTo - static_cast<int>(m_range->firstRunTo)) * 100)));
  std::string bestRun = !isBestRunExists
                            ? "<small>None</small>"
                            : fmt::format(
                                  "{}<small>.{:02d}%</small> - {}<small>.{:02d}%</small>",
                                  static_cast<int>(m_range->bestRunFrom),
                                  static_cast<int>(std::round((m_range->bestRunFrom - static_cast<int>(m_range->bestRunFrom)) * 100)),
                                  static_cast<int>(m_range->bestRunTo),
                                  static_cast<int>(std::round((m_range->bestRunTo - static_cast<int>(m_range->bestRunTo)) * 100)));

  std::vector<MetaData> tableData = {
      {"Attempts:", attempts},
      {"Completed in:", attemptsToComplete},
      {"Completions:", completions},
      {"Best Run:", bestRun},
      {"First Run:", firstRun},
      // {"Time Played:", "5<small>h</small> 20<small>m</small> 32<small>s</small>"},
      {"Time Played:", "beta"},
  };

  m_table = MetaTable::create(tableData, m_head->getContentWidth(), {0, 5.f, 5.f, 5.f});
  m_table->setVisible(m_isExpanded);
  m_content->addChild(m_table);

  return true;
}

void StageRangeCell::updateLayoutWrapper()
{
  m_content->updateLayout();
  m_content->setPositionY(m_content->getContentHeight());
  this->setContentSize(m_content->getContentSize());

  updateBackgroundTexture();
  UpdateScrollLayoutEvent().post();
}

void StageRangeCell::updateBackgroundTexture()
{
  if (m_background)
    m_background->removeFromParentAndCleanup(true);

  const auto contentSize = m_content->getContentSize();
  const auto bg_spr = m_disabled
                          ? "range-disabled-bg.png"_spr
                      : m_checked   ? "range-completed-bg.png"_spr
                      : m_isCurrent ? "range-current-bg.png"_spr
                                    : "range-default-bg.png"_spr;

  m_background = CCScale9Sprite::create(bg_spr);
  m_background->setContentSize(contentSize);
  m_background->setPosition({contentSize.width / 2, contentSize.height / 2});
  m_background->setZOrder(0);

  this->addChild(m_background);
}

void StageRangeCell::updateExpandButton()
{
  if (m_expandBtnMenu)
    m_expandBtnMenu->removeFromParentAndCleanup(true);

  const auto cellSize = m_head->getContentSize();

  // ! ---  Expand Button Menu --- !
  m_expandBtnMenu = CCMenu::create();
  m_expandBtnMenu->setAnchorPoint({1.f, 0.5f});
  m_expandBtnMenu->setPosition({cellSize.width - 5.f, cellSize.height / 2});
  m_expandBtnMenu->setLayout(
      RowLayout::create()
          ->setGap(2.5f)
          ->setAutoScale(false)
          ->setAutoGrowAxis(true)
          ->setAxisAlignment(AxisAlignment::End)
          ->setCrossAxisAlignment(AxisAlignment::Center));
  m_expandBtnMenu->getLayout()->ignoreInvisibleChildren(true);
  m_head->addChild(m_expandBtnMenu);

  // ! ---  Expand Button --- !
  const auto sprName = m_isExpanded
                           ? "purple-chevron-up-btn.png"_spr
                           : "purple-chevron-down-btn.png"_spr;
  auto spr = CCSprite::createWithSpriteFrameName(sprName);
  const auto expandBtn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(StageRangeCell::onExpand));
  expandBtn->ignoreAnchorPointForPosition(true);
  expandBtn->setScale(.5f);
  expandBtn->m_baseScale = expandBtn->getScale();

  m_expandBtnMenu->addChild(expandBtn);
  m_expandBtnMenu->updateLayout();
}

void StageRangeCell::updateTextColors()
{
  if (m_rangeLabel)
  {
    if (m_checked)
      m_rangeLabel->setVariant(Label::Variant::Green);
    else if (m_disabled)
      m_rangeLabel->setVariant(Label::Variant::Red);
    else
      m_rangeLabel->setVariant(Label::Variant::Orange);
  }
}

void StageRangeCell::updateMetaContent()
{
  if (m_table)
    m_table->setVisible(m_isExpanded);
}

void StageRangeCell::onToggle(CCObject *sender)
{
  if (!m_disabled)
    m_checked = !m_checked;

  m_checkbox->toggle(m_checked);

  updateTextColors();
  updateBackgroundTexture();

  if (!m_level || m_disabled || m_id.empty())
    return;

  Profile profile = GlobalStore::get()->getProfileByLevel(m_level);
  Stage *currentStage = getFirstUncheckedStage(profile);

  if (!currentStage && !profile.data.stages.empty())
    currentStage = &profile.data.stages.back();

  if (!currentStage)
    return;

  for (auto &range : currentStage->ranges)
  {

    if (range.id == m_id)
    {
      range.checked = !range.checked;

      if (!range.checked)
      {
        range.attempts--;
        range.completionCounter--;
        range.attemptsToComplete = 0;
        range.firstRunFrom = 0;
        range.firstRunTo = 0;
        range.completedAt = 0;

        currentStage->checked = false;
      }

      if (range.checked && range.completionCounter <= 0)
        range.completionCounter = 1;
    }
  }

  GlobalStore::get()->updateProfile(profile);
}

void StageRangeCell::onExpand(CCObject *sender)
{
  m_isExpanded = !m_isExpanded;

  updateExpandButton();
  updateMetaContent();
  updateLayoutWrapper();
}

void StageRangeCell::setDisabled(bool disabled)
{
  m_disabled = disabled;

  if (m_disabled)
    m_checkbox->setColor({100, 100, 100});
  else
    m_checkbox->setColor({255, 255, 255});

  updateTextColors();
  updateBackgroundTexture();
}