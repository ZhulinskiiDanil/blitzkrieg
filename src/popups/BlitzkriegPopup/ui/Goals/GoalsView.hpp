#pragma once
#include <Geode/Geode.hpp>
#include <ctime>

#include "../../../../store/Achievements/Achievements.hpp"
#include "../../../../ui/Label.hpp"
#include "../../../../ui/RectNode.hpp"

using namespace geode::prelude;

// Goals tab content: the daily goal on the left,
// the achievements of the level profile on the right
class GoalsView : public CCNode
{
private:
  static constexpr float GOAL_WIDTH = 128.f;
  static constexpr float GAP = 8.f;
  static constexpr float RING_RADIUS = 34.f;
  static constexpr float RING_THICKNESS = 6.f;
  static constexpr float PILL_HEIGHT = 15.f;
  static constexpr float ROW_HEIGHT = 26.f;
  static constexpr float HEADER_HEIGHT = 14.f;

  struct TypeButton
  {
    DailyGoalType type;
    RectNode *bg = nullptr;
  };

  CCSize m_size;
  // Achievements unlocked after this are marked as new
  std::time_t m_seenAt = 0;

  // ! --- Goal card, updated in place, its buttons stay --- !
  CCPoint m_ringCenter;
  CCDrawNode *m_ring = nullptr;
  CCLabelBMFont *m_progressLabel = nullptr;
  CCLabelBMFont *m_targetLabel = nullptr;
  CCLabelBMFont *m_captionLabel = nullptr;
  CCLabelBMFont *m_amountLabel = nullptr;
  std::vector<TypeButton> m_typeButtons;

  void drawGoal();
  void updateGoal();
  void drawAchievements(Profile const *profile);

  void onGoalType(CCObject *sender);
  void onGoalLess(CCObject *);
  void onGoalMore(CCObject *);

public:
  static GoalsView *create(CCSize const &size, Profile const *profile, std::time_t seenAt);
  bool init(CCSize const &size, Profile const *profile, std::time_t seenAt);
};
