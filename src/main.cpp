#include <Geode/Geode.hpp>
#include <Geode/binding/FMODAudioEngine.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <fmt/core.h>

#include "./serialization/profile/index.hpp"
#include "./store/GlobalStore.hpp"

using namespace geode::prelude;

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

        std::vector<Profile> profiles;
        bool hasRespawned = false;
        bool isNoclip = false;

        float attStartTime = 0.f;
        float attEndTime = 0.f;
    };

    static void onModify(auto &self)
    {
        if (!self.setHookPriorityPre("PlayLayer::destroyPlayer", Priority::First))
            geode::log::warn("Failed to set hook priority.");
    }

public:
    bool init(GJGameLevel *level, bool p1, bool p2)
    {
        if (!PlayLayer::init(level, p1, p2))
            return false;

        m_fields->profiles = GlobalStore::get()->getProfiles();
        return true;
    }

    void resetLevel()
    {
        PlayLayer::resetLevel();
        resetState();

        m_fields->hasRespawned = true;
        m_fields->attStartTime = this->timeForPos(
            m_player1->getPosition(), 0, 0, true, 0);

        GlobalStore::get()
            ->setRunStart(this->getCurrentPercent());
    }

    void levelComplete()
    {
        PlayLayer::levelComplete();

        if (!m_fields->hasRespawned)
            return;
        m_fields->hasRespawned = false;

        if (!m_level->isPlatformer())
        {
            m_fields->attEndTime = this->timeForPos(
                {m_levelLength, 0}, 0, 0, true, 0);
            GlobalStore::get()->setRunEnd(100.f);
            checkRun();
        }

        resetState();
    }

    void checkRun()
    {
        auto currentProfile = GlobalStore::get()->getProfileByLevel(DTPlayLayer::get()->m_level);
        const bool ignorePractice = Mod::get()->getSettingValue<bool>("ignore-practice-mode");

        if (ignorePractice && this->m_isPracticeMode)
            return;

        if (isLegal() && !currentProfile.id.empty())
        {
            const auto timePlayedForAttempt =
                m_fields->attEndTime - m_fields->attStartTime;
            int res = GlobalStore::get()->checkRun(
                currentProfile.id,
                timePlayedForAttempt);

            if (res != -1)
                playSound(!!res);
        }
    }

    void destroyPlayer(PlayerObject *player, GameObject *p1)
    {
        PlayLayer::destroyPlayer(player, p1);

        // First object is anticheat, store it
        if (!m_fields->disabledCheat)
            m_fields->disabledCheat = p1;

        if (!m_fields->isNoclip && m_fields->disabledCheat != p1 && !player->m_isDead)
            m_fields->isNoclip = true;

        if (!player->m_isDead || !m_fields->hasRespawned || m_level->isPlatformer())
            return;

        m_fields->hasRespawned = false;

        m_fields->attEndTime = this->timeForPos(
            m_player1->getPosition(), 0, 0, true, 0);
        GlobalStore::get()->setRunEnd(this->getCurrentPercent());
        checkRun();
    }

    void playSound(bool isStage)
    {
        auto sfxStagePath = Mod::get()->getSettingValue<std::filesystem::path>("sfx-stage-path");
        auto sfxProgressPath = Mod::get()->getSettingValue<std::filesystem::path>("sfx-progress-path");
        auto sfxUseCustomSounds = Mod::get()->getSettingValue<bool>("sfx-use-custom-sounds");

        auto stageSound = !sfxStagePath.empty() && sfxUseCustomSounds
                              ? geode::utils::string::pathToString(sfxStagePath)
                              : "stage_complete.mp3"_spr;
        auto progressSound = !sfxProgressPath.empty() && sfxUseCustomSounds
                                 ? geode::utils::string::pathToString(sfxProgressPath)
                                 : "progress_complete.mp3"_spr;

        if (isStage)
            FMODAudioEngine::sharedEngine()->playEffect(stageSound);
        else
            FMODAudioEngine::sharedEngine()->playEffect(progressSound);
    }

    void resetState()
    {
        m_fields->isNoclip = false;
        m_fields->disabledCheat = nullptr;
        m_fields->attStartTime = 0;
        m_fields->attEndTime = 0;
        GlobalStore::get()->resetRun();
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
};