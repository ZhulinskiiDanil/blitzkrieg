#include "./index.hpp"

StartPosLayer *StartPosLayer::create()
{
  auto ret = new StartPosLayer();
  if (ret->init())
  {
    ret->autorelease();
    return ret;
  }
  delete ret;
  return nullptr;
}

CCScene *StartPosLayer::scene()
{
  auto ret = CCScene::create();
  AppDelegate::get()->m_runningScene = ret;
  ret->addChild(StartPosLayer::create());
  return ret;
}

bool StartPosLayer::init()
{
  if (!CCLayer::init())
    return false;

  setID("StartPosLayer");
  auto winSize = CCDirector::get()->getWinSize();

  auto BG = CCSprite::create("GJ_gradientBG.png");
  BG->setColor({0, 102, 255});
  BG->setAnchorPoint({0.5f, 0.5f});
  BG->setScaleX(winSize.width / BG->getContentWidth());
  BG->setScaleY(winSize.height / BG->getContentHeight());
  BG->setPosition(winSize / 2.0f);
  addChild(BG);

  m_levelsCountLabel = CCLabelBMFont::create("", "goldFont.fnt");
  m_levelsCountLabel->setAnchorPoint({1.0f, 1.0f});
  m_levelsCountLabel->setScale(0.45f);
  m_levelsCountLabel->setPosition({winSize.width - 7.0f, winSize.height - 3.0f});
  m_levelsCountLabel->setID("level-count-label");
  addChild(m_levelsCountLabel);

  m_levelList = GJListLayer::create(
      nullptr,
      "Startpos copies",
      {0, 0, 0, 180},
      356.0f, 220.0f, 0);
  m_levelList->setPosition(winSize / 2.0f - m_levelList->getContentSize() / 2.0f);
  m_levelList->setColor({194, 114, 62});
  m_levelList->setID("GJListLayer");
  addChild(m_levelList, 5);

  m_searchBarMenu = CCMenu::create();
  m_searchBarMenu->setContentSize({356.0f, 30.0f});
  m_searchBarMenu->setPosition({0.0f, 190.0f});
  m_searchBarMenu->setID("search-bar-menu");
  m_levelList->addChild(m_searchBarMenu);

  auto searchBarBG = CCLayerColor::create({194, 114, 62, 255}, 356.0f, 30.0f);
  searchBarBG->setID("search-bar-backgrownd");
  m_searchBarMenu->addChild(searchBarBG);

  m_searchBar = TextInput::create(367.0f, "Search levels...");
  m_searchBar->setMaxCharCount(35);
  m_searchBar->setPosition({152.0f, 15.0f});
  m_searchBar->setTextAlign(TextInputAlign::Left);
  m_searchBar->getInputNode()->setLabelPlaceholderScale(0.70f);
  m_searchBar->getInputNode()->setMaxLabelScale(0.70f);
  m_searchBar->setScale(0.75f);
  m_searchBar->setID("search-bar");
  m_searchBarMenu->addChild(m_searchBar);

  auto searchBtn = CCMenuItemExt::createSpriteExtraWithFrameName("gj_findBtn_001.png", 0.7f, [this](auto) { /*search();*/ });
  searchBtn->setPosition({m_searchBarMenu->getContentWidth() -
                              searchBtn->getContentWidth() * searchBtn->getScaleX() / 2.0f - 15.0f,
                          15.0f});
  searchBtn->setID("search-button");
  m_searchBarMenu->addChild(searchBtn);

  auto btnsMenu = CCMenu::create();
  btnsMenu->setPosition({0.0f, 0.0f});
  btnsMenu->setID("buttons-menu");
  addChild(btnsMenu, 2);

  m_backButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_arrow_01_001.png", 1.0f, [this](auto)
                                                               { CCDirector::get()->popSceneWithTransition(0.5f, kPopTransitionFade); });
  m_backButton->setPosition({25.0f, winSize.height - 25.0f});
  m_backButton->setID("back-button");
  btnsMenu->addChild(m_backButton);

  m_leftButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_arrow_03_001.png", 1.0f, [this](auto) { /*page(m_page - 1);*/ });
  m_leftButton->setPosition({24.0f, winSize.height / 2.0f});
  m_leftButton->setID("prev-page-button");
  btnsMenu->addChild(m_leftButton);

  auto rightBtnSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
  rightBtnSpr->setFlipX(true);
  m_rightButton = CCMenuItemExt::createSpriteExtra(rightBtnSpr, [this](auto) { /*page(m_page + 1);*/ });
  m_rightButton->setPosition({winSize.width - 24.0f, winSize.height / 2.0f});
  m_rightButton->setID("next-page-button");
  btnsMenu->addChild(m_rightButton);

  m_infoButton = InfoAlertButton::create("Startpos copies", m_info, 1.0f);
  m_infoButton->setPosition({30.0f, 30.0f});
  m_infoButton->setID("info-button");
  btnsMenu->addChild(m_infoButton);

  auto refreshBtnSpr = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
  auto refreshButton = CCMenuItemExt::createSpriteExtra(refreshBtnSpr, [this](auto)
                                                        {
                                                          m_searchBar->setString("");
                                                          // g_levelFilters = g_defaultFilters;
                                                          // g_storedFilters = g_defaultFilters;

                                                          // showLoading();
                                                          // loadGlobalList();
                                                        });
  refreshButton->setPosition({winSize.width - refreshBtnSpr->getContentWidth() / 2.0f - 4.0f, refreshBtnSpr->getContentHeight() / 2.0f + 4.0f});
  refreshButton->setID("refresh-button");
  btnsMenu->addChild(refreshButton);

  auto pageBtnSpr = CCSprite::create("GJ_button_02.png");
  pageBtnSpr->setScale(0.7f);
  m_pageLabel = CCLabelBMFont::create("1", "bigFont.fnt");
  m_pageLabel->setScale(0.8f);
  m_pageLabel->setPosition(pageBtnSpr->getContentSize() / 2.0f);
  pageBtnSpr->addChild(m_pageLabel);
  m_pageButton = CCMenuItemExt::createSpriteExtra(pageBtnSpr, [this](auto)
                                                  {
                                                    // auto popup = SetIDPopup::create(m_page + 1, 1, (m_searchResults.size() + (m_lvlsPerPage == 10 ? 9 : 19)) / m_lvlsPerPage, "Go to Page", "Go", true, 1, 60.0f, false, false);
                                                    // popup->m_delegate = this;
                                                    // popup->show();
                                                  });
  m_pageButton->setPosition({winSize.width - m_pageButton->getContentWidth() * m_pageButton->getScaleX() / 2.0f - 3.0f, winSize.height - 39.5f});
  m_pageButton->setID("page-button");
  btnsMenu->addChild(m_pageButton);

  m_levelsList = CCLayer::create();
  m_levelsList->ignoreAnchorPointForPosition(true);
  m_levelsList->setAnchorPoint({0, 0});
  m_levelsList->setLayout(ColumnLayout::create()
                              ->setGap(5)
                              ->setAxisReverse(true)
                              ->setAxisAlignment(AxisAlignment::End)
                              ->setAutoGrowAxis(m_levelList->getContentHeight())
                              ->ignoreInvisibleChildren(false));
  m_levelsList->setContentSize(m_levelList->getContentSize());
  m_levelList->addChild(m_levelsList);

  loadStartPosLevelList();

  return true;
}

