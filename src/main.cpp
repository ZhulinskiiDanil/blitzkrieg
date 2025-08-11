#include <Geode/Geode.hpp>
#include <Geode/binding/FMODAudioEngine.hpp>
#include <Geode/binding/AchievementNotifier.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include <fmt/core.h>
#include <cvolton.level-id-api/include/EditorIDs.hpp>

#include "./utils/loadProfiles.hpp"

using namespace geode::prelude;
using json = nlohmann::json;

json serializeRange(const Range& range) {
    return {
        {"id", range.id},
        {"from", range.from},
        {"to", range.to},
        {"checked", range.checked},
        {"note", range.note},
        {"attempts", range.attempts},
        {"completionCounter", range.completionCounter}
    };
};

json serializeStage(const Stage& stage) {
    json jRanges = json::array();
    for (const auto& range : stage.ranges) {
        jRanges.push_back(serializeRange(range));
    }
    return {
        {"id", stage.id},
        {"stage", stage.stage},
        {"checked", stage.checked},
        {"note", stage.note},
        {"attempts", stage.attempts},
        {"completionCounter", stage.completionCounter},
        {"ranges", jRanges}
    };
};

json serializeProfileData(const ProfileData& data) {
    json jStages = json::array();
    for (const auto& stage : data.stages) {
        jStages.push_back(serializeStage(stage));
    }
    return {
        {"tags", data.tags},
        {"stages", jStages}
    };
};

json serializeProfile(const Profile& profile) {
    return {
        {"id", profile.id},
        {"profileName", profile.profileName},
        {"data", serializeProfileData(profile.data)}
    };
};


class $modify(MenuLayer) {
    bool init() {
        if (!MenuLayer::init())
            return false;

        return true;
    }
};


class $modify(PlayLayer) {

    struct Run {
        int start = 0;
        int end = 0;
    };

    struct Fields {
        CCObject* disabledCheat = nullptr;

        std::vector<Profile> profiles;  // все профили
        Profile* currentProfile = nullptr; // текущий профиль
        bool hasRespawned = false;
        bool isNoclip = false;
        Run currentRun;

        // Звуки закрытия ранов и стадий
        FMOD::Sound* rangeCompleteSound = nullptr;
        FMOD::Sound* stageCompleteSound = nullptr;
    };

public:
    bool init(GJGameLevel* level, bool p1, bool p2) {
        if (!PlayLayer::init(level, p1, p2)) return false;
        loadData();

        int levelId = EditorIDs::getID(level);
        geode::log::debug("Level ID: {}", levelId);

        // TODO: Do level selecting by user
        // Сейчас это будет работать только для уровня у которого в редакторе ID 1422
        // И только с The Yangire таблицей для меня, потом сделать нормально
        if (levelId == 1165) {
            m_fields->currentProfile = getProfileByName("The Yangire");
        }

        if (!m_fields->currentProfile) {
            geode::log::error("Profile The Yangire not found!");
            loadSounds();
        }

        return true;
    }

    void loadData() {
        auto resourcesDir = geode::Mod::get()->getResourcesDir();
        auto fullPath = (resourcesDir / "export.json").string();

        try {
            m_fields->profiles = loadProfiles(fullPath);
        } catch (const std::exception& e) {
            geode::log::error("Profile loading error: {}", e.what());
        }
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        geode::log::debug("LEVEL RESET");

        m_fields->hasRespawned = true;
        m_fields->currentRun.start = static_cast<int>(this->getCurrentPercent());

        if (!m_level->isPlatformer()) {
            geode::log::debug(
                "RESET LEVEL, RUN: start = {}, end = {}",
                m_fields->currentRun.start,
                m_fields->currentRun.end
            );
        }

        resetState();
    }

    void levelComplete() {
        PlayLayer::levelComplete();

        if (!m_fields->hasRespawned) return;
        m_fields->hasRespawned = false;

        if (!m_level->isPlatformer()) {
            m_fields->currentRun.end = 100;

            geode::log::debug("LEVEL COMPLETED");

            geode::log::debug(
                "LEVEL COMPLETED, RUN: start = {}, end = {}",
                m_fields->currentRun.start,
                m_fields->currentRun.end
            );

            checkRun(m_fields->currentRun);
        }

        resetState();
    }

