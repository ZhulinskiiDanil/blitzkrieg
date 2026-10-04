#pragma once
#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

#include "StagePage.hpp"
#include "../../../../events/StageSwitchedEvent.hpp"
#include "../../../../events/StageRangesChangedEvent.hpp"

#include "../../../../ui/RectNode.hpp"
#include "../../../../utils/getMetaInfoFromStages.hpp"
#include "../../../../serialization/profile/index.hpp"
#include "../../../../store/GlobalStore.hpp"

using namespace geode::prelude;

class StageListLayer : public CCLayer
{
private:
  static constexpr float SLIDE_DURATION = .3f;
  // Stage dots are drawn under the list
  static constexpr float DOTS_OFFSET_Y = -12.f;

  CCSize m_contentSize;

  // Pages are clipped to the list while they slide
  CCClippingNode *m_clip = nullptr;
  // A ring of three pages: [0] previous, [1] current, [2] next.
  // After a slide the ring rotates and only the page that left is rebuilt.
  std::array<StagePage *, 3> m_pages{};
  // Expanded runs by range id, survives rebuilds and stage switches
  std::unordered_map<std::string, bool> m_expandedRanges;

  CCMenu *m_buttonMenuLeft = nullptr;
  CCMenu *m_buttonMenuRight = nullptr;
  CCLabelBMFont *m_keybindLabelLeft = nullptr;
  CCLabelBMFont *m_keybindLabelRight = nullptr;

  struct StageDot
  {
    CCMenuItemSpriteExtra *item = nullptr;
    CCNode *hitArea = nullptr;
    RectNode *rect = nullptr;
  };

  CCMenu *m_dotsMenu = nullptr;
  std::vector<StageDot> m_dots;

  ListenerHandle m_listenerStageRangesChanged;

  GJGameLevel *m_level = nullptr;

  // Stages are resolved from the store on every use,
  // so the list never renders from a stale copy
  std::string m_profileId;
  // Index among considered stages (stages with at least one considered range)
  int m_stageIndex = 0;

  StagePageOptions m_options;

  bool m_isSliding = false;
  int m_slideDirection = 0;
  bool m_reloadQueued = false;

  Profile *getProfile() const;
  std::vector<Stage *> getStages() const;
  // First stage that is not completed, -1 when every stage is completed
  int getProgressIndex() const;

  void createArrows();
  void createDots();
  void updateNavigation();

  void buildPage(StagePage *page, int stageIndex, bool keepScroll);
  void layoutPages();

  void switchStage(int index);
  void onSlideFinished();
  void sendStageSwitched();

  // Rebuilds on the next frame, never inside a cell callback
  void queueReload();

  void onPrevStage();
  void onNextStage();
  void onPrevStageBtn(CCObject *);
  void onNextStageBtn(CCObject *);
  void onStageDot(CCObject *);

public:
  static StageListLayer *create(GJGameLevel *level, const CCSize &contentSize);
  bool init(GJGameLevel *level, const CCSize &contentSize);

  // keepScroll keeps the distance from the top of each page
  void reload(bool keepScroll = false);
  void setSortBy(StageListSortBy);
  void setRunsVisabilityForCompleted(bool);

  Stage *getCurrentStage() const;
  int getStageIndex() const { return m_stageIndex; }
  int getStagesCount() const { return static_cast<int>(getStages().size()); }
};
