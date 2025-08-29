#pragma once

#include <Geode/Geode.hpp>

#include "../../../serialization/profile/index.hpp"

using namespace geode::prelude;

class RangeLabel : public CCNode
{
private:
  Range *m_range;
  CCLabelBMFont *m_label;
  CCLabelBMFont *m_noteLabel;
  bool enabled = false;

  const ccColor3B enabledColor = {99, 224, 110};
  const ccColor3B disabledColor = {253, 165, 106};

  void updateColor();

public:
  static RangeLabel *create(
      Range *range,
      const std::string &font = "gjFont17.fnt");

  bool init(Range *range, const std::string &font = "gjFont17.fnt");

  CCLabelBMFont *getLabel() const { return m_label; }

  void setRange(int from, int to);

  void setEnabled(bool value);
};