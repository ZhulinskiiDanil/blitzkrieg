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

  CCNode *m_profilesListNode;
  CCNode *m_currentStageNode;
  CCNode *m_currentStageGraphNode;
  CCNode *m_helpNode;
  CCLabelBMFont *m_currentStageTitleLabel;
  UILabel *m_totalStatLabel;
  StageListLayer *m_stageList;

  geode::comm::ListenerHandle m_stageChangedListener;
  geode::comm::ListenerHandle m_stageRangesChangedListener;

  void drawTabs();
  void layoutTabs();
  void drawContent();

  void drawProfilesList();
  void drawCurrentStage();
  void drawStagesGraph();
  void drawNewsSection();

  void drawCurrentStageTitle(std::vector<Stage> &stages, UIPadding padding);

  bool init(GJGameLevel *);

  void onTabButton(CCObject *);
  void onSettingsButton(CCObject *);
  void activateTab(NavTabButton *btnToActivate, bool animate = true);
  void onToggleSort(CCObject *sender);
  void onToggleVisability(CCObject *sender);

  ~BlitzkriegPopup()
  {
    m_stageChangedListener.destroy();
  }

public:
  static BlitzkriegPopup *create(GJGameLevel *);
};
