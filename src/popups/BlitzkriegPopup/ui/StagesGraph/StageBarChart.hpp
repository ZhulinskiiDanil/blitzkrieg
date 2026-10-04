#pragma once
#include <Geode/Geode.hpp>
#include <functional>
#include <optional>
#include <vector>

#include "StageGraphData.hpp"
#include "../../../../ui/Label.hpp"
#include "../../../../ui/RectNode.hpp"

using namespace geode::prelude;

// Bar chart of attempts or playtime per stage.
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

  CCSize m_size;
  CCSize m_plotSize;

  // Everything is drawn in plot space, the origin is the bottom-left of the plot
  CCNode *m_plot = nullptr;
  CCNode *m_axisNode = nullptr;
  CCNode *m_barsNode = nullptr;
  CCNode *m_bestNode = nullptr;
  RectNode *m_highlight = nullptr;
  CCNode *m_tooltip = nullptr;
  RectNode *m_tooltipBg = nullptr;
  UILabel *m_tooltipLabel = nullptr;
  CCLabelBMFont *m_tooltipHint = nullptr;
  CCMenu *m_hitMenu = nullptr;

  std::vector<StageGraphColumn> m_columns;
  std::vector<RectNode *> m_bars;
  StageGraphMetric m_metric = StageGraphMetric::Attempts;
  float m_axisMax = 1.f;
  float m_axisStep = 1.f;

  std::optional<float> m_bestX;
  float m_bestPercent = 0.f;

  int m_selected = -1;
  // Selected by the mouse, cleared when the mouse leaves the plot
  bool m_selectedByHover = false;

  float getSlotWidth() const;
  float getColumnCenterX(float x) const;
  float getBarHeight(StageGraphColumn const &column) const;

  void rebuild(bool animate);
  void drawAxis();
  void drawBars(bool animate);
  void drawBestLine();
  void drawHitAreas();

  void select(int index, bool byHover);
  void updateTooltip();

  void onColumn(CCObject *sender);

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
