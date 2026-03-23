#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>

#include "../../config.hpp"
#include "../../serialization/startpos/index.hpp"
#include "../../utils/ui/replacePracticeBarWithProgression.hpp"

using namespace geode::prelude;

class $modify(BlitzLevelInfoLayer, LevelInfoLayer)
{
    struct Fields
    {
        CircleButtonSprite *m_startPosSprite;
        StartPosLevel *m_startposLevel;
        CCMenuItemSpriteExtra *m_startposBtn;
        TaskHolder<web::WebResponse> m_listener;
    };

private:
    void loadStartPosLevel();
    void onOpenStartPos(CCObject *sender);

public:
    bool init(GJGameLevel *level, bool challenge);
};
