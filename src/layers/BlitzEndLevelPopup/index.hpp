#include <Geode/modify/EndLevelLayer.hpp>
#include <Geode/loader/Event.hpp>

#include "../../popups/StagesPopup/ProfilesChangedEvent.hpp"
#include "../../popups/StagesPopup/index.hpp"

#include "../../store/GlobalStore.hpp"
#include "../../serialization/profile/index.hpp"
#include "../../utils/selectJsonFile.hpp"

using namespace geode::prelude;

class $modify(BlitzEndLevelLayer, EndLevelLayer) {
    struct Fields {
        Profile currentProfile;
        ListenerHandle m_listener;
    };

private:
     void onPopup();

public:
     void customSetup();
     void onExit();
};
