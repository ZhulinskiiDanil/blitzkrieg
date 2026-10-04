#include "index.hpp"
#include "Geode/ui/GeodeUI.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>

#include "../../events/StageSwitchedEvent.hpp"

BlitzkriegPopup *BlitzkriegPopup::create(GJGameLevel *level)
{
  BlitzkriegPopup *ret = new BlitzkriegPopup();

  if (ret->init(level))
  {
    ret->autorelease();
    return ret;
  }

  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool BlitzkriegPopup::init(GJGameLevel *level)
{
  if (!Popup::init(420, 250, "GJ_square01_custom.png"_spr))
    return false;

  if (!Mod::get()->hasSavedValue("sort-stage-runs-asc-enabled"))
    Mod::get()->setSavedValue("sort-stage-runs-asc-enabled", true);
  if (!Mod::get()->hasSavedValue("hide-stage-completed-runs-enabled"))
    Mod::get()->setSavedValue("hide-stage-completed-runs-enabled", false);

  std::string levelId = level->m_levelID ? utils::numToString(level->m_levelID.value()) : utils::numToString(EditorIDs::getID(level));

  this->m_level = level;
  this->m_levelId = levelId;

  drawTabs();
  drawContent();

  return true;
}

void BlitzkriegPopup::drawContent()
{
  auto profile = GlobalStore::get()->getProfileByLevel(m_levelId);
  NavTabButton *activeButton = tabButtons[0];

  if (m_isFirstLaunch)
  {
    m_isFirstLaunch = false;

    if (profile)
    {
      activeButton = tabButtons[1];
      activateTab(activeButton, false);

      return;
    }
  }

  for (auto *btn : tabButtons)
  {
    if (btn->isActive())
    {
      activeButton = btn;
      break;
    }
  }

  if (activeButton)
  {
    const auto btnId = activeButton->getID();

    // ! --- Clear Old Content --- !

    for (auto *container : contentContainers)
      if (container)
        container->removeFromParentAndCleanup(true);

    m_stageChangedListener.destroy();
    contentContainers.clear();

    if (btnId == "profiles-tab"_spr)
      drawProfilesList();
    else if (btnId == "stage-browser-tab"_spr)
      drawCurrentStage();
    else if (btnId == "stage-graph-tab"_spr)
      drawStagesGraph();
    else if (btnId == "news-tab"_spr)
      drawNewsSection();
  }
}

void BlitzkriegPopup::drawProfilesList()
{
  UIPadding padding{12.f, 45.f, 10.f, 10.f}; // top, bottom, left, right

  auto profiles = GlobalStore::get()->getProfiles();
  auto profile = GlobalStore::get()->getProfileByLevel(m_levelId);

  m_profilesListNode = CCNode::create();
  m_profilesListNode->setID("blitzkrieg-popup-profiles-list"_spr);
  m_profilesListNode->setTag(1);

  const auto contentSize = CCSize(
      m_size.width - padding.left - padding.right,
      m_size.height - padding.top - padding.bottom);

  // ! --- ProfilesListLayer --- !
  auto listLayer = ProfilesListLayer::create(m_level, profiles, contentSize);
  listLayer->setPosition({padding.left, padding.bottom});
  m_profilesListNode->addChild(listLayer);

  m_mainLayer->addChild(m_profilesListNode);
  contentContainers.push_back(m_profilesListNode);
}

void BlitzkriegPopup::drawCurrentStage()
{
  // The bottom padding leaves room for the stage dots
  UIPadding padding{55.f, 24.f, 10.f, 10.f}; // top, bottom, left, right

  auto profile = GlobalStore::get()->getProfileByLevel(m_levelId);

  const auto contentSize = CCSize(
      m_size.width - padding.left - padding.right,
      m_size.height - padding.top - padding.bottom);

  m_currentStageNode = CCNode::create();
  m_currentStageNode->setID("blitzkrieg-popup-current-stage"_spr);
  m_currentStageNode->setTag(2);

  m_mainLayer->addChild(m_currentStageNode);
  contentContainers.push_back(m_currentStageNode);

  if (!profile || profile->data.stages.size() <= 0)
  {
    auto errorLabel = CCLabelBMFont::create("Attach your profile first", "bigFont.fnt");
    errorLabel->setScale(.75f);
    errorLabel->setOpacity(255 * .6f);
    errorLabel->setPosition(m_size / 2 + CCPoint{0.f, 15.f});

    m_currentStageNode->addChild(errorLabel);

    auto profilesBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Open Profiles"),
        this,
        menu_selector(BlitzkriegPopup::onOpenProfiles));
    profilesBtn->setScale(.7f);
    profilesBtn->m_baseScale = .7f;

    auto profilesMenu = CCMenu::createWithItem(profilesBtn);
    profilesMenu->setPosition(m_size / 2 - CCPoint{0.f, 15.f});
    m_currentStageNode->addChild(profilesMenu);
    return;
  }

  // ! --- Title --- !
  drawCurrentStageTitle(padding);

  auto filterButtonsMenu = CCMenu::create();
  filterButtonsMenu->setLayout(RowLayout::create()
                                   ->setGap(5)
                                   ->setAxisReverse(true)
                                   ->setAutoScale(false)
                                   ->setAutoGrowAxis(true));

  filterButtonsMenu->ignoreAnchorPointForPosition(false);
  filterButtonsMenu->setPosition({m_size.width - padding.right, m_size.height - padding.top + 5});
  filterButtonsMenu->setAnchorPoint({1, 0});

  // ! --- Sort Toggle Button --- !
  auto sortBtnSpriteUp = CCSprite::createWithSpriteFrameName("sort-up-square-btn.png"_spr);
  auto sortBtnSpriteDown = CCSprite::createWithSpriteFrameName("sort-down-square-btn.png"_spr);
  auto sortBtnCheckbox = CCMenuItemToggler::create(sortBtnSpriteDown, sortBtnSpriteUp, this, menu_selector(BlitzkriegPopup::onToggleSort));
  sortBtnCheckbox->toggle(!Mod::get()->getSavedValue<bool>("sort-stage-runs-asc-enabled"));
  sortBtnCheckbox->setAnchorPoint({0, 0});
  sortBtnCheckbox->setScale(.75f);
  filterButtonsMenu->addChild(sortBtnCheckbox);

  // ! --- Visability Toggle Button --- !
  auto visabilityBtnSpriteOn = CCSprite::createWithSpriteFrameName("check-mark-square-btn.png"_spr);
  auto visabilityBtnSpriteOff = CCSprite::createWithSpriteFrameName("check-mark-gray-square-btn.png"_spr);
  auto visabilityBtnCheckbox = CCMenuItemToggler::create(visabilityBtnSpriteOff, visabilityBtnSpriteOn, this, menu_selector(BlitzkriegPopup::onToggleVisability));
  visabilityBtnCheckbox->toggle(!Mod::get()->getSavedValue<bool>("hide-stage-completed-runs-enabled"));
  visabilityBtnCheckbox->setAnchorPoint({0, 0});
  visabilityBtnCheckbox->setScale(.75f);
  filterButtonsMenu->addChild(visabilityBtnCheckbox);

  m_currentStageNode->addChild(filterButtonsMenu);
  filterButtonsMenu->updateLayout();

  // ! --- Stage Progress --- !
  // From the inner edge of the list to the filter buttons
  const CCPoint progressOrigin{padding.left + 5.f, m_size.height - padding.top + 3.f};
  const float filtersLeft = filterButtonsMenu->boundingBox().getMinX();

  drawStageProgressBar(progressOrigin, filtersLeft - 8.f - progressOrigin.x);
  m_totalStatMaxWidth = filtersLeft - 8.f - m_totalStatLabel->getPositionX();

  // ! --- StageListLayer --- !
  auto stageListContentSize = CCSize(contentSize.width, contentSize.height);
  m_stageList = StageListLayer::create(m_level, stageListContentSize);
  m_stageList->setPosition({padding.left, padding.bottom});

  m_stageList->setSortBy(!sortBtnCheckbox->isToggled() ? StageListSortBy::ASC : StageListSortBy::DESC);
  m_stageList->setRunsVisabilityForCompleted(!visabilityBtnCheckbox->isToggled());
  m_stageList->reload();

  m_currentStageNode->addChild(m_stageList);

  updateStageHeader(
      m_stageList->getStageIndex(),
      m_stageList->getStagesCount(),
      m_stageList->getCurrentStage());
}

