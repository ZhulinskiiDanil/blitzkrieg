#pragma once
#include <Geode/Geode.hpp>
#include <functional>
#include <string>
#include <vector>

#include "StageBarChart.hpp"
#include "StageGraphData.hpp"
#include "StageTimelineChart.hpp"
#include "../../../../ui/Label.hpp"
#include "../../../../ui/RectNode.hpp"
#include "../../../../store/GlobalStore.hpp"

using namespace geode::prelude;

class StagesGraphLayer : public CCLayer
{
private:
  static constexpr float SIDE_PADDING = 10.f;
  // Tabs sit on the popup border, same top padding as the profiles tab
  static constexpr float TOP_PADDING = 12.f;
  // Room for the keybind hint under the chart
  static constexpr float BOTTOM_PADDING = 11.f;
  static constexpr float SUMMARY_HEIGHT = 30.f;
  static constexpr float CONTROLS_HEIGHT = 16.f;
  static constexpr float ROW_GAP = 8.f;
  static constexpr float METRIC_BUTTON_WIDTH = 52.f;
  static constexpr float METRIC_BUTTON_GAP = 4.f;
  static constexpr float INFO_SIZE = 9.f;

  struct MetricButton
  {
    StageGraphMetric metric;
    CCMenuItemSpriteExtra *item = nullptr;
    RectNode *bg = nullptr;
  };

  CCSize m_size;
  GJGameLevel *m_level = nullptr;

  StageBarChart *m_chart = nullptr;
  StageTimelineChart *m_timeline = nullptr;
  // Bar colors and the info button, the timeline has neither
  CCNode *m_legend = nullptr;
  std::vector<MetricButton> m_metricButtons;
  StageGraphMetric m_metric = StageGraphMetric::Attempts;

  void drawEmptyState(const char *text, bool withProfilesButton);
  void drawSummary(std::vector<StageGraphColumn> const &columns, float top);
  // Returns the right edge of the switch
  float drawMetricSwitch(float y);
  // Right-aligned, shrinks to stay right of `left`
  void drawLegend(float y, float left);
  void drawKeybindHint();
  void listenToKeybinds();

  void applyMetric(StageGraphMetric metric);
  void updateMetricButtons();
  // Shows the chart of the current metric
  void updateChartVisibility();

  void moveSelection(int delta);
  void openSelected();

  void onMetric(CCObject *sender);
  // The tag is the stage to select, a second click opens it
  void onCard(CCObject *sender);
  void onOpenProfilesBtn(CCObject *);

public:
  // Set by the popup right after create.
  // rangeId is the run to scroll to, empty for the whole stage.
  std::function<void(int stageIndex, std::string const &rangeId)> onOpenStage;
  std::function<void()> onOpenProfiles;

  static StagesGraphLayer *create(GJGameLevel *level, const CCSize &contentSize);
  bool init(GJGameLevel *level, const CCSize &contentSize);
};
