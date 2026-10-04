#pragma once
#include <Geode/Geode.hpp>
#include <functional>
#include <vector>

#include "StageBarChart.hpp"
#include "StageGraphData.hpp"
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
  static constexpr float BOTTOM_PADDING = 8.f;
  static constexpr float SUMMARY_HEIGHT = 30.f;
  static constexpr float CONTROLS_HEIGHT = 16.f;
  static constexpr float ROW_GAP = 8.f;

  struct MetricButton
  {
    StageGraphMetric metric;
    CCMenuItemSpriteExtra *item = nullptr;
    RectNode *bg = nullptr;
  };

  CCSize m_size;
  GJGameLevel *m_level = nullptr;

  StageBarChart *m_chart = nullptr;
  std::vector<MetricButton> m_metricButtons;
  StageGraphMetric m_metric = StageGraphMetric::Attempts;

  void drawEmptyState(const char *text, bool withProfilesButton);
  void drawSummary(std::vector<StageGraphColumn> const &columns, float top);
  void drawMetricSwitch(float y);
  void drawLegend(float y, bool withBestLine);
  void updateMetricButtons();

  void onMetric(CCObject *sender);
  void onOpenProfilesBtn(CCObject *);

public:
  // Set by the popup right after create
  std::function<void(int stageIndex)> onOpenStage;
  std::function<void()> onOpenProfiles;

  static StagesGraphLayer *create(GJGameLevel *level, const CCSize &contentSize);
  bool init(GJGameLevel *level, const CCSize &contentSize);
};
