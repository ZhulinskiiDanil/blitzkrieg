#include "index.hpp"

StagesPopup *StagesPopup::create(GJGameLevel *level)
{
  StagesPopup *ret = new StagesPopup();

  // Remember the delegate
  // ret->m_delegate = delegate;

  // If it turns out to be initialized the model, then we call autorelease
  // So that it itself is cleansed from memory when it will not be used
  if (ret->initAnchored(420, 250, level))
  {
    ret->autorelease();
    return ret;
  }

  // Failed to initialize, so we delete
  delete ret;
  return nullptr;
}

bool StagesPopup::setup(GJGameLevel *level)
{
  this->m_level = level;

  drawTabs();
  drawContent();

  return true;
}

void StagesPopup::drawContent()
{
  geode::TabButton *activeButton = tabButtons[0];

  for (auto *btn : tabButtons)
  {
    if (btn->isToggled())
    {
      activeButton = btn;
      break;
    }
  }

  if (activeButton)
  {
    const auto btnId = activeButton->getID();

    // Clear old content
    for (auto *container : contentContainers)
    {
      if (container)
      {
        container->removeFromParentAndCleanup(true);
      }
    }

    contentContainers.clear();

    if (btnId == "tabBtnProfilesList"_spr)
      drawProfilesList();
    else if (btnId == "tabBtnCurrentStage"_spr)
      drawCurrentStage();
    else if (btnId == "tabBtnLastRuns"_spr)
      drawLastRuns();
  }
}

void StagesPopup::drawProfilesList()
{
  if (auto oldList = m_mainLayer->getChildByID("stages-popup-profiles-list"_spr))
  {
    oldList->removeFromParentAndCleanup(true);
  }

  Padding padding{45.f, 45.f, 10.f, 10.f}; // top, bottom, left, right

  const auto profiles = getProfiles();
  const auto currentProfile = getProfileByLevel(m_level);

  const auto profileListContainer = CCNode::create();
  profileListContainer->setID("stages-popup-profiles-list"_spr);
  profileListContainer->setTag(1);

  const auto mainSize = m_mainLayer->getContentSize();
  const auto contentSize = CCSize(
      mainSize.width - padding.left - padding.right,
      mainSize.height - padding.top - padding.bottom);

  // Заголовок
  auto titleLabel = CCLabelBMFont::create("Profiles List", "goldFont.fnt");
  titleLabel->setScale(1.f);
  titleLabel->setAnchorPoint({0.f, 0.5f});
  titleLabel->setPosition({padding.left + 5.f, mainSize.height - padding.top / 2 - 5.f});
  profileListContainer->addChild(titleLabel);

  // Сам список со всеми кнопками внутри
  auto listLayer = ProfilesListLayer::create(m_level, profiles, currentProfile, contentSize);
  listLayer->setPosition({padding.left, padding.bottom});
  profileListContainer->addChild(listLayer);

  m_mainLayer->addChild(profileListContainer);
  contentContainers.push_back(profileListContainer);
}

void StagesPopup::drawCurrentStage()
{
  if (auto oldList = m_mainLayer->getChildByID("stages-popup-current-stage"_spr))
  {
    oldList->removeFromParentAndCleanup(true);
  }

  Padding padding{45.f, 10.f, 10.f, 10.f}; // top, bottom, left, right

  Profile profile = getProfileByLevel(m_level);
  Stage *currentStage = getFirstUncheckedStage(profile);

  if (!currentStage)
  {
    if (!profile.data.stages.empty())
      currentStage = &profile.data.stages.back();
    else
      currentStage = nullptr;
  }

  const auto mainSize = m_mainLayer->getContentSize();
  const auto contentSize = CCSize(
      mainSize.width - padding.left - padding.right,
      mainSize.height - padding.top - padding.bottom);

  const auto currStageContainer = CCNode::create();
  currStageContainer->setID("stages-popup-current-stage"_spr);
  currStageContainer->setTag(2);

  auto titleLabel = CCLabelBMFont::create(
      "Current stage",
      "goldFont.fnt");
  titleLabel->setScale(1.f);
  titleLabel->setAnchorPoint({0.f, 0.5f});
  titleLabel->setPosition({padding.left + 5.f, mainSize.height - padding.top / 2 - 5.f});

  // ! --- Create StageRangesList --- !
  auto rangesListContentSize = CCSize(contentSize.width, contentSize.height);
  auto rangesList = StageRangesList::create(currentStage, m_level, rangesListContentSize);
  rangesList->setPosition({padding.left, padding.bottom});

  currStageContainer->addChild(rangesList);
  currStageContainer->addChild(titleLabel);
  m_mainLayer->addChild(currStageContainer);
  contentContainers.push_back(currStageContainer);
}