void BlitzkriegPopup::drawStagesGraph()
{
  m_currentStageGraphNode = CCNode::create();
  m_currentStageGraphNode->setID("blitzkrieg-popup-stage-graph"_spr);
  m_currentStageGraphNode->setTag(3);

  // ! --- StagesGraphLayer --- !
  auto stagesGraphLayer = StagesGraphLayer::create(m_level, m_size);
  m_currentStageGraphNode->addChild(stagesGraphLayer);

  m_mainLayer->addChild(m_currentStageGraphNode);
  contentContainers.push_back(m_currentStageGraphNode);
}

void BlitzkriegPopup::drawNewsSection()
{
  m_helpNode = CCNode::create();
  m_helpNode->setID("blitzkrieg-popup-help"_spr);
  m_helpNode->setTag(4);

  // ! --- NewsLayer --- !
  auto helpLayer = NewsLayer::create(m_size);
  m_helpNode->addChild(helpLayer);

  m_mainLayer->addChild(m_helpNode);
  contentContainers.push_back(m_helpNode);
}

void BlitzkriegPopup::drawTabs()
{
  // Settings
  const float TAB_BUTTONS_GAP = 3.f;
  // How deep tabs go down into the popup border
  const float TAB_BORDER_OVERLAP = 3.3f;
  // Space between the close button in the top-left corner and the first tab
  const float TAB_CLOSE_BUTTON_GAP = 6.f;
  // Left offset used when the close button is not found
  const float TAB_MIN_LEFT_OFFSET = 30.f;

  struct TabInfo
  {
    const char *id;
    const char *label;
    const char *icon;
  };

  const TabInfo tabs[] = {
      {"profiles-tab"_spr, "Profiles", "tab-icon-profiles.png"_spr},
      {"stage-browser-tab"_spr, "Stage Browser", "tab-icon-stages.png"_spr},
      {"stage-graph-tab"_spr, "Stage Graph", "tab-icon-graph.png"_spr},
      {"news-tab"_spr, "News", "tab-icon-news.png"_spr},
  };

  auto oldTabsNode = m_mainLayer->getChildByID("blitzkrieg-popup-tabs-node"_spr);

  if (oldTabsNode)
    oldTabsNode->removeFromParentAndCleanup(true);

  // ! --- Main Container --- !
  auto tabsNode = CCNode::create();
  tabsNode->setID("blitzkrieg-popup-tabs-node"_spr);

  // ! --- Menu --- !
  auto tabMenu = CCMenu::create();
  // Tabs are attached to the left side, right after the close button
  float tabsLeft = TAB_MIN_LEFT_OFFSET;

  if (m_closeBtn && m_closeBtn->getParent())
  {
    const auto closeBox = m_closeBtn->boundingBox();
    const auto closeRightWorld = m_closeBtn->getParent()->convertToWorldSpace(
        {closeBox.getMaxX(), closeBox.getMidY()});
    const float closeRight = m_mainLayer->convertToNodeSpace(closeRightWorld).x;

    tabsLeft = std::max(tabsLeft, closeRight + TAB_CLOSE_BUTTON_GAP);
  }

  tabMenu->setPosition({tabsLeft, m_size.height - TAB_BORDER_OVERLAP});
  tabMenu->setZOrder(1);
  tabMenu->setID("blitzkrieg-popup-tab-menu"_spr);

  // ! --- Buttons --- !
  m_tabButtonsGap = TAB_BUTTONS_GAP;
  tabButtons.clear();

  for (auto const &tab : tabs)
  {
    auto *btn = NavTabButton::create(
        tab.label,
        tab.icon,
        this,
        menu_selector(BlitzkriegPopup::onTabButton));
    btn->setID(tab.id);
    // Tabs animate their width, the row follows every frame
    btn->setOnResize([this]()
                     { layoutTabs(); });

    tabMenu->addChild(btn);
    tabButtons.push_back(btn);
  }

  tabButtons.front()->setActive(true, false);
  layoutTabs();

  tabsNode->addChild(tabMenu);
  m_mainLayer->addChild(tabsNode);

  // ! --- Options Button --- !
  CCNode *m = m_closeBtn->getParent();
  CCSprite *settingsSpr = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
  settingsSpr->setScale(0.8f);

  CCMenuItemSpriteExtra *settingsBtn = CCMenuItemSpriteExtra::create(settingsSpr, this, menu_selector(BlitzkriegPopup::onSettingsButton));

  settingsBtn->setPositionX(this->m_bgSprite->getContentWidth() - 3);
  settingsBtn->setPositionY(3);
  m->addChild(settingsBtn);
}

