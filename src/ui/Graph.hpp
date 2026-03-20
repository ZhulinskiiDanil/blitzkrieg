#pragma once

#include <Geode/Geode.hpp>

#include "./GraphPoint.hpp"
#include "./GraphPointDisplayList.hpp"
#include "../utils/generateUUID.hpp"
#include "../events/GraphPointHoverEvent.hpp"

using namespace geode::prelude;

struct GraphDot
{
  float x;
  float y;
};

struct GraphLine
{
  std::string uuid;
  std::vector<GraphDot> points;
  ccColor3B color;
  ccColor3B dotsColor;
};

class Graph : public CCLayer
{
public:
  static Graph *create(const CCSize &size);
  bool init(const CCSize &size);

  // ===== DATA ======
  std::string addLine(const std::vector<GraphDot> &points, const ccColor3B &color, const ccColor3B &dotsColor);
  void setFormatterByLineUUID(std::string lineUUID, std::function<std::string(float x, float y, float val)> formatter);

  // ===== SETTINGS =====
  void setGrid(int everyX, int everyY, const ccColor4B &color);
  void setLabels(int everyX, int everyY, const ccColor4B &color);
  void setLineColor(const std::string &uuid, const ccColor3B &color);
  void setLineDotsColor(const std::string &uuid, const ccColor3B &color);

  // redraw graph
  void redraw();

private:
  std::string m_uuid;
  std::unordered_map<std::string, std::function<std::string(float x, float y, float val)>> m_formatters;

  CCSize m_size;
  GraphPointDisplayList *m_selectedPointsList;
  std::vector<GraphLine> m_lines;

  geode::comm::ListenerHandle m_graphPointHoverListener;

  int m_gridEveryX = 10;
  int m_gridEveryY = 10;
  ccColor4B m_gridColor = {50, 50, 50, 255};

  int m_labelEveryX = 10;
  int m_labelEveryY = 10;
  ccColor4B m_labelColor = {200, 200, 200, 255};

  CCNode *m_container = nullptr;
};