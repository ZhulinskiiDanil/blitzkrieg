#include "RectNode.hpp"

RectNode *RectNode::create(CCSize size,
                           ccColor4F color,
                           float radius)
{
  auto ret = new RectNode();
  if (ret && ret->init(size, color, radius))
  {
    ret->autorelease();
    return ret;
  }
  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool RectNode::init(CCSize size,
                    ccColor4F color,
                    float radius)
{
  if (!CCLayer::init())
    return false;

  m_size = size;
  m_color = color;
  m_radius = radius;

  m_draw = CCDrawNode::create();
  m_draw->setContentSize(m_size);
  this->setContentSize(m_size);
  this->addChild(m_draw);

  redraw();
  return true;
}

void RectNode::setSize(CCSize size)
{
  m_size = size;

  this->setContentSize(m_size);

  if (m_draw)
    m_draw->setContentSize(m_size);

  redraw();
}

void RectNode::setColor(ccColor4F color)
{
  m_color = color;
  redraw();
}

void RectNode::setRadius(float radius)
{
  m_radius = radius;
  redraw();
}

void RectNode::redraw()
{
  m_draw->clear();

  float w = m_size.width;
  float h = m_size.height;

  if (w <= 0.f || h <= 0.f)
    return;

  float r = std::clamp(
      m_radius,
      0.f,
      std::min(w, h) / 2.f);

  if (r <= 0.01f)
  {
    CCPoint vertices[] = {
        {0.f, 0.f},
        {w, 0.f},
        {w, h},
        {0.f, h},
    };

    m_draw->drawPolygon(
        vertices,
        4,
        m_color,
        0.f,
        {0.f, 0.f, 0.f, 0.f});

    return;
  }

  std::vector<CCPoint> vertices;

  auto isSamePoint = [](
                         CCPoint const &a,
                         CCPoint const &b)
  {
    constexpr float epsilon = 0.0001f;

    return std::abs(a.x - b.x) < epsilon &&
           std::abs(a.y - b.y) < epsilon;
  };

  auto pushUnique = [&](
                        CCPoint const &point)
  {
    if (!vertices.empty() &&
        isSamePoint(vertices.back(), point))
    {
      return;
    }

    vertices.push_back(point);
  };

  auto addCorner = [&](
                       CCPoint const &center,
                       float startAngle)
  {
    int segments = std::clamp(
        static_cast<int>(
            std::ceil(r * 2.f)),
        3,
        16);

    for (int i = 0; i <= segments; ++i)
    {
      float progress =
          static_cast<float>(i) /
          static_cast<float>(segments);

      float angle =
          startAngle +
          M_PI_2 * progress;

      pushUnique({
          center.x + std::cos(angle) * r,
          center.y + std::sin(angle) * r,
      });
    }
  };

  // Обход против часовой стрелки.
  addCorner(
      {w - r, r},
      -M_PI_2);

  addCorner(
      {w - r, h - r},
      0.f);

  addCorner(
      {r, h - r},
      M_PI_2);

  addCorner(
      {r, r},
      M_PI);

  // Последняя точка также может совпасть с первой.
  if (vertices.size() > 1 &&
      isSamePoint(
          vertices.front(),
          vertices.back()))
  {
    vertices.pop_back();
  }

  if (vertices.size() < 3)
    return;

  m_draw->drawPolygon(
      vertices.data(),
      static_cast<unsigned int>(
          vertices.size()),
      m_color,
      0.f,
      {0.f, 0.f, 0.f, 0.f});
}