// Tabs change width when activated, so the row is laid out manually.
// The row grows to the right from the menu origin,
// buttons stand on it with their bottom edge.
void BlitzkriegPopup::layoutTabs()
{
  float x = 0.f;

  for (auto *btn : tabButtons)
  {
    const float width = btn->getContentWidth();

    btn->setAnchorPoint({.5f, 0.f});
    btn->setPosition({x + width / 2, 0.f});

    x += width + m_tabButtonsGap;
  }
}

void BlitzkriegPopup::activateTab(NavTabButton *sender, bool animate)
{
  if (!sender)
    return;

  for (auto *btn : tabButtons)
  {
    if (btn)
      btn->setActive(btn == sender, animate);
  }

  layoutTabs();
  drawContent();
}

void BlitzkriegPopup::onTabButton(CCObject *obj)
{
  auto *btn = typeinfo_cast<NavTabButton *>(obj);

  // Clicking the already open tab does nothing
  if (!btn || btn->isActive())
    return;

  activateTab(btn);
}

void BlitzkriegPopup::onToggleSort(CCObject *sender)
{
  if (auto checkbox = typeinfo_cast<CCMenuItemToggler *>(sender))
  {
    bool isToggled = checkbox->isToggled();

    if (m_stageList)
    {
      m_stageList->setSortBy(isToggled ? StageListSortBy::ASC : StageListSortBy::DESC);
      m_stageList->reload(true);
    }

    Mod::get()->setSavedValue("sort-stage-runs-asc-enabled", isToggled);
  }
}

