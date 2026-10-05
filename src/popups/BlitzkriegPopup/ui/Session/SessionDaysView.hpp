#pragma once
#include <Geode/Geode.hpp>
#include <ctime>
#include <map>
#include <string>
#include <vector>

#include "../../../../serialization/profile/index.hpp"
#include "../../../../ui/Label.hpp"
#include "../../../../ui/RectNode.hpp"

using namespace geode::prelude;

// Days of the Session tab: a few chips, a heatmap of the last weeks
// and the numbers of the hovered or tapped day
class SessionDaysView : public CCNode
{
private:
  static constexpr int WEEKS = 26;
  static constexpr float CHIPS_HEIGHT = 16.f;
  static constexpr float INFO_HEIGHT = 22.f;
  static constexpr float GAP = 8.f;
  static constexpr float CELL_GAP = 2.f;
  // Room for the weekday labels on the left and the months on top
  static constexpr float WEEKDAYS_WIDTH = 20.f;
  static constexpr float MONTHS_HEIGHT = 9.f;

  struct DayCell
  {
    std::string key;
    std::time_t noon = 0;
    CCPoint position;
    RectNode *rect = nullptr;
  };

  CCSize m_size;
  std::map<std::string, DayStats> m_history;
  // Profiles are summed up when every level is shown, the best from 0% means nothing then
  bool m_showBestFromZero = true;

  CCNode *m_grid = nullptr;
  float m_cellSize = 10.f;
  std::vector<DayCell> m_cells;
  RectNode *m_selection = nullptr;
  UILabel *m_info = nullptr;

  int m_selected = -1;
  CCPoint m_lastMousePos = {-1.f, -1.f};

  DayStats getDay(std::string const &key) const;

  void drawChips(float top);
  void drawGrid(float top, float bottom);
  void drawInfo(float y);

  void select(int index);
  int getCellAt(CCPoint const &gridPoint) const;

  void onCell(CCObject *sender);

public:
  static SessionDaysView *create(
      CCSize const &size,
      std::map<std::string, DayStats> history,
      bool showBestFromZero);

  bool init(
      CCSize const &size,
      std::map<std::string, DayStats> history,
      bool showBestFromZero);

  void update(float dt) override;
};

// Sums days of several profiles, the best from 0% is the highest one
void addHistory(std::map<std::string, DayStats> &into, std::map<std::string, DayStats> const &from);
