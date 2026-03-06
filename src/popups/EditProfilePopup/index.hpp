#pragma once

#include <Geode/Geode.hpp>

#include "../StagesPopup/ProfilesChangedEvent.hpp"
#include "../../store/GlobalStore.hpp"

#include "../../serialization/profile/index.hpp"

using namespace geode::prelude;

class EditProfilePopup : public geode::Popup
{
private:
  CCMenu *m_buttonMenu = nullptr;
  TextInput *m_input = nullptr;
  Profile *m_profile = nullptr;

  bool init(Profile *);

  void onSave(CCObject *);
  void onCancel(CCObject *);

public:
  static EditProfilePopup *create(Profile *);
};
