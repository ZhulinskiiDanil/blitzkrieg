#pragma once

#include <Geode/Geode.hpp>
#include <unordered_map>

#include "./RectNode.hpp"
#include "./GraphPointDisplay.hpp"
#include "../utils/formatTimePlayed.hpp"
#include "./types/index.hpp"

using namespace geode::prelude;

class GraphPointDisplayList : public CCLayer
{
private:
  RectNode *m_bg;
  CCLayer *m_container;
  Padding m_padding;

  std::unordered_map<std::string, CCNode *> m_items;

public:
  static GraphPointDisplayList *create();
  bool init();

  void updatePointText(const std::string &text, const std::string &pointUUID, bool enabled);
};