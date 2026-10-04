#pragma once

#include <string>
#include <vector>

#include <Geode/Geode.hpp>

#include "../../../../../serialization/profile/index.hpp"
#include "../../../../../store/GlobalStore.hpp"
#include "../../../../../utils/getMetaInfoFromStages.hpp"
#include "../../../../../utils/generateBackupFilename.hpp"
#include "../../../../../utils/filterProfileForExport.hpp"

using namespace geode::prelude;

class WhiteListExport : public geode::Popup
{
private:
  std::vector<Profile> m_profiles;

  // Parallel to m_profiles, index == CCMenuItemToggler tag
  std::vector<bool> m_selected;
  std::vector<CCMenuItemToggler *> m_profileCheckboxes;

  bool m_includeSecrets = false;
  bool m_includeProgression = true;
  bool m_includeAttempts = true;
  bool m_includeNotes = true;

  ScrollLayer *m_scroll = nullptr;
  Label *m_warningLabel = nullptr;

  void createProfilesList();
  void createTitleRow();
  void createOptionCheckboxes();
  void createWarning();
  void createBottomButtons();

  CCNode *createProfileCell(Profile const &profile, int index, float width);

  CCMenu *createOptionCheckbox(
      CCSize const &size,
      std::string const &label,
      bool checked,
      cocos2d::SEL_MenuHandler selector);

  void refreshProfileCheckboxes(bool selected);

  void onToggleProfile(CCObject *sender);
  void onSelectAll(CCObject *sender);
  void onDeselectAll(CCObject *sender);

  void onToggleSecrets(CCObject *sender);
  void onToggleProgression(CCObject *sender);
  void onToggleAttempts(CCObject *sender);
  void onToggleNotes(CCObject *sender);

  void onExport(CCObject *sender);
  void onCancel(CCObject *sender);

  bool init(std::vector<Profile> const &profiles);

public:
  static WhiteListExport *create(std::vector<Profile> const &profiles);
};
