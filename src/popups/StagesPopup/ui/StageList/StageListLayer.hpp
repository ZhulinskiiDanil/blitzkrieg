#pragma once
#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>
#include <vector>

#include "StageRangeCell.hpp"
#include "../../StageChangedEvent.hpp"
#include "../../StagesChangedEvent.hpp"
#include "../../../../serialization/profile/index.hpp"

using namespace geode::prelude;

class StageListLayer : public CCLayer
{
private:
  CCSize m_contentSize;
  ScrollLayer *m_scroll = nullptr;
  CCSprite *m_lockSpr = nullptr;

  CCMenu *m_buttonMenuLeft = nullptr;
  CCMenu *m_buttonMenuRight = nullptr;
  CCMenuItemSpriteExtra *m_buttonLeft = nullptr;
  CCMenuItemSpriteExtra *m_buttonRight = nullptr;

  EventListener<EventFilter<StagesChangedEvent>> m_listener;

  Profile m_profile;
  GJGameLevel *m_level = nullptr;
  std::vector<Stage> *m_stages = nullptr;
  Stage *m_uncheckedStage = nullptr;
  Stage *m_stage = nullptr;
  int m_currentIndex;

public:
  static StageListLayer *create(Stage *stage, GJGameLevel *level, const CCSize &contentSize);
  bool init(Stage *stage, GJGameLevel *level, const CCSize &contentSize);

  void reload();
  void drawArrows();

  void onPrevStage(CCObject *);
  void onNextStage(CCObject *);

  void scrollToTop();

  ScrollLayer *getScrollLayer() const { return m_scroll; }
};
