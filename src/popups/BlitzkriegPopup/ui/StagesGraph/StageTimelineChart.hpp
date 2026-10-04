#pragma once
#include <Geode/Geode.hpp>
#include <ctime>
#include <functional>
#include <string>
#include <vector>

#include "StageGraphData.hpp"
#include "../../../../ui/Label.hpp"
#include "../../../../ui/RectNode.hpp"

using namespace geode::prelude;

// Completed stages over time, a step for every stage completion.
// Long pauses are squeezed into narrow breaks, so the active days fill the width.
// Hover (desktop) or tap selects a step, a click on the selected step opens the stage.
class StageTimelineChart : public CCLayer
{
private:
  // Same frame as StageBarChart, so switching between them does not jump
  static constexpr float AXIS_LEFT = 34.f;
  static constexpr float AXIS_BOTTOM = 14.f;
  static constexpr float PLOT_TOP = 12.f;
  static constexpr int GRID_LINES = 4;
  // Room on both sides of the time axis, in parts of the width
  static constexpr float X_MARGIN = .03f;
  // Pauses longer than this become a break
  static constexpr std::time_t LONG_GAP = 3 * 24 * 60 * 60;
  static constexpr float BREAK_WIDTH = 18.f;
  // Breaks never take more than this part of the width
  static constexpr float MAX_BREAKS_SHARE = .4f;

  // Time to x, linear between neighbours
  struct Knot
  {
    std::time_t time = 0;
    float x = 0.f;
  };

  struct Break
  {
    float left = 0.f;
    float right = 0.f;
    std::time_t duration = 0;
  };

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
  std::vector<Knot> m_knots;
  std::vector<Break> m_breaks;
  bool m_levelDone = false;
  float m_axisMax = 1.f;
  float m_axisStep = 1.f;
  int m_totalStages = 0;

  int m_selected = -1;
  bool m_selectedByHover = false;
  // Keyboard selection stays until the mouse moves
  CCPoint m_lastMousePos = {-1.f, -1.f};

  CCPoint getPointPosition(Point const &point) const;
  float getTimeX(std::time_t time) const;

  void buildTimeAxis();
  // A flat part of the line, dashed and lighter over breaks
  void drawFlat(CCDrawNode *line, CCDrawNode *fill, float fromX, float toX, float y, bool dashed);

  void drawEmptyState();
  void drawUndatedNote(int count);
  void drawAxis();
  void drawLine();
  void drawHitAreas();

  void select(int index, bool byHover);
  void updateTooltip();

  void onPoint(CCObject *sender);

public:
  // Same as StageBarChart, a point is a whole stage so rangeId is empty
  std::function<void(int stageIndex, std::string const &rangeId)> onOpenStage;

  static StageTimelineChart *create(const CCSize &size);
  bool init(const CCSize &size);

  void setData(std::vector<StageGraphColumn> const &columns);

  // ! --- Keyboard --- !
  // From the last point when nothing is selected
  void moveSelection(int delta);
  void openSelected();

  void update(float dt) override;
};
