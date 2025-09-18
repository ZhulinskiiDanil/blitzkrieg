#pragma once
#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>
#include "RangeLabel.hpp"

#include "../../StagesChangedEvent.hpp"
#include "../../../../serialization/profile/index.hpp"
#include "../../../../store/GlobalStore.hpp"
#include "../../../../utils/getFirstUncheckedStage.hpp"

using namespace geode::prelude;

class StageRangeCell : public CCLayer
{
private:
  Range *m_range;
  RangeLabel *m_rangeLabel = nullptr;
  CCMenuItemToggler *m_checkbox = nullptr;
  CCScale9Sprite *m_background = nullptr;
  GJGameLevel *m_level = nullptr;
  bool m_checked = false;
  bool m_disabled = false;
  bool m_isCurrent = false;

  void onToggle(CCObject *sender);
  void updateBackgroundTexture();

public:
  static StageRangeCell *create(Range *range, GJGameLevel *level, const CCSize &cellSize);

  bool init(Range *range, GJGameLevel *level, const CCSize &cellSize);

  void setDisabled(bool disabled);
  bool isDisabled() const { return m_disabled; }
  bool isChecked() const { return m_checked; }
};
