#pragma once

#include <Geode/Geode.hpp>

#include "../StagesPopup/ProfilesChangedEvent.hpp"
#include "../../store/GlobalStore.hpp"

#include "../../serialization/profile/index.hpp"
#include "../../utils/mergeProfiles.hpp"
#include "../../utils/generateProfile.hpp"
#include "../../utils/generateBackupFilename.hpp"
#include "../../utils/getAllStartposesFromProfile.hpp"
#include "../../utils/getDisabledStartposesFromStage.hpp"
#include "../../utils/findStartposesFromCurrentLevel.hpp"

#include "../../ui/ToggablePercentagesList.hpp"

using namespace geode::prelude;

class EditProfilePopup : public geode::Popup
{
private:
  CCMenu *m_buttonMenu = nullptr;
  TextInput *m_input = nullptr;
  Profile *m_profile = nullptr;
  GJGameLevel *m_level = nullptr;

  ToggablePercentagesList *m_percentagesList = nullptr;

  std::vector<float> m_2_1_percentages = {};
  std::vector<float> m_2_2_percentages = {};

  bool init(Profile *profile, GJGameLevel *level);

  void findStartPoses();

  void onSave(CCObject *);
  void onCancel(CCObject *);

public:
  static EditProfilePopup *create(Profile *profile, GJGameLevel *level);
};
