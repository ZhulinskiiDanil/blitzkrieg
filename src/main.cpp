#include "./serialization/profile/index.hpp"

#include <fmt/core.h>

#include <Geode/Geode.hpp>
#include <Geode/binding/FMODAudioEngine.hpp>
#include <Geode/binding/AchievementNotifier.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include "./serialization/profile/index.hpp"
#include "./utils/getProfiles.hpp"
#include "./utils/saveProfile.hpp"
#include "./utils/getProfileByLevel.hpp"

using namespace geode::prelude;

struct Run
{
    int start = 0;
    int end = 0;
};

class $modify(MenuLayer)
{
    bool init()
    {
        if (!MenuLayer::init())
            return false;

        return true;
    }
};

class $modify(DTPlayLayer, PlayLayer)
{

    struct Fields
    {
        CCObject *disabledCheat = nullptr;

        std::vector<Profile> profiles; // Все профили
        bool hasRespawned = false;
        bool isNoclip = false;
        Run currentRun;

        // Звуки закрытия ранов и стадий
        FMOD::Sound *rangeCompleteSound = nullptr;
        FMOD::Sound *stageCompleteSound = nullptr;
    };

public:
    bool init(GJGameLevel *level, bool p1, bool p2)
    {
        if (!PlayLayer::init(level, p1, p2))
            return false;
        loadData();
        // loadSounds();

        return true;
    }

    void loadData()
    {
        m_fields->profiles = getProfiles();
    }

    void resetLevel()
    {
        PlayLayer::resetLevel();
        geode::log::debug("LEVEL RESET");

        m_fields->hasRespawned = true;
        m_fields->currentRun.start = static_cast<int>(this->getCurrentPercent());

        if (!m_level->isPlatformer())
        {
            geode::log::debug(
                "RESET LEVEL, RUN: start = {}, end = {}",
                m_fields->currentRun.start,
                m_fields->currentRun.end);
        }

        resetState();
    }

    void levelComplete()
    {
        PlayLayer::levelComplete();

        if (!m_fields->hasRespawned)
            return;
        m_fields->hasRespawned = false;

        if (!m_level->isPlatformer())
        {
            m_fields->currentRun.end = 100;

            geode::log::debug("LEVEL COMPLETED");

            geode::log::debug(
                "LEVEL COMPLETED, RUN: start = {}, end = {}",
                m_fields->currentRun.start,
                m_fields->currentRun.end);

            checkRun(m_fields->currentRun);
        }

        resetState();
    }

    void destroyPlayer(PlayerObject *player, GameObject *p1)
    {
        // First object is anticheat, store it
        if (!m_fields->disabledCheat)
        {
            m_fields->disabledCheat = p1;
        }

        PlayLayer::destroyPlayer(player, p1);

        if (!m_fields->isNoclip && m_fields->disabledCheat != p1 && !player->m_isDead)
        {
            m_fields->isNoclip = true;
            geode::log::debug("UUPS... PLAYER STILL ALIVE");
        }

        if (!player->m_isDead)
            return;
        if (!m_fields->hasRespawned)
            return;
        m_fields->hasRespawned = false;

        if (!m_level->isPlatformer())
        {
            m_fields->currentRun.end = this->getCurrentPercent();

            geode::log::debug("PLAYER DESTROYED, RUN: start = {}, end = {}",
                              m_fields->currentRun.start, m_fields->currentRun.end);

            checkRun(m_fields->currentRun);
        }
    }

    void playSound(bool isStage)
    {
        if (isStage)
            FMODAudioEngine::sharedEngine()->playEffect("stage_complete.mp3"_spr);
        else
            FMODAudioEngine::sharedEngine()->playEffect("range_complete.mp3"_spr);
    }

    void resetState()
    {
        geode::log::debug("RESET STATE");
        m_fields->isNoclip = false;
        m_fields->disabledCheat = nullptr;
    }

    Profile *getProfileByName(const std::string &name)
    {
        for (auto &profile : m_fields->profiles)
        {
            if (profile.profileName == name)
            {
                return &profile;
            }
        }

        return nullptr;
    }

    bool isLegal()
    {
        return !m_fields->isNoclip;
    }

    void checkRun(const Run &run)
    {
        auto currentProfile = getProfileByLevel(DTPlayLayer::get()->m_level);

        if (!isLegal())
        {
            geode::log::error("Not legal run");
            return;
        }

        if (currentProfile.id.empty())
        {
            geode::log::error("Current profile not set");
            return;
        }

        bool canPlaySound = false;
        bool isStageClosed = false;
        int runStart = static_cast<int>(std::floor(run.start));
        int runEnd = static_cast<int>(std::floor(run.end));

        geode::log::debug("CHECKING RUNG: {}-{} / IS LEGAL: {}", runStart, runEnd, isLegal());

        int totalStages = currentProfile.data.stages.size();

        // Идем по стадиям по порядку, пока не найдем незакрытую
        for (auto &stage : currentProfile.data.stages)
        {
            // Если стадия уже закрыта, переходим к следующей
            if (stage.checked)
                continue;

            bool checkedRangeThisRun = false;

            // Собираем все подходящие range, покрываемые текущим run
            std::vector<Range *> candidates;

            for (auto &range : stage.ranges)
            {
                if (range.checked)
                    continue;

                if (runStart <= range.from && runEnd >= range.to)
                {
                    candidates.push_back(&range);
                }
            }

            if (!candidates.empty())
            {
                // Выбираем range с минимальным 'from'
                auto *toCheck = *std::min_element(candidates.begin(), candidates.end(),
                                                  [](Range *a, Range *b)
                                                  { return a->from < b->from; });

                std::string fromTo = fmt::format(
                    "Stage {}/{}: {} - {}",
                    stage.stage,
                    totalStages,
                    toCheck->from,
                    toCheck->to);
                std::string currentRunNote = fmt::format(
                    "{} - {}",
                    runStart,
                    runEnd);

                AchievementNotifier::sharedState()->notifyAchievement(
                    "New run closed",
                    fromTo.c_str(),
                    "GJ_completesIcon_001.png",
                    true);

                toCheck->checked = true;
                toCheck->note = currentRunNote;
                canPlaySound = true;
                isStageClosed = false;
                checkedRangeThisRun = true;

                geode::log::debug("CHECKED RANGE {}-{}", toCheck->from, toCheck->to);
            }

            // Если закрыли range, проверяем закрыта ли теперь вся стадия
            if (checkedRangeThisRun)
            {
                bool allChecked = std::all_of(stage.ranges.begin(), stage.ranges.end(), [](const Range &r)
                                              { return r.checked; });

                if (allChecked && !stage.checked)
                {
                    stage.checked = true;
                    geode::log::debug("Stage closed");
                    canPlaySound = true;
                    isStageClosed = true;
                }

                break; // Выходим из цикла стадий, не закрываем следующие стадии пока не закроется эта
            }
            else
            {
                // Если не закрыли ни одного range в этой стадии, значит дальше закрывать нельзя
                break;
            }
        }

        if (canPlaySound)
        {
            // true если нужно проиграть звук стейджа
            // false если нужно проиграть звук range
            saveProfile(currentProfile);
            playSound(isStageClosed);
        }
    }
};