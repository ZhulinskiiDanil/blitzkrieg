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

        // Звуки закрытия ранов и стадий
        FMOD::Sound *rangeCompleteSound = nullptr;
        FMOD::Sound *stageCompleteSound = nullptr;
    };

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

        m_fields->hasRespawned = true;
        GlobalStore::get()->setRunStart(this->getCurrentPercent());
        GlobalStore::get()->setRunEnd(0.f);

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
            GlobalStore::get()->setRunEnd(100.f);
            auto currentProfile = GlobalStore::get()->getProfileByLevel(DTPlayLayer::get()->m_level);

            if (isLegal() && !currentProfile.id.empty())
            {
                int res = GlobalStore::get()->checkRun(currentProfile.id);

                if (res != -1)
                    playSound(!!res);
            };
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
            GlobalStore::get()->setRunEnd(this->getCurrentPercent());
            auto currentProfile = GlobalStore::get()->getProfileByLevel(DTPlayLayer::get()->m_level);

            if (isLegal() && !currentProfile.id.empty())
            {
                int res = GlobalStore::get()->checkRun(currentProfile.id);

                if (res != -1)
                    playSound(!!res);
            };
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