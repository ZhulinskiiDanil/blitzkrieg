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

  CC_SAFE_DELETE(ret);
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

  this->setContentSize(m_contentSize);

  float padding = 5.f;

  // ! --- ScrollLayer --- !
  m_scroll = ScrollLayer::create(contentSize);
  m_scroll->setContentSize({contentSize.width - padding * 2, contentSize.height - padding * 2});
  m_scroll->setPosition({padding, padding});

  m_scroll->m_contentLayer->setLayout(
      ColumnLayout::create()
          ->setGap(2.5f)
          ->setAxisReverse(true)
          ->setAxisAlignment(AxisAlignment::End)
          ->setAutoGrowAxis(m_scroll->getContentHeight()));

  this->addChild(m_scroll);

  // ! --- Borders --- !
  auto borders = ListBorders::create();
  borders->setSpriteFrames("list-top.png"_spr, "list-side.png"_spr, 2.f); // 2.1f
  borders->updateLayout();
  borders->setContentSize({contentSize.width, contentSize.height - 3});
  borders->setPosition({contentSize.width / 2, contentSize.height / 2 - .5f});
  borders->setAnchorPoint({0.5f, 0.5f});
  this->addChild(borders);

  for (auto child : CCArrayExt<CCNodeRGBA *>(borders->getChildren()))
  {
    child->setColor(ccc3(15, 15, 15));
  }

  // ! --- Bottom buttons --- !
  auto btnsGap = 5.f;

  auto btnSprCreate = ButtonSprite::create("Create");
  auto btnCreate = CCMenuItemSpriteExtra::create(
      btnSprCreate, this, menu_selector(ProfilesListLayer::onCreate));
  btnCreate->setScale(.75f);
  btnCreate->m_baseScale = .75f;
  btnCreate->ignoreAnchorPointForPosition(true);

  auto btnSprImport = ButtonSprite::create("Import");
  auto btnImport = CCMenuItemSpriteExtra::create(
      btnSprImport, this, menu_selector(ProfilesListLayer::onImport));
  btnImport->setScale(.75f);
  btnImport->m_baseScale = .75f;
  btnImport->ignoreAnchorPointForPosition(true);

  auto btnSprExport = ButtonSprite::create("Export");
  auto btnExport = CCMenuItemSpriteExtra::create(
      btnSprExport, this, menu_selector(ProfilesListLayer::onExport));
  btnExport->setScale(.75f);
  btnExport->m_baseScale = .75f;
  btnExport->ignoreAnchorPointForPosition(true);

  auto btnMenu = CCMenu::create();
  btnMenu->setLayout(
      RowLayout::create()
          ->setGap(btnsGap)
          ->setAutoScale(false)
          ->setAutoGrowAxis(true)
          ->setAxisAlignment(AxisAlignment::End)
          ->setCrossAxisAlignment(AxisAlignment::Center));
  btnMenu->getLayout()->ignoreInvisibleChildren(true);

  btnMenu->addChild(btnCreate);
  btnMenu->addChild(btnImport);
  btnMenu->addChild(btnExport);
  btnMenu->setAnchorPoint({0.f, .5f});
  btnMenu->setPosition({0.f, -20.f});

  this->addChild(btnMenu);
  btnMenu->updateLayout();

  m_listener = EventListener<EventFilter<ProfilesChangedEvent>>(
      [this](ProfilesChangedEvent *)
      {
        m_profiles = GlobalStore::get()->getProfiles();
        reload();

        return ListenerResult::Propagate;
      });

  reload();
  return true;
}

void ProfilesListLayer::reload()
{
  if (!m_level)
    return;

  m_scroll->m_contentLayer->removeAllChildrenWithCleanup(true);

  std::stable_sort(m_profiles.begin(), m_profiles.end(),
                   [](const Profile &a, const Profile &b)
                   {
                     return GlobalStore::get()->isProfilePinned(a.id) &&
                            !GlobalStore::get()->isProfilePinned(b.id);
                   });

  bool hasPinned = std::any_of(m_profiles.begin(), m_profiles.end(),
                               [](const Profile &p)
                               { return GlobalStore::get()->isProfilePinned(p.id); });

  bool hasDefault = std::any_of(m_profiles.begin(), m_profiles.end(),
                                [](const Profile &p)
                                { return !GlobalStore::get()->isProfilePinned(p.id); });

  bool headerPinnedAdded = false;
  bool headerOtherAdded = false;

  for (size_t i = 0; i < m_profiles.size(); ++i)
  {
    const auto &profile = m_profiles[i];
    bool isCurrentProfile = (!m_currentProfile.id.empty() && m_currentProfile.id == profile.id);
    bool pinned = GlobalStore::get()->isProfilePinned(profile.id);

    if (hasPinned && hasDefault)
    {
      if (pinned && !headerPinnedAdded)
      {
        drawSectionHeader("Pinned");
        headerPinnedAdded = true;
      }
      else if (!pinned && !headerOtherAdded)
      {
        drawSectionHeader("Other");
        headerOtherAdded = true;
      }
    }

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

void ProfilesListLayer::drawSectionHeader(const std::string &title)
{
  float height = 20.f;
  float padding = 4.f;

  auto header = CCNode::create();
  header->setLayout(ColumnLayout::create()
                        ->setGap(2.5f)
                        ->setAutoScale(false)
                        ->setAutoGrowAxis(true)
                        ->setAxisReverse(true)
                        ->setCrossAxisLineAlignment(AxisAlignment::Start));

  auto label = CCLabelBMFont::create(title.c_str(), "bigFont.fnt");
  label->setScale(0.6f);
  header->addChild(label);
  header->updateLayout();

  auto line = CCLayerColor::create({255, 255, 255, 50},
                                   m_scroll->m_contentLayer->getContentSize().width - padding * 2,
                                   1.f);
  header->addChild(line);
  header->updateLayout();

  m_scroll->m_contentLayer->addChild(header);
  header->updateLayout();
}

void ProfilesListLayer::scrollToTop()
{
  if (m_scroll)
    m_scroll->scrollToTop();
}

void ProfilesListLayer::onImport(CCObject *obj)
{
  selectJsonFile([this](std::string jsonContent)
                 {
        if (jsonContent.empty())
            return;

        auto res = matjson::parseAs<std::vector<Profile>>(jsonContent);
        if (res.isErr()) {
            geode::log::error("JSON parse error: {}", res.unwrapErr());
            return;
        }

        auto profiles = res.unwrap();

        if (profiles.empty() || !res.isOk()) {
            geode::log::error("Parsed JSON is empty or not a valid profiles array");
            return;
        }

        if (!res.isOk()) {
            geode::log::error("JSON Parse error at import: {}", res.unwrapErr());
            return;
        }

        GlobalStore::get()->addProfiles(profiles);
        ProfilesChangedEvent().post();

        // Update profiles list
        reload(); });
}

void ProfilesListLayer::onExport(CCObject *obj)
{
  const auto profiles = GlobalStore::get()->getProfiles();
  auto resourcesDir = geode::Mod::get()->getSaveDir();

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

void ProfilesListLayer::onCreate(CCObject *sender)
{
  if (!m_level)
    return;

  const auto createProfilePopup = CreateProfilePopup::create(m_level);
  createProfilePopup->show();
}