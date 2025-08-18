#include "index.hpp"

StagesPopup *StagesPopup::create(GJGameLevel *level)
{
  StagesPopup *ret = new StagesPopup();

  // Запоминаем делегата
  // ret->m_delegate = delegate;

  // Если получается инициализироват модалку, то вызываем autorelease
  // Чтобы она сама очистилась из памяти когда не будет использоваться
  if (ret->initAnchored(420, 250, level))
  {
    ret->autorelease();
    return ret;
  }

  // Не удалось инициализировать, поэтому удаляем
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
      break; // нашли активную кнопку, дальше искать не нужно
    }
  }

  // Проверка
  if (activeButton)
  {
    const auto tag = activeButton->getTag();

    // Clear old content
    for (auto *container : contentContainers)
    {
      if (container)
      {
        container->removeFromParentAndCleanup(true);
      }
    }

    contentContainers.clear();

    if (tag == 1)
      drawProfilesList();
    else if (tag == 2)
      drawCurrentStage();
  }
}

void StagesPopup::drawProfilesList()
{
  auto oldList = dynamic_cast<CCNode *>(m_mainLayer->getChildByID("stages-popup-profiles-list"_spr));
  if (oldList)
  {
    oldList->removeFromParentAndCleanup(true);
  }

  Padding padding{50.f, 50.f, 20.f, 20.f}; // top, bottom, left, right

  const auto profiles = getProfiles();
  const auto currentProfile = getProfileByLevel(m_level);

  const auto profileListContainer = CCNode::create();
  profileListContainer->setID("stages-popup-profiles-list"_spr);
  profileListContainer->setTag(1);
  const auto mainSize = m_mainLayer->getContentSize();
  const auto contentSize = CCSize(
      mainSize.width - padding.left - padding.right,
      mainSize.height - padding.top - padding.bottom);

  auto titleLabel = CCLabelBMFont::create(
      "Profiles List",
      "goldFont.fnt");
  titleLabel->setScale(1.f);
  titleLabel->setAnchorPoint({0.5f, 0.5f});
  titleLabel->setPosition({mainSize.width / 2, mainSize.height - 25.f});

  // ScrollLayer that fills the main layer
  auto scroll = ScrollLayer::create(contentSize);

  // Уменьшаем видимую область ScrollLayer на padding
  scroll->setContentSize({contentSize.width - 16,
                          contentSize.height - 16});

  // Сдвигаем позицию ScrollLayer внутри родителя
  scroll->setPosition({padding.left + 8,
                       padding.bottom + 8});

  // Column layout: vertical list, aligned to bottom, auto-grow height
  scroll->m_contentLayer->setLayout(
      ColumnLayout::create()
          ->setGap(5.f)
          ->setAxisAlignment(AxisAlignment::End)
          ->setAutoGrowAxis(scroll->getContentHeight()));

  // Add each profile as a cell
  for (size_t i = 0; i < profiles.size(); ++i)
  {
    bool isCurrentProfile = false;

    if (!currentProfile.id.empty())
    {
      isCurrentProfile = (currentProfile.id == profiles[i].id);
    }

    ProfileStats stats = getProfileStats(profiles[i]);
    bool isAllStagesCompleted = !stats.currentStage.has_value() || stats.currentStage >= stats.totalStages;

    auto cellSize = CCSize(scroll->getContentWidth(), 40.f);

    auto cell = CCLayer::create();
    cell->setContentSize(cellSize);

    auto cellBackground = CCScale9Sprite::create("square02b_small.png");
    cellBackground->setContentSize(cellSize);
    cellBackground->setPosition({cellSize.width / 2, cellSize.height / 2});
    cellBackground->setColor({0, 0, 0});
    cellBackground->setOpacity(255 * 0.3f);
    cell->addChild(cellBackground);

    // Profile name label
    std::string profileName = profiles[i].profileName;

    // Ограничиваем 20 символами
    if (profileName.length() > 20)
    {
      profileName = profileName.substr(0, 20) + "...";
    }

    // Profile name label
    auto profileLabel = CCLabelBMFont::create(
        profileName.c_str(),
        "bigFont.fnt");
    profileLabel->setScale(.5f);
    profileLabel->setAnchorPoint({0, 0.5f});
    profileLabel->setPosition({10.f, 25.f});
    if (stats.currentStage > stats.totalStages / 2)
    {
      profileLabel->setColor({253, 165, 106});
    }
    if (isAllStagesCompleted)
    {
      profileLabel->setColor({99, 224, 110});
    }
    cell->addChild(profileLabel);

    std::string labelText;
    if (isAllStagesCompleted)
    {
      labelText = "Stages: " + std::to_string(stats.totalStages) + "/" +
                  std::to_string(stats.totalStages); // все пройдены
    }
    else
    {
      labelText = "Stages: " + std::to_string(stats.currentStage.value()) + "/" +
                  std::to_string(stats.totalStages);
    }

    // Profile name label
    auto statLabel = CCLabelBMFont::create(
        labelText.c_str(),
        "bigFont.fnt");
    statLabel->setScale(0.25f);
    statLabel->setAnchorPoint({0, 0.5f});
    statLabel->setPosition({10.f, 10.f});
    statLabel->setOpacity(255 * 0.5f);
    cell->addChild(statLabel);

    // Can select if it isn't a current profile
    if (!isCurrentProfile)
    {
      // "Select" button
      auto btnSpr = ButtonSprite::create("Select");
      btnSpr->setScale(0.6f);

      auto btn = CCMenuItemSpriteExtra::create(
          btnSpr,
          this,
          menu_selector(StagesPopup::onProfileSelect));
      btn->setTag(static_cast<int>(i));

      auto menu = CCMenu::create();
      menu->addChild(btn);
      menu->setPosition({scroll->getContentWidth() - btn->getContentWidth() / 2 - 10.f, 20.f});
      cell->addChild(menu);
    }
    else
    {
      // "Deselect" button
      auto btnSpr = ButtonSprite::create("Deselect");
      btnSpr->setScale(0.6f);

      auto btn = CCMenuItemSpriteExtra::create(
          btnSpr,
          this,
          menu_selector(StagesPopup::onProfileDeselect));
      btn->setTag(static_cast<int>(i));

      auto menu = CCMenu::create();
      menu->addChild(btn);
      menu->setPosition({scroll->getContentWidth() - btn->getContentWidth() / 2 - 10.f, 20.f});
      cell->addChild(menu);
    }

    scroll->m_contentLayer->addChild(cell);
  }

  scroll->m_contentLayer->updateLayout();
  scroll->scrollToTop();

  // Create and configure list borders
  auto borders = ListBorders::create();
  borders->setContentSize(contentSize);
  borders->setAnchorPoint({0.5f, 0.5f});
  borders->setPosition({contentSize.width / 2 + padding.left,
                        contentSize.height / 2 + padding.bottom});

  auto btnsGap = 5.f;

  // Create button
  auto btnSprCreate = ButtonSprite::create("Create");
  btnSprCreate->setScale(1.f);
  btnSprCreate->setAnchorPoint({0.f, 0.5f});

  auto btnCreate = CCMenuItemSpriteExtra::create(
      btnSprCreate,
      this,
      menu_selector(StagesPopup::onCreate));

  // Import button
  auto btnSprImport = ButtonSprite::create("Import");
  btnSprImport->setScale(1.f);
  btnSprImport->setAnchorPoint({0.f, 0.5f});

  auto btnImport = CCMenuItemSpriteExtra::create(
      btnSprImport,
      this,
      menu_selector(StagesPopup::onImport));

  // Export button
  auto btnSprExport = ButtonSprite::create("Export");
  btnSprExport->setScale(1.f);
  btnSprExport->setAnchorPoint({0.f, 0.5f});

  auto btnExport = CCMenuItemSpriteExtra::create(
      btnSprExport,
      this,
      menu_selector(StagesPopup::onExport));

  auto btnMenu = CCMenu::create();
  btnMenu->addChild(btnCreate);
  btnMenu->addChild(btnImport);
  btnMenu->addChild(btnExport);
  btnMenu->alignItemsHorizontallyWithPadding(btnsGap);
  btnMenu->setPosition({mainSize.width / 2, 28.f});

  profileListContainer->addChild(titleLabel);
  profileListContainer->addChild(scroll);
  profileListContainer->addChild(borders);
  profileListContainer->addChild(btnMenu);

  m_mainLayer->addChild(profileListContainer);
  contentContainers.push_back(profileListContainer);
}

