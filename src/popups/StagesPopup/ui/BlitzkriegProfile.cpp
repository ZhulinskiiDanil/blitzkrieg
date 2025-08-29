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

  this->setContentSize(size);

  createBackground();
  createLabels();
  createButton();

  m_listener = EventListener<EventFilter<ProfilesChangedEvent>>(
      [this](ProfilesChangedEvent *)
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

  Profile current = getProfileByLevel(m_level);

  bool shouldBeCurrent = (!current.id.empty() && current.id == m_profile.id);
  if (m_isCurrent != shouldBeCurrent)
  {
    m_isCurrent = shouldBeCurrent;
    updateButton();
  }
}

void BlitzkriegProfile::updateButton()
{
  m_buttonMenu->removeAllChildren();
  CCMenuItemSpriteExtra *btn;
  if (!m_isCurrent)
  {
    auto spr = ButtonSprite::create("Select");
    spr->setScale(0.6f);
    btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(BlitzkriegProfile::onProfileSelect));
  }
  else
  {
    auto spr = ButtonSprite::create("Deselect");
    spr->setScale(0.6f);
    btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(BlitzkriegProfile::onProfileDeselect));
  }
  m_buttonMenu->addChild(btn);
  m_buttonMenu->setPosition({m_size.width - btn->getContentWidth() / 2 - 10.f, m_size.height / 2});
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

void BlitzkriegProfile::createButton()
{
  m_buttonMenu = CCMenu::create();
  this->addChild(m_buttonMenu);
  updateButton();
}

// ! --- Handlers --- !
void BlitzkriegProfile::onProfileSelect(CCObject *obj)
{
  linkProfileWithLevel(m_profile, m_level);
  ProfilesChangedEvent().post();

  m_isCurrent = true;
  updateButton();
}

void BlitzkriegProfile::onProfileDeselect(CCObject *obj)
{
  if (!m_level)
    return;

  unlinkProfileFromLevel(m_profile, m_level);
  ProfilesChangedEvent().post();

  m_isCurrent = false;
  updateButton();
}