void StagesPopup::drawLastRuns()
{
  drawCurrentStage();
}

void StagesPopup::drawTabs()
{
  // ! --- CLEANUP OLD CONTAINER --- !
  auto oldTabsNode = m_mainLayer->getChildByID("stages-popup-tabs-node"_spr);
  if (oldTabsNode)
    oldTabsNode->removeFromParentAndCleanup(true);

  const auto mainSize = m_mainLayer->getContentSize();
  const float btnsGap = 2.f;

  // ! --- MAIN CONTAINER --- !
  auto tabsNode = CCNode::create();
  tabsNode->setID("stages-popup-tabs-node"_spr);

  // ! --- BUTTONS --- !
  auto tabBtnProfilesList = TabButton::create(
      "Profiles List",
      this,
      menu_selector(StagesPopup::onProfilesListToggle));
  tabBtnProfilesList->setAnchorPoint({0.5f, 0.f});
  tabBtnProfilesList->setTag(1);
  tabBtnProfilesList->setID("tabBtnProfilesList"_spr);
  tabBtnProfilesList->toggle(true);

  auto tabBtnCurrentStage = TabButton::create(
      "Current Stage",
      this,
      menu_selector(StagesPopup::onCurrentStageToggle));
  tabBtnCurrentStage->setAnchorPoint({0.5f, 0.f});
  tabBtnCurrentStage->setTag(2);
  tabBtnCurrentStage->setID("tabBtnCurrentStage"_spr);

  // auto tabBtnLastRuns = TabButton::create(
  //     "Last runs",
  //     this,
  //     menu_selector(StagesPopup::onCurrentStageToggle));
  // tabBtnLastRuns->setAnchorPoint({0.5f, 0.f});
  // tabBtnLastRuns->setTag(3);
  // tabBtnLastRuns->setID("tabBtnLastRuns"_spr);

  // ! --- MENU --- !
  auto tabMenu = CCMenu::create();
  tabMenu->addChild(tabBtnProfilesList);
  tabMenu->addChild(tabBtnCurrentStage);
  // tabMenu->addChild(tabBtnLastRuns);
  tabMenu->alignItemsHorizontallyWithPadding(btnsGap);
  tabMenu->setPosition({mainSize.width / 2, mainSize.height - 3.5f});
  tabMenu->setZOrder(1);
  tabMenu->setID("stages-popup-tab-menu"_spr);

  tabButtons.clear();
  tabButtons.push_back(tabBtnProfilesList);
  tabButtons.push_back(tabBtnCurrentStage);
  // tabButtons.push_back(tabBtnLastRuns);

  // ! --- TAB BUTTONS BACKGROUNDS --- !
  for (auto *btn : tabButtons)
  {
    auto gradient = CCSprite::create("tab-gradient-mask.png"_spr);
    gradient->setAnchorPoint({0.5f, 0.f});
    gradient->setPosition(tabMenu->convertToWorldSpace(btn->getPosition()));
    gradient->setColor({190, 235, 65});
    gradient->setZOrder(0);

    tabsNode->addChild(gradient);
  }

  tabsNode->addChild(tabMenu);
  m_mainLayer->addChild(tabsNode);
}

void StagesPopup::activateTab(TabButton *sender)
{
  if (!sender)
    return;

  // Deactivate all buttons except sender
  for (auto *btn : tabButtons)
  {
    if (!btn)
      continue;

    if (btn != sender)
      btn->toggle(false);
  }

  sender->toggle(true);
  drawContent();
}

void StagesPopup::onProfilesListToggle(CCObject *obj)
{
  auto *btnProfiles = typeinfo_cast<TabButton *>(obj);
  activateTab(btnProfiles);
}

void StagesPopup::onCurrentStageToggle(CCObject *obj)
{
  auto *btnStage = typeinfo_cast<TabButton *>(obj);
  activateTab(btnStage);
}
