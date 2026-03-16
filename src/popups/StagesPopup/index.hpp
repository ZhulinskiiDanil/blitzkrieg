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

struct Padding
{
  float top;
  float bottom;
  float left;
  float right;
};

class StagesPopup : public geode::Popup
{
private:
  std::vector<TabButton *> tabButtons;
  std::vector<CCNode *> contentContainers;
  std::vector<CCMenuItemToggle *> stageCheckboxes;
  GJGameLevel *m_level = nullptr;
  std::string m_levelId;
  bool m_isFirstLaunch = true;

  CCNode *m_currentStageNode = nullptr;
  CCNode *m_profilesListNode = nullptr;
  CCLabelBMFont *m_currentStageTitleLabel = nullptr;
  Label *m_totalStatLabel = nullptr;

  geode::comm::ListenerHandle m_stageChangedListener;
  geode::comm::ListenerHandle m_stageRangesChangedListener;

  void drawTabs();
  void drawContent();
  void drawProfilesList();
  void drawCurrentStage();
  void drawCurrentStageTitle(std::vector<Stage> &stages, Padding padding);
  void drawLastRuns();

  bool init(GJGameLevel *);

  void onCurrentStageToggle(CCObject *);
  void onProfilesListToggle(CCObject *);
  void onSettingsButton(CCObject *);
  void activateTab(TabButton *btnToActivate);

  ~StagesPopup()
  {
    m_stageChangedListener.destroy();
  }

public:
  static StagesPopup *create(GJGameLevel *);
};
