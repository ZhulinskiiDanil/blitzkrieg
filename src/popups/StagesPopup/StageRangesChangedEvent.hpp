#pragma once
#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>

using namespace geode::prelude;

class StageRangesChangedEvent : public Event<StageRangesChangedEvent, bool()>
{
  using Event::Event;
};
