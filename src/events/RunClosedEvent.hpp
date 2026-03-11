#pragma once
#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>

#include "../serialization/profile/index.hpp"

using namespace geode::prelude;

class RunClosedEvent : public Event<RunClosedEvent, bool(float from, float to, Profile *profile, Range *closedRange, Stage *closedStage)>
{
protected:
  float m_from;
  float m_to;
  Profile *m_profile = nullptr;
  Range *m_closedRange = nullptr;
  Stage *m_closedStage = nullptr;

public:
  using Event::Event;
};
