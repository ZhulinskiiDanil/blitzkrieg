#pragma once

#include <Geode/Geode.hpp>
#include "../../../../ui/Include.hpp"

using namespace geode::prelude;

class MetaLabel : public CCLayer
{
private:
  UILabel *m_label = nullptr;
  UILabel *m_value = nullptr;
  CCDrawNode *m_line = nullptr;
  float m_gap = 5.f;

public:
  static MetaLabel *create(float width, const std::string &label, const std::string &value);
  bool init(float width, const std::string &label, const std::string &value);
};