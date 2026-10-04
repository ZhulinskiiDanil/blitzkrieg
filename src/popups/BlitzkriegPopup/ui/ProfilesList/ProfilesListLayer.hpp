#pragma once
#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>

#include "./BlitzkriegProfile.hpp"
#include "./WhiteListExport/WhiteListExport.hpp"
#include "../../../CreateProfilePopup/index.hpp"

#include "../../../../ui/RectNode.hpp"
#include "../../../../events/ProfileChangedEvent.hpp"
#include "../../../../events/ProfilesChangedEvent.hpp"
#include "../../../../serialization/profile/index.hpp"
#include "../../../../store/GlobalStore.hpp"
#include "../../../../utils/generateProfile.hpp"
#include "../../../../utils/selectJsonFile.hpp"
#include "../../../../utils/findStartposesFromCurrentLevel.hpp"

using namespace geode::prelude;

class ProfilesListLayer : public CCLayer
{
private:
  ScrollLayer *m_scroll = nullptr;
  GJGameLevel *m_level = nullptr;
  CCNode *m_emptyState = nullptr;

  // Profiles list changed: added, removed, pinned, moved, edited
  ListenerHandle m_profilesListener;
  // Linked profile of the current level changed
  ListenerHandle m_profileListener;

  CCSize m_contentSize;

  std::vector<Profile> m_profiles;

  bool m_reloadQueued = false;
  bool m_queuedKeepScroll = true;

  void onCreate(CCObject *sender);
  void onImport(CCObject *sender);
  void onExport(CCObject *sender);

  void createEmptyState();

  // Reloads profiles from the store on the next frame.
  // Several requests in one frame result in a single reload.
  // Rows are never destroyed while their own button callback is running.
  void queueReload(bool keepScroll = true);

public:
  static ProfilesListLayer *create(
      GJGameLevel *level,
      std::vector<Profile> const &profiles,
      const CCSize &contentSize);

  bool init(
      GJGameLevel *level,
      std::vector<Profile> const &profiles,
      const CCSize &contentSize);

  // keepScroll keeps the distance from the top of the list
  void reload(bool keepScroll = false);
  void scrollToTop();
  ScrollLayer *getScrollLayer() const { return m_scroll; }
};
