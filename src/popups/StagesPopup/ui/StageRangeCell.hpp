#pragma once
#include <Geode/Geode.hpp>
#include "RangeLabel.hpp"

#include "../StagesChangedEvent.hpp"
#include "../../../utils/saveProfile.hpp"
#include "../../../utils/getProfileByLevel.hpp"
#include "../../../utils/getFirstUncheckedStage.hpp"

using namespace geode::prelude;

class StageRangeCell : public CCLayer
{
private:
  int m_from;
  int m_to;

  RangeLabel *m_rangeLabel = nullptr;
  CCMenuItemToggler *m_checkbox = nullptr;
  CCScale9Sprite *m_background = nullptr;
  bool m_checked = false;
  GJGameLevel *m_level = nullptr; // <- добавили уровень

  void onToggle(CCObject *sender);

public:
  static StageRangeCell *create(int from, int to, bool checked, GJGameLevel *level, const CCSize &cellSize);

  bool init(int from, int to, bool checked, GJGameLevel *level, const CCSize &cellSize);

  bool isChecked() const { return m_checked; }
  void setChecked(bool checked);
};
