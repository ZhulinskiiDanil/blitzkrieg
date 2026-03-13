#include "CustomActions.hpp"

//
// ! --- SizeTo --- !
//
CCResizeTo *CCResizeTo::create(float duration, float s)
{
  CCResizeTo *pSizeTo = new CCResizeTo();
  pSizeTo->initWithDuration(duration, s);
  pSizeTo->autorelease();

  return pSizeTo;
}

bool CCResizeTo::initWithDuration(float duration, float s)
{
  if (CCActionInterval::initWithDuration(duration))
  {
    m_fEndContentWidth = s;
    m_fEndContentHeight = s;

    return true;
  }

  return false;
}

CCResizeTo *CCResizeTo::create(float duration, float sx, float sy)
{
  CCResizeTo *pSizeTo = new CCResizeTo();
  pSizeTo->initWithDuration(duration, sx, sy);
  pSizeTo->autorelease();

  return pSizeTo;
}

bool CCResizeTo::initWithDuration(float duration, float sx, float sy)
{
  if (CCActionInterval::initWithDuration(duration))
  {
    m_fEndContentWidth = sx;
    m_fEndContentHeight = sy;

    return true;
  }

  return false;
}
void CCResizeTo::startWithTarget(CCNode *pTarget)
{
  CCActionInterval::startWithTarget(pTarget);
  m_fStartContentWidth = pTarget->getContentWidth();
  m_fStartContentHeight = pTarget->getContentHeight();
  m_fDeltaWidth = m_fEndContentWidth - m_fStartContentWidth;
  m_fDeltaHeight = m_fEndContentHeight - m_fStartContentHeight;
}

void CCResizeTo::update(float time)
{
  if (m_pTarget)
  {
    CCSize contentSize{
        m_fStartContentWidth + m_fDeltaWidth * time,
        m_fStartContentHeight + m_fDeltaHeight * time};
    m_pTarget->setContentSize(contentSize);
  }
}

//
// ! --- ActionFloat --- !
//
CCActionFloat *CCActionFloat::create(
    float duration,
    float from,
    float to,
    std::function<void(float)> callback)
{
  auto ret = new CCActionFloat();

  if (ret && ret->initWithDuration(duration, from, to, callback))
  {
    ret->autorelease();
    return ret;
  }

  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool CCActionFloat::initWithDuration(
    float duration,
    float from,
    float to,
    std::function<void(float)> callback)
{
  if (!CCActionInterval::initWithDuration(duration))
    return false;

  m_from = from;
  m_to = to;
  m_delta = to - from;
  m_callback = callback;

  return true;
}

void CCActionFloat::update(float time)
{
  if (m_callback)
  {
    float value = m_from + m_delta * time;
    m_callback(value);
  }
}

//
// ! --- CallFunc --- !
//
CallFunc *CallFunc::create(std::function<void()> func)
{
  auto ret = new CallFunc();

  if (ret && ret->initWithFunction(func))
  {
    ret->autorelease();
    return ret;
  }

  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool CallFunc::initWithFunction(std::function<void()> func)
{
  m_func = func;
  return true;
}

void CallFunc::update(float time)
{
  execute();
}

void CallFunc::execute()
{
  if (m_func)
  {
    m_func();
  }
}