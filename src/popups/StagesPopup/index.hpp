#pragma once

#include <string>

#include <cvolton.level-id-api/include/EditorIDs.hpp>

#include <Geode/Geode.hpp>
#include <Geode/ui/General.hpp>
#include <Geode/utils/file.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/ui/BasedButton.hpp>
#include <Geode/cocos/cocoa/CCGeometry.h>
#include <Geode/cocos/extensions/GUI/CCControlExtension/CCScale9Sprite.h>

#include "./ui/StageRangesList.hpp"
#include "./ui/ProfilesListLayer.hpp"

#include "../../serialization/profile/index.hpp"
#include "../../utils/getProfiles.hpp"
#include "../../utils/getProfileByLevel.hpp"
#include "../../utils/getFirstUncheckedStage.hpp"

using namespace geode::prelude;

struct Padding
{
  float top;
  float bottom;
  float left;
  float right;
};

// class StagesPopupDelegate {
// public:
//   virtual void onFolderCreate(std::string);
// };

class StagesPopup : public geode::Popup<GJGameLevel *>
{
private:
  std::vector<TabButton *> tabButtons;
  std::vector<CCNode *> contentContainers;
  std::vector<CCMenuItemToggle *> stageCheckboxes;
  GJGameLevel *m_level = nullptr;

  void drawTabs();
  void drawContent();
  void drawProfilesList();
  void drawCurrentStage();
  void drawLastRuns();

  bool setup(GJGameLevel *) override;

  void onCurrentStageToggle(CCObject *);
  void onProfilesListToggle(CCObject *);
  void activateTab(TabButton *btnToActivate);

public:
  // static StagesPopup* create(StagesPopupDelegate*);
  static StagesPopup *create(GJGameLevel *);
};