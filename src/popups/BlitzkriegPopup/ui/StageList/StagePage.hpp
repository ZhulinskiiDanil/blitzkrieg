#pragma once
#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>
#include <functional>
#include <string>
#include <unordered_map>

#include "StageRangeCell.hpp"
#include "../../../../events/UpdateScrollLayoutEvent.hpp"

#include "../../../../ui/RectNode.hpp"
#include "../../../../serialization/profile/index.hpp"
#include "../../../../store/GlobalStore.hpp"

using namespace geode::prelude;

enum StageListSortBy
{
  ASC,
  DESC
};

struct StagePageOptions
{
  StageListSortBy sortBy = StageListSortBy::ASC;
  bool hideCompletedRuns = false;
};

// One stage of the Stage Browser: a scrollable grid of runs.
// StageListLayer keeps three pages (previous, current, next) and slides between them.
class StagePage : public CCNode
{
private:
  // Locked stage card: lock icon, text and button on one background
  static constexpr float LOCK_PANEL_WIDTH = 190.f;
  static constexpr float LOCK_PANEL_PADDING = 9.f;
  static constexpr float LOCK_PANEL_GAP = 6.f;
  static constexpr float LOCK_PANEL_RADIUS = 6.f;
  static constexpr float LOCK_PANEL_OUTLINE = 1.f;

  CCSize m_size;
  GJGameLevel *m_level = nullptr;

  ScrollLayer *m_scroll = nullptr;
  CCLayer *m_content = nullptr;
  CCNode *m_lockPanel = nullptr;
  RectNode *m_lockPanelBorder = nullptr;
  RectNode *m_lockPanelFill = nullptr;
  CCSprite *m_lockIcon = nullptr;
  CCLabelBMFont *m_lockLabel = nullptr;
  CCMenu *m_lockMenu = nullptr;
  CCMenuItemSpriteExtra *m_lockButton = nullptr;
  CCNode *m_emptyState = nullptr;

  ListenerHandle m_listenerUpdateScrollLayout;

  // Owned by StageListLayer and shared by its pages, keyed by range id
  std::unordered_map<std::string, bool> *m_expandedRanges = nullptr;

  int m_stageIndex = -1;

  void createLockPanel();
  // The card height follows its content, the text changes with the stage
  void layoutLockPanel();
  void createEmptyState();

  void onGoToCurrent(CCObject *);

public:
  // Called by the "Go to current" button of a locked stage
  std::function<void()> onGoToCurrentStage;

  static StagePage *create(
      GJGameLevel *level,
      const CCSize &size,
      std::unordered_map<std::string, bool> *expandedRanges);

  bool init(
      GJGameLevel *level,
      const CCSize &size,
      std::unordered_map<std::string, bool> *expandedRanges);

  // stage == nullptr clears the page.
  // currentIndex is the stage the player is on, stages after it are locked.
  // Without keepScroll the page scrolls to the current run, or to the top.
  // focusRangeId scrolls to that run instead and flashes it.
  void build(
      Stage *stage,
      int stageIndex,
      int currentIndex,
      StagePageOptions const &options,
      bool keepScroll,
      std::string const &focusRangeId = {});

  // -1 when the page is empty
  int getStageIndex() const { return m_stageIndex; }
};
