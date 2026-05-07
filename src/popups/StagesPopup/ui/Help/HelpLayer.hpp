#pragma once
#include <Geode/Geode.hpp>

#include "../../../../ui/types/index.hpp"
#include "../../../../ui/RectNode.hpp"

using namespace geode::prelude;

class HelpLayer : public CCLayer
{
private:
  CCSize m_size;
  ScrollLayer *m_scroll;

public:
  static HelpLayer *create(const CCSize &contentSize);
  bool init(const CCSize &contentSize);
};