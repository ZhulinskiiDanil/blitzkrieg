#pragma once
#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>

using namespace geode::prelude;

class UpdateScrollLayoutEvent : public Event<UpdateScrollLayoutEvent, bool()>
{
  using Event::Event;
};
