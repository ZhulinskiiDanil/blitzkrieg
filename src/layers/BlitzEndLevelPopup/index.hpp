#include <Geode/modify/EndLevelLayer.hpp>

#include "../../popups/StagesPopup/index.hpp"
#include "../../utils/selectJsonFile.hpp"

using namespace geode::prelude;

class $modify(BlitzEndLevelLayer, EndLevelLayer)
{
    struct Fields
    {
        CCSprite *m_logoSpr;
        CCMenuItemSpriteExtra *m_modBtn;
    };

private:
    void onPopup();

public:
    void customSetup();

    void onHideLayer(CCObject *sender)
    {
        EndLevelLayer::onHideLayer(sender);
        m_fields->m_modBtn->setVisible(!m_hidden);
    }
};
