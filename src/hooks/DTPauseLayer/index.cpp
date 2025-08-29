#include "./index.hpp"

void DTPauseLayer::customSetup()
{
    PauseLayer::customSetup();
    auto level = PlayLayer::get()->m_level;
    m_fields->currentProfile = getProfileByLevel(level);

    auto sideMenu = this->getChildByID("left-button-menu");

    auto s = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
    auto s2 = CCSprite::create("logo.png"_spr);
    s2->setPosition(s->getContentSize() / 2);
    s->addChild(s2);
    s->setScale(0.75f);
    auto button = CCMenuItemSpriteExtra::create(
        s,
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

    m_fields->m_listener = EventListener<EventFilter<ProfilesChangedEvent>>(
        [this](ProfilesChangedEvent *)
        {
            auto level = PlayLayer::get()->m_level;
            m_fields->currentProfile = getProfileByLevel(level);
            updateCurrentProfileLabel(m_fields->currentProfile);

            geode::log::info("PROFILES CHANGED, UPDATE UI");
            return ListenerResult::Propagate;
        });
}

void DTPauseLayer::onQuit(CCObject *sender)
{
    geode::log::debug("DTPauseLayer QUIT");
    m_fields->m_listener.disable();
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

    StagesPopup::create(level)
        ->show();
}