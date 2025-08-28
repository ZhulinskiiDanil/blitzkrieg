#pragma once
#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>

#include "../StagesChangedEvent.hpp"
#include "StageRangeCell.hpp"

using namespace geode::prelude;

class StageRangesList : public CCLayer
{
private:
  ScrollLayer *m_scroll = nullptr;
  CCSize m_contentSize;
  GJGameLevel *m_level = nullptr;
  EventListener<EventFilter<StagesChangedEvent>> m_listener;
  Stage *m_stage = nullptr;

public:
  static StageRangesList *create(Stage *stage, GJGameLevel *level, const CCSize &contentSize);
  bool init(Stage *stage, GJGameLevel *level, const CCSize &contentSize);

  void reload(); // обновить список
  void scrollToTop();
  ScrollLayer *getScrollLayer() const { return m_scroll; }
};
