#include "index.hpp"

// StagesPopup* StagesPopup::create(CreateFolderDelegate* delegate) {
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

  Padding padding{50.f, 50.f, 20.f, 20.f}; // top, bottom, left, right

  const auto profiles = getProfiles();
  const auto currentProfile = getProfileByLevel(level);

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
    cell->setContentSize(cellSize); // cell size

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
    auto stageLabel = CCLabelBMFont::create(
        profileName.c_str(),
        "bigFont.fnt");
    stageLabel->setScale(0.6f);
    stageLabel->setAnchorPoint({0, 0.5f});
    stageLabel->setPosition({10.f, 25.f});
    if (stats.currentStage > stats.totalStages / 2)
    {
      stageLabel->setColor({253, 165, 106});
    }
    if (isAllStagesCompleted)
    {
      stageLabel->setColor({99, 224, 110});
    }
    cell->addChild(stageLabel);

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
      // "Selected" label
      auto selectedLabel = CCLabelBMFont::create(
          "Selected",
          "bigFont.fnt");
      selectedLabel->setScale(0.35f);
      selectedLabel->setAnchorPoint({1, 0.5f});
      selectedLabel->setPosition({scroll->getContentWidth() - 10.f,
                                  20.f});
      selectedLabel->setOpacity(255 * 0.5f); // полупрозрачный, как statLabel
      cell->addChild(selectedLabel);
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

  // Import button
  auto btnSprImport = ButtonSprite::create("Import");
  btnSprImport->setScale(1.f);
  btnSprImport->setAnchorPoint({0.f, 0.5f});

  auto btnImport = CCMenuItemSpriteExtra::create(
      btnSprImport,
      this,
      menu_selector(StagesPopup::onImport));

  // Export button
  auto btnSprExport = ButtonSprite::create("Export all");
  btnSprExport->setScale(1.f);
  btnSprExport->setAnchorPoint({0.f, 0.5f});

  auto btnExport = CCMenuItemSpriteExtra::create(
      btnSprExport,
      this,
      menu_selector(StagesPopup::onExport));

  auto btnMenu = CCMenu::create();
  btnMenu->addChild(btnImport);
  btnMenu->addChild(btnExport);
  btnMenu->alignItemsHorizontallyWithPadding(btnsGap);
  btnMenu->setPosition({mainSize.width / 2, 28.f});

  m_mainLayer->addChild(btnMenu);
  m_mainLayer->addChild(titleLabel);
  m_mainLayer->addChild(scroll);
  m_mainLayer->addChild(borders);

  return true;
}

void StagesPopup::onImport(CCObject *obj)
{
  onClose(nullptr);
  selectJsonFile([this](std::string jsonContent)
                 {
        if (jsonContent.empty()) {
            geode::log::debug("Файл не выбран или пуст");
            return;
        }

        try {
            json parsed = json::parse(jsonContent);

            if (!parsed.is_array()) {
                geode::log::debug("Ошибка: ожидается массив профилей");
                return;
            }

            std::vector<Profile> profiles = parsed.get<std::vector<Profile>>();
            geode::log::debug("Загружено {} профилей", profiles.size());

            saveProfiles(profiles);
        }
        catch (const json::parse_error& e) {
            geode::log::debug("Ошибка парсинга JSON: {}", e.what());
        }
        catch (const json::type_error& e) {
            geode::log::debug("Ошибка типов JSON: {}", e.what());
        }
        catch (const std::exception& e) {
            geode::log::debug("Другая ошибка: {}", e.what());
        } });
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

    geode::log::debug("LINK PROFILE \"{}\" WITH LEVEL \"{}\"", profile.profileName, level->m_levelName);

    linkProfileWithLevel(profile, level);
    onClose(nullptr);
  }
}