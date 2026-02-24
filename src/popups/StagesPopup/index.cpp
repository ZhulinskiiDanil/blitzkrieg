#include "index.hpp"
#include "StageChangedEvent.hpp"

StagesPopup *StagesPopup::create(GJGameLevel *level)
{
  StagesPopup *ret = new StagesPopup();

  if (ret->init(level))
  {
    ret->autorelease();
    return ret;
  }

  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool StagesPopup::init(GJGameLevel *level)
{
  if (!Popup::init(420, 250, "GJ_square01_custom.png"_spr))
  {
    return false;
  }

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

  Padding padding{12.f, 45.f, 10.f, 10.f}; // top, bottom, left, right

  const auto profiles = GlobalStore::get()->getProfiles();

  const auto profileListContainer = CCNode::create();
  profileListContainer->setID("stages-popup-profiles-list"_spr);
  profileListContainer->setTag(1);

  const auto contentSize = CCSize(
      m_size.width - padding.left - padding.right,
      m_size.height - padding.top - padding.bottom);

  // ! --- ProfilesListLayer --- !
  auto listLayer = ProfilesListLayer::create(m_level, profiles, contentSize);
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

  Padding padding{55.f, 10.f, 10.f, 10.f}; // top, bottom, left, right

  Profile profile = GlobalStore::get()->getProfileByLevel(m_level);
  Stage *currentStage = getFirstUncheckedStage(profile);

  if (!currentStage && !profile.data.stages.empty())
    currentStage = &profile.data.stages.back();

  const auto contentSize = CCSize(
      m_size.width - padding.left - padding.right,
      m_size.height - padding.top - padding.bottom);

  m_currentStageNode = CCNode::create();
  m_currentStageNode->setID("stages-popup-current-stage"_spr);
  m_currentStageNode->setTag(2);

  // ! --- Title --- !
  drawCurrentStageTitle(
      currentStage,
      profile.data.stages.size(), padding);

  // ! --- StageListLayer --- !
  auto stageListContentSize = CCSize(contentSize.width, contentSize.height);
  auto stageList = StageListLayer::create(currentStage, m_level, stageListContentSize);
  stageList->setPosition({padding.left, padding.bottom});

  m_currentStageNode->addChild(stageList);
  m_mainLayer->addChild(m_currentStageNode);
  contentContainers.push_back(m_currentStageNode);
}

void StagesPopup::drawCurrentStageTitle(Stage *currentStage, int totalStages, Padding padding)
{
  if (m_currentStageNode)
  {
    std::string title = fmt::format(
        "Stage: {}/{}",
        currentStage ? geode::utils::numToString(currentStage->stage) : "?",
        totalStages > 0
            ? geode::utils::numToString(totalStages)
            : "?");

    m_currentStageTitleLabel = CCLabelBMFont::create(
        title.c_str(),
        "goldFont.fnt");
    m_currentStageTitleLabel->setPosition({m_size.width / 2, m_size.height - padding.top / 2 + 5}); // n - 2.5f
    m_currentStageNode->addChild(m_currentStageTitleLabel);

    std::string stat = "";

    float totalAttempts = 0;
    float totalTimePlayed = 0;

    if (currentStage)
    {
      for (const auto &range : currentStage->ranges)
      {
        totalAttempts += range.attempts;
        totalTimePlayed += range.timePlayed;
      }
    }

    stat += fmt::format("{} <small>Attempts</small> ", totalAttempts);
    stat += formatTimePlayed(totalTimePlayed);

    m_totalStatLabel = Label::create(stat, "bigFont.fnt", .4f);
    m_totalStatLabel->setPosition({m_size.width / 2, m_size.height - padding.top / 2 - 15});
    m_currentStageNode->addChild(m_totalStatLabel);

    m_stageChangedListener = StageChangedEvent().listen(
        [this](int totalStages, Stage *currentStage)
        {
          std::string newTitle = fmt::format(
              "Stage: {}/{}",
              currentStage ? geode::utils::numToString(currentStage->stage) : "?",
              totalStages > 0
                  ? geode::utils::numToString(totalStages)
                  : "?");

          m_currentStageTitleLabel->setString(newTitle.c_str());

          std::string stat = "";

          float totalAttempts = 0;
          float totalTimePlayed = 0;

          if (currentStage)
          {
            for (const auto &range : currentStage->ranges)
            {
              totalAttempts += range.attempts;
              totalTimePlayed += range.timePlayed;
            }
          }

          stat += fmt::format("{} <small>Attempts</small> ", totalAttempts);
          stat += formatTimePlayed(totalTimePlayed);

          m_totalStatLabel->setText(stat);

          return ListenerResult::Propagate;
        });
    // m_stageChangedListener.leak();
  }
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
  tabMenu->setPosition({m_size.width / 2, m_size.height - 3.5f});
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
    gradient->setColor({82, 82, 82});
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
