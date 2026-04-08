#pragma once
#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>
#include <vector>

#include "StageRangeCell.hpp"
#include "../../../../events/StageSwitchedEvent.hpp"
#include "../../../../events/StagesChangedEvent.hpp"
#include "../../../../events/StageRangesChangedEvent.hpp"
#include "../../../../events/UpdateScrollLayoutEvent.hpp"

#include "../../../../ui/RectNode.hpp"
#include "../../../../utils/getFirstUncheckedStage.hpp"
#include "../../../../utils/getMetaInfoFromStages.hpp"
#include "../../../../serialization/profile/index.hpp"
#include "../../../../store/GlobalStore.hpp"

using namespace geode::prelude;

enum StageListSortBy
{
  ASC,
  DESC
};

class StageListLayer : public CCLayer
{
private:
  CCSize m_contentSize;
  CCSprite *m_lockSpr;
  ScrollLayer *m_scroll;
  CCLayer *m_content;

  CCMenu *m_buttonMenuLeft;
  CCMenu *m_buttonMenuRight;
  CCMenuItemSpriteExtra *m_buttonLeft;
  CCMenuItemSpriteExtra *m_buttonRight;

  // EventListener<EventFilter<StagesChangedEvent>>
  ListenerHandle m_listener;
  // EventListener<EventFilter<UpdateScrollLayoutEvent>>
  ListenerHandle m_listenerUpdateScrollLayout;
  ListenerHandle m_listenerStageRangesChanged;

  GJGameLevel *m_level;
  Profile *m_profile;
  std::vector<Stage> *m_stages;
  Stage *m_stage;
  Stage *m_uncheckedStage;
  int m_currentIndex;

  // Sort / Filters
  StageListSortBy m_sortBy = StageListSortBy::ASC;
  bool m_hideCompletedRuns = false;

  void onPrevStage();
  void onNextStage();
  void onPrevStageBtn(CCObject *);
  void onNextStageBtn(CCObject *);

public:
  static StageListLayer *create(Stage *stage, GJGameLevel *level, const CCSize &contentSize);
  bool init(Stage *stage, GJGameLevel *level, const CCSize &contentSize);

  void reload();
  void drawArrows();
  void setSortBy(StageListSortBy);
  void setRunsVisabilityForCompleted(bool);

  void scrollToTop();

  ScrollLayer *getScrollLayer() const { return m_scroll; }
};
