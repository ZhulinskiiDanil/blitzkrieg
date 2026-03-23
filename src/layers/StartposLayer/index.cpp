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
      "Nayloren's StartPos copies",
      {0, 0, 0, 180},
      356.0f, 220.0f, 0);
  m_levelList->setPosition(winSize / 2.0f - m_levelList->getContentSize() / 2.0f);
  m_levelList->setColor({194, 114, 62});
  m_levelList->setID("GJListLayer");
  addChild(m_levelList, 5);

  m_searchBarMenu = CCMenu::create();
  m_searchBarMenu->setContentSize({m_levelList->getContentWidth(), 30});
  m_searchBarMenu->setPosition({0, m_levelList->getContentHeight() - 30});
  m_searchBarMenu->setID("search-bar-menu");
  m_levelList->addChild(m_searchBarMenu);

  auto searchBarBG = RectNode::create(
      {m_searchBarMenu->getContentWidth(), m_searchBarMenu->getContentHeight()},
      ccc4FFromccc4B({0, 0, 0, static_cast<int>(255 * .1f)}));
  searchBarBG->setID("search-bar-backgrownd");
  m_searchBarMenu->addChild(searchBarBG);

  // ! --- Search Button --- !
  auto searchBtn = CCMenuItemExt::createSpriteExtraWithFrameName("gj_findBtn_001.png", 0.7f, [this](auto)
                                                                 {
                                                                  if (m_searchBar) {
                                                                    m_page = 1;
                                                                    m_query = m_searchBar->getString();
                                                                    loadStartPosLevelList();
                                                                  } });
  searchBtn->setAnchorPoint({1, 0.5f});
  searchBtn->setPosition({m_searchBarMenu->getContentWidth() - 6, m_searchBarMenu->getContentHeight() / 2});
  searchBtn->setID("search-button");
  m_searchBarMenu->addChild(searchBtn);

  // ! --- Search Bar --- !
  m_searchBar = TextInput::create(
      (m_searchBarMenu->getContentWidth() - searchBtn->getScaledContentWidth() - 5 * 3 - 1) * (1 / 0.75f),
      "Search levels...");
  m_searchBar->setScale(0.75f);
  m_searchBar->setMaxCharCount(35);
  m_searchBar->setAnchorPoint({0, 0.5f});
  m_searchBar->ignoreAnchorPointForPosition(true);
  m_searchBar->setPosition({6, 0});
  m_searchBar->setTextAlign(TextInputAlign::Left);
  m_searchBar->getInputNode()->setLabelPlaceholderScale(0.70f);
  m_searchBar->getInputNode()->setMaxLabelScale(0.70f);
  m_searchBar->setID("search-bar");
  m_searchBarMenu->addChild(m_searchBar);

  auto btnsMenu = CCMenu::create();
  btnsMenu->setPosition({0.0f, 0.0f});
  btnsMenu->setID("buttons-menu");
  addChild(btnsMenu, 2);

  m_backButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_arrow_01_001.png", 1.0f, [this](auto)
                                                               { CCDirector::get()->popSceneWithTransition(0.5f, kPopTransitionFade); });
  m_backButton->setPosition({25.0f, winSize.height - 25.0f});
  m_backButton->setID("back-button");
  btnsMenu->addChild(m_backButton);

  m_leftButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_arrow_03_001.png", 1.0f, [this](auto)
                                                               { page(m_page - 1); });
  m_leftButton->setPosition({24.0f, winSize.height / 2.0f});
  m_leftButton->setID("prev-page-button");
  btnsMenu->addChild(m_leftButton);

  auto rightBtnSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
  rightBtnSpr->setFlipX(true);
  m_rightButton = CCMenuItemExt::createSpriteExtra(rightBtnSpr, [this](auto)
                                                   { page(m_page + 1); });
  m_rightButton->setPosition({winSize.width - 24.0f, winSize.height / 2.0f});
  m_rightButton->setID("next-page-button");
  btnsMenu->addChild(m_rightButton);

  m_infoButton = InfoAlertButton::create("StartPos copies", m_info, 1.0f);
  m_infoButton->setPosition({30.0f, 30.0f});
  m_infoButton->setID("info-button");
  btnsMenu->addChild(m_infoButton);

  auto refreshBtnSpr = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
  auto refreshButton = CCMenuItemExt::createSpriteExtra(refreshBtnSpr, [this](auto)
                                                        {
                                                          m_searchBar->setString(m_query);

                                                          toggleLoading(true);
                                                          loadStartPosLevelList(); });
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

  m_loadingCircle = LoadingCircle::create();
  m_loadingCircle->setParentLayer(this);
  m_loadingCircle->setID("loading-circle");
  m_loadingCircle->show();

  loadStartPosLevelList();

  return true;
}

StartPosLayer::~StartPosLayer()
{
  GameLevelManager::get()->m_levelManagerDelegate = nullptr;
}

