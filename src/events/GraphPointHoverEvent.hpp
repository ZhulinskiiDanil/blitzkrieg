#pragma once
#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>

using namespace geode::prelude;

class GraphPointHoverEvent : public Event<GraphPointHoverEvent, bool(std::string graphUUID, std::string lineUUID, std::string pointUUID, bool hovered, float val, float x, float y)>
{
protected:
  std::string graphUUID;
  std::string lineUUID;
  std::string pointUUID;
  bool hovered;
  float val;
  float x;
  float y;

public:
  using Event::Event;
};
