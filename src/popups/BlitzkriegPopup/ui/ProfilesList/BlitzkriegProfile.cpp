#include "BlitzkriegProfile.hpp"

#include <algorithm>

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
  m_summary = getProfileSummary(m_profile);
  m_isCurrent = linkedProfile && linkedProfile->id == profile.id;
  m_level = level;
  m_size = size;
  m_isPinned = GlobalStore::get()->isProfilePinned(m_profile.id);

  this->setContentSize(size);

  // The menu goes first: labels and the progress bar fit into the space left of it
  createBackground();
  createMenu();
  createLabels();
  createProgressBar();

  return true;
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
  this->addChild(m_toolsMenu, 1);

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
  if (!m_isCurrent)
  {
    auto bg = CCScale9Sprite::create("square02b_small.png");
    bg->setContentSize(m_size);
    bg->setPosition(m_size.width / 2, m_size.height / 2);
    bg->setColor({0, 0, 0});
    bg->setOpacity(255 * 0.3f);
    this->addChild(bg);
    return;
  }

  // ! --- Linked profile: accent outline + accent-tinted fill --- !
  // The inner fill is opaque, so the outline does not show through it
  const float radius = 5.f;
  const float outline = 1.f;

  // Inset on the top, left and right, see SCROLL_CLIP_INSET
  const CCSize borderSize{
      m_size.width - SCROLL_CLIP_INSET * 2,
      m_size.height - SCROLL_CLIP_INSET,
  };

  auto border = RectNode::create(
      borderSize,
      ccc4FFromccc3B(ACCENT_COLOR),
      radius);
  border->setPosition({SCROLL_CLIP_INSET, 0.f});
  this->addChild(border);

  auto fill = RectNode::create(
      {borderSize.width - outline * 2, borderSize.height - outline * 2},
      ccc4FFromccc4B({57, 26, 36, 255}),
      radius - outline);
  fill->setPosition({SCROLL_CLIP_INSET + outline, outline});
  this->addChild(fill);
}

std::string BlitzkriegProfile::getDisplayName() const
{
  std::string name = m_profile.profileName;

  if (Mod::get()->getSettingValue<bool>("enable-streamer-mode") && !name.empty())
    name = name.substr(0, 1) + "...";

  return name;
}

float BlitzkriegProfile::getContentRight()
{
  float buttonsLeft = m_size.width;

  auto includeNode = [this, &buttonsLeft](CCNode *node)
  {
    const auto box = node->boundingBox();
    const auto world = node->getParent()->convertToWorldSpace(box.origin);
    buttonsLeft = std::min(buttonsLeft, this->convertToNodeSpace(world).x);
  };

  for (auto *child : CCArrayExt<CCNode *>(m_toolsMenu->getChildren()))
  {
    if (child != m_buttonMenu)
      includeNode(child);
  }

  for (auto *child : CCArrayExt<CCNode *>(m_buttonMenu->getChildren()))
    includeNode(child);

  // Never let labels collapse completely if the measurement goes wrong
  return std::max(buttonsLeft - CONTENT_TO_BUTTONS_GAP, m_size.width * .4f);
}

