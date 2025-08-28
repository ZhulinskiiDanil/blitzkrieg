#pragma once
#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>

#include "../ProfilesChangedEvent.hpp"

#include "../../../serialization/profile/index.hpp"
#include "../../../utils/getProfileStats.hpp"
#include "../../../utils/getProfileByLevel.hpp"
#include "../../../utils/linkProfileWithLevel.hpp"
#include "../../../utils/unlinkProfileFromLevel.hpp"

using namespace geode::prelude;

class BlitzkriegProfile : public CCLayer
{
protected:
  Profile m_profile;
  ProfileStats m_stats;
  bool m_isCurrent = false;

  GJGameLevel *m_level = nullptr;
  EventListener<EventFilter<ProfilesChangedEvent>> m_listener;

  CCSize m_size;
  CCMenu *m_buttonMenu = nullptr;
  CCObject *m_button = nullptr;

  void createBackground();
  void createLabels();
  void createButton();
  void updateButton();
  void updateFromCurrentProfile();

  // --- Handlers ---
  void onProfileSelect(CCObject *obj);
  void onProfileDeselect(CCObject *obj);

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
