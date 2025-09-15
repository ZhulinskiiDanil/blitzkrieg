#pragma once
#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>

#include "BlitzkriegProfile.hpp"
#include "../../ProfilesChangedEvent.hpp"
#include "../../../CreateProfilePopup/index.hpp"

#include "../../../../serialization/profile/index.hpp"
#include "../../../../utils/getProfiles.hpp"
#include "../../../../utils/generateProfile.hpp"
#include "../../../../utils/addProfiles.hpp"
#include "../../../../utils/saveProfiles.hpp"
#include "../../../../utils/selectJsonFile.hpp"
#include "../../../../utils/generateBackupFilename.hpp"

using namespace geode::prelude;

class ProfilesListLayer : public CCLayer
{
private:
  ScrollLayer *m_scroll = nullptr;
  GJGameLevel *m_level = nullptr;
  EventListener<EventFilter<ProfilesChangedEvent>> m_listener;
  CCSize m_contentSize;

  std::vector<Profile> m_profiles;
  Profile m_currentProfile;

  void onCreate(CCObject *sender);
  void onImport(CCObject *sender);
  void onExport(CCObject *sender);

public:
  static ProfilesListLayer *create(
      GJGameLevel *level,
      std::vector<Profile> const &profiles,
      Profile const &current,
      const CCSize &contentSize);

  bool init(
      GJGameLevel *level,
      std::vector<Profile> const &profiles,
      Profile const &current,
      const CCSize &contentSize);

  void reload();
  void scrollToTop();
  ScrollLayer *getScrollLayer() const { return m_scroll; }
};