void BlitzkriegProfile::createLabels()
{
  const float contentRight = getContentRight();
  const float nameY = 27.f;

  // ! --- Name --- !
  auto nameLabel = CCLabelBMFont::create(getDisplayName().c_str(), "bigFont.fnt");
  nameLabel->setAnchorPoint({0, 0.5f});
  nameLabel->setPosition({PADDING_X, nameY});

  if (m_summary.isCompleted())
    nameLabel->setColor(COMPLETED_COLOR);

  this->addChild(nameLabel);

  // ! --- Linked badge --- !
  CCLabelBMFont *badge = nullptr;
  float badgeSpace = 0.f;

  if (m_isCurrent)
  {
    badge = CCLabelBMFont::create("LINKED", "bigFont.fnt");
    badge->setScale(.25f);
    badge->setColor(ACCENT_COLOR);
    badge->setAnchorPoint({0, 0.5f});
    badgeSpace = badge->getScaledContentWidth() + LINKED_BADGE_GAP;
  }

  // Long names shrink first and are cut only when that is not enough,
  // so the name never pushes the badge under the buttons
  fitLabelWidth(
      nameLabel,
      getDisplayName(),
      contentRight - PADDING_X - badgeSpace,
      NAME_SCALE,
      NAME_MIN_SCALE);

  if (badge)
  {
    badge->setPosition({PADDING_X + nameLabel->getScaledContentWidth() + LINKED_BADGE_GAP, nameY});
    this->addChild(badge);
  }

  // ! --- Info line: stage, attempts, time played --- !
  const std::string stageText =
      m_summary.totalStages > 0
          ? fmt::format("Stage {}/{}", m_summary.currentStage(), m_summary.totalStages)
          : std::string("<small>No stages</small>");

  const std::string info = fmt::format(
      "{}   {} <small>Attempts</small>   {}",
      stageText,
      m_summary.attempts,
      formatTimePlayed(m_summary.timePlayed));

  auto infoLabel = UILabel::create(info, "bigFont.fnt", .28f);
  infoLabel->ignoreAnchorPointForPosition(false);
  infoLabel->setAnchorPoint({0, 0.5f});
  infoLabel->setPosition({PADDING_X, 14.f});

  const float infoMaxWidth = contentRight - PADDING_X;
  if (infoLabel->getContentWidth() > infoMaxWidth)
    infoLabel->setScale(infoMaxWidth / infoLabel->getContentWidth());

  this->addChild(infoLabel);
}

void BlitzkriegProfile::createProgressBar()
{
  const float width = getContentRight() - PADDING_X;
  const float y = 4.f;

  if (width <= 0.f)
    return;

  auto track = RectNode::create(
      {width, PROGRESS_BAR_HEIGHT},
      ccc4FFromccc4B({70, 70, 70, 255}),
      PROGRESS_BAR_HEIGHT / 2);
  track->setPosition({PADDING_X, y});
  this->addChild(track);

  const float fillWidth = width * m_summary.progress();

  if (fillWidth <= 0.f)
    return;

  auto fill = RectNode::create(
      {fillWidth, PROGRESS_BAR_HEIGHT},
      ccc4FFromccc3B(m_summary.isCompleted() ? COMPLETED_COLOR : ACCENT_COLOR),
      PROGRESS_BAR_HEIGHT / 2);
  fill->setPosition({PADDING_X, y});
  this->addChild(fill);
}

// ! --- Handlers --- !
// Handlers only change the store and send events.
// ProfilesListLayer rebuilds rows on the next frame, so a row is never
// destroyed while its own button callback is running.
void BlitzkriegProfile::onToggleProfile(CCObject *obj)
{
  auto now = std::chrono::steady_clock::now();

  if (m_profileToggleDisabled || (now - m_lastToggleTime) < debounceDuration)
    return;

  m_lastToggleTime = now;
  // The row is rebuilt after the event, block repeated clicks until then
  m_profileToggleDisabled = true;

  if (m_isCurrent)
    unlinkProfileFromLevel(m_profile, m_level);
  else
    linkProfileWithLevel(m_profile, m_level);

  ProfileChangedEvent().send();
}

void BlitzkriegProfile::onTogglePinProfile(CCObject *obj)
{
  m_isPinned = !m_isPinned;
  GlobalStore::get()->pinProfileById(m_profile.id, m_isPinned);
  ProfilesChangedEvent().send();
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
  // Captured by value: the row may be rebuilt while the dialog is open
  const std::string profileId = m_profile.id;

  geode::createQuickPopup(
      "Delete Profile",
      fmt::format("Are you sure you want to delete profile \"{}\"?", getDisplayName()),
      "Cancel",
      "Delete",
      [profileId](auto, bool confirmed)
      {
        if (!confirmed)
          return;

        GlobalStore::get()->removeProfileById(profileId);
        ProfilesChangedEvent().send();
      });
}