void StagesPopup::drawCurrentStage()
{
  auto oldList = dynamic_cast<CCNodeContainer *>(m_mainLayer->getChildByID("stages-popup-current-stage"_spr));
  if (oldList)
  {
    oldList->removeFromParentAndCleanup(true);
  }

  Padding padding{50.f, 10.f, 10.f, 10.f}; // top, bottom, left, right

  Profile profile = getProfileByLevel(m_level);
  Stage *currentStage = getFirstUncheckedStage(profile);

  const auto mainSize = m_mainLayer->getContentSize();
  const auto contentSize = CCSize(
      mainSize.width - padding.left - padding.right,
      mainSize.height - padding.top - padding.bottom);

  const auto currStageContainer = CCNodeContainer::create();
  currStageContainer->setID("stages-popup-current-stage"_spr);
  currStageContainer->setTag(2);

  auto titleLabel = CCLabelBMFont::create(
      "Current stage",
      "goldFont.fnt");
  titleLabel->setScale(1.f);
  titleLabel->setAnchorPoint({0.5f, 0.5f});
  titleLabel->setPosition({mainSize.width / 2, mainSize.height - 25.f});

  // ScrollLayer that fills the main layer
  auto scroll = ScrollLayer::create(contentSize);

  // Уменьшаем видимую область ScrollLayer на padding
  scroll->setContentSize({contentSize.width - 16,
                          contentSize.height - 16});

  // Сдвигаем позицию ScrollLayer внутри родителя
  scroll->setPosition({padding.left + 8,
                       padding.bottom + 8});

  // Column layout: vertical list, aligned to bottom, auto-grow height
  scroll->m_contentLayer->setLayout(
      ColumnLayout::create()
          ->setGap(5.f)
          ->setAxisAlignment(AxisAlignment::End)
          ->setAutoGrowAxis(scroll->getContentHeight()));

  // Create and configure list borders
  auto borders = ListBorders::create();
  borders->setContentSize(contentSize);
  borders->setAnchorPoint({0.5f, 0.5f});
  borders->setPosition({contentSize.width / 2 + padding.left,
                        contentSize.height / 2 + padding.bottom});

  // Draw stages

  if (currentStage)
  {
    for (auto &range : currentStage->ranges)
    {
      auto cellSize = CCSize(scroll->getContentWidth(), 25.f);

      auto cell = CCLayer::create();
      cell->setContentSize(cellSize);

      auto cellBackground = CCScale9Sprite::create("square02b_small.png");
      cellBackground->setContentSize(cellSize);
      cellBackground->setPosition({cellSize.width / 2, cellSize.height / 2});
      cellBackground->setColor({0, 0, 0});
      cellBackground->setOpacity(255 * 0.3f);
      cell->addChild(cellBackground);

      std::string rangeText = std::to_string(range.from) + "-" +
                              std::to_string(range.to) +
                              " (Total: " +
                              std::to_string(std::abs(range.to - range.from)) + "%)";

      // Range
      auto rangeLabel = CCLabelBMFont::create(
          rangeText.c_str(),
          "bigFont.fnt");
      rangeLabel->setScale(.5f);
      rangeLabel->setAnchorPoint({0, 0.5f});
      rangeLabel->setPosition({5.f, cellSize.height / 2});

      if (!range.checked)
        rangeLabel->setColor({253, 165, 106});
      else
        rangeLabel->setColor({99, 224, 110});

      cell->addChild(rangeLabel);
      scroll->m_contentLayer->addChild(cell);
    }
  }

  // After all cells added
  scroll->m_contentLayer->updateLayout();
  scroll->scrollToTop();

  currStageContainer->addChild(scroll);
  currStageContainer->addChild(borders);
  currStageContainer->addChild(titleLabel);
  m_mainLayer->addChild(currStageContainer);
  contentContainers.push_back(currStageContainer);
}

