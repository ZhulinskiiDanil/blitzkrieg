#pragma once
#include <Geode/Geode.hpp>
#include "RangeLabel.hpp"

#include "../StagesChangedEvent.hpp"
#include "../../../serialization/profile/index.hpp"
#include "../../../utils/saveProfile.hpp"
#include "../../../utils/getProfileByLevel.hpp"
#include "../../../utils/getFirstUncheckedStage.hpp"

using namespace geode::prelude;

class StageRangeCell : public CCLayer
{
private:
  Range *m_range;
  RangeLabel *m_rangeLabel = nullptr;
  CCMenuItemToggler *m_checkbox = nullptr;
  CCScale9Sprite *m_background = nullptr;
  bool m_checked = false;
  GJGameLevel *m_level = nullptr; // <- добавили уровень

  void onToggle(CCObject *sender);

public:
  static StageRangeCell *create(Range *range, GJGameLevel *level, const CCSize &cellSize);

  bool init(Range *range, GJGameLevel *level, const CCSize &cellSize);

  bool isChecked() const { return m_checked; }
  void setChecked(bool checked);
};
