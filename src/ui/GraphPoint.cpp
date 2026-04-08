#include "GraphPoint.hpp"

GraphPoint *GraphPoint::create(std::string graphUUID, std::string lineUUID, float val, ccColor4F color)
{
  auto ret = new GraphPoint();
  if (ret && ret->init(graphUUID, lineUUID, val, color))
  {
    ret->autorelease();
    return ret;
  }
  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool GraphPoint::init(std::string graphUUID, std::string lineUUID, float val, ccColor4F color)
{
  if (!CCLayer::init())
    return false;

  m_val = val;
  m_uuid = generateUUID();
  m_graphUUID = graphUUID;
  m_lineUUID = lineUUID;

  this->setTouchPriority(1);
  this->setTouchEnabled(true);
  this->setContentSize({m_size, m_size});
  this->setAnchorPoint({0.5f, 0.5f});
  this->ignoreAnchorPointForPosition(false);

  m_drawNode = CCDrawNode::create();

  m_drawNode->drawCircle(
      ccp(m_size / 2, m_size / 2),
      2.f,
      color,
      0.f,
      color,
      20);

  this->addChild(m_drawNode);
  this->scheduleUpdate();

  return true;
}

std::string GraphPoint::getUUID()
{
  return m_uuid;
}

// TODO: make sensor support
bool GraphPoint::ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent)
{
  auto thisPos = this->getPosition();

  // toggle(true, thisPos.x, thisPos.y);
  // log::debug("ccTouchBegan");

  return true;
}

void GraphPoint::ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent)
{
  // log::debug("ccTouchMoved");
}

void GraphPoint::ccTouchEnd(CCTouch *pTouch, CCEvent *pEvent)
{
  auto thisPos = this->getPosition();

  // toggle(false, thisPos.x, thisPos.y);
  // log::debug("ccTouchEnd");
}

CCRect GraphPoint::getWorldSpaceBoundingBox(CCNode *node)
{
  auto parent = node->getParent();
  auto bounding_box = node->boundingBox();
  CCPoint bb_min(bounding_box.getMinX(), bounding_box.getMinY());
  CCPoint bb_max(bounding_box.getMaxX(), bounding_box.getMaxY());

  auto min = parent ? parent->convertToWorldSpace(bb_min) : bb_min;
  auto max = parent ? parent->convertToWorldSpace(bb_max) : bb_max;

  return CCRectMake(min.x, min.y, max.x - min.x, max.y - min.y);
}

void GraphPoint::update(float dt)
{
  auto mousePos = getMousePos();
  auto thisPos = this->getPosition();

  if (getWorldSpaceBoundingBox(this).containsPoint(mousePos))
  {
    if (!m_hovered)
      toggle(true, thisPos.x, thisPos.y);
  }
  else if (m_hovered)
  {
    toggle(false, thisPos.x, thisPos.y);
  }
}

void GraphPoint::toggle(bool isToggled, float x, float y)
{
  m_hovered = isToggled;

  if (isToggled)
  {
    GraphPointHoverEvent().send(m_graphUUID, m_lineUUID, m_uuid, true, m_val, x, y);
    this->setScale(1.5f);
  }
  else
  {
    GraphPointHoverEvent().send(m_graphUUID, m_lineUUID, m_uuid, false, m_val, x, y);
    this->setScale(1);
  }
}