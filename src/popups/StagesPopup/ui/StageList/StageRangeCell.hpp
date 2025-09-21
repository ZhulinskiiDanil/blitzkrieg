#pragma once
#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>
#include "../../../../ui/Include.hpp"

#include "../../StagesChangedEvent.hpp"
#include "../../UpdateScrollLayoutEvent.hpp"
#include "../../../../store/GlobalStore.hpp"
#include "../../../../serialization/profile/index.hpp"
#include "../../../../utils/getFirstUncheckedStage.hpp"

#include "MetaTable.hpp"

using namespace geode::prelude;

class StageRangeCell : public CCLayer
{
private:
  Range *m_range;
  Label *m_rangeLabel = nullptr;
  CCLayer *m_content = nullptr;
  CCLayer *m_metaContent = nullptr;
  CCLayer *m_head = nullptr;
  CCMenu *m_expandBtnMenu = nullptr;
  CCMenuItemToggler *m_checkbox = nullptr;
  CCScale9Sprite *m_background = nullptr;
  MetaTable *m_table = nullptr;
  GJGameLevel *m_level = nullptr;
  bool m_checked = false;
  bool m_disabled = false;
  bool m_isCurrent = false;
  bool m_isExpanded = false;

  void onToggle(CCObject *sender);
  void updateBackgroundTexture();
  void updateTextColors();
  void updateExpandButton();
  void updateMetaContent();
  void updateLayoutWrapper();
  void onExpand(CCObject *);

public:
  static StageRangeCell *create(Range *range, GJGameLevel *level, const CCSize &cellSize);

  bool init(Range *range, GJGameLevel *level, const CCSize &cellSize);

  void setDisabled(bool disabled);
  bool isDisabled() const { return m_disabled; }
  bool isChecked() const { return m_checked; }
};
