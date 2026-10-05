#pragma once
#include <Geode/Geode.hpp>
#include <cstdint>
#include <functional>

#include "../../../../store/SessionStore/SessionAttempt.hpp"
#include "../../../../ui/RectNode.hpp"

using namespace geode::prelude;

// One attempt of the session log: a one line summary,
// a click shows the reason, the rule and every candidate run
class SessionAttemptCell : public CCNode, public CCTouchDelegate
{
private:
  static constexpr float HEAD_HEIGHT = 22.f;
  // A touch that moved further is a scroll, not a tap
  static constexpr float TAP_DISTANCE = 6.f;
  static constexpr float PADDING = 6.f;
  static constexpr float DETAILS_GAP = 3.f;
  static constexpr float TABLE_ROW_HEIGHT = 9.f;

  std::uint64_t m_id = 0;
  bool m_expanded = false;
  CCPoint m_touchStart;

  // Built top to bottom, the cell height follows
  float buildDetails(CCNode *details, SessionAttempt const &attempt, float width);
  void buildHead(SessionAttempt const &attempt, float width, float top);

  // On the head, inside the visible part of the list, with no popup above
  bool canTakeTouch(CCTouch *touch);

public:
  void onEnter() override;
  void onExit() override;

  bool ccTouchBegan(CCTouch *touch, CCEvent *event) override;
  void ccTouchEnded(CCTouch *touch, CCEvent *event) override;

  std::function<void(std::uint64_t id, bool expanded)> onExpandChanged;

  static SessionAttemptCell *create(SessionAttempt const &attempt, float width, bool expanded);
  bool init(SessionAttempt const &attempt, float width, bool expanded);
};

// Colors shared by the cell and the summary of the tab
ccColor3B getAttemptOutcomeColor(AttemptOutcome outcome);
