#pragma once

#include <string>

#include <cvolton.level-id-api/include/EditorIDs.hpp>

#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/ui/General.hpp>
#include <Geode/utils/file.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/ui/BasedButton.hpp>
#include <Geode/cocos/cocoa/CCGeometry.h>
#include <Geode/cocos/extensions/GUI/CCControlExtension/CCScale9Sprite.h>

#include "./StageChangedEvent.hpp"

#include "./ui/StageList/StageListLayer.hpp"
#include "./ui/ProfilesList/ProfilesListLayer.hpp"

#include "../../ui/Include.hpp"
#include "../../serialization/profile/index.hpp"
#include "../../store/GlobalStore.hpp"
#include "../../utils/getFirstUncheckedStage.hpp"
#include "../../utils/generateProfile.hpp"
#include "../../utils/formatTimePlayed.hpp"

using namespace geode::prelude;

struct Padding
{
  float top;
  float bottom;
  float left;
  float right;
};

class StagesPopup : public geode::Popup//<GJGameLevel *>
{
private:
  std::vector<TabButton *> tabButtons;
  std::vector<CCNode *> contentContainers;
  std::vector<CCMenuItemToggle *> stageCheckboxes;
  GJGameLevel *m_level = nullptr;

  CCNode *m_currentStageNode = nullptr;
  CCNode *m_profilesListNode = nullptr;
  CCLabelBMFont *m_currentStageTitleLabel = nullptr;
  Label *m_totalStatLabel = nullptr;
  // EventListener<EventFilter<StageChangedEvent>> m_stageChangedListener;
  geode::comm::ListenerHandle m_stageChangedListener;

  void drawTabs();
  void drawContent();
  void drawProfilesList();
  void drawCurrentStage();
  void drawCurrentStageTitle(Stage *currentStage, int totalStages, Padding padding);
  void drawLastRuns();

  bool init(GJGameLevel *);

  void onCurrentStageToggle(CCObject *);
  void onProfilesListToggle(CCObject *);
  void activateTab(TabButton *btnToActivate);

public:
  // static StagesPopup* create(StagesPopupDelegate*);
  static StagesPopup *create(GJGameLevel *);
};
