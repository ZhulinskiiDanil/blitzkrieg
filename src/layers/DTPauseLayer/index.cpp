#include "./index.hpp"

void DTPauseLayer::customSetup()
{
    PauseLayer::customSetup();
    auto level = PlayLayer::get()->m_level;
    m_fields->currentProfile = GlobalStore::get()->getProfileByLevel(level);

    auto sideMenu = this->getChildByID("left-button-menu");

    auto spr = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
    auto spr2 = CCSprite::create("logo.png"_spr);
    spr2->setPosition(spr->getContentSize() / 2);
    spr2->setAnchorPoint({.5f, .5f});
    spr2->setScale(.85f);
    spr->setScale(.75f);
    spr->addChild(spr2);
    auto button = CCMenuItemSpriteExtra::create(
        spr,
        nullptr,
        this,
        menu_selector(DTPauseLayer::onPopup));

    if (!m_fields->currentProfile.id.empty())
    {
        updateCurrentProfileLabel(m_fields->currentProfile);
    }

    button->setID("dt-skull-button");
    sideMenu->addChild(button);
    sideMenu->updateLayout();

    m_fields->m_listener = ProfilesChangedEvent().listen(
        [this]()
        {
            auto level = PlayLayer::get()->m_level;
            m_fields->currentProfile = GlobalStore::get()->getProfileByLevel(level);
            updateCurrentProfileLabel(m_fields->currentProfile);

            return ListenerResult::Propagate;
        });
    m_fields->m_listener.leak();
}

void DTPauseLayer::onQuit(CCObject *sender)
{
    m_fields->m_listener.destroy();
    PauseLayer::onQuit(sender);
}

void DTPauseLayer::updateCurrentProfileLabel(Profile currentProfile)
{

    auto bg = this->getChildByID("background");

    if (auto old = this->getChildByID("bk-profileLabel"_spr))
    {
        old->removeFromParentAndCleanup(true);
    }

    if (m_fields->currentProfile.id.empty())
        return;

    const auto stats = getProfileStats(m_fields->currentProfile);
    std::string profileString =
        m_fields->currentProfile.profileName + " " +
        (stats.currentStage ? geode::utils::numToString(*stats.currentStage) : "0") + "/" +
        geode::utils::numToString(stats.totalStages);

    // Создаём новую
    auto profileLabel = CCLabelBMFont::create(profileString.c_str(), "goldFont.fnt");
    profileLabel->setScale(0.4f);
    profileLabel->setAnchorPoint({0.5f, 0.f});
    profileLabel->setPosition({bg->getContentWidth() / 2,
                               bg->getPositionY() - bg->getContentHeight() / 2 + 8.f});
    profileLabel->setOpacity(255 * 0.75f);
    profileLabel->setID("bk-profileLabel"_spr);

    this->addChild(profileLabel);
}

void DTPauseLayer::onPopup(CCObject *)
{
    const auto level = PlayLayer::get()->m_level;

    std::string levelId = geode::utils::numToString(EditorIDs::getID(level));

    StagesPopup::create(level)
        ->show();
}
