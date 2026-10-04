#pragma once
#include <chrono>
#include <cmath>
#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>

#include "../../../../events/ProfileChangedEvent.hpp"
#include "../../../../events/ProfilesChangedEvent.hpp"

#include "../../../../serialization/profile/index.hpp"
#include "../../../../store/GlobalStore.hpp"
#include "../../../../ui/Include.hpp"
#include "../../../../ui/RectNode.hpp"
#include "../../../../ui/ScrollClip.hpp"
#include "../../../../utils/linkProfileWithLevel.hpp"
#include "../../../../utils/unlinkProfileFromLevel.hpp"
#include "../../../../utils/getProfileSummary.hpp"
#include "../../../../utils/formatTimePlayed.hpp"
#include "../../../../utils/ui/fitLabelWidth.hpp"
#include "../../../EditProfilePopup/index.hpp"

using namespace geode::prelude;

// A row of the profiles list.
// Rows are rebuilt by ProfilesListLayer whenever profiles or the link change,
// so a row only renders the state it was created with.
class BlitzkriegProfile : public CCLayer
{
protected:
  static constexpr float PADDING_X = 10.f;
  static constexpr float CONTENT_TO_BUTTONS_GAP = 8.f;
  static constexpr float LINKED_BADGE_GAP = 5.f;
  static constexpr float PROGRESS_BAR_HEIGHT = 2.f;

  // Name label shrinks down to NAME_MIN_SCALE, after that it is cut with "..."
  static constexpr float NAME_SCALE = .5f;
  static constexpr float NAME_MIN_SCALE = .35f;

  static constexpr ccColor3B ACCENT_COLOR = {255, 0, 82};
  static constexpr ccColor3B COMPLETED_COLOR = {99, 224, 110};

  Profile m_profile;
  ProfileSummary m_summary;
  bool m_isCurrent = false;
  bool m_isPinned = false;
  bool m_profileToggleDisabled = false;
  std::chrono::steady_clock::time_point m_lastToggleTime;

  const std::chrono::milliseconds debounceDuration{300};

  GJGameLevel *m_level = nullptr;

  CCSize m_size;
  CCMenu *m_toolsMenu = nullptr;
  CCMenu *m_buttonMenu = nullptr;

  void createBackground();
  void createLabels();
  void createProgressBar();
  void createMenu();
  void createButton(const char *spriteFrameName, cocos2d::SEL_MenuHandler callback);

  void updateButtons();

  // Name with streamer mode applied
  std::string getDisplayName() const;
  // Right edge of the area available for labels and the progress bar
  float getContentRight();

  // --- Handlers ---
  void onToggleProfile(CCObject *obj);
  void onTogglePinProfile(CCObject *obj);
  void onUpProfile(CCObject *obj);
  void onEditProfile(CCObject *obj);
  void onDeleteProfile(CCObject *obj);

public:
  static BlitzkriegProfile *create(Profile const &profile,
                                   GJGameLevel *level,
                                   CCSize const &size);

  bool init(Profile const &profile,
            GJGameLevel *level,
            CCSize const &size);

  Profile const &getProfile() const { return m_profile; }
  bool isCurrentProfile() const { return m_isCurrent; }
};