void StartPosLayer::loadLevelsFinished(CCArray *levels, char const *key)
{
  toggleLoading(false);

  for (GJGameLevel *level : CCArrayExt<GJGameLevel *>(levels))
    m_levels.push_back(level);

  reload();
}

void StartPosLayer::loadLevelsFailed(char const *key, int p1)
{
  toggleLoading(false);

  FLAlertLayer::create(
      "Load Error",
      fmt::format("Failed to load Nayloren's levels: {} (key: {})", p1, key),
      "OK")
      ->show();

  log::warn("Failed to load Nayloren's levels: {} (key: {})", p1, key);
}

void StartPosLayer::loadLevels()
{
  m_levels.clear();
  m_downloadUrls.clear();

  std::string levelsIdsString;

  for (size_t i = 0; i < m_searchResults.size(); ++i)
  {
    auto levelId = m_searchResults[i].levelId;
    auto originalId = m_searchResults[i].originalId;

    int id = levelId == -1 ? originalId : levelId;

    if (id <= 0)
      continue;

    if (!levelsIdsString.empty())
      levelsIdsString += ",";

    levelsIdsString += std::to_string(id);
  }
  auto gdStr = gd::string{levelsIdsString.data(), levelsIdsString.size()};

  auto glm = GameLevelManager::get();
  glm->m_levelManagerDelegate = this;
  glm->getOnlineLevels(GJSearchObject::create(SearchType::Type26, gdStr));
}

void StartPosLayer::reload()
{
  if (auto listView = m_levelList->m_listView)
  {
    listView->removeFromParentAndCleanup(true);
    m_levelList->m_listView = nullptr;
  }

  auto listView = CustomListView::create(CCArray::create(), BoomListType::Level, 190.0f, m_levelList->getContentWidth());
  listView->retain();
  m_levelList->addChild(listView, 6, 9);
  m_levelList->m_listView = listView;

  auto tableView = listView->m_tableView;
  tableView->m_delegate = this;
  tableView->m_tableDelegate = this;
  tableView->m_dataSource = this;
  tableView->setTouchEnabled(true);

  tableView->setContentSize({m_levelList->getContentWidth(), 190.0f});

  tableView->reloadData();
  tableView->moveToTop();
}

void StartPosLayer::page(size_t page)
{
  m_page = std::clamp(page, (size_t)1, m_totalPages);

  toggleLoading(true);
  loadStartPosLevelList();
}

void StartPosLayer::toggleLoading(bool isToggled)
{
  if (isToggled)
  {
    m_pageLabel->setString(fmt::to_string(m_page).c_str());
    m_loadingCircle->setVisible(true);
    if (auto listView = m_levelList->m_listView)
      listView->setVisible(false);
    m_levelsCountLabel->setVisible(false);
    m_leftButton->setVisible(false);
    m_rightButton->setVisible(false);
    m_pageButton->setVisible(false);
  }
  else
  {
    m_loadingCircle->setVisible(false);
    m_isLoading = false;
  }
}

void StartPosLayer::onOpenDownloadLink(CCObject *sender)
{
  int i = sender->getTag();
  auto it = m_downloadUrls.find(i);

  if (it != m_downloadUrls.end())
  {
    std::string url = it->second;

    if (!url.empty())
    {
      geode::createQuickPopup(
          "Open Link",
          fmt::format("Do you want to open this link?\n\n{}", url),
          "Cancel",
          "Open",
          [url](auto, bool btn2)
          {
            if (btn2)
            {
              geode::utils::web::openLinkInBrowser(url);
            }
          });
    }
  }
}

// ! === Rows === !

unsigned int StartPosLayer::numberOfSectionsInTableView(TableView *table)
{
  return 1;
}

int StartPosLayer::numberOfRowsInSection(unsigned int section, TableView *table)
{
  // At least 2 elements to fix the scroll even if we draw 1 or 0
  return std::max(m_searchResults.size(), (size_t)2);
}

