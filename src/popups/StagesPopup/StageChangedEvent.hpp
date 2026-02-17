#pragma once
#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>

#include "../../serialization/profile/index.hpp"

using namespace geode::prelude;

class StageChangedEvent : public Event<StageChangedEvent, bool(int totalStages, Stage*)>
{
protected:
  Stage *m_currentStage;
  int m_totalStages;

public:
  using Event::Event;
  // StageChangedEvent(Stage *currentStage, int totalStages)
  // {
  //   m_currentStage = currentStage;
  //   m_totalStages = totalStages;
  // }

  Stage *getCurrentStage() { return m_currentStage; }
  int getTotalStages() { return m_totalStages; }
};
