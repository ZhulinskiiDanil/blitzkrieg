#pragma once
#include <Geode/Geode.hpp>
#include <functional>
#include <optional>
#include <vector>

#include "StageGraphData.hpp"
#include "../../../../ui/Label.hpp"
#include "../../../../ui/RectNode.hpp"

using namespace geode::prelude;

// Bar chart of attempts or playtime per stage, dividers split every bar into its runs.
// Hover (desktop) or tap selects a column, a click on the selected column opens it.
class StageBarChart : public CCLayer
{
private:
  // Room for the Y labels on the left
  static constexpr float AXIS_LEFT = 34.f;
  // Room for the X labels at the bottom
  static constexpr float AXIS_BOTTOM = 14.f;
  // Room for the best run label at the top
  static constexpr float PLOT_TOP = 12.f;
  static constexpr float BAR_WIDTH_RATIO = .6f;
  static constexpr float MAX_BAR_WIDTH = 22.f;
  static constexpr int GRID_LINES = 4;
  // Runs thinner than this are merged into a neighbour
  static constexpr float MIN_SEGMENT_HEIGHT = 3.f;
  static constexpr float PLACEHOLDER_HEIGHT = 3.f;
  static constexpr float TWEEN_DURATION = .3f;

  // One run of a bar, animated from `from` to `to`
  struct BarSegment
  {
    // What is drawn right now
    float y = 0.f;
    float height = 0.f;
    float fromY = 0.f;
    float fromHeight = 0.f;
    float toY = 0.f;
    float toHeight = 0.f;
  };

  struct BarColumn
  {
    // Same order as StageGraphColumn::runs
    std::vector<BarSegment> segments;
    // One solid bar in the stage color, the runs are split by thin dividers on top
    RectNode *bar = nullptr;
    CCDrawNode *dividers = nullptr;
    float x = 0.f;
    RectNode *placeholder = nullptr;
    float delay = 0.f;
  };

  CCSize m_size;
  CCSize m_plotSize;

  // Everything is drawn in plot space, the origin is the bottom-left of the plot
  CCNode *m_plot = nullptr;
  CCNode *m_axisNode = nullptr;
  CCNode *m_barsNode = nullptr;
  CCNode *m_averageNode = nullptr;
  CCNode *m_bestNode = nullptr;
  RectNode *m_highlight = nullptr;
  RectNode *m_segmentHighlight = nullptr;
  CCNode *m_tooltip = nullptr;
  RectNode *m_tooltipBg = nullptr;
  std::vector<UILabel *> m_tooltipLines;
  CCLabelBMFont *m_tooltipHint = nullptr;
  CCMenu *m_hitMenu = nullptr;

  std::vector<StageGraphColumn> m_columns;
  std::vector<BarColumn> m_bars;
  StageGraphMetric m_metric = StageGraphMetric::Attempts;
  float m_axisMax = 1.f;
  float m_axisStep = 1.f;
  float m_barWidth = 1.f;

  float m_tweenTime = 0.f;
  float m_tweenEnd = 0.f;

  std::optional<float> m_bestX;
  float m_bestPercent = 0.f;

  int m_selected = -1;
  // Run of the selected column under the mouse, -1 for the whole stage
  int m_selectedRun = -1;
  // Selected by the mouse, cleared when the mouse leaves the plot
  bool m_selectedByHover = false;

  float getSlotWidth() const;
  float getColumnCenterX(float x) const;
  float getBarHeight(StageGraphColumn const &column) const;
  // Target height of every run of the column for the current metric
  std::vector<float> getSegmentHeights(StageGraphColumn const &column) const;
  int getRunAt(int column, float y) const;

  void rebuild(bool animate);
  void drawAxis();
  void createBars();
  void retargetBars(bool fromZero);
  void applyBars(float time);
  void drawAverageLine();
  void drawBestLine();
  void drawHitAreas();

  void select(int index, int run, bool byHover);
  void updateHighlight();
  void updateTooltip();

  void onColumn(CCObject *sender);
  void onTween(float dt);

public:
  std::function<void(int stageIndex)> onOpenStage;

  static StageBarChart *create(const CCSize &size);
  bool init(const CCSize &size);

  void setData(std::vector<StageGraphColumn> columns, StageGraphMetric metric);
  void setMetric(StageGraphMetric metric);
  // x on the column axis, see mapPercentFromZero
  void setBestFromZero(std::optional<float> x, float percent);

  void update(float dt) override;
};
