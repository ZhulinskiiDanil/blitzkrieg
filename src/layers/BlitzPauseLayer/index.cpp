#include "./index.hpp"

void BlitzPauseLayer::customSetup()
{
    PauseLayer::customSetup();
    auto level = PlayLayer::get()->m_level;
    m_fields->currentProfile = GlobalStore::get()->getProfileByLevel(level);

    auto leftMenu = this->getChildByID("left-button-menu");

    auto modIcon = CCSprite::create("logo-circle.png"_spr);
    auto modBtnSpr = CircleButtonSprite::create(modIcon);
    modBtnSpr->setScale(0.75f);

    auto modBtn = CCMenuItemExt::createSpriteExtra(modBtnSpr, [this](auto)
                                                   { onPopup(); });
    modBtn->setID("blitzkrieg-button");
    leftMenu->addChild(modBtn);
    leftMenu->updateLayout();

    if (!m_fields->currentProfile.id.empty())
        updateCurrentProfileLabel(m_fields->currentProfile);

    m_fields->m_listener = ProfilesChangedEvent().listen([this]()
                                                         {
            auto level = PlayLayer::get()->m_level;
            m_fields->currentProfile = GlobalStore::get()->getProfileByLevel(level);
            updateCurrentProfileLabel(m_fields->currentProfile);

            return ListenerResult::Propagate; });
}

void BlitzPauseLayer::onQuit(CCObject *sender)
{
    m_fields->m_listener.destroy();
    PauseLayer::onQuit(sender);
}

void BlitzPauseLayer::updateCurrentProfileLabel(Profile currentProfile)
{
    auto bg = this->getChildByID("background");

    if (auto old = this->getChildByID("bk-profileLabel"_spr))
        old->removeFromParentAndCleanup(true);
    if (m_fields->currentProfile.id.empty())
        return;

    const auto stagesMetaInfo = getMetaInfoFromStages(m_fields->currentProfile.data.stages);
    std::string profileString = fmt::format(
        "Stage: {}/{}",
        geode::utils::numToString(stagesMetaInfo.completed + 1),
        geode::utils::numToString(stagesMetaInfo.total));

    auto profileLabel = CCLabelBMFont::create(profileString.c_str(), "goldFont.fnt");
    profileLabel->setScale(0.4f);
    profileLabel->setAnchorPoint({0.5f, 0.f});
    profileLabel->setPosition({bg->getContentWidth() / 2, bg->getPositionY() - bg->getContentHeight() / 2 + 8.f});
    profileLabel->setOpacity(255.0f * 0.75f);
    profileLabel->setID("bk-profileLabel"_spr);

    this->addChild(profileLabel);
}

void BlitzPauseLayer::onPopup()
{
    const auto level = PlayLayer::get()->m_level;
    std::string levelId = geode::utils::numToString(EditorIDs::getID(level));

    StagesPopup::create(level)->show();
}