void StagesPopup::drawTabs()
{
  auto oldMenu = dynamic_cast<CCMenu *>(m_mainLayer->getChildByID("stages-popup-tab-menu"_spr));
  if (oldMenu)
  {
    oldMenu->removeFromParentAndCleanup(true);
  }

  const auto mainSize = m_mainLayer->getContentSize();
  const float btnsGap = 10.f;

  // Profiles-List tab button
  auto tabBtnProfilesList = TabButton::create(
      "Profiles List",
      this,
      menu_selector(StagesPopup::onProfilesListToggle));
  tabBtnProfilesList->setAnchorPoint({0.5f, 0.f});
  tabBtnProfilesList->setTag(1);
  tabBtnProfilesList->setID("tabBtnProfilesList"_spr);
  tabBtnProfilesList->toggle(true);

  // Current-Stage tab button
  auto tabBtnCurrentStage = TabButton::create(
      "Current Stage",
      this,
      menu_selector(StagesPopup::onCurrentStageToggle));
  tabBtnCurrentStage->setAnchorPoint({0.5f, 0.f});
  tabBtnCurrentStage->setTag(2);
  tabBtnCurrentStage->setID("tabBtnCurrentStage"_spr);

  auto tabMenu = CCMenu::create();
  tabMenu->addChild(tabBtnProfilesList);
  tabMenu->addChild(tabBtnCurrentStage);
  tabMenu->alignItemsHorizontallyWithPadding(btnsGap);
  tabMenu->setPosition({mainSize.width / 2, mainSize.height - 3.5f});
  tabMenu->setID("stages-popup-tab-menu"_spr);

  tabButtons.clear();
  tabButtons.push_back(tabBtnProfilesList);
  tabButtons.push_back(tabBtnCurrentStage);

  m_mainLayer->addChild(tabMenu);
}