StartPosLayer::~StartPosLayer()
{
  GameLevelManager::get()->m_levelManagerDelegate = nullptr;
}

void StartPosLayer::loadLevelsFinished(CCArray *levels, char const *key)
{
  log::debug("loadLevelsFinished");

  for (GJGameLevel *level : CCArrayExt<GJGameLevel *>(levels))
  {
    m_levels.push_back(level);
  }

  reload();
}

void StartPosLayer::loadLevelsFailed(char const *key, int p1)
{
  log::warn("Failed to load featured levels: {} (key: {})", p1, key);
}

void StartPosLayer::loadLevels()
{
  std::string levelsIdsString;

  for (size_t i = 0; i < m_searchResults.size(); ++i)
  {
    auto levelId = m_searchResults[i].levelId;

    if (levelId == -1)
      continue;

    if (i > 0 && i != m_searchResults.size())
    {
      levelsIdsString += ",";
    }

    levelsIdsString += std::to_string(levelId);
  }

  auto glm = GameLevelManager::get();
  glm->m_levelManagerDelegate = this;
  glm->getOnlineLevels(GJSearchObject::create(SearchType::Type26, levelsIdsString));
}

void StartPosLayer::reload()
{
  for (int i = 0; i < m_levels.size(); i++)
  {
    auto level = m_levels[i];

    auto levelCell = LevelCell::create(m_levelsList->getContentWidth(), 90);
    levelCell->setAnchorPoint({0, 0});
    levelCell->loadFromLevel(level);
    levelCell->updateBGColor(i);
    m_levelsList->addChild(levelCell);
  }

  m_levelsList->updateLayout();
}

void StartPosLayer::loadStartPosLevelList()
{
  m_listener.spawn(web::WebRequest().get(fmt::format("{}/levels?page={}&limit={}", API_URL, m_page, m_lvlsPerPage)),
                   [this](web::WebResponse value)
                   {
                     if (value.ok())
                     {
                       auto data = value.json();
                       auto json = data.ok().value();
                       auto res = json.as<PaginationResponse<std::vector<StartPosLevel>>>();

                       if (!res.isErr())
                       {
                         auto data = res.unwrap();

                         m_searchResults = data.data;
                         m_totalLevels = data.total;

                         loadLevels();
                       }
                     }
                     else
                     {
                       log::error("Error");
                     }
                   });

  auto req = web::WebRequest();
}