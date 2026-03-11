#pragma once
#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>

using namespace geode::prelude;

class ProfileChangedEvent : public Event<ProfileChangedEvent, bool()>
{
  using Event::Event;
};
