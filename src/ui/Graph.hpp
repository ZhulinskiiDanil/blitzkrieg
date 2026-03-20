#pragma once

#include <Geode/Geode.hpp>
using namespace geode::prelude;

struct GraphLine
{
  std::vector<CCPoint> points;
  ccColor3B color;
};

class Graph : public CCLayer
{
public:
  static Graph *create(const CCSize &size);
  bool init(const CCSize &size);

  // ===== DATA =====
  void setData(const std::vector<CCPoint> &points, const ccColor3B &color);

  // ===== SETTINGS =====
  void setGrid(int everyX, int everyY, const ccColor4B &color);
  void setLabels(int everyX, int everyY, const ccColor4B &color);
  void addLine(const std::vector<CCPoint> &points, const ccColor3B &color);
  void setLineColor(int index, const ccColor3B &color);

  // redraw graph
  void redraw();

private:
  CCSize m_contentSize;
  std::vector<GraphLine> m_lines;

  int m_gridEveryX = 10;
  int m_gridEveryY = 10;
  ccColor4B m_gridColor = {50, 50, 50, 255};

  int m_labelEveryX = 10;
  int m_labelEveryY = 10;
  ccColor4B m_labelColor = {200, 200, 200, 255};

  CCNode *m_container = nullptr;
};