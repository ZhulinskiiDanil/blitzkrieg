#include "./index.hpp"

void BlitzEndLevelLayer::customSetup()
{
    EndLevelLayer::customSetup();
    auto hideLayerMenu = this->getChildByID("hide-layer-menu");
    m_fields->m_logoSpr = CCSprite::create("logo-circle.png"_spr);
    m_fields->m_logoSpr->setOpacity(255 * .5f);
    m_fields->m_modBtn = CCMenuItemExt::createSpriteExtra(m_fields->m_logoSpr, [this](auto)
                                                          { onPopup(); });
    m_fields->m_modBtn->setScale(0.8f);
    m_fields->m_modBtn->m_baseScale = m_fields->m_modBtn->getScale();

    m_fields->m_modBtn->setID("blitzkrieg-button");
    hideLayerMenu->addChild(m_fields->m_modBtn);
    hideLayerMenu->updateLayout();
}

void BlitzEndLevelLayer::onPopup()
{
    auto level = PlayLayer::get()->m_level;
    std::string levelId = level->m_levelID ? utils::numToString(level->m_levelID.value()) : utils::numToString(EditorIDs::getID(level));

    StagesPopup::create(level)->show();
}
