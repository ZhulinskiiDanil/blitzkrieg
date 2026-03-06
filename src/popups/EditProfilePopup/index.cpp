#include "index.hpp"

EditProfilePopup *EditProfilePopup::create(Profile *profile)
{
  EditProfilePopup *ret = new EditProfilePopup();

  if (ret->init(profile))
  {
    ret->autorelease();
    return ret;
  }

  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool EditProfilePopup::init(Profile *profile)
{
  if (!Popup::init(320, 114, "GJ_square01_custom.png"_spr))
    return false;

  m_profile = profile;

  setTitle("Edit Profile");

  // ! --- Input --- !
  m_input = TextInput::create(m_size.width - 20.f, "Profile name", "bigFont.fnt");
  m_input->getInputNode()->setMaxLabelLength(32);
  m_input->setAnchorPoint({0.f, 1.f});
  m_input->setPosition(10.f, m_size.height - 40.f);
  m_input->setString(profile->profileName, false);

  m_mainLayer->addChild(m_input);

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

  m_buttonMenu->addChild(btnSave);
  btnSave->ignoreAnchorPointForPosition(true);

  // ! --- Cancel Button --- !
  m_button2 = ButtonSprite::create("Cancel", 0, 0, "goldFont.fnt", "GJ_button_01.png", 0.0f, .8f);
  m_button2->setScale(.8f);

  auto btnCancel = CCMenuItemSpriteExtra::create(
      m_button2,
      this,
      menu_selector(EditProfilePopup::onCancel));

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

void EditProfilePopup::onCancel(CCObject *)
{
  this->onClose(nullptr);
}

void EditProfilePopup::onSave(CCObject *)
{
  if (!m_profile)
    return;

  auto newProfile = *m_profile;
  newProfile.profileName = m_input->getString();
  GlobalStore::get()->updateProfile(newProfile);

  ProfilesChangedEvent().send();

  this->onClose(nullptr);
}