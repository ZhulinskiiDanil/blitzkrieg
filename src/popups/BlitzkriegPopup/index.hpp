#pragma once

#include <optional>
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
#include "./ui/StageList/StageHeader.hpp"
#include "./ui/Session/SessionLayer.hpp"
#include "./ui/Backups/BackupsLayer.hpp"
#include "./ui/Goals/GoalsLayer.hpp"
#include "./ui/ProfilesList/ProfilesListLayer.hpp"
#include "./ui/StagesGraph/StagesGraphLayer.hpp"
#include "./ui/News/NewsLayer.hpp"
#include "./ui/NavTabs/NavTabButton.hpp"

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

class BlitzkriegPopup : public geode::Popup
{
private:
  std::vector<NavTabButton *> tabButtons;
  std::vector<CCNode *> contentContainers;
  std::vector<CCMenuItemToggle *> stageCheckboxes;
  GJGameLevel *m_level;
  std::string m_levelId;
  bool m_isFirstLaunch = true;
  float m_tabButtonsGap = 0.f;

  CCNode *m_profilesListNode = nullptr;
  CCNode *m_currentStageNode = nullptr;
  CCNode *m_currentStageGraphNode = nullptr;
  CCNode *m_helpNode = nullptr;
  CCNode *m_sessionNode = nullptr;
  CCNode *m_goalsNode = nullptr;
  CCNode *m_backupsNode = nullptr;
  StageHeader *m_stageHeader = nullptr;
  StageListLayer *m_stageList = nullptr;
  // Stage to open when the Stage Browser is drawn, set by the Stage Graph
  std::optional<int> m_requestedStageIndex;
  // Run of that stage to scroll to, empty for the whole stage
  std::string m_requestedRangeId;

  geode::comm::ListenerHandle m_stageChangedListener;
  geode::comm::ListenerHandle m_stageRangesChangedListener;

  void drawTabs();
  void layoutTabs();
  void drawContent();

  void drawProfilesList();
  void drawCurrentStage();
  void drawStagesGraph();
  void drawSession();
  void drawGoals();
  void drawBackups();
  void drawNewsSection();

  // From the left edge to the filter buttons, right above the list
  void drawStageHeader(CCPoint const &origin, float width);
  // stageIndex is the index among considered stages
  void updateStageHeader(int stageIndex, int totalStages, Stage *stage);

  bool init(GJGameLevel *);

  void onTabButton(CCObject *);
  void onSettingsButton(CCObject *);
  void activateTab(NavTabButton *btnToActivate, bool animate = true);
  void onToggleSort(CCObject *sender);
  void onToggleVisability(CCObject *sender);
  void onOpenProfiles(CCObject *sender);
  void openStageInBrowser(int stageIndex, std::string const &rangeId = {});

  ~BlitzkriegPopup()
  {
    m_stageChangedListener.destroy();
  }

public:
  static BlitzkriegPopup *create(GJGameLevel *);
};
