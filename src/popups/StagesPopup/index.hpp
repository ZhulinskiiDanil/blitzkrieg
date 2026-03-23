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

#include "./ui/StageList/StageListLayer.hpp"
#include "./ui/ProfilesList/ProfilesListLayer.hpp"
#include "./ui/StagesGraph/StagesGraphLayer.hpp"

#include "../../ui/types/index.hpp"
#include "../../ui/Include.hpp"
#include "../../store/GlobalStore.hpp"
#include "../../events/StageSwitchedEvent.hpp"
#include "../../events/StageRangesChangedEvent.hpp"
#include "../../serialization/profile/index.hpp"

#include "../../utils/getFirstUncheckedStage.hpp"
#include "../../utils/generateProfile.hpp"
#include "../../utils/formatTimePlayed.hpp"
#include "../../utils/getMetaInfoFromStages.hpp"

using namespace geode::prelude;

class StagesPopup : public geode::Popup
{
private:
  std::vector<TabButton *> tabButtons;
  std::vector<CCNode *> contentContainers;
  std::vector<CCMenuItemToggle *> stageCheckboxes;
  GJGameLevel *m_level;
  std::string m_levelId;
  bool m_isFirstLaunch = true;

  CCNode *m_profilesListNode;
  CCNode *m_currentStageNode;
  CCNode *m_currentStageGraphNode;
  CCLabelBMFont *m_currentStageTitleLabel;
  Label *m_totalStatLabel;
  StageListLayer *m_stageList;

  geode::comm::ListenerHandle m_stageChangedListener;
  geode::comm::ListenerHandle m_stageRangesChangedListener;

  void drawTabs();
  void drawContent();
  void drawProfilesList();
  void drawCurrentStage();
  void drawCurrentStageTitle(std::vector<Stage> &stages, Padding padding);
  void drawStagesGraph();

  bool init(GJGameLevel *);

  void onCurrentStageToggle(CCObject *);
  void onProfilesListToggle(CCObject *);
  void onSettingsButton(CCObject *);
  void activateTab(TabButton *btnToActivate);
  void onToggleSort(CCObject *sender);
  void onToggleVisability(CCObject *sender);

  ~StagesPopup()
  {
    m_stageChangedListener.destroy();
  }

public:
  static StagesPopup *create(GJGameLevel *);
};
