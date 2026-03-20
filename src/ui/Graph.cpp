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

  m_contentSize = size;
  this->setContentSize(m_contentSize);

  m_container = CCNode::create();
  this->addChild(m_container);

  return true;
}

void Graph::addLine(const std::vector<CCPoint> &points, const ccColor3B &color)
{
  m_lines.push_back({points, color});
}

void Graph::setData(const std::vector<CCPoint> &points, const ccColor3B &color)
{
  m_lines.clear();
  m_lines.push_back({points, color});
}

void Graph::setLineColor(int index, const ccColor3B &color)
{
  if (index < 0 || index >= m_lines.size())
    return;
  m_lines[index].color = color;
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

  CCPoint scale{
      (maxX > minX) ? m_contentSize.width / (maxX - minX) : 1.f,
      (maxY > minY) ? m_contentSize.height / (maxY - minY) : 1.f};

  // ===== CLIPPING =====
  CCPoint maskShape[4] = {
      ccp(0, 0),
      ccp(m_contentSize.width, 0),
      ccp(m_contentSize.width, m_contentSize.height),
      ccp(0, m_contentSize.height)};

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
          ccp(i * scale.x, m_contentSize.height),
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
          ccp(m_contentSize.width, i * scale.y),
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