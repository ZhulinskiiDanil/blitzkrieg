#pragma once
#include <chrono>
#include <cmath>
#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>

#include "../../../../events/ProfileChangedEvent.hpp"
#include "../../../../events/ProfilesChangedEvent.hpp"

#include "../../../../serialization/profile/index.hpp"
#include "../../../../store/GlobalStore.hpp"
#include "../../../../utils/linkProfileWithLevel.hpp"
#include "../../../../utils/unlinkProfileFromLevel.hpp"
#include "../../../../utils/getMetaInfoFromStages.hpp"
#include "../../../EditProfilePopup/index.hpp"

using namespace geode::prelude;

class BlitzkriegProfile : public CCLayer
{
protected:
  Profile m_profile;
  StageMetaInfo *m_stageMetaInfo;
  bool m_isCurrent = false;
  bool m_isPinned = false;
  bool m_profileToggleDisabled = false;
  std::chrono::steady_clock::time_point m_lastToggleTime;

  const std::chrono::milliseconds debounceDuration{300};

  GJGameLevel *m_level;
  // EventListener<EventFilter<ProfileChangedEvent>>
  ListenerHandle m_listener;

  CCSize m_size;
  CCMenu *m_toolsMenu;
  CCMenu *m_buttonMenu;

  void createBackground();
  void createLabels();
  void createMenu();
  void createButton(const char *spriteFrameName, cocos2d::SEL_MenuHandler callback);

  void updateButtons();
  void updateFromCurrentProfile();

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
