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
  if (!Popup::init(320, 114, "GJ_square01_custom.png"_spr))
    return false;

  findStartPoses();

  m_profile = profile;
  m_level = level;

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
  if (!m_profile || !m_level)
    return;

  auto profileWithNewName = *m_profile;
  profileWithNewName.profileName = m_input->getString();

  auto regeneratedProfile = generateProfile("_mergeProfile", m_2_1_percentages);
  auto mergedProfile = mergeProfiles(profileWithNewName, regeneratedProfile.as<Profile>().unwrap(), true);

  // Save to test
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

  GlobalStore::get()->updateProfile(mergedProfile);

  matjson::Value jProfiles = mergedProfile;
  auto jsonString = jProfiles.dump(matjson::NO_INDENTATION);
  auto result = geode::utils::file::writeString(backupFile, jsonString);

  // if (result)
  //   geode::utils::file::openFolder(backupFile);
  // else
  //   geode::log::error("Unable to save JSON: {}", result.unwrapErr());

  ProfilesChangedEvent().send();
  this->onClose(nullptr);
}

void EditProfilePopup::findStartPoses()
{
  for (auto child : CCArrayExt<StartPosObject *>(PlayLayer::get()->m_objects))
  {
    if (auto startPos = typeinfo_cast<StartPosObject *>(child))
    {
      const float levelLength = PlayLayer::get()->m_levelLength;
      const float levelTime = PlayLayer::get()->timeForPos({levelLength, 0.f}, 0.f, 0.f, true, 0.f);

      const float startPosX = startPos->getPositionX();
      const float startPosPercentByPosX = (startPosX / levelLength) * 100.f;

      const float startPosTime = PlayLayer::get()->timeForPos({startPosX, 0.f}, 0.f, 0.f, true, 0.f);
      const float startPosPercentByTime = (startPosTime / levelTime) * 100.f;

      m_2_1_percentages.push_back(startPosPercentByPosX);
      m_2_2_percentages.push_back(startPosPercentByTime);
    }
  }

  std::sort(m_2_1_percentages.begin(), m_2_1_percentages.end());
  std::sort(m_2_2_percentages.begin(), m_2_2_percentages.end());
}