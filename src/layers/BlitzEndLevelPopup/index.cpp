#include "./index.hpp"

void BlitzEndLevelLayer::customSetup() {
    EndLevelLayer::customSetup();
    auto level = PlayLayer::get()->m_level;
    m_fields->currentProfile = GlobalStore::get()->getProfileByLevel(level);

    auto hideLayerMenu = this->getChildByID("hide-layer-menu");

    auto modIcon = CCSprite::create("logo-circle.png"_spr);
    auto modBtn = CCMenuItemExt::createSpriteExtra(modIcon, [this](auto) { onPopup(); });
    modBtn->setScale(0.8f);
    modBtn->m_baseScale = modBtn->getScale();
    modBtn->setID("blitzkrieg-button");
    hideLayerMenu->addChild(modBtn);
    hideLayerMenu->updateLayout();
}

void BlitzEndLevelLayer::onExit() {
    m_fields->m_listener.destroy();
    EndLevelLayer::onExit();
}

void BlitzEndLevelLayer::onPopup() {
    const auto level = PlayLayer::get()->m_level;
    std::string levelId = geode::utils::numToString(EditorIDs::getID(level));

    StagesPopup::create(level)->show();
}
