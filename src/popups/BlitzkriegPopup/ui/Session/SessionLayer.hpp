#pragma once
#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>
#include <cstdint>
#include <string>
#include <unordered_set>
#include <vector>

#include "SessionAttemptCell.hpp"
#include "../../../../store/SessionStore/SessionStore.hpp"
#include "../../../../ui/RectNode.hpp"

using namespace geode::prelude;

// Session tab: every attempt since the last reset,
// where it went and why, with export and reset
class SessionLayer : public CCLayer
{
private:
  static constexpr float SIDE_PADDING = 10.f;
  static constexpr float TOP_PADDING = 12.f;
  static constexpr float BOTTOM_PADDING = 10.f;
  static constexpr float CONTROLS_HEIGHT = 16.f;
  static constexpr float ROW_GAP = 7.f;
  // Cells are built for the newest attempts only, "Show more" adds a page
  static constexpr std::size_t PAGE_SIZE = 150;

  struct PillButton
  {
    CCMenuItemSpriteExtra *item = nullptr;
    RectNode *bg = nullptr;
  };

  CCSize m_size;
  std::string m_levelId;

  CCNode *m_summary = nullptr;
  PillButton m_levelFilterButton;
  ScrollLayer *m_scroll = nullptr;
  CCNode *m_emptyState = nullptr;

  bool m_thisLevelOnly = true;
  std::size_t m_shownCount = PAGE_SIZE;
  std::unordered_set<std::uint64_t> m_expanded;

  bool m_rebuildQueued = false;
  ListenerHandle m_sessionChangedListener;

  // Newest first, filtered by the level toggle
  std::vector<SessionAttempt const *> getVisibleAttempts() const;

  void drawControls(float y);
  void drawList(float top);
  PillButton createPill(CCMenu *menu, const char *text, float width, float x, SEL_MenuHandler selector);
  void updateLevelFilterButton();

  void updateSummary(std::vector<SessionAttempt const *> const &attempts);
  // keepScroll keeps the distance from the top of the list
  void rebuildList(bool keepScroll);
  // On the next frame, never inside a cell callback
  void queueRebuild(bool keepScroll);

  void onLevelFilter(CCObject *);
  void onExport(CCObject *);
  void onReset(CCObject *);
  void onShowMore(CCObject *);

public:
  static SessionLayer *create(GJGameLevel *level, const CCSize &size);
  bool init(GJGameLevel *level, const CCSize &size);
};
