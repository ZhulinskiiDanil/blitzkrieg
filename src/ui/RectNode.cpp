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
  float r = std::min(m_radius, std::min(w, h) / 2.f);

  if (r <= 0.01f)
  {
    CCPoint verts[] = {
        {0, 0},
        {w, 0},
        {w, h},
        {0, h}};

    m_draw->drawPolygon(verts, 4, m_color, 0.f, m_color);
    return;
  }

  std::vector<CCPoint> verts;

  auto addCorner = [&](CCPoint center, float startAngle)
  {
    int segments = std::clamp(
        int(r * 4),
        6,
        24);

    for (int i = 0; i <= segments; i++)
    {
      float angle = startAngle + (M_PI_2 * i / segments);
      verts.push_back({center.x + cosf(angle) * r,
                       center.y + sinf(angle) * r});
    }
  };

  addCorner({w - r, r}, -M_PI_2);
  addCorner({w - r, h - r}, 0);
  addCorner({r, h - r}, M_PI_2);
  addCorner({r, r}, M_PI);

  m_draw->drawPolygon(
      verts.data(),
      verts.size(),
      m_color,
      0.f,
      {0, 0, 0, 0});
}