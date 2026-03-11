#pragma once
#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>

#include "../serialization/profile/index.hpp"

using namespace geode::prelude;

class StageSwitchedEvent : public Event<StageSwitchedEvent, bool(int totalStages, Stage *currentStage)>
{
protected:
  int m_totalStages;
  Stage *m_currentStage;

public:
  using Event::Event;
};
