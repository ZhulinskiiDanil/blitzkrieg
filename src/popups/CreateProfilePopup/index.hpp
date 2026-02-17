#pragma once

#include <string>
#include <Geode/Geode.hpp>
#include <cvolton.level-id-api/include/EditorIDs.hpp>

#include "../StagesPopup/ProfilesChangedEvent.hpp"

#include "../../store/GlobalStore.hpp"
#include "../../utils/generateProfile.hpp"

using namespace geode::prelude;

class CreateProfilePopup : public geode::Popup
{
private:
  GJGameLevel *m_level = nullptr;

  ListBorders *m_borders1 = nullptr;
  ListBorders *m_borders2 = nullptr;
  CCMenu *m_buttonMenu = nullptr;
  TextInput *m_input = nullptr;

  std::vector<float>
      m_2_1_percentages = {};
  std::vector<float> m_2_2_percentages = {};

  CCMenuItemToggler *m_percentagesCheckbox = nullptr;
  bool m_percentagesChecked = false;

  CCMenuItemToggler *m_useCheckbox = nullptr;
  bool m_useChecked = false;

  CCMenuItemToggler *m_pinCheckbox = nullptr;
  bool m_pinChecked = false;

  void onCreateProfile(CCObject *);
  void onTogglePercentages(CCObject *);
  void onToggleUse(CCObject *);
  void onTogglePin(CCObject *);

  void findStartPoses();

  bool init(GJGameLevel *);

public:
  static CreateProfilePopup *create(GJGameLevel *);
};