TableViewCell *StartPosLayer::cellForRowAtIndexPath(CCIndexPath &indexPath, TableView *table)
{
  int index = indexPath.m_row;
  auto startposLevel = m_searchResults[index];

  if (startposLevel.levelId > 0)
  {
    auto cell = LevelCell::create(m_levelList->getContentWidth(), m_levelCellHeigth);
    GJGameLevel *levelObj = nullptr;

    for (int i = 0; i < m_levels.size(); i++)
    {
      if (m_levels[i]->m_levelID == startposLevel.levelId)
        levelObj = m_levels[i];
    }

    if (!levelObj)
    {
      return nullptr;
    }

    cell->loadFromLevel(levelObj);
    cell->updateBGColor(index);

    return cell;
  }
  else if (!startposLevel.downloadUrl.empty())
  {
    auto cell = new TableViewCell("text-cell", m_levelList->getContentWidth(), m_customCellHeigth);
    cell->m_mainLayer->setContentSize({m_levelList->getContentWidth(), m_customCellHeigth});

    // ! --- Level Name Label --- !
    auto levelNameLabel = CCLabelBMFont::create(startposLevel.levelName.c_str(), "bigFont.fnt");

    float availableWidth = cell->m_mainLayer->getContentWidth() - 20.0f;

    float labelWidth = levelNameLabel->getContentWidth();

    float scale = 1.0f;
    if (labelWidth > 0.0f)
      scale = availableWidth / labelWidth;

    scale = std::min(scale, 0.75f);

    levelNameLabel->setScale(scale);
    levelNameLabel->setAnchorPoint({0, 1});
    levelNameLabel->setPosition(10, cell->m_mainLayer->getContentHeight() - 8);
    cell->m_mainLayer->addChild(levelNameLabel);

    // ! --- Author Label --- !
    auto authorLabel = CCLabelBMFont::create("By Nayloren", "goldFont.fnt");
    authorLabel->setScale(.6f);
    authorLabel->setAnchorPoint({0, 1});
    authorLabel->setPosition(10, levelNameLabel->getPositionY() - levelNameLabel->getScaledContentHeight() - 1);
    cell->m_mainLayer->addChild(authorLabel);

    // ! --- Button --- !
    auto spr = ButtonSprite::create("Download .gmd");
    auto btn = CCMenuItemSpriteExtra::create(
        spr, this, menu_selector(StartPosLayer::onOpenDownloadLink));
    btn->setAnchorPoint({0, 0});
    btn->setScale(.8f);
    btn->setTag(index);
    btn->m_baseScale = .75f;

    m_downloadUrls[index] = startposLevel.downloadUrl;

    // ! --- Button Menu --- !
    auto btnMenu = CCMenu::create();
    btnMenu->setContentSize(btn->getScaledContentSize());
    btnMenu->setPosition({10, 10});

    btnMenu->addChild(btn);
    cell->m_mainLayer->addChild(btnMenu);

    return cell;
  }

  // else if (startposLevel.originalId > 0 && !startposLevel.downloadUrl.empty())
  // {
  //   auto cell = LevelCell::create(m_levelList->getContentWidth(), m_levelCellHeigth);
  //   GJGameLevel *levelObj = nullptr;

  //   for (int i = 0; i < m_levels.size(); i++)
  //   {
  //     if (m_levels[i]->m_levelID == startposLevel.originalId)
  //       levelObj = m_levels[i];
  //   }
  //   if (!levelObj)
  //     return nullptr;

  //   cell->loadFromLevel(levelObj);
  //   cell->updateBGColor(index);

  //   return cell;
  // }

  return nullptr;
}

float StartPosLayer::cellHeightForRowAtIndexPath(CCIndexPath &indexPath, TableView *table)
{
  bool isDefaultLevel = m_searchResults[indexPath.m_row].levelId > 0;
  bool isOriginalLevel = m_searchResults[indexPath.m_row].originalId > 0 && !m_searchResults[indexPath.m_row].downloadUrl.empty();
  bool isCustomLevel = !m_searchResults[indexPath.m_row].downloadUrl.empty();

  if (isDefaultLevel || isOriginalLevel)
    return m_levelCellHeigth;
  else if (isCustomLevel)
    return m_customCellHeigth;
  else
    return m_levelCellHeigth;
}

void StartPosLayer::loadStartPosLevelList()
{
  if (m_isLoading)
    return;
  else
    toggleLoading(true);

  std::string reqUrl = fmt::format("{}/levels?page={}&limit={}&levelName={}", API_URL, m_page, m_lvlsPerPage, encodeURIComponent(m_query));

  m_listener.spawn(web::WebRequest().get(reqUrl),
                   [this](web::WebResponse value)
                   {
                     if (value.ok() && value.code() == 200)
                     {
                       auto data = value.json();
                       auto json = data.ok().value();
                       auto res = json.as<PaginationResponse<std::vector<StartPosLevel>>>();

                       if (!res.isErr())
                       {
                         auto data = res.unwrap();

                         m_searchResults = data.data;
                         m_totalLevels = data.total;
                         m_totalPages = data.totalPages;

                         if (data.data.size() > 0)
                         {
                           loadLevels();
                         }
                         else
                         {
                           toggleLoading(false);
                         }
                       }
                       else
                       {
                         toggleLoading(false);
                       }
                     }
                     else
                     {
                       toggleLoading(false);

                       std::string body = value.string().ok() ? value.string().unwrap() : "no body";

                       FLAlertLayer::create(
                           "Load Error",
                           fmt::format("HTTP {}: {}", value.code(), body),
                           "OK")
                           ->show();

                       log::warn("HTTP error {}: {}", value.code(), body);
                     }
                   });

  auto req = web::WebRequest();
}