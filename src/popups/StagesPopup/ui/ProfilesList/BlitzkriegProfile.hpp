#pragma once
#include <chrono>
#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>

#include "../../ProfileChangedEvent.hpp"
#include "../../ProfilesChangedEvent.hpp"

#include "../../../../serialization/profile/index.hpp"
#include "../../../../store/GlobalStore.hpp"
#include "../../../../utils/getProfileStats.hpp"
#include "../../../../utils/linkProfileWithLevel.hpp"
#include "../../../../utils/unlinkProfileFromLevel.hpp"

using namespace geode::prelude;

class BlitzkriegProfile : public CCLayer
{
protected:
  Profile m_profile;
  ProfileStats m_stats;
  bool m_isCurrent = false;
  bool m_isPinned = false;
  bool m_profileToggleDisabled = false;
  std::chrono::steady_clock::time_point m_lastToggleTime;

  const std::chrono::milliseconds debounceDuration{300};

  GJGameLevel *m_level = nullptr;
  EventListener<EventFilter<ProfileChangedEvent>> m_listener;

  CCSize m_size;
  CCMenu *m_buttonMenu = nullptr;

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
  void onDeleteProfile(CCObject *obj);

public:
  static BlitzkriegProfile *create(Profile const &profile,
                                   GJGameLevel *level,
                                   bool isCurrent,
                                   CCSize const &size);

  bool init(Profile const &profile,
            GJGameLevel *level,
            bool isCurrent,
            CCSize const &size);

  Profile const &getProfile() const { return m_profile; }
  bool isCurrentProfile() const { return m_isCurrent; }
};
