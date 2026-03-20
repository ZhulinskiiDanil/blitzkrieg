#pragma once
#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>
#include <fmt/core.h>
#include <string>

#include "../../../../ui/Include.hpp"
#include "../../../../ui/RectNode.hpp"
#include "../../../../events/StagesChangedEvent.hpp"
#include "../../../../events/StageRangesChangedEvent.hpp"
#include "../../../../events/UpdateScrollLayoutEvent.hpp"
#include "../../../../store/GlobalStore.hpp"
#include "../../../../serialization/profile/index.hpp"
#include "../../../../utils/getFirstUncheckedStage.hpp"
#include "../../../../utils/formatTimePlayed.hpp"
#include "../../../../helpers/CustomActions.hpp"

#include "MetaTable.hpp"

using namespace geode::prelude;

class StageRangeCell : public CCLayer
{
private:
  CCSize m_size;
  RectNode *m_lineBg;
  RectNode *m_midBg;
  RectNode *m_mid;
  Label *m_rangeLabel = nullptr;
  CCLayer *m_content = nullptr;
  CCLayer *m_metaContent = nullptr;
  CCLayer *m_head = nullptr;
  CCMenu *m_expandBtnMenu = nullptr;
  CCMenuItemToggler *m_checkbox = nullptr;
  CCScale9Sprite *m_background = nullptr;
  MetaTable *m_table = nullptr;

  GJGameLevel *m_level = nullptr;
  Range *m_range;
  std::string m_id;
  float m_from;
  float m_to;
  
  bool m_checked = false;
  bool m_disabled = false;
  bool m_isCurrent = false;
  bool m_isExpanded = false;

  void onToggle(CCObject *sender);
  void updateTextColors();
  void updateMetaContent();
  void updateExpandButton();
  void updateTexture();
  void updateLayoutWrapper(bool isInitialRender = false);
  void onExpand(CCObject *);
  void onFinishExpandAnimation();
  void onFinishTableAnimation();

public:
  static StageRangeCell *create(Range *range, GJGameLevel *level, const CCSize &cellSize);

  std::function<void(StageRangeCell *target, bool triggerCallback)> onExpandChanged;

  bool init(Range *range, GJGameLevel *level, const CCSize &cellSize);

  void setExpanded(bool expanded, bool triggerCallback = false);
  void setDisabled(bool disabled);

  bool isDisabled() const { return m_disabled; }
  bool isChecked() const { return m_checked; }
  bool isExpanded() const { return m_isExpanded; }
};
