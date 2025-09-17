#include "BlitzkriegProfile.hpp"

BlitzkriegProfile *BlitzkriegProfile::create(Profile const &profile,
                                             GJGameLevel *level,
                                             bool isCurrent,
                                             CCSize const &size)
{
  auto ret = new BlitzkriegProfile();
  if (ret && ret->init(profile, level, isCurrent, size))
  {
    ret->autorelease();
    return ret;
  }

  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool BlitzkriegProfile::init(Profile const &profile,
                             GJGameLevel *level,
                             bool isCurrent,
                             CCSize const &size)
{
  if (!CCLayer::init())
    return false;

  m_profile = profile;
  m_stats = getProfileStats(profile);
  m_isCurrent = isCurrent;
  m_level = level;
  m_size = size;
  m_isPinned = GlobalStore::get()->isProfilePinned(m_profile.id);

  this->setContentSize(size);

  createBackground();
  createLabels();
  createMenu();

  m_listener = EventListener<EventFilter<ProfileChangedEvent>>(
      [this](ProfileChangedEvent *)
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

  Profile current = GlobalStore::get()->getProfileByLevel(m_level);
  bool shouldBeCurrent = (!current.id.empty() && current.id == m_profile.id);

  if (m_isCurrent != shouldBeCurrent)
  {
    m_isCurrent = shouldBeCurrent;
    updateButtons();
    m_buttonMenu->updateLayout();
  }
}

void BlitzkriegProfile::createMenu()
{
  m_buttonMenu = CCMenu::create();
  m_buttonMenu->setAnchorPoint({1.f, 0.5f});
  m_buttonMenu->setPosition({m_size.width - 5.f, m_size.height / 2});
  m_buttonMenu->setLayout(
      RowLayout::create()
          ->setGap(2.5f)
          ->setAutoScale(false)
          ->setAutoGrowAxis(true)
          ->setAxisAlignment(AxisAlignment::End)
          ->setCrossAxisAlignment(AxisAlignment::Center));
  m_buttonMenu->getLayout()->ignoreInvisibleChildren(true);
  this->addChild(m_buttonMenu);

  updateButtons();
  m_buttonMenu->updateLayout();
}

void BlitzkriegProfile::updateButtons()
{
  m_buttonMenu->removeAllChildrenWithCleanup(true);

  // ! Select Button
  createButton(
      m_isCurrent ? "unlink-profile-btn.png"_spr : "link-profile-btn.png"_spr,
      menu_selector(BlitzkriegProfile::onToggleProfile));
  // ! Up Button
  createButton(
      "up-profile-btn.png"_spr,
      menu_selector(BlitzkriegProfile::onUpProfile));
  // ! Pin/Unpin Button
  createButton(
      m_isPinned ? "unpin-profile-btn.png"_spr : "pin-profile-btn.png"_spr,
      menu_selector(BlitzkriegProfile::onTogglePinProfile));
  // ! Delete Button
  createButton(
      "delete-profile-btn.png"_spr,
      menu_selector(BlitzkriegProfile::onDeleteProfile));
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
  std::string profileName = m_profile.profileName;
  if (profileName.length() > 20)
    profileName = profileName.substr(0, 20) + "...";

  auto nameLabel = CCLabelBMFont::create(profileName.c_str(), "bigFont.fnt");
  nameLabel->setScale(.5f);
  nameLabel->setAnchorPoint({0, 0.5f});
  nameLabel->setPosition({10.f, 25.f});

  if (m_stats.currentStage > m_stats.totalStages / 2)
    nameLabel->setColor({253, 165, 106});
  if (!m_stats.currentStage.has_value() || m_stats.currentStage >= m_stats.totalStages)
    nameLabel->setColor({99, 224, 110});

  this->addChild(nameLabel);

  std::string stagesText;
  if (!m_stats.currentStage.has_value() || m_stats.currentStage >= m_stats.totalStages)
  {
    stagesText = "Stages: " + geode::utils::numToString(m_stats.totalStages) + "/" +
                 geode::utils::numToString(m_stats.totalStages);
  }
  else
  {
    stagesText = "Stages: " + geode::utils::numToString(m_stats.currentStage.value()) + "/" +
                 geode::utils::numToString(m_stats.totalStages);
  }

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
  ProfileChangedEvent().post();
}

void BlitzkriegProfile::onTogglePinProfile(CCObject *obj)
{
  m_isPinned = !m_isPinned;
  GlobalStore::get()->pinProfileById(m_profile.id, m_isPinned);
  updateButtons();

  ProfilesChangedEvent().post();
}

void BlitzkriegProfile::onUpProfile(CCObject *obj)
{
  GlobalStore::get()->upProfileById(m_profile.id);
  ProfilesChangedEvent().post();
}

void BlitzkriegProfile::onDeleteProfile(CCObject *obj)
{
  geode::createQuickPopup(
      "Delete Profile",
      fmt::format("Are you sure you want to delete profile \"{}\"?", m_profile.profileName), // message text
      "Cancel",
      "Delete",
      [this](auto, bool confirmed)
      {
        if (confirmed)
        {
          GlobalStore::get()->removeProfileById(m_profile.id);
          this->removeFromParentAndCleanup(true);
          ProfilesChangedEvent().post();
        }
      });
}
