#pragma once
#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>

#include "../serialization/profile/index.hpp"

using namespace geode::prelude;

// stageIndex is the index among considered stages, totalStages is their count
class StageSwitchedEvent : public Event<StageSwitchedEvent, bool(int stageIndex, int totalStages, Stage *stage)>
{
public:
  using Event::Event;
};