void BlitzkriegPopup::onToggleVisability(CCObject *sender)
{
  if (auto checkbox = typeinfo_cast<CCMenuItemToggler *>(sender))
  {
    bool isToggled = checkbox->isToggled();

    if (m_stageList)
    {
      m_stageList->setRunsVisabilityForCompleted(isToggled);
      m_stageList->reload(true);
    }

    Mod::get()->setSavedValue("hide-stage-completed-runs-enabled", isToggled);
  }
}

void BlitzkriegPopup::drawCurrentStageTitle(UIPadding padding)
{
  m_currentStageTitleLabel = CCLabelBMFont::create("", "goldFont.fnt");
  m_currentStageTitleLabel->setPosition({padding.left + 5, m_size.height - padding.top / 2 + 5}); // n - 2.5f
  m_currentStageTitleLabel->setAnchorPoint({0, 0.5});
  m_currentStageNode->addChild(m_currentStageTitleLabel);

  m_totalStatLabel = UILabel::create("", "bigFont.fnt", .4f);
  m_totalStatLabel->setPosition({padding.left + 6, m_size.height - padding.top / 2 - 15});
  m_totalStatLabel->setAnchorPoint({0, 0.5});
  m_currentStageNode->addChild(m_totalStatLabel);

  // StageListLayer sends it on stage switch and when runs are checked
  m_stageChangedListener = StageSwitchedEvent().listen(
      [this](int stageIndex, int totalStages, Stage *stage)
      {
        updateStageHeader(stageIndex, totalStages, stage);
        return ListenerResult::Propagate;
      });
}

