#pragma once

#include <Geode/Geode.hpp>
using namespace geode::prelude;

class RectNode : public CCLayer
{
private:
  CCDrawNode *m_draw = nullptr;

  CCSize m_size = {100.f, 50.f};
  ccColor4F m_color = {1.f, 1.f, 1.f, 1.f};

  float m_radius = 0.f;

  void redraw();

public:
  static RectNode *create(CCSize size,
                          ccColor4F color = {1, 1, 1, 1},
                          float radius = 0.f);

  bool init(CCSize size,
            ccColor4F color = {1, 1, 1, 1},
            float radius = 0.f);

  void setSize(CCSize size);
  void setColor(ccColor4F color);
  void setRadius(float radius);
};