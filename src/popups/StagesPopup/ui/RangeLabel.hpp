#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class RangeLabel : public CCNode
{
private:
  CCLabelBMFont *m_label;
  bool enabled = false;

  const ccColor3B enabledColor = {99, 224, 110};
  const ccColor3B disabledColor = {253, 165, 106};

  void updateColor();

public:
  static RangeLabel *create(int from, int to, bool enabled, const std::string &font = "bigFont.fnt");

  bool init(int from, int to, const std::string &font = "bigFont.fnt");

  CCLabelBMFont *getLabel() const { return m_label; }

  void setRange(int from, int to);

  void setEnabled(bool value);
};