void BlitzkriegPopup::updateStageHeader(int stageIndex, int totalStages, Stage *stage)
{
  if (!m_currentStageTitleLabel || !m_totalStatLabel)
    return;

  const int shownStage = stage ? stageIndex + 1 : 0;

  m_currentStageTitleLabel->setString(
      fmt::format("Stage: {}/{}", shownStage, totalStages).c_str());

  const int attempts = stage ? getStageAttempts(stage) : 0;
  const float timePlayed = stage ? getStagePlaytime(stage) : 0.f;

  int totalRuns = 0;
  int completedRuns = 0;

  if (stage)
  {
    for (auto const &range : stage->ranges)
    {
      if (!range.consider)
        continue;

      totalRuns++;

      if (range.checked)
        completedRuns++;
    }
  }

  std::string stat = fmt::format(
      "{}/{} <small>Runs</small>   {} <small>Attempts</small>   {}",
      completedRuns,
      totalRuns,
      attempts,
      formatTimePlayed(timePlayed));

  // Hint where the progress is when another stage is open
  if (auto *profile = GlobalStore::get()->getProfileByLevel(m_levelId))
  {
    const auto stages = getConsideredStages(profile->data.stages);

    for (std::size_t i = 0; i < stages.size(); ++i)
    {
      if (isStageDeepChecked(*stages[i]))
        continue;

      if (static_cast<int>(i) != stageIndex)
        stat += fmt::format("   <small>Current: {}</small>", i + 1);

      break;
    }
  }

  m_totalStatLabel->setText(stat);
  m_totalStatLabel->setScale(1.f);

  const float statWidth = m_totalStatLabel->getContentWidth();

  if (m_totalStatMaxWidth > 0.f && statWidth > m_totalStatMaxWidth)
    m_totalStatLabel->setScale(m_totalStatMaxWidth / statWidth);

  // ! --- Progress bar --- !
  if (m_stageProgressFill)
  {
    const float progress =
        totalRuns > 0 ? static_cast<float>(completedRuns) / totalRuns : 0.f;
    const float fillWidth = m_stageProgressWidth * progress;
    const bool isCompleted = totalRuns > 0 && completedRuns >= totalRuns;

    m_stageProgressFill->setVisible(fillWidth > 0.f);
    m_stageProgressFill->setSize({std::max(fillWidth, 1.f), 2.f});
    m_stageProgressFill->setColor(ccc4FFromccc4B(
        isCompleted ? ccColor4B{99, 224, 110, 255} : ccColor4B{255, 0, 82, 255}));
  }
}

void BlitzkriegPopup::drawStageProgressBar(CCPoint const &origin, float width)
{
  const float height = 2.f;

  m_stageProgressFill = nullptr;
  m_stageProgressWidth = std::max(width, 0.f);

  if (m_stageProgressWidth <= 0.f)
    return;

  auto track = RectNode::create(
      {m_stageProgressWidth, height},
      ccc4FFromccc4B({70, 70, 70, 255}),
      height / 2);
  track->setPosition(origin);
  m_currentStageNode->addChild(track);

  m_stageProgressFill = RectNode::create(
      {1.f, height},
      ccc4FFromccc4B({255, 0, 82, 255}),
      height / 2);
  m_stageProgressFill->setPosition(origin);
  m_stageProgressFill->setVisible(false);
  m_currentStageNode->addChild(m_stageProgressFill);
}

void BlitzkriegPopup::onOpenProfiles(CCObject *)
{
  // The button lives in the content that the tab switch removes
  geode::queueInMainThread(
      [self = Ref<BlitzkriegPopup>(this)]()
      {
        if (!self->tabButtons.empty())
          self->activateTab(self->tabButtons.front());
      });
}

void BlitzkriegPopup::onSettingsButton(CCObject *)
{
  geode::openSettingsPopup(Mod::get(), false);
}
