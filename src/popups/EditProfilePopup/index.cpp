#include "index.hpp"

EditProfilePopup *EditProfilePopup::create(Profile *profile, GJGameLevel *level)
{
  EditProfilePopup *ret = new EditProfilePopup();

  if (ret->init(profile, level))
  {
    ret->autorelease();
    return ret;
  }

  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool EditProfilePopup::init(Profile *profile, GJGameLevel *level)
{
  if (!Popup::init(320, 266, "GJ_square01_custom.png"_spr))
    return false;

  findStartPoses();

  m_level = level;
  m_profile = profile;
  m_webhookEnabled = profile->discordWebhookForRunNotificationsEnabled;

  setTitle("Edit Profile");

  // ! --- Profile Name Input --- !
  m_profileNameInput = TextInput::create(m_size.width - 20.f, "Profile name", "bigFont.fnt");
  m_profileNameInput->getInputNode()->setMaxLabelLength(32);
  m_profileNameInput->setAnchorPoint({0.f, 1.f});
  m_profileNameInput->setPosition(10.f, m_size.height - 30.f - 10.f);
  m_profileNameInput->setString(profile->profileName, false);

  m_mainLayer->addChild(m_profileNameInput);

  // ! --- Discord Webhook Input --- !
  m_discordWebhookInput = TextInput::create(m_size.width - 20.f, "Discord webhook URL", "bigFont.fnt");
  m_discordWebhookInput->getInputNode()->setMaxLabelLength(128);
  m_discordWebhookInput->setFilter("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_/:.");
  m_discordWebhookInput->setAnchorPoint({0, 1});
  m_discordWebhookInput->setPosition(10.f, m_profileNameInput->getPositionY() - 30.f - 10.f);
  m_discordWebhookInput->setString(profile->discordWebhookForRunNotifications, false);

  m_mainLayer->addChild(m_discordWebhookInput);

  // ! --- Use Checkbox --- !
  m_useWebhookCheckbox = CCMenuItemToggler::createWithStandardSprites(
      this, menu_selector(EditProfilePopup::onToggleWebhookEnabled), 1.0f);
  m_useWebhookCheckbox->setScale(0.5f);
  m_useWebhookCheckbox->ignoreAnchorPointForPosition(true);
  m_useWebhookCheckbox->setContentSize({32.f, 32.f});
  m_useWebhookCheckbox->toggle(m_webhookEnabled);

  // ! --- Checkbox Text --- !
  auto useCheckboxLabel = CCLabelBMFont::create("Enable Discord Notifications", "bigFont.fnt");
  useCheckboxLabel->setScale(0.4f);
  useCheckboxLabel->setAnchorPoint({0, .5f});

  // ! --- Checkbox Menu --- !
  auto useCheckboxMenu = CCMenu::createWithItem(m_useWebhookCheckbox);
  useCheckboxMenu->setAnchorPoint({0, 1});
  useCheckboxMenu->ignoreAnchorPointForPosition(false);
  useCheckboxMenu->setPosition(10.f, m_discordWebhookInput->getPositionY() - 30.f - 4.f);
  useCheckboxMenu->setLayout(RowLayout::create()
                                 ->setGap(2.0f)
                                 ->setAutoGrowAxis(true)
                                 ->setAutoScale(false));

  useCheckboxMenu->addChild(useCheckboxLabel);
  m_mainLayer->addChild(useCheckboxMenu);
  useCheckboxMenu->updateLayout();

  // ! --- Percentages List --- !
  m_percentagesList = ToggablePercentagesList::create(
      {m_mainLayer->getContentWidth() - 24.0f, 80.0f},
      getAllStartposesFromProfile(*profile),
      getDisabledStartposesFromStage(profile->data.stages[0]));
  m_percentagesList->setAnchorPoint({.5f, 1});
  m_percentagesList->setPosition(m_mainLayer->getContentWidth() / 2, useCheckboxMenu->getPositionY() - 16.f - 10.f);
  m_mainLayer->addChild(m_percentagesList);

  // ! --- Buttons Menu --- !
  m_buttonMenu = CCMenu::create();
  m_buttonMenu->setAnchorPoint({.0f, 0.f});
  m_buttonMenu->setPosition({10.f, 10.f});

  // ! --- Save Button --- !
  m_button1 = ButtonSprite::create("Save", 0, 0, "goldFont.fnt", "GJ_button_01.png", 0.0f, .8f);
  m_button1->setScale(.8f);

  auto btnSave = CCMenuItemSpriteExtra::create(
      m_button1,
      this,
      menu_selector(EditProfilePopup::onSave));
  btnSave->m_baseScale = .8f;

  m_buttonMenu->addChild(btnSave);
  btnSave->ignoreAnchorPointForPosition(true);

  // ! --- Cancel Button --- !
  m_button2 = ButtonSprite::create("Cancel", 0, 0, "goldFont.fnt", "GJ_button_01.png", 0.0f, .8f);
  m_button2->setScale(.8f);

  auto btnCancel = CCMenuItemSpriteExtra::create(
      m_button2,
      this,
      menu_selector(EditProfilePopup::onCancel));
  btnCancel->m_baseScale = .8f;

  m_buttonMenu->addChild(btnCancel);
  btnCancel->ignoreAnchorPointForPosition(true);

  // ! --- Button Menu Layout --- !
  m_buttonMenu->setLayout(
      RowLayout::create()
          ->setGap(5.f)
          ->setAutoScale(false)
          ->setAutoGrowAxis(true)
          ->setAxisAlignment(AxisAlignment::Center)
          ->setCrossAxisAlignment(AxisAlignment::Center));

  m_mainLayer->addChild(m_buttonMenu);
  m_buttonMenu->updateLayout();

  return true;
}

void EditProfilePopup::onToggleWebhookEnabled(CCObject *)
{
  if (m_useWebhookCheckbox)
    m_webhookEnabled = !m_webhookEnabled;
}

void EditProfilePopup::onCancel(CCObject *)
{
  this->onClose(nullptr);
}

void EditProfilePopup::onSave(CCObject *)
{
  if (!m_profile || !m_level)
    return;

  auto regeneratedProfile = generateProfile("_mergeProfile", m_percentagesList->getEnabledStartposes());
  auto mergedProfile = mergeProfiles(*m_profile, regeneratedProfile.as<Profile>().unwrap(), true);

  mergedProfile.profileName = m_profileNameInput->getString();
  mergedProfile.discordWebhookForRunNotifications = m_discordWebhookInput->getString();
  mergedProfile.discordWebhookForRunNotificationsEnabled = m_webhookEnabled;

  GlobalStore::get()->updateProfile(mergedProfile);

  ProfilesChangedEvent().send();
  this->onClose(nullptr);
}

void EditProfilePopup::findStartPoses()
{
  auto startposes = findStartposesFromCurrentLevel();
  m_2_1_percentages = startposes.percentages_2_1;
  m_2_2_percentages = startposes.percentages_2_2;
}