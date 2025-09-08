#pragma once

#include <string>
#include <Geode/Geode.hpp>
#include <cvolton.level-id-api/include/EditorIDs.hpp>

#include "../StagesPopup/ProfilesChangedEvent.hpp"

#include "../../utils/saveProfile.hpp"
#include "../../utils/generateProfile.hpp"

using namespace geode::prelude;

class CreateProfilePopup : public geode::Popup<GJGameLevel *>
{
private:
  GJGameLevel *m_level = nullptr;
  CCMenuItemToggler *m_checkbox = nullptr;

  ListBorders *m_borders1 = nullptr;
  ListBorders *m_borders2 = nullptr;
  CCMenu *m_buttonMenu = nullptr;
  TextInput *m_input = nullptr;

  std::vector<float>
      m_2_1_percentages = {};
  std::vector<float> m_2_2_percentages = {};
  bool m_checked = false;

  void onCreateProfile(CCObject *);
  void onToggle(CCObject *);

  void findStartPoses();

  bool setup(GJGameLevel *) override;

public:
  static CreateProfilePopup *create(GJGameLevel *);
};