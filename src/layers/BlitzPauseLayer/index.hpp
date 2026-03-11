#include <Geode/modify/PauseLayer.hpp>
#include <Geode/loader/Event.hpp>

#include "../../popups/StagesPopup/index.hpp"

#include "../../utils/selectJsonFile.hpp"
#include "../../utils/getMetaInfoFromStages.hpp"
#include "../../serialization/profile/index.hpp"
#include "../../store/GlobalStore.hpp"

using namespace geode::prelude;

class $modify(BlitzPauseLayer, PauseLayer)
{
    struct Fields
    {
        ListenerHandle m_listener;
    };

private:
    void onPopup();

public:
    void customSetup();
    void onQuit(CCObject *);
};