void StagesPopup::activateTab(TabButton *btnToActivate)
{
  if (!btnToActivate)
    return;

  // Деактивируем все кнопки кроме btnToActivate
  for (auto *btn : tabButtons)
  {
    if (!btn)
      continue;

    if (btn != btnToActivate)
      btn->toggle(false);
  }

  btnToActivate->toggle(true);

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

void StagesPopup::onCreate(CCObject *obj)
{
  geode::utils::web::openLinkInBrowser("https://dgkr-community.vercel.app/blitzkrieg?helpCreateProfile=true");
}

void StagesPopup::onImport(CCObject *obj)
{
  selectJsonFile([this](std::string jsonContent)
                 {
        if (jsonContent.empty()) {
            geode::log::debug("File doesn't selected or empty");
            return;
        }

        // Парсим JSON без исключений
        json parsed = json::parse(jsonContent, nullptr, false);

        if (parsed.is_discarded()) {
            geode::log::debug("JSON Parse error");
            return;
        }

        if (!parsed.is_array()) {
            geode::log::debug("Error: Expected profiles array");
            return;
        }

        std::vector<Profile> profiles;

        // Здесь используется твой from_json для Profile
        for (auto const& item : parsed) {
            if (!item.is_object()) {
                geode::log::debug("Skip JSON element: not an object");
                continue;
            }

            Profile p{};
            // метод get_to НЕ бросает исключений, если j.is_discarded() == false
            if (!item.is_discarded()) {
                item.get_to(p);
                profiles.push_back(std::move(p));
            }
        }

        saveProfiles(profiles);
        ProfilesChangedEvent().post();

        // Update content
        drawContent(); });
}

void StagesPopup::onExport(CCObject *obj)
{
  const auto profiles = getProfiles();
  auto resourcesDir = geode::Mod::get()->getResourcesDir();

  // Формируем папку backup
  auto backupDir = resourcesDir / "backups";
  auto backupFile = backupDir / backup::generateBackupFilename();

  const auto res = geode::utils::file::createDirectory(backupDir);

  if (res)
  {
    json jProfiles = json::array();
    for (const auto &profile : profiles)
    {
      jProfiles.push_back(serializeProfile(profile));
    }

    auto result = geode::utils::file::writeString(backupFile, jProfiles.dump());
    if (result)
    {
      geode::utils::file::openFolder(backupFile);
    }
    else
    {
      geode::log::error("Unable to save JSON: {}", result.unwrapErr());
    }
  }
  else
  {
    geode::log::error("");
  };
}

void StagesPopup::onProfileSelect(CCObject *obj)
{
  const auto level = this->m_level;

  if (level)
  {
    const auto profiles = getProfiles();
    const auto index = obj->getTag();
    if (index >= profiles.size())
      return;

    const auto &profile = profiles[index];

    linkProfileWithLevel(profile, level);
    ProfilesChangedEvent().post();

    // Update content
    drawContent();
  }
}

void StagesPopup::onProfileDeselect(CCObject *obj)
{
  const auto level = this->m_level;

  if (level)
  {
    const auto profiles = getProfiles();
    const auto index = obj->getTag();
    if (index >= profiles.size())
      return;

    const auto &profile = profiles[index];

    unlinkProfileFromLevel(profile, level);
    ProfilesChangedEvent().post();

    // Update content
    drawContent();
  }
}