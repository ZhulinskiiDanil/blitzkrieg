#include "./index.hpp"
#include "../../layers/DTLayer/index.hpp"
#include "../../managers/DTPopupManager/index.hpp"

void DTPauseLayer::customSetup() {
    PauseLayer::customSetup();

    auto sideMenu = this->getChildByID("left-button-menu");

    auto s = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
    auto s2 = CCSprite::createWithSpriteFrameName("miniSkull_001.png");
    s2->setPosition(s->getContentSize() / 2);
    s->addChild(s2);
    s->setScale(0.75f);
    auto button = CCMenuItemSpriteExtra::create(
        s,
        nullptr,
        this,
        menu_selector(DTPauseLayer::onDTMiniMenuOpened)
    );

    button->setID("dt-skull-button");
    sideMenu->addChild(button);
    sideMenu->updateLayout();
}

void DTPauseLayer::onDTMiniMenuOpened(CCObject*) {
    auto dtLayer = DTLayer::create(DTPopupManager::getCurrentLevel());
    this->addChild(dtLayer);
}