#include "./index.hpp"

void BlitzEndLevelLayer::customSetup()
{
    EndLevelLayer::customSetup();
    auto hideLayerMenu = this->getChildByID("hide-layer-menu");
    auto modCircleLogo = CCSprite::create("logo-circle.png"_spr);
    auto modBtn = CCMenuItemExt::createSpriteExtra(modCircleLogo, [this](auto)
                                                   { onPopup(); });
    modBtn->setScale(0.8f);
    modBtn->m_baseScale = modBtn->getScale();

    modBtn->setID("blitzkrieg-button");
    hideLayerMenu->addChild(modBtn);
    hideLayerMenu->updateLayout();
}

void BlitzEndLevelLayer::onPopup()
{
    auto level = PlayLayer::get()->m_level;
    std::string levelId = level->m_levelID ? utils::numToString(level->m_levelID.value()) : utils::numToString(EditorIDs::getID(level));

    StagesPopup::create(level)->show();
}
