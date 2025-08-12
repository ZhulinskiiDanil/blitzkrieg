#include <iostream>

#include "./index.hpp"
#include "../../layers/DTLayer/index.hpp"
#include "../../managers/DTPopupManager/index.hpp"

#include "../../serialization/profile/index.hpp"
#include "../../utils/selectJsonFile.hpp"
#include "../../utils/saveProfiles.hpp"

void DTPauseLayer::customSetup()
{
    PauseLayer::customSetup();

    auto sideMenu = this->getChildByID("left-button-menu");

    auto s = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
    // auto s2 = CCSprite::createWithSpriteFrameName("miniSkull_001.png");
    // s2->setPosition(s->getContentSize() / 2);
    // s->addChild(s2);
    s->setScale(0.75f);
    auto button = CCMenuItemSpriteExtra::create(
        s,
        nullptr,
        this,
        menu_selector(DTPauseLayer::onImport));

    button->setID("dt-skull-button");
    sideMenu->addChild(button);
    sideMenu->updateLayout();
}

void DTPauseLayer::onPopup(CCObject *)
{
    geode::log::debug("Popup");
    StagesPopup::create()->show();
}

void DTPauseLayer::onImport(CCObject *)
{
    selectJsonFile([this](std::string jsonContent)
                   {
        if (jsonContent.empty()) {
            geode::log::debug("Файл не выбран или пуст");
            return;
        }

        try {
            json parsed = json::parse(jsonContent);

            if (!parsed.is_array()) {
                geode::log::debug("Ошибка: ожидается массив профилей");
                return;
            }

            std::vector<Profile> profiles = parsed.get<std::vector<Profile>>();
            geode::log::debug("Загружено {} профилей", profiles.size());

            saveProfiles(profiles);
        }
        catch (const json::parse_error& e) {
            geode::log::debug("Ошибка парсинга JSON: {}", e.what());
        }
        catch (const json::type_error& e) {
            geode::log::debug("Ошибка типов JSON: {}", e.what());
        }
        catch (const std::exception& e) {
            geode::log::debug("Другая ошибка: {}", e.what());
        } });
}
