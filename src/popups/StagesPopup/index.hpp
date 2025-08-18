#pragma once

#include <string>

#include <cvolton.level-id-api/include/EditorIDs.hpp>
#include <nlohmann/json.hpp>

#include <Geode/Geode.hpp>
#include <Geode/ui/General.hpp>
#include <Geode/utils/file.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/ui/BasedButton.hpp>
#include <Geode/cocos/cocoa/CCGeometry.h>
#include <Geode/cocos/extensions/GUI/CCControlExtension/CCScale9Sprite.h>

#include "ProfilesChangedEvent.hpp"

#include <ctime>
#include <iomanip>
#include <sstream>

#include "../../serialization/profile/index.hpp"
#include "../../utils/getProfiles.hpp"
#include "../../utils/getProfileByLevel.hpp"
#include "../../utils/saveProfiles.hpp"
#include "../../utils/linkProfileWithLevel.hpp"
#include "../../utils/unlinkProfileFromLevel.hpp"
#include "../../utils/selectJsonFile.hpp"
#include "../../utils/getProfileStats.hpp"
#include "../../utils/generateBackupFilename.hpp"
#include "../../utils/getFirstUncheckedStage.hpp"

using namespace geode::prelude;
using json = nlohmann::json;

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
  GJGameLevel *m_level = nullptr;

  void drawTabs();
  void drawContent();
  void drawProfilesList();
  void drawCurrentStage();

  bool setup(GJGameLevel *) override;
  void onProfileSelect(CCObject *);
  void onProfileDeselect(CCObject *);
  void onCreate(CCObject *);
  void onImport(CCObject *);
  void onExport(CCObject *);

  void onCurrentStageToggle(CCObject *);
  void onProfilesListToggle(CCObject *);
  void activateTab(TabButton *btnToActivate);

public:
  // static StagesPopup* create(StagesPopupDelegate*);
  static StagesPopup *create(GJGameLevel *);
};