#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

/** @brief Sizes a CCNode object to a zoom factor by modifying it's contentSize attribute.
 @warning This action doesn't support "reverse"
 */
class CCResizeTo : public CCActionInterval
{
public:
  /** initializes the action with the same scale factor for X and Y */
  bool initWithDuration(float duration, float s);

  /** initializes the action with and X factor and a Y factor */
  bool initWithDuration(float duration, float sx, float sy);
  /**
   *  @js NA
   *  @lua NA
   */
  // virtual CCObject* copyWithZone(CCZone* pZone);
  void startWithTarget(CCNode *pTarget) override;
  void update(float time) override;

public:
  /** creates the action with the same scale factor for X and Y */
  static CCResizeTo *create(float duration, float s);

  /** creates the action with and X factor and a Y factor */
  static CCResizeTo *create(float duration, float sx, float sy);

protected:
  float m_fContentWidth;
  float m_fContentHeight;
  float m_fStartContentWidth;
  float m_fStartContentHeight;
  float m_fEndContentWidth;
  float m_fEndContentHeight;
  float m_fDeltaWidth;
  float m_fDeltaHeight;

  bool m_isContainer;
};

class CCActionFloat : public CCActionInterval
{
public:
  static CCActionFloat *create(
      float duration,
      float from,
      float to,
      std::function<void(float)> callback);

  bool initWithDuration(
      float duration,
      float from,
      float to,
      std::function<void(float)> callback);

  void update(float time) override;

protected:
  float m_from;
  float m_to;
  float m_delta;

  std::function<void(float)> m_callback;
};