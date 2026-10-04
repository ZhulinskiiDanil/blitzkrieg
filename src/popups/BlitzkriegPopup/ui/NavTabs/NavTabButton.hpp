#pragma once

#include <Geode/Geode.hpp>

#include <functional>

using namespace geode::prelude;

// Navigation tab of BlitzkriegPopup.
// Active tab shows icon + label, inactive tab shows only the icon.
// Switching between the two states is animated: the tab smoothly grows
// or shrinks while the label is typed in / erased letter by letter.
// The background switches to the new state instantly.
//
// Sprites (resources/sprites/tabs, authored at UHD scale):
//   tab-bg-active.png / tab-bg-inactive.png - 9-slice backgrounds,
//     the middle third is stretched horizontally
//   tab-icon-*.png - icons, scaled to fit ICON_SIZE
class NavTabButton : public CCMenuItemSpriteExtra
{
public:
  static constexpr float HEIGHT = 26.f;
  static constexpr float INACTIVE_WIDTH = 30.f;
  static constexpr float ICON_SIZE = 15.f;
  static constexpr float PADDING_X = 9.f;
  static constexpr float ICON_LABEL_GAP = 5.f;
  static constexpr float LABEL_SCALE = .4f;

  // Vertical offsets from the tab middle. Negative moves content down.
  // The background has a thick outline on top and an open bottom,
  // so the visual center of its fill is slightly below the middle.
  static constexpr float ICON_OFFSET_Y = -1.f;
  static constexpr float LABEL_OFFSET_Y = -1.f;

  static constexpr GLubyte INACTIVE_ICON_OPACITY = 150;

  // Duration of the expand / collapse animation, in seconds
  static constexpr float ANIMATION_DURATION = .2f;

  // Part of the expansion that passes before the first letter is typed.
  // Typing then follows the tab width, so the visible part of the label
  // always fits into the tab. Collapsing erases letters in reverse.
  static constexpr float TYPING_START = .15f;

  static NavTabButton *create(
      const char *label,
      const char *iconFile,
      CCObject *target,
      SEL_MenuHandler callback);

  // animate = false applies the state instantly, e.g. when the popup opens
  void setActive(bool active, bool animate = true);
  bool isActive() const;

  // Called every time the tab width changes, including animation frames.
  // The owner uses it to re-layout the row of tabs.
  void setOnResize(std::function<void()> callback);

  void update(float dt) override;

private:
  bool init(
      const char *label,
      const char *iconFile,
      CCObject *target,
      SEL_MenuHandler callback);

  // progress: 0 = fully inactive, 1 = fully active
  void applyProgress(float progress);
  void setVisibleChars(std::size_t count);

  CCNode *m_visual = nullptr;
  CCScale9Sprite *m_bgActive = nullptr;
  CCScale9Sprite *m_bgInactive = nullptr;
  CCSprite *m_icon = nullptr;
  CCLabelBMFont *m_label = nullptr;

  std::function<void()> m_onResize;

  std::string m_labelText;
  std::size_t m_visibleChars = 0;
  // Width of the fully active tab, measured once with the full label
  float m_activeWidth = INACTIVE_WIDTH;

  bool m_active = false;
  // Linear animation time in [0, ANIMATION_DURATION],
  // moves up while activating and down while deactivating
  float m_animationTime = 0.f;
  bool m_animating = false;
};
