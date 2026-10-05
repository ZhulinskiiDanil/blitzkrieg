#pragma once
#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>

using namespace geode::prelude;

// An attempt was added to the session log, or the log was reset
class SessionChangedEvent : public Event<SessionChangedEvent, bool()>
{
  using Event::Event;
};
