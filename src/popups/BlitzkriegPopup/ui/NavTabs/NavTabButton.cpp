#include "NavTabButton.hpp"

#include <algorithm>
#include <cmath>

namespace
{
  float lerp(float from, float to, float t)
  {
    return from + (to - from) * t;
  }

  // Symmetric easing, so reversing the animation midway stays continuous
  float easeInOutCubic(float t)
  {
    return t < .5f
               ? 4.f * t * t * t
               : 1.f - std::pow(-2.f * t + 2.f, 3.f) / 2.f;
  }
}

NavTabButton *NavTabButton::create(
    const char *label,
    const char *iconFile,
    CCObject *target,
    SEL_MenuHandler callback)
{
  auto *ret = new NavTabButton();

  if (ret->init(label, iconFile, target, callback))
  {
    ret->autorelease();
    return ret;
  }

  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool NavTabButton::init(
    const char *label,
    const char *iconFile,
    CCObject *target,
    SEL_MenuHandler callback)
{
  m_visual = CCNode::create();

  // ! --- Backgrounds --- !
  m_bgActive = CCScale9Sprite::create("tab-bg-active.png"_spr);
  m_bgActive->setAnchorPoint({0.f, 0.f});
  m_visual->addChild(m_bgActive, 0);

  m_bgInactive = CCScale9Sprite::create("tab-bg-inactive.png"_spr);
  m_bgInactive->setAnchorPoint({0.f, 0.f});
  m_visual->addChild(m_bgInactive, 0);

  // ! --- Icon --- !
  m_icon = CCSprite::create(iconFile);
  const auto iconSize = m_icon->getContentSize();
  m_icon->setScale(ICON_SIZE / std::max({iconSize.width, iconSize.height, 1.f}));
  m_visual->addChild(m_icon, 1);

  // ! --- Label --- !
  m_labelText = label;

  m_label = CCLabelBMFont::create(label, "bigFont.fnt");
  m_label->setScale(LABEL_SCALE);
  m_label->setAnchorPoint({0.f, .5f});
  m_visual->addChild(m_label, 1);

  m_activeWidth = PADDING_X * 2 + ICON_SIZE + ICON_LABEL_GAP +
                  m_label->getScaledContentWidth();
  m_visibleChars = m_labelText.size();

  if (!CCMenuItemSpriteExtra::init(m_visual, nullptr, target, callback))
    return false;

  this->setSizeMult(1.1f);
  this->applyProgress(0.f);

  return true;
}

void NavTabButton::setActive(bool active, bool animate)
{
  if (m_active == active && (animate || !m_animating))
    return;

  m_active = active;

  if (!animate)
  {
    m_animating = false;
    m_animationTime = active ? ANIMATION_DURATION : 0.f;
    this->unscheduleUpdate();
    this->applyProgress(active ? 1.f : 0.f);
    return;
  }

  // Continue from the current animation time,
  // so switching tabs quickly does not cause jumps
  if (!m_animating)
  {
    m_animating = true;
    this->scheduleUpdate();
  }
}

bool NavTabButton::isActive() const
{
  return m_active;
}

void NavTabButton::setOnResize(std::function<void()> callback)
{
  m_onResize = std::move(callback);
}

void NavTabButton::update(float dt)
{
  if (!m_animating)
    return;

  const float direction = m_active ? 1.f : -1.f;

  m_animationTime = std::clamp(
      m_animationTime + dt * direction,
      0.f,
      ANIMATION_DURATION);

  const bool finished = m_active
                            ? m_animationTime >= ANIMATION_DURATION
                            : m_animationTime <= 0.f;

  this->applyProgress(
      easeInOutCubic(m_animationTime / ANIMATION_DURATION));

  if (finished)
  {
    m_animating = false;
    this->unscheduleUpdate();
  }
}

void NavTabButton::setVisibleChars(std::size_t count)
{
  count = std::min(count, m_labelText.size());

  if (count == m_visibleChars)
    return;

  m_visibleChars = count;
  m_label->setString(m_labelText.substr(0, count).c_str());
}

void NavTabButton::applyProgress(float progress)
{
  const float width = lerp(INACTIVE_WIDTH, m_activeWidth, progress);
  const CCSize size{width, HEIGHT};

  // ! --- Backgrounds: switch to the target state right away --- !
  m_bgActive->setContentSize(size);
  m_bgActive->setVisible(m_active);

  m_bgInactive->setContentSize(size);
  m_bgInactive->setVisible(!m_active);

  // ! --- Icon: slides from the center to the left padding --- !
  m_icon->setPosition({
      lerp(INACTIVE_WIDTH / 2, PADDING_X + ICON_SIZE / 2, progress),
      HEIGHT / 2 + ICON_OFFSET_Y,
  });
  m_icon->setOpacity(static_cast<GLubyte>(
      lerp(INACTIVE_ICON_OPACITY, 255.f, progress)));

  // ! --- Label: typed in while expanding, erased while collapsing --- !
  const float typingProgress = std::clamp(
      (progress - TYPING_START) / (1.f - TYPING_START),
      0.f,
      1.f);

  // floor keeps the typed part at or behind the tab growth
  this->setVisibleChars(static_cast<std::size_t>(
      std::floor(m_labelText.size() * typingProgress)));

  m_label->setPosition({
      PADDING_X + ICON_SIZE + ICON_LABEL_GAP,
      HEIGHT / 2 + LABEL_OFFSET_Y,
  });
  m_label->setVisible(m_visibleChars > 0);

  // ! --- Sizes --- !
  // Set manually instead of updateSprite(): it reads the scaled size,
  // which is wrong while the press animation is running.
  m_visual->setContentSize(size);
  m_visual->setAnchorPoint({.5f, .5f});
  m_visual->setPosition(size / 2);
  this->setContentSize(size);

  if (m_onResize)
    m_onResize();
}
