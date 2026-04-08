#include "./index.hpp"

bool BlitzLevelInfoLayer::init(GJGameLevel *level, bool challenge)
{
    if (!LevelInfoLayer::init(level, challenge))
        return false;

    auto practiceModeBar = this->getChildByID("practice-mode-bar");
    auto practiceModeLabel = this->getChildByID("practice-mode-label");
    auto practiceModePercentage = this->getChildByID("practice-mode-percentage");

    m_fields->m_startPosSprite = CircleButtonSprite::createWithSprite("startpos-button.png"_spr);

    m_fields->m_startposBtn = CCMenuItemSpriteExtra::create(m_fields->m_startPosSprite, this, menu_selector(BlitzLevelInfoLayer::onOpenStartPos));
    m_fields->m_startposBtn->setID("startpos-btn"_spr);
    m_fields->m_startPosSprite->setVisible(false);
    m_fields->m_startposBtn->setVisible(false);

    auto menu = this->getChildByIDRecursive("left-side-menu");
    menu->addChild(m_fields->m_startposBtn);
    menu->updateLayout();

    replacePracticeBarWithProgression(utils::numToString(level->m_levelID.value()), practiceModeBar, practiceModeLabel, practiceModePercentage);
    loadStartPosLevel();

    return true;
}

void BlitzLevelInfoLayer::loadStartPosLevel()
{
    auto levelId = m_level->m_levelID;
    std::string reqUrl = fmt::format("{}/levels/originalId:{}", API_URL, levelId);

    m_fields->m_listener.spawn(web::WebRequest().get(reqUrl),
                               [this](web::WebResponse value)
                               {
                                   if (value.ok())
                                   {
                                       auto data = value.json();
                                       auto json = data.ok().value();
                                       auto res = json.as<StartPosLevel>();

                                       if (!res.isErr())
                                       {
                                           auto resUnwraped = res.unwrap();

                                           if (resUnwraped.originalId > 0)
                                           {
                                               m_fields->m_startposLevel = new StartPosLevel(resUnwraped);
                                               m_fields->m_startPosSprite->setVisible(true);
                                               m_fields->m_startposBtn->setVisible(true);

                                               auto menu = this->getChildByIDRecursive("left-side-menu");
                                               menu->updateLayout();
                                           }
                                       }
                                       else
                                       {
                                           log::warn("Parse error: {}", res.unwrapErr());
                                       }
                                   }
                                   else
                                   {
                                       log::warn("Error while loading levels: {}", value.errorMessage());
                                   }
                               });
}

void BlitzLevelInfoLayer::onOpenStartPos(CCObject *sender)
{
    if (!m_fields->m_startposLevel || m_fields->m_startposLevel->levelId <= 0)
        return;

    auto levelBrowserLayer = LevelBrowserLayer::create(GJSearchObject::create(
        SearchType::Search,
        fmt::format("{}", m_fields->m_startposLevel->levelId)));
    auto scene = CCScene::create();
    scene->addChild(levelBrowserLayer);
    CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(.5f, scene));
}