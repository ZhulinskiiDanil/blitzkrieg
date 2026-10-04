#pragma once
#include <Geode/Geode.hpp>
#include <string>
#include <vector>

#include "../../../../ui/Label.hpp"
#include "../../../../ui/RectNode.hpp"
#include "../../../../serialization/profile/index.hpp"

using namespace geode::prelude;

enum class StageHeaderStatus
{
  Completed,
  Current,
  Locked
};

// Header of the Stage Browser:
// a stage badge, a status chip, one segment per run and a short stats line
class StageHeader : public CCNode
{
private:
  static constexpr float HEIGHT = 36.f;
  static constexpr float BADGE_SIZE = 34.f;
  static constexpr float BADGE_OUTLINE = 1.5f;
  static constexpr float BADGE_RADIUS = 7.f;
  static constexpr float CONTENT_GAP = 9.f;

  static constexpr float CHIP_HEIGHT = 11.f;
  static constexpr float CHIP_DOT_SIZE = 5.f;
  static constexpr float CHIP_DOT_GAP = 4.f;

  // Segments are 1.5 times smaller than the space they could fill
  static constexpr float SEGMENT_SCALE = 1.f / 1.5f;
  static constexpr float SEGMENT_HEIGHT = 5.f * SEGMENT_SCALE;
  static constexpr float SEGMENT_GAP = 2.f;
  // Narrower segments turn into one continuous bar
  static constexpr float MIN_SEGMENT_WIDTH = 3.f;
  static constexpr float STAT_ICON_SIZE = 9.f;

  float m_width = 0.f;

  CCNode *m_badge = nullptr;
  RectNode *m_badgeBorder = nullptr;
  RectNode *m_badgeFill = nullptr;
  CCLabelBMFont *m_badgeLabel = nullptr;

  CCNode *m_chip = nullptr;
  RectNode *m_chipDot = nullptr;
  CCLabelBMFont *m_chipLabel = nullptr;

  CCNode *m_segments = nullptr;
  CCNode *m_stats = nullptr;

  int m_shownIndex = -1;

  float getContentX() const { return BADGE_SIZE + CONTENT_GAP; }
  float getContentWidth() const { return m_width - getContentX(); }

  void updateBadge(int stageIndex, StageHeaderStatus status);
  void updateChip(StageHeaderStatus status);
  void updateSegments(Stage *stage, std::string const &currentRangeId);
  void updateStats(Stage *stage);

public:
  static StageHeader *create(float width);
  bool init(float width);

  // stage == nullptr shows an empty header
  void setStage(
      int stageIndex,
      Stage *stage,
      StageHeaderStatus status,
      std::string const &currentRangeId);
};
