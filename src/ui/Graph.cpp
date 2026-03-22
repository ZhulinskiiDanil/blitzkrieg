#include "./Graph.hpp"

Graph *Graph::create(const CCSize &size)
{
  auto ret = new Graph();
  if (ret && ret->init(size))
  {
    ret->autorelease();
    return ret;
  }
  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool Graph::init(const CCSize &size)
{
  if (!CCLayer::init())
    return false;

  m_uuid = generateUUID();
  m_size = size;
  this->setContentSize(m_size);

  m_container = CCNode::create();
  this->addChild(m_container);

  m_selectedPointsList = GraphPointDisplayList::create();
  m_selectedPointsList->setAnchorPoint({.5f, 1});
  m_selectedPointsList->setPosition(m_size / 2);
  m_selectedPointsList->setVisible(false);
  this->addChild(m_selectedPointsList);

  m_graphPointHoverListener = GraphPointHoverEvent().listen(
      [this](std::string graphUUID, std::string lineUUID, std::string pointUUID, bool hovered, float val, float x, float y)
      {
        if (graphUUID != m_uuid)
          return;

        std::string text;
        m_selectedPointsList->setVisible(hovered);
        // m_selectedPointsList->stopAllActions();

        if (hovered)
        {
          auto it = m_formatters.find(lineUUID);

          if (it != m_formatters.end())
            text = it->second(x, y, val);
          else
            text = fmt::format("(X: {:.2f}, Y: {:.2f})", x, y);
        }

        bool isYReversed = y > m_size.height / 2;
        bool isXReversed = x > m_size.width / 2;
        auto anchorXTo = isXReversed ? 1.f : 0.f;
        auto anchorYTo = isYReversed ? 1.f : 0.f;

        m_selectedPointsList->setAnchorPoint({isXReversed ? 1.f : 0.f, isYReversed ? 1.f : 0.f});
        m_selectedPointsList->setPosition({isXReversed ? x - 2.f : x + 2.f, isYReversed ? y - 2.f : y + 2.f});
        m_selectedPointsList->updatePointText(text, pointUUID, hovered);

        // auto moveTo = CCMoveTo::create(0.1f, {isXReversed ? x - 2.f : x + 2.f,
        //                                       isYReversed ? y - 2.f : y + 2.f});
        // auto easeMove = CCEaseInOut::create(moveTo, 2.f);

        // m_selectedPointsList->runAction(easeMove);
      });

  return true;
}

std::string Graph::addLine(const std::vector<GraphDot> &points, const ccColor3B &color, const ccColor3B &dotsColor)
{
  auto uuid = generateUUID();
  m_lines.push_back({uuid, points, color, dotsColor});

  return uuid;
}

void Graph::setFormatterByLineUUID(std::string lineUUID, std::function<std::string(float x, float y, float val)> formatter)
{
  m_formatters[lineUUID] = formatter;
}

void Graph::setLineColor(const std::string &uuid, const ccColor3B &color)
{
  for (auto &line : m_lines)
  {
    if (line.uuid == uuid)
    {
      line.color = color;
      return;
    }
  }
}

void Graph::setLineDotsColor(const std::string &uuid, const ccColor3B &color)
{
  for (auto &line : m_lines)
  {
    if (line.uuid == uuid)
    {
      line.dotsColor = color;
      return;
    }
  }
}

void Graph::setGrid(int everyX, int everyY, const ccColor4B &color)
{
  m_gridEveryX = everyX;
  m_gridEveryY = everyY;
  m_gridColor = color;
}

void Graph::setLabels(int everyX, int everyY, const ccColor4B &color)
{
  m_labelEveryX = everyX;
  m_labelEveryY = everyY;
  m_labelColor = color;
}

void Graph::redraw()
{
  if (!m_container || m_lines.empty())
    return;

  m_container->removeAllChildren();

  float minX = std::numeric_limits<float>::infinity();
  float minY = std::numeric_limits<float>::infinity();
  float maxX = -std::numeric_limits<float>::infinity();
  float maxY = -std::numeric_limits<float>::infinity();

  for (const auto &line : m_lines)
  {
    for (const auto &p : line.points)
    {
      minX = std::min(minX, p.x);
      minY = std::min(minY, p.y);
      maxX = std::max(maxX, p.x);
      maxY = std::max(maxY, p.y);
    }
  }

  int linesYCount = (maxY - minY) / m_gridEveryY;
  maxY += (maxY - minY) / linesYCount * static_cast<int>(linesYCount * 0.2);

  CCPoint scale{
      (maxX > minX) ? m_size.width / (maxX - minX) : 1.f,
      (maxY > minY) ? m_size.height / (maxY - minY) : 1.f};

  // ===== CLIPPING =====
  CCPoint maskShape[4] = {
      ccp(0, 0),
      ccp(m_size.width, 0),
      ccp(m_size.width, m_size.height),
      ccp(0, m_size.height)};

  auto clipping = CCClippingNode::create();
  m_container->addChild(clipping);

  auto mask = CCDrawNode::create();
  mask->drawPolygon(maskShape, 4, ccColor4F({0, 0, 0, 0}), 1, ccColor4F({1, 1, 1, 1}));
  mask->setZOrder(1);
  clipping->setStencil(mask);
  clipping->addChild(mask);

  // ===== GRID =====
  auto grid = CCDrawNode::create();
  clipping->addChild(grid);

  for (int i = 0; i <= maxX; i++)
  {
    if (i % m_gridEveryX == 0)
    {
      grid->drawSegment(
          ccp(i * scale.x, 0),
          ccp(i * scale.x, m_size.height),
          0.3f,
          ccc4FFromccc4B(m_gridColor));
    }
  }

  for (int i = 0; i <= maxY; i++)
  {
    if (i % m_gridEveryY == 0)
    {
      grid->drawSegment(
          ccp(0, i * scale.y),
          ccp(m_size.width, i * scale.y),
          0.3f,
          ccc4FFromccc4B(m_gridColor));
    }
  }

  // ===== LINES =====
  for (const auto &l : m_lines)
  {
    if (l.points.size() < 2)
      continue;

    auto line = CCDrawNode::create();

    for (int i = 1; i < l.points.size(); i++)
    {
      CCPoint a = ccp(
          (l.points[i - 1].x - minX) * scale.x,
          (l.points[i - 1].y - minY) * scale.y);

      CCPoint b = ccp(
          (l.points[i].x - minX) * scale.x,
          (l.points[i].y - minY) * scale.y);

      line->drawSegment(a, b, 1.f, ccc4FFromccc3B(l.color));
      line->setZOrder(0);

      auto pointA = GraphPoint::create(m_uuid, l.uuid, l.points[i - 1].y, ccc4FFromccc3B(l.dotsColor));
      pointA->setPosition(a);
      pointA->setZOrder(1);
      clipping->addChild(pointA);

      if (i == l.points.size() - 1)
      {
        auto pointB = GraphPoint::create(m_uuid, l.uuid, l.points[i - 1].y, ccc4FFromccc3B(l.dotsColor));
        pointB->setPosition(b);
        pointB->setZOrder(1);
        clipping->addChild(pointB);
      }
    }

    clipping->addChild(line);
  }

  // ===== LABELS =====

  // X labels
  for (int i = minX; i <= maxX; i++)
  {
    if (i % m_labelEveryX == 0)
    {
      auto labelX = CCLabelBMFont::create(std::to_string(i).c_str(), "chatFont.fnt");
      labelX->setScale(0.4f);
      labelX->setPosition({(i - minX) * scale.x, -10});
      labelX->setColor({m_labelColor.r, m_labelColor.g, m_labelColor.b});
      labelX->setOpacity(m_labelColor.a);
      m_container->addChild(labelX);
    }
  }

  // Y labels
  for (int i = minY; i <= maxY; i++)
  {
    if (i % m_labelEveryY == 0)
    {
      auto labelY = CCLabelBMFont::create(std::to_string(i).c_str(), "chatFont.fnt");
      labelY->setScale(0.4f);
      labelY->setPosition({-15, (i - minY) * scale.y});
      labelY->setColor({m_labelColor.r, m_labelColor.g, m_labelColor.b});
      labelY->setOpacity(m_labelColor.a);
      m_container->addChild(labelY);
    }
  }
}