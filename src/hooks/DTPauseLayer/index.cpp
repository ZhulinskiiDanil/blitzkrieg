#include <iostream>

#include "./index.hpp"
#include "../../layers/DTLayer/index.hpp"
#include "../../managers/DTPopupManager/index.hpp"

#include "../../serialization/profile/index.hpp"
#include "../../utils/selectJsonFile.hpp"
#include "../../utils/saveProfiles.hpp"

void DTPauseLayer::customSetup()
{
    PauseLayer::customSetup();

    auto sideMenu = this->getChildByID("left-button-menu");

    auto s = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
    // auto s2 = CCSprite::createWithSpriteFrameName("miniSkull_001.png");
    // s2->setPosition(s->getContentSize() / 2);
    // s->addChild(s2);
    s->setScale(0.75f);
    auto button = CCMenuItemSpriteExtra::create(
        s,
        nullptr,
        this,
        menu_selector(DTPauseLayer::onPopup));

    button->setID("dt-skull-button");
    sideMenu->addChild(button);
    sideMenu->updateLayout();
}

void DTPauseLayer::onPopup(CCObject *)
{
    const auto level = PlayLayer::get()->m_level;

    StagesPopup::create(level)
        ->show();
}