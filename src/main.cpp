#include "./serialization/profile/index.hpp"

#include <fmt/core.h>

#include <Geode/Geode.hpp>
#include <Geode/binding/FMODAudioEngine.hpp>
#include <Geode/binding/AchievementNotifier.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include "./serialization/profile/index.hpp"
#include "./store/GlobalStore.hpp"

using namespace geode::prelude;

struct Run
{
    float start = 0;
    float end = 0;
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

        return true;
    }

    void loadData()
    {
        m_fields->profiles = GlobalStore::get()->getProfiles();
    }

    void resetLevel()
    {
        PlayLayer::resetLevel();

        m_fields->hasRespawned = true;
        m_fields->currentRun.start = this->getCurrentPercent();

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
        }

        if (!player->m_isDead)
            return;
        if (!m_fields->hasRespawned)
            return;
        m_fields->hasRespawned = false;

        if (!m_level->isPlatformer())
        {
            m_fields->currentRun.end = this->getCurrentPercent();
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
        auto currentProfile = GlobalStore::get()->getProfileByLevel(DTPlayLayer::get()->m_level);

        if (!isLegal())
            return;

        if (currentProfile.id.empty())
            return;

        bool canPlaySound = false;
        bool isStageClosed = false;
        float runStart = run.start;
        float runEnd = run.end;

        int totalStages = currentProfile.data.stages.size();

        for (auto &stage : currentProfile.data.stages)
        {
            if (stage.checked)
                continue;

            bool checkedRangeThisRun = false;

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
                auto *toCheck = *std::min_element(candidates.begin(), candidates.end(),
                                                  [](Range *a, Range *b)
                                                  { return a->from < b->from; });

                std::string fromTo = fmt::format(
                    "Stage {}/{}: {:.2f} - {:.2f}",
                    stage.stage,
                    totalStages,
                    toCheck->from,
                    toCheck->to);
                std::string currentRunNote = fmt::format(
                    "{:.2f} - {:.2f}",
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
            }

            // Если закрыли range, проверяем закрыта ли теперь вся стадия
            if (checkedRangeThisRun)
            {
                bool allChecked = std::all_of(stage.ranges.begin(), stage.ranges.end(), [](const Range &r)
                                              { return r.checked; });

                if (allChecked && !stage.checked)
                {
                    stage.checked = true;
                    canPlaySound = true;
                    isStageClosed = true;
                }

                break;
            }
            else
                break;
        }

        if (canPlaySound)
        {
            // true если нужно проиграть звук стейджа
            // false если нужно проиграть звук range
            GlobalStore::get()->updateProfile(currentProfile);
            playSound(isStageClosed);
        }
    }
};