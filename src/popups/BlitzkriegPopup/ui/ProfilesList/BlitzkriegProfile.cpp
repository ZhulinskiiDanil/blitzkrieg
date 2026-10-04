#include "BlitzkriegProfile.hpp"

BlitzkriegProfile *BlitzkriegProfile::create(Profile const &profile,
                                             GJGameLevel *level,
                                             CCSize const &size)
{
  auto ret = new BlitzkriegProfile();
  if (ret && ret->init(profile, level, size))
  {
    ret->autorelease();
    return ret;
  }

  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool BlitzkriegProfile::init(Profile const &profile,
                             GJGameLevel *level,
                             CCSize const &size)
{
  if (!CCLayer::init())
    return false;

  const auto linkedProfile = GlobalStore::get()->getProfileByLevel(level);

  m_profile = profile;
  m_stageMetaInfo = new StageMetaInfo(getMetaInfoFromStages(m_profile.data.stages));
  m_isCurrent = linkedProfile && linkedProfile->id == profile.id;
  m_level = level;
  m_size = size;
  m_isPinned = GlobalStore::get()->isProfilePinned(m_profile.id);

  this->setContentSize(size);

  createBackground();
  createLabels();
  createMenu();

  m_listener = ProfileChangedEvent().listen(
      [this]()
      {
        updateFromCurrentProfile();
        return ListenerResult::Propagate;
      });

  return true;
}

void BlitzkriegProfile::updateFromCurrentProfile()
{
  if (!m_level)
    return;

  Profile *current = GlobalStore::get()->getProfileByLevel(m_level);
  bool shouldBeCurrent = (current && current->id == m_profile.id);

  if (m_isCurrent != shouldBeCurrent)
  {
    m_isCurrent = shouldBeCurrent;
    updateButtons();
    m_buttonMenu->updateLayout();
  }
}

void BlitzkriegProfile::createMenu()
{
  m_toolsMenu = CCMenu::create();
  m_toolsMenu->setAnchorPoint({1.f, 0.5f});
  m_toolsMenu->setPosition({m_size.width - 5.f, m_size.height / 2});
  m_toolsMenu->setLayout(
      RowLayout::create()
          ->setGap(8.0f)
          ->setAutoScale(false)
          ->setAutoGrowAxis(true)
          ->setAxisAlignment(AxisAlignment::End)
          ->setCrossAxisAlignment(AxisAlignment::Center));
  this->addChild(m_toolsMenu);

  m_buttonMenu = CCMenu::create();
  m_buttonMenu->setAnchorPoint({1.0f, 0.5f});
  m_buttonMenu->setPosition({m_toolsMenu->getContentWidth(), m_toolsMenu->getContentHeight() / 2.0f});
  m_buttonMenu->setLayout(
      RowLayout::create()
          ->setGap(2.5f)
          ->setAutoScale(false)
          ->setAutoGrowAxis(true)
          ->setAxisAlignment(AxisAlignment::End)
          ->setCrossAxisAlignment(AxisAlignment::Center));
  // m_buttonMenu->getLayout()->ignoreInvisibleChildren(true); // not required since geode v5
  m_toolsMenu->addChild(m_buttonMenu);

  updateButtons();
  m_buttonMenu->updateLayout();
}

void BlitzkriegProfile::updateButtons()
{
  if (!m_buttonMenu)
    return;

  m_buttonMenu->removeAllChildrenWithCleanup(true);
  m_toolsMenu->removeAllChildrenWithCleanup(true);

  // ! Pin/Unpin Button
  auto pinnedSpr = CCSprite::createWithSpriteFrameName("pin.png"_spr);
  pinnedSpr->setColor({245, 174, 125});
  auto unpinnedSpr = CCSprite::createWithSpriteFrameName("pin.png"_spr);
  unpinnedSpr->setOpacity(128);
  auto pinBtn = CCMenuItemToggler::create(unpinnedSpr, pinnedSpr, this,
                                          menu_selector(BlitzkriegProfile::onTogglePinProfile));
  pinBtn->toggle(m_isPinned);
  pinBtn->ignoreAnchorPointForPosition(true);
  pinBtn->setScale(.5f);

  m_toolsMenu->addChild(pinBtn);
  m_toolsMenu->updateLayout();
  // ! Select Button
  createButton(
      m_isCurrent ? "unlink-profile-btn.png"_spr : "link-profile-btn.png"_spr,
      menu_selector(BlitzkriegProfile::onToggleProfile));
  // ! Up Button
  createButton(
      "up-profile-btn.png"_spr,
      menu_selector(BlitzkriegProfile::onUpProfile));
  // ! Edit Button
  createButton(
      "edit-profile-btn.png"_spr,
      menu_selector(BlitzkriegProfile::onEditProfile));
  // ! Delete Button
  createButton(
      "delete-profile-btn.png"_spr,
      menu_selector(BlitzkriegProfile::onDeleteProfile));

  m_toolsMenu->addChild(m_buttonMenu);
  m_toolsMenu->updateLayout();
}

void BlitzkriegProfile::createButton(
    const char *spriteFrameName,
    cocos2d::SEL_MenuHandler callback)
{
  auto spr = CCSprite::createWithSpriteFrameName(spriteFrameName);
  const auto btn = CCMenuItemSpriteExtra::create(spr, this, callback);
  btn->ignoreAnchorPointForPosition(true);
  btn->setScale(.75f);
  btn->m_baseScale = .75f;

  m_buttonMenu->addChild(btn);
  m_buttonMenu->updateLayout();
}

void BlitzkriegProfile::createBackground()
{
  auto bg = CCScale9Sprite::create("square02b_small.png");
  bg->setContentSize(m_size);
  bg->setPosition(m_size.width / 2, m_size.height / 2);
  bg->setColor({0, 0, 0});
  bg->setOpacity(255 * 0.3f);
  this->addChild(bg);
}

void BlitzkriegProfile::createLabels()
{
  bool streamerModEnabled = Mod::get()->getSettingValue<bool>("enable-streamer-mode");

  std::string profileName = m_profile.profileName;

  if (streamerModEnabled && !profileName.empty())
    profileName = profileName.substr(0, 1) + "...";

  if (profileName.length() > 20)
    profileName = profileName.substr(0, 20) + "...";

  auto nameLabel = CCLabelBMFont::create(profileName.c_str(), "bigFont.fnt");
  nameLabel->setScale(.5f);
  nameLabel->setAnchorPoint({0, 0.5f});
  nameLabel->setPosition({10.f, 25.f});

  int currentStageNumber = std::max(m_stageMetaInfo->completed, 1);

  if (currentStageNumber >= m_stageMetaInfo->total)
    nameLabel->setColor({99, 224, 110});

  this->addChild(nameLabel);

  std::string stagesText;
  if (currentStageNumber >= m_stageMetaInfo->total)
    stagesText = "Stages: " + geode::utils::numToString(m_stageMetaInfo->total) + "/" +
                 geode::utils::numToString(m_stageMetaInfo->total);
  else
    stagesText = "Stages: " + geode::utils::numToString(currentStageNumber) + "/" +
                 geode::utils::numToString(m_stageMetaInfo->total);

  auto stagesLabel = CCLabelBMFont::create(stagesText.c_str(), "bigFont.fnt");
  stagesLabel->setScale(0.25f);
  stagesLabel->setAnchorPoint({0, 0.5f});
  stagesLabel->setPosition({10.f, 10.f});
  stagesLabel->setOpacity(255 * 0.5f);
  this->addChild(stagesLabel);
}

// ! --- Handlers --- !
void BlitzkriegProfile::onToggleProfile(CCObject *obj)
{
  auto now = std::chrono::steady_clock::now();

  if (m_profileToggleDisabled || (now - m_lastToggleTime) < debounceDuration)
    return;

  m_lastToggleTime = now;

  if (m_isCurrent)
    unlinkProfileFromLevel(m_profile, m_level);
  else
    linkProfileWithLevel(m_profile, m_level);

  updateButtons();
  m_buttonMenu->updateLayout();
  ProfileChangedEvent().send();
}

void BlitzkriegProfile::onTogglePinProfile(CCObject *obj)
{
  m_isPinned = !m_isPinned;
  GlobalStore::get()->pinProfileById(m_profile.id, m_isPinned);
  ProfilesChangedEvent().send();

  updateButtons();
}

void BlitzkriegProfile::onUpProfile(CCObject *obj)
{
  GlobalStore::get()->upProfileById(m_profile.id);
  ProfilesChangedEvent().send();
}

void BlitzkriegProfile::onEditProfile(CCObject *obj)
{
  EditProfilePopup::create(&m_profile, m_level)->show();
}

void BlitzkriegProfile::onDeleteProfile(CCObject *obj)
{
  geode::createQuickPopup(
      "Delete Profile",
      fmt::format("Are you sure you want to delete profile \"{}\"?", m_profile.profileName),
      "Cancel",
      "Delete",
      [this](auto, bool confirmed)
      {
        if (confirmed)
        {
          GlobalStore::get()->removeProfileById(m_profile.id);
          this->removeFromParentAndCleanup(true);
          ProfilesChangedEvent().send();
        }
      });
}
