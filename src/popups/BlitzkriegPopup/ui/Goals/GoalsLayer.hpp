#pragma once
#include <Geode/Geode.hpp>

#include "GoalsView.hpp"

using namespace geode::prelude;

// Goals tab: the daily goal and the achievements of the level profile.
// Opening it marks the achievements as seen, the New marks stay until the next time.
class GoalsLayer : public CCLayer
{
private:
  static constexpr float SIDE_PADDING = 10.f;
  static constexpr float TOP_PADDING = 12.f;
  static constexpr float BOTTOM_PADDING = 10.f;

public:
  static GoalsLayer *create(GJGameLevel *level, const CCSize &size);
  bool init(GJGameLevel *level, const CCSize &size);
};
