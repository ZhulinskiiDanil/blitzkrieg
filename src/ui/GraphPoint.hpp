#pragma once

#include <Geode/Geode.hpp>

#include "../utils/generateUUID.hpp"
#include "../events/GraphPointHoverEvent.hpp"

using namespace geode::prelude;

class GraphPoint : public CCLayer
{
private:
  CCDrawNode *m_drawNode;

  float m_val;
  std::string m_uuid;
  std::string m_graphUUID;
  std::string m_lineUUID;
  bool m_hovered = false;
  float m_size = 8;

public:
  static GraphPoint *create(std::string graphUUID, std::string lineUUID, float val, ccColor4F color = ccc4FFromccc3B({255, 255, 255}));
  bool init(std::string graphUUID, std::string lineUUID, float val, ccColor4F color = ccc4FFromccc3B({255, 255, 255}));

  void update(float dt);
  CCRect getWorldSpaceBoundingBox(CCNode *node);
  void toggle(bool isToggled, float x, float y);

  std::string getUUID();

  bool ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent);
  void ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent);
  void ccTouchEnd(CCTouch *pTouch, CCEvent *pEvent);
};