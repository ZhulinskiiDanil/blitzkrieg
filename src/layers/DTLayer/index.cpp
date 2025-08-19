#include "index.hpp"
#include <Geode/ui/GeodeUI.hpp>

DTLayer *DTLayer::create(GJGameLevel *const &Level)
{
    auto ret = new DTLayer();
    if (ret && ret->initAnchored(368, 280, Level, "square01_001.png", {0.f, 0.f, 94.f, 94.f}))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
};

bool DTLayer::setup(GJGameLevel *const &level)
{
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    m_Level = level;

    geode::log::debug("Level ID: {}", level->getID());

    this->setID("dt-layer-blitzkrieg");

    this->setZOrder(100);
    this->m_buttonMenu->setZOrder(1);
    m_buttonMenu->setPosition(m_size);
    m_closeBtn->setPosition(m_closeBtn->getPosition() - m_size / 2);
    m_mainLayer->setZOrder(1);

    alignmentNode = CCNode::create();
    alignmentNode->setPosition(m_size / 2);
    alignmentNode->setZOrder(2);
    m_mainLayer->addChild(alignmentNode);

    alignmentNode->setID("ALIGNMENT-NODE");

    auto sideBG = CCScale9Sprite::create("square01_001.png", {0, 0, 94, 94});
    sideBG->setContentSize({154, 145});
    sideBG->setPosition(alignmentNode->getPosition() + ccp(233.5f, 10));
    sideBG->setScale(0.65f);
    m_mainLayer->addChild(sideBG);

    sideBG->setID("SIDE-BG");

    this->scheduleUpdate();

    return true;
}

void DTLayer::onClose(CCObject *)
{
    this->setKeypadEnabled(false);
    this->setTouchEnabled(false);
    this->removeFromParentAndCleanup(true);
}