    void destroyPlayer(PlayerObject* player, GameObject* p1) {
        // First object is anticheat, store it
        if (!m_fields->disabledCheat) {
            m_fields->disabledCheat = p1;
        }

        PlayLayer::destroyPlayer(player, p1);

        if (!m_fields->isNoclip && m_fields->disabledCheat != p1 && !player->m_isDead) {
            m_fields->isNoclip = true;
            geode::log::debug("UUPS... PLAYER STILL ALIVE");
        }

        if (!player->m_isDead) return;
        if (!m_fields->hasRespawned) return;
        m_fields->hasRespawned = false;

        if (!m_level->isPlatformer()) {
            m_fields->currentRun.end = this->getCurrentPercent();

            geode::log::debug("PLAYER DESTROYED, RUN: start = {}, end = {}",
                m_fields->currentRun.start, m_fields->currentRun.end);

            checkRun(m_fields->currentRun);
        }
    }

    void loadSounds() {
        auto resourcesDir = geode::Mod::get()->getResourcesDir();
        auto rangePath = (resourcesDir / "range_complete.mp3").string();
        auto stagePath = (resourcesDir / "stage_complete.mp3").string();

        auto system = FMODAudioEngine::sharedEngine()->m_system;

        if (!m_fields->rangeCompleteSound) {
            if (system->createSound(rangePath.c_str(), FMOD_DEFAULT, nullptr, &m_fields->rangeCompleteSound) != FMOD_OK) {
                geode::log::error("Failed to load range_complete.mp3");
            }
        }

        if (!m_fields->stageCompleteSound) {
            if (system->createSound(stagePath.c_str(), FMOD_DEFAULT, nullptr, &m_fields->stageCompleteSound) != FMOD_OK) {
                geode::log::error("Failed to load stage_complete.mp3");
            }
        }
    }

    void playSound(bool isStage) {
        FMOD::Sound* sound = isStage ? m_fields->stageCompleteSound : m_fields->rangeCompleteSound;
        if (!sound) return;

        FMOD::Channel* channel = nullptr;
        auto system = FMODAudioEngine::sharedEngine()->m_system;

        if (system->playSound(sound, nullptr, false, &channel) != FMOD_OK) {
            geode::log::error("Failed to play sound");
        }
    }

    void resetState() {
        geode::log::debug("RESET STATE");
        m_fields->isNoclip = false;
        m_fields->disabledCheat = nullptr;
    }

    Profile* getProfileByName(const std::string& name) {
        for (auto& profile : m_fields->profiles) {
            if (profile.profileName == name) {
                return &profile;
            }
        }

        return nullptr;
    }

    void saveData() {
        if (!m_fields->currentProfile) {
            geode::log::error("Current profile not set");
            return;
        }

        try {
            // Копируем вектор профилей
            auto newProfiles = m_fields->profiles; // копия

            // Находим индекс профиля с таким же именем
            auto it = std::find_if(newProfiles.begin(), newProfiles.end(), [&](const Profile& p) {
                return p.profileName == m_fields->currentProfile->profileName;
            });

            if (it != newProfiles.end()) {
                // Удаляем старый профиль
                newProfiles.erase(it);
            }

            // Добавляем обновленный currentProfile
            newProfiles.push_back(*m_fields->currentProfile);

            // Сериализуем newProfiles в JSON
            nlohmann::json j = nlohmann::json::array();
            for (auto& profile : newProfiles) {
                j.push_back(serializeProfile(profile));
            }

            // Записываем в файл
            auto resourcesDir = geode::Mod::get()->getResourcesDir();
            auto fullPath = (resourcesDir / "export.json").string();

            std::ofstream file(fullPath);
            if (!file.is_open()) {
                geode::log::error("Failed to open export.json for writing");
                return;
            }
            file << j.dump(4);
            file.close();

            geode::log::info("Saved profiles to export.json");

            // Обновляем m_fields->profiles на новую копию, чтобы в памяти тоже было актуально
            m_fields->profiles = newProfiles;

        } catch (const std::exception& e) {
            geode::log::error("Error saving profiles: {}", e.what());
        }
    }

