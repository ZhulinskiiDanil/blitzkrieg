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
    else if (btnId == "session-tab"_spr)
      drawSession();
    else if (btnId == "goals-tab"_spr)
      drawGoals();
    else if (btnId == "backups-tab"_spr)
      drawBackups();
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

  // The header of a previous draw is gone with its container
  m_stageHeader = nullptr;

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

  // ! --- Header --- !
  const CCPoint headerOrigin{padding.left + 2.f, m_size.height - padding.top + 4.f};
  const float filtersLeft = filterButtonsMenu->boundingBox().getMinX();

  drawStageHeader(headerOrigin, filtersLeft - 10.f - headerOrigin.x);

  // ! --- StageListLayer --- !
  auto stageListContentSize = CCSize(contentSize.width, contentSize.height);
  m_stageList = StageListLayer::create(
      m_level,
      stageListContentSize,
      m_requestedStageIndex,
      m_requestedRangeId);
  m_requestedStageIndex.reset();
  m_requestedRangeId.clear();
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
  stagesGraphLayer->onOpenStage = [this](int stageIndex, std::string const &rangeId)
  {
    openStageInBrowser(stageIndex, rangeId);
  };
  stagesGraphLayer->onOpenProfiles = [this]()
  {
    onOpenProfiles(nullptr);
  };
  m_currentStageGraphNode->addChild(stagesGraphLayer);

  m_mainLayer->addChild(m_currentStageGraphNode);
  contentContainers.push_back(m_currentStageGraphNode);
}

void BlitzkriegPopup::drawSession()
{
  m_sessionNode = CCNode::create();
  m_sessionNode->setID("blitzkrieg-popup-session"_spr);
  m_sessionNode->setTag(5);

  // ! --- SessionLayer --- !
  m_sessionNode->addChild(SessionLayer::create(m_level, m_size));

  m_mainLayer->addChild(m_sessionNode);
  contentContainers.push_back(m_sessionNode);
}

void BlitzkriegPopup::drawGoals()
{
  m_goalsNode = CCNode::create();
  m_goalsNode->setID("blitzkrieg-popup-goals"_spr);
  m_goalsNode->setTag(7);

  // ! --- GoalsLayer --- !
  m_goalsNode->addChild(GoalsLayer::create(m_level, m_size));

  m_mainLayer->addChild(m_goalsNode);
  contentContainers.push_back(m_goalsNode);
}

void BlitzkriegPopup::drawBackups()
{
  m_backupsNode = CCNode::create();
  m_backupsNode->setID("blitzkrieg-popup-backups"_spr);
  m_backupsNode->setTag(6);

  // ! --- BackupsLayer --- !
  m_backupsNode->addChild(BackupsLayer::create(m_size));

  m_mainLayer->addChild(m_backupsNode);
  contentContainers.push_back(m_backupsNode);
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
      {"session-tab"_spr, "Session", "tab-icon-session.png"_spr},
      {"goals-tab"_spr, "Goals", "tab-icon-goals.png"_spr},
      {"backups-tab"_spr, "Backups", "tab-icon-backups.png"_spr},
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

void BlitzkriegPopup::drawStageHeader(CCPoint const &origin, float width)
{
  m_stageHeader = StageHeader::create(std::max(width, 100.f));
  m_stageHeader->setPosition(origin);
  m_currentStageNode->addChild(m_stageHeader);

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
  if (!m_stageHeader)
    return;

  auto *profile = GlobalStore::get()->getProfileByLevel(m_levelId);

  // ! --- Status: stages before the first unchecked one are done, after it locked --- !
  auto status = StageHeaderStatus::Completed;
  std::string currentRangeId;

  if (profile)
  {
    const auto stages = getConsideredStages(profile->data.stages);

    for (std::size_t i = 0; i < stages.size(); ++i)
    {
      if (isStageDeepChecked(*stages[i]))
        continue;

      const int progressIndex = static_cast<int>(i);

      if (stageIndex == progressIndex)
        status = StageHeaderStatus::Current;
      else if (stageIndex > progressIndex)
        status = StageHeaderStatus::Locked;

      break;
    }

    currentRangeId = GlobalStore::get()->getCurrentRange(profile->id).id;
  }

  m_stageHeader->setStage(stageIndex, stage, status, currentRangeId);
}

void BlitzkriegPopup::openStageInBrowser(int stageIndex, std::string const &rangeId)
{
  // The graph that asks for it is removed by the tab switch
  geode::queueInMainThread(
      [self = Ref<BlitzkriegPopup>(this), stageIndex, rangeId]()
      {
        if (self->tabButtons.size() < 2)
          return;

        self->m_requestedStageIndex = stageIndex;
        self->m_requestedRangeId = rangeId;
        self->activateTab(self->tabButtons[1]);
      });
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
