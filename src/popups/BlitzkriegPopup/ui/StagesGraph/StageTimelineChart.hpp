#pragma once
#include <Geode/Geode.hpp>
#include <ctime>
#include <functional>
#include <vector>

#include "StageGraphData.hpp"
#include "../../../../ui/Label.hpp"
#include "../../../../ui/RectNode.hpp"

using namespace geode::prelude;

// Completed stages over time, a step for every stage completion.
// Hover (desktop) or tap selects a step, a click on the selected step opens the stage.
class StageTimelineChart : public CCLayer
{
private:
  // Same frame as StageBarChart, so switching between them does not jump
  static constexpr float AXIS_LEFT = 34.f;
  static constexpr float AXIS_BOTTOM = 14.f;
  static constexpr float PLOT_TOP = 12.f;
  static constexpr int GRID_LINES = 4;
  static constexpr int DATE_LABELS = 4;
  // Room on both sides of the time axis, in parts of the width
  static constexpr float X_MARGIN = .03f;

  struct Point
  {
    std::time_t time = 0;
    // Index of the stage among considered stages
    int stageIndex = 0;
    // Completed stages including this one
    int completed = 0;
    int attempts = 0;
    float timePlayed = 0.f;
  };

  CCSize m_size;
  CCSize m_plotSize;

  CCNode *m_plot = nullptr;
  CCDrawNode *m_selectedDot = nullptr;
  CCNode *m_tooltip = nullptr;
  RectNode *m_tooltipBg = nullptr;
  UILabel *m_tooltipTitle = nullptr;
  UILabel *m_tooltipStats = nullptr;
  CCLabelBMFont *m_tooltipHint = nullptr;
  CCMenu *m_hitMenu = nullptr;

  std::vector<Point> m_points;
  std::time_t m_start = 0;
  std::time_t m_end = 0;
  float m_axisMax = 1.f;
  float m_axisStep = 1.f;
  int m_totalStages = 0;

  int m_selected = -1;
  bool m_selectedByHover = false;

  CCPoint getPointPosition(Point const &point) const;
  float getTimeX(std::time_t time) const;

  void drawEmptyState();
  void drawUndatedNote(int count);
  void drawAxis();
  void drawLine(bool levelDone);
  void drawHitAreas();

  void select(int index, bool byHover);
  void updateTooltip();

  void onPoint(CCObject *sender);

public:
  std::function<void(int stageIndex)> onOpenStage;

  static StageTimelineChart *create(const CCSize &size);
  bool init(const CCSize &size);

  void setData(std::vector<StageGraphColumn> const &columns);

  void update(float dt) override;
};