    bool isLegal() {
        // // Mega Hack indicator adress 0x1b59b581c90
        // uintptr_t addr = 0x1b59b581c90;

        // if (addr == 0) {
        //     geode::log::debug("Address is null!");
        //     return false;
        // }

        // CCLabelBMFont* ci = nullptr;

        // // Лучше получить детей в вектор до начала перебора
        // auto childrenCopy = CCArrayExt<CCNode>(m_uiLayer->getChildren());

        // for (auto* node : childrenCopy) {
        //     if (!node) continue;
        //     if (node->getTag() != 4326) continue;
        //     if (auto lbl = typeinfo_cast<CCLabelBMFont*>(node)) {
        //         auto text = std::string_view(lbl->getString());
        //         if (text == ".") {
        //             ci = lbl;
        //             break;
        //         }
        //     }
        // }

        // geode::log::debug("CHECK IF LEGAL");

        // if (ci) {
        //     auto ciChildren = CCArrayExt<CCNode>(ci->getChildren());
        //     if (auto indicator = typeinfo_cast<CCFontSprite*>(ciChildren.front())) {
        //         auto color = indicator->getColor();
        //         geode::log::debug("Got CCFontSprite with color: r={}, g={}, b={}", color.r, color.g, color.b);
        //     } else {
        //         geode::log::warn("First child is not a CCFontSprite");
        //     }
        // } else {
        //     geode::log::debug("ci NOT FOUND");
        // }

        // auto obj = reinterpret_cast<CCNode*>(0x1b59b13e540);

        // try {
        //     geode::log::debug("X: {}, Y: {}", obj->getPositionX(), obj->getPositionY());
        // } catch (...) {
        //     geode::log::error("Exception caught while accessing object!");
        //     return false;
        // }

        return !m_fields->isNoclip;
    }

    void checkRun(const Run& run) {
        if (!isLegal()) {
            return;
        }

        if (!m_fields->currentProfile) {
            geode::log::error("Current profile not set");
            return;
        }

        bool canPlaySound = false;
        bool isStageClosed = false;
        int runStart = static_cast<int>(std::floor(run.start));
        int runEnd   = static_cast<int>(std::floor(run.end));

        geode::log::debug("CHECKING RUNG: {}-{} / IS LEGAL: {}", runStart, runEnd, isLegal());

        int totalStages = m_fields->currentProfile->data.stages.size();

        // Идем по стадиям по порядку, пока не найдем незакрытую
        for (auto& stage : m_fields->currentProfile->data.stages) {
            // Если стадия уже закрыта, переходим к следующей
            if (stage.checked) continue;

            bool checkedRangeThisRun = false;

            // Собираем все подходящие range, покрываемые текущим run
            std::vector<Range*> candidates;

            for (auto& range : stage.ranges) {
                if (range.checked) continue;

                if (runStart <= range.from && runEnd >= range.to) {
                    candidates.push_back(&range);
                }
            }

            if (!candidates.empty()) {
                // Выбираем range с минимальным 'from'
                auto* toCheck = *std::min_element(candidates.begin(), candidates.end(),
                    [](Range* a, Range* b) { return a->from < b->from; });

                std::string fromTo = fmt::format(
                    "Stage {}/{}: {} - {}",
                    stage.stage,
                    totalStages,
                    toCheck->from,
                    toCheck->to
                );

                AchievementNotifier::sharedState()->notifyAchievement(
                    "New run closed",
                    fromTo.c_str(),
                    "GJ_completesIcon_001.png",
                    true
                );

                toCheck->checked = true;
                canPlaySound = true;
                isStageClosed = false;
                checkedRangeThisRun = true;
                saveData();

                geode::log::debug("CHECKED RANGE {}-{}", toCheck->from, toCheck->to);
            }

            // Если закрыли range, проверяем закрыта ли теперь вся стадия
            if (checkedRangeThisRun) {
                bool allChecked = std::all_of(stage.ranges.begin(), stage.ranges.end(), [](const Range& r) {
                    return r.checked;
                });

                if (allChecked && !stage.checked) {
                    stage.checked = true;
                    geode::log::debug("Stage closed");
                    canPlaySound = true;
                    isStageClosed = true;
                }

                break;  // Выходим из цикла стадий, не закрываем следующие стадии пока не закроется эта
            }
            else {
                // Если не закрыли ни одного range в этой стадии, значит дальше закрывать нельзя
                break;
            }
        }

        if (canPlaySound) {
            // true если нужно проиграть звук стейджа
            // false если нужно проиграть звук range
            playSound(isStageClosed);
        }
    }
};