#include <string>
#include <iostream>

#include <Geode/modify/PauseLayer.hpp>
#include <Geode/loader/Event.hpp>

#include "../../popups/StagesPopup/ProfilesChangedEvent.hpp"
#include "../../popups/StagesPopup/index.hpp"

#include "../../store/GlobalStore.hpp"
#include "../../serialization/profile/index.hpp"
#include "../../utils/selectJsonFile.hpp"

using namespace geode::prelude;

// class $modify(DTPauseLayer, PauseLayer), public CreateFolderDelegate {
class $modify(DTPauseLayer, PauseLayer)
{
  struct Fields
  {
    Profile currentProfile;
    // EventListener<EventFilter<ProfilesChangedEvent>>
    ListenerHandle m_listener;
  };

private:
  void updateCurrentProfileLabel(Profile currentProfile);
  void onPopup(CCObject *);

public:
  void customSetup();
  void onQuit(CCObject *);
};
