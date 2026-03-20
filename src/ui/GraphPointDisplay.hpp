#pragma once

#include <Geode/Geode.hpp>

#include "./Label.hpp"
#include "./RectNode.hpp"

using namespace geode::prelude;

class GraphPointDisplay : public CCLayer
{
private:
  Label *m_label = nullptr;

public:
  static GraphPointDisplay *create(std::string text);
  bool init(std::string text);

  void setText(const std::string &text);
};