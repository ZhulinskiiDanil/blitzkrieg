#include "./index.hpp"

void BlitzPauseLayer::customSetup()
{
    PauseLayer::customSetup();
    auto level = PlayLayer::get()->m_level;
    auto leftMenu = this->getChildByID("left-button-menu");

    auto modCircleLogo = CCSprite::create("logo-circle.png"_spr);
    auto modBtn = CCMenuItemExt::createSpriteExtra(modCircleLogo, [this](auto)
                                                   { onPopup(); });
    modBtn->setScale(0.8f);
    modBtn->m_baseScale = modBtn->getScale();

    modBtn->setID("blitzkrieg-button");
    leftMenu->addChild(modBtn);
    leftMenu->updateLayout();

    auto practiceModeBar = this->getChildByID("practice-progress-bar");
    auto practiceModeLabel = this->getChildByID("practice-mode-label");
    auto practiceModePercentage = this->getChildByID("practice-progress-label");

    std::string levelId = level->m_levelID ? geode::utils::numToString(level->m_levelID.value()) : geode::utils::numToString(EditorIDs::getID(level));
    replacePracticeBarWithProgression(levelId, practiceModeBar, practiceModeLabel, practiceModePercentage);
}

void BlitzPauseLayer::onQuit(CCObject *sender)
{
    m_fields->m_listener.destroy();
    PauseLayer::onQuit(sender);
}

void BlitzPauseLayer::onPopup()
{
    const auto level = PlayLayer::get()->m_level;
    StagesPopup::create(level)->show();
}
