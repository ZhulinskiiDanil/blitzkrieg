#include "ProfilesListLayer.hpp"

ProfilesListLayer *ProfilesListLayer::create(
    GJGameLevel *level,
    std::vector<Profile> const &profiles,
    Profile const &current,
    const CCSize &contentSize)
{
  auto ret = new ProfilesListLayer();
  if (ret && ret->init(level, profiles, current, contentSize))
  {
    ret->autorelease();
    return ret;
  }
  delete ret;
  return nullptr;
}

bool ProfilesListLayer::init(
    GJGameLevel *level,
    std::vector<Profile> const &profiles,
    Profile const &current,
    const CCSize &contentSize)
{
  if (!CCLayer::init())
    return false;

  m_contentSize = contentSize;
  m_profiles = profiles;
  m_currentProfile = current;
  m_level = level;

  // ! --- ScrollLayer --- !
  m_scroll = ScrollLayer::create(contentSize);
  m_scroll->setContentSize({contentSize.width - 16, contentSize.height - 16});
  m_scroll->setPosition({8, 8});

  m_scroll->m_contentLayer->setLayout(
      ColumnLayout::create()
          ->setGap(5.f)
          ->setAxisAlignment(AxisAlignment::End)
          ->setAutoGrowAxis(m_scroll->getContentHeight()));

  this->addChild(m_scroll);

  // ! --- Borders --- !
  auto borders = ListBorders::create();
  borders->setContentSize(contentSize);
  borders->setAnchorPoint({0.5f, 0.5f});
  borders->setPosition(contentSize / 2);
  this->addChild(borders);

  // ! --- Bottom buttons --- !
  auto btnsGap = 5.f;

  auto btnSprCreate = ButtonSprite::create("Create");
  btnSprCreate->setScale(1.f);
  btnSprCreate->setAnchorPoint({0.f, 0.5f});
  auto btnCreate = CCMenuItemSpriteExtra::create(
      btnSprCreate, this, menu_selector(ProfilesListLayer::onCreate));

  auto btnSprImport = ButtonSprite::create("Import");
  btnSprImport->setScale(1.f);
  btnSprImport->setAnchorPoint({0.f, 0.5f});
  auto btnImport = CCMenuItemSpriteExtra::create(
      btnSprImport, this, menu_selector(ProfilesListLayer::onImport));

  auto btnSprExport = ButtonSprite::create("Export");
  btnSprExport->setScale(1.f);
  btnSprExport->setAnchorPoint({0.f, 0.5f});
  auto btnExport = CCMenuItemSpriteExtra::create(
      btnSprExport, this, menu_selector(ProfilesListLayer::onExport));

  auto btnMenu = CCMenu::create();
  btnMenu->addChild(btnCreate);
  btnMenu->addChild(btnImport);
  btnMenu->addChild(btnExport);
  btnMenu->alignItemsHorizontallyWithPadding(btnsGap);
  btnMenu->setPosition({contentSize.width / 2, -20.f}); // под списком
  this->addChild(btnMenu);

  reload();

  m_listener = EventListener<EventFilter<ProfilesChangedEvent>>(
      [this](ProfilesChangedEvent *)
      {
        m_profiles = getProfiles();
        return ListenerResult::Propagate;
      });

  return true;
}

void ProfilesListLayer::reload()
{
  if (!m_level)
    return;

  m_scroll->m_contentLayer->removeAllChildrenWithCleanup(true);

  for (size_t i = 0; i < m_profiles.size(); ++i)
  {
    const auto &profile = m_profiles[i];
    bool isCurrentProfile = (!m_currentProfile.id.empty() && m_currentProfile.id == profile.id);

    auto profileItem = BlitzkriegProfile::create(
        profile,
        m_level,
        isCurrentProfile,
        CCSize(m_scroll->getContentWidth(), 40.f));

    if (profileItem)
    {
      m_scroll->m_contentLayer->addChild(profileItem);
    }
  }

  m_scroll->m_contentLayer->updateLayout();
  scrollToTop();
}

void ProfilesListLayer::scrollToTop()
{
  if (m_scroll)
    m_scroll->scrollToTop();
}

void ProfilesListLayer::onCreate(CCObject *obj)
{
  geode::utils::web::openLinkInBrowser("https://dgkr-community.vercel.app/blitzkrieg?helpCreateProfile=true");
}

void ProfilesListLayer::onImport(CCObject *obj)
{
  selectJsonFile([this](std::string jsonContent)
                 {
        if (jsonContent.empty()) {
            geode::log::debug("File not selected or empty");
            return;
        }

        auto res = matjson::parseAs<std::vector<Profile>>(jsonContent);
        if (res.isErr()) {
            geode::log::debug("JSON parse error: {}", res.unwrapErr());
            return;
        }

        auto profiles = res.unwrap();

        if (profiles.empty() || !res.isOk()) {
            geode::log::debug("Parsed JSON is empty or not a valid profiles array");
            return;
        }

        if (!res.isOk()) {
            geode::log::debug("JSON Parse error at import: {}", res.unwrapErr());
            return;
        }

        saveProfiles(profiles);
        ProfilesChangedEvent().post();

        // Update profiles list
        reload(); });
}

void ProfilesListLayer::onExport(CCObject *obj)
{
  const auto profiles = getProfiles();
  auto resourcesDir = geode::Mod::get()->getResourcesDir();

  // Create Backup folder
  auto backupDir = resourcesDir / "backups";
  auto backupFile = backupDir / backup::generateBackupFilename();

  const auto res = geode::utils::file::createDirectory(backupDir);

  if (!res)
  {
    geode::log::error("Unable to create backup directory: {}", res.unwrapErr());
    return;
  }

  matjson::Value jProfiles = profiles;
  auto jsonString = jProfiles.dump(matjson::NO_INDENTATION);
  auto result = geode::utils::file::writeString(backupFile, jsonString);

  if (result)
    geode::utils::file::openFolder(backupFile);
  else
    geode::log::error("Unable to save JSON: {}", result.unwrapErr());
}