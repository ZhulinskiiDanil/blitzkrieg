#include "./index.hpp"
#include <Geode/modify/LevelSearchLayer.hpp>

class $modify(MyLevelSearchLayer, LevelSearchLayer) {
    bool init(int type) {
        if (!LevelSearchLayer::init(type)) return false;

        auto filterMenu = this->getChildByID("other-filter-menu");
        
        auto startposBtnSpr = CircleButtonSprite::createWithSpriteFrameName("startpos-button.png"_spr);
        startposBtnSpr->setScale(0.8f);
        auto startposBtn = CCMenuItemSpriteExtra::create(startposBtnSpr, this, menu_selector(MyLevelSearchLayer::onStartposBtn));
        startposBtn->setID("startpos-button");
        filterMenu->addChild(startposBtn);
        filterMenu->updateLayout();

        return true;
    }

    void onStartposBtn(CCObject* sender) {
        CCDirector::get()->pushScene(CCTransitionFade::create(0.5f, StartposLayer::scene()));
    }
};