#include <Geode/Geode.hpp>
#include <Geode/binding/FMODAudioEngine.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include <chrono>
#include <deque>
#include <optional>

#include "./utils/generateDSMessageByClosedRun.hpp"
#include "./events/RunClosedEvent.hpp"
#include "./serialization/profile/index.hpp"
#include "./store/GlobalStore.hpp"

using namespace geode::prelude;

namespace BKGlobal
{
    FMOD::ChannelGroup *sfxGroup = nullptr;
}

class $modify(BlitzPlayLayer, PlayLayer)
{
    struct Fields
    {
        geode::comm::ListenerHandle runClosedListener;
        async::TaskHolder<utils::web::WebResponse> m_task;

        CCObject *firstIgnoredDamageObject = nullptr;

        std::vector<Profile> profiles;

        bool hasRespawned = false;
        bool isNoclip = false;
        bool isSpeedhack = false;
        int speedhackViolations = 0;

        bool speedhackArmed = false;
        float speedhackStartX = 0.f;

        double realDeltaSum = 0.0;
        double gameDeltaSum = 0.0;

        double expectedTimeWarp = 1.0;

        std::optional<
            std::chrono::steady_clock::time_point>
            speedhackLastFrame;

        std::deque<double> realDeltaHistory;
        std::deque<double> gameDeltaHistory;

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

        auto engine = FMODAudioEngine::get();
        auto system = engine->m_system;

        if (BKGlobal::sfxGroup == nullptr)
            system->createChannelGroup("blitzkrieg", &BKGlobal::sfxGroup);

        if (Mod::get()->getSettingValue<bool>("ignore-built-in-game-sfx"))
            BKGlobal::sfxGroup->setVolume(Mod::get()->getSettingValue<float>("sfx-volume"));
        else
            BKGlobal::sfxGroup->setVolume(GameManager::get()->m_sfxVolume * Mod::get()->getSettingValue<float>("sfx-volume"));

        m_fields->runClosedListener = RunClosedEvent().listen(
            [this](float from, float to, Profile *profile, Range *closedRange, Stage *closedStage)
            {
                if (!profile->discordWebhookForRunNotificationsEnabled)
                    return ListenerResult::Propagate;

                std::string msg = generateDSMessageByClosedRun(from, to, profile, closedRange, closedStage);
                std::string webhook = profile->discordWebhookForRunNotifications;

                utils::web::MultipartForm form;
                form.param("content", msg);

                auto req = utils::web::WebRequest()
                               .bodyMultipart(form)
                               .timeout(std::chrono::seconds(15))
                               .post(webhook);

                m_fields->m_task.spawn(
                    std::move(req),
                    [](utils::web::WebResponse res)
                    {
                        if (!res.ok())
                            geode::log::error("Failed to send webhook: {} (code {})", res.errorMessage(), res.code());
                    });

                return ListenerResult::Propagate;
            });

        m_fields->profiles = GlobalStore::get()->getProfiles();
        return true;
    }

    void postUpdate(float dt)
    {
        if (
            Mod::get()->getSettingValue<bool>(
                "enable-speedhack-detection"))
        {
            checkSpeedhack(dt);
        }

        PlayLayer::postUpdate(dt);
    }

    void updateTimeWarp(float timeWarp)
    {
        PlayLayer::updateTimeWarp(timeWarp);

        m_fields->expectedTimeWarp =
            static_cast<double>(timeWarp);

        resetSpeedhackSamples();
    }

    void startGame()
    {
        PlayLayer::startGame();

        prepareSpeedhackDetection();
    }

    void resetLevel()
    {
        PlayLayer::resetLevel();
        resetState();

        prepareSpeedhackDetection();

        m_fields->hasRespawned = true;
        m_fields->attStartTime = this->timeForPos(m_player1->getPosition(), 0, 0, true, 0);

        GlobalStore::get()->setRunStart(
            this->getCurrentPercent());
    }

    void levelComplete()
    {
        PlayLayer::levelComplete();

        if (!m_fields->hasRespawned)
            return;
        m_fields->hasRespawned = false;

        if (!m_level->isPlatformer())
        {
            m_fields->attEndTime = this->timeForPos({m_levelLength, 0}, 0, 0, true, 0);
            GlobalStore::get()->setRunEnd(100.f);
            checkRun();
        }

        resetState();
    }

    void checkRun()
    {
        auto currentProfile = GlobalStore::get()->getProfileByLevel(BlitzPlayLayer::get()->m_level);
        const bool ignorePractice = Mod::get()->getSettingValue<bool>("ignore-practice-mode");

        if (ignorePractice && this->m_isPracticeMode)
            return;

        if (isLegal() && currentProfile)
        {
            const auto timePlayedForAttempt = m_fields->attEndTime - m_fields->attStartTime;
            int res = GlobalStore::get()->checkRun(currentProfile->id, timePlayedForAttempt);

            if (res != -1)
                playSound(!!res);
        }
    }

    void destroyPlayer(
        PlayerObject *player,
        GameObject *obj)
    {
        PlayLayer::destroyPlayer(player, obj);

        resetSpeedhackSamples();

        if (!m_fields->firstIgnoredDamageObject)
            m_fields->firstIgnoredDamageObject = obj;

        if (
            !m_fields->isNoclip &&
            m_fields->firstIgnoredDamageObject != obj &&
            !player->m_isDead &&
            !m_levelEndAnimationStarted)
        {
            m_fields->isNoclip = true;
        }

        if (
            !player->m_isDead ||
            !m_fields->hasRespawned ||
            m_level->isPlatformer())
        {
            return;
        }

        m_fields->hasRespawned = false;

        m_fields->attEndTime =
            this->timeForPos(
                m_player1->getPosition(),
                0,
                0,
                true,
                0);

        GlobalStore::get()->setRunEnd(
            this->getCurrentPercent());

        checkRun();
    }

    static FMOD_RESULT fmodNonBlockCallback(FMOD_SOUND *a, FMOD_RESULT b)
    {
        log::info("nonBlockCallback called");

        // auto engine = FMODAudioEngine::get();
        // auto system = engine->m_system;
        // FMOD::Channel *playingChannel;

        return FMOD_OK;
    }

    void playSound(bool isStage)
    {
        if (Mod::get()->getSettingValue<bool>("disable-run-notification-sound"))
            return;

        FMOD_RESULT result;
        FMOD::Sound *sound;
        FMOD_CREATESOUNDEXINFO exinfo;
        FMOD::Channel *playingChannel;

        memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
        exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
        exinfo.nonblockcallback = BlitzPlayLayer::fmodNonBlockCallback;

        if (Mod::get()->getSettingValue<bool>("ignore-built-in-game-sfx"))
            BKGlobal::sfxGroup->setVolume(Mod::get()->getSettingValue<float>("sfx-volume"));
        else
            BKGlobal::sfxGroup->setVolume(GameManager::get()->m_sfxVolume * Mod::get()->getSettingValue<float>("sfx-volume"));

        auto engine = FMODAudioEngine::get();
        auto system = engine->m_system;

        auto sfxStagePath = Mod::get()->getSettingValue<std::filesystem::path>("sfx-stage-path");
        auto sfxProgressPath = Mod::get()->getSettingValue<std::filesystem::path>("sfx-progress-path");
        auto sfxUseCustomSounds = Mod::get()->getSettingValue<bool>("sfx-use-custom-sounds");

        auto stageSound = !sfxStagePath.empty() &&
                                  sfxUseCustomSounds
                              ? geode::utils::string::pathToString(sfxStagePath)
                              : fmt::format("{}/stage_complete.mp3", Mod::get()->getResourcesDir());
        auto progressSound = !sfxProgressPath.empty() &&
                                     sfxUseCustomSounds
                                 ? geode::utils::string::pathToString(sfxProgressPath)
                                 : fmt::format("{}/progress_complete.mp3", Mod::get()->getResourcesDir());

        std::string actualSound = isStage ? stageSound : progressSound;
        result = system->createStream(actualSound.c_str(), FMOD_DEFAULT | FMOD_LOOP_OFF | FMOD_2D | FMOD_LOWMEM, &exinfo, &sound);

        if (result != FMOD_OK)
        {
            log::info("FMOD ERROR {}", (int)result);
        }
        else
        {
            auto res = system->playSound(sound, BKGlobal::sfxGroup, false, &playingChannel);

            if (res != FMOD_OK)
                log::info("FMOD ERROR STARTING AUDIO: {}", (int)res);
        }
    }

    void resetState()
    {
        m_fields->isNoclip = false;
        m_fields->isSpeedhack = false;

        m_fields->speedhackArmed = false;
        m_fields->speedhackStartX = 0.f;

        m_fields->firstIgnoredDamageObject = nullptr;

        m_fields->attStartTime = 0.f;
        m_fields->attEndTime = 0.f;

        resetSpeedhackSamples();

        GlobalStore::get()->resetRun();
    }

    Profile *getProfileByName(const std::string &name)
    {
        for (auto &profile : m_fields->profiles)
            if (profile.profileName == name)
                return &profile;

        return nullptr;
    }

    void prepareSpeedhackDetection()
    {
        m_fields->speedhackArmed = false;

        m_fields->speedhackStartX =
            m_player1
                ? m_player1->getPositionX()
                : 0.f;

        resetSpeedhackSamples();
    }

    void resetSpeedhackSamples()
    {
        m_fields->speedhackLastFrame.reset();

        m_fields->realDeltaHistory.clear();
        m_fields->gameDeltaHistory.clear();

        m_fields->realDeltaSum = 0.0;
        m_fields->gameDeltaSum = 0.0;

        m_fields->speedhackViolations = 0;
    }

    void checkSpeedhack(float gameDelta)
    {
        if (
            m_fields->isSpeedhack ||
            !m_player1 ||
            m_player1->m_isDead ||
            m_isPaused)
        {
            return;
        }

        const auto now =
            std::chrono::steady_clock::now();

        if (!m_fields->speedhackArmed)
        {
            constexpr float movementThreshold = 5.f;

            const float movement =
                std::abs(
                    m_player1->getPositionX() -
                    m_fields->speedhackStartX);

            if (movement < movementThreshold)
            {
                m_fields->speedhackLastFrame = now;

                return;
            }

            m_fields->speedhackArmed = true;

            resetSpeedhackSamples();

            m_fields->speedhackLastFrame = now;

            log::debug(
                "Speedhack detector armed "
                "(movement: {:.2f})",
                movement);

            return;
        }

        if (!m_fields->speedhackLastFrame)
        {
            m_fields->speedhackLastFrame = now;
            return;
        }

        const std::chrono::duration<double> elapsed =
            now - *m_fields->speedhackLastFrame;

        m_fields->speedhackLastFrame = now;

        const double realDelta =
            elapsed.count();

        if (realDelta <= 0.0)
        {
            return;
        }

        if (realDelta > 0.2)
        {
            resetSpeedhackSamples();
            m_fields->speedhackLastFrame = now;
            return;
        }

        const double gameDt =
            static_cast<double>(gameDelta);

        m_fields->realDeltaHistory.push_back(realDelta);
        m_fields->gameDeltaHistory.push_back(gameDt);

        m_fields->realDeltaSum += realDelta;
        m_fields->gameDeltaSum += gameDt;

        constexpr std::size_t maxSamples = 30;
        constexpr std::size_t minSamples = 12;

        constexpr double softTolerance = 0.05;
        constexpr double hardTolerance = 0.15;

        constexpr int requiredViolations = 4;

        if (
            m_fields->realDeltaHistory.size() >
            maxSamples)
        {
            m_fields->realDeltaSum -=
                m_fields->realDeltaHistory.front();

            m_fields->gameDeltaSum -=
                m_fields->gameDeltaHistory.front();

            m_fields->realDeltaHistory.pop_front();
            m_fields->gameDeltaHistory.pop_front();
        }

        if (
            m_fields->realDeltaHistory.size() <
                minSamples ||
            m_fields->realDeltaSum <= 0.0)
        {
            return;
        }

        const double actualTimeWarp =
            m_fields->gameDeltaSum /
            m_fields->realDeltaSum;

        const double difference =
            std::abs(
                actualTimeWarp -
                m_fields->expectedTimeWarp);

        if (difference > hardTolerance)
        {
            m_fields->speedhackViolations += 2;
        }
        else if (difference > softTolerance)
        {
            ++m_fields->speedhackViolations;
        }
        else
        {
            m_fields->speedhackViolations = 0;
        }

        if (m_fields->speedhackViolations > 0)
        {
            log::debug(
                "Speedhack violation {}/{} "
                "(actual: {:.3f}, expected: {:.3f}, diff: {:.3f})",
                m_fields->speedhackViolations,
                requiredViolations,
                actualTimeWarp,
                m_fields->expectedTimeWarp,
                difference);
        }

        if (
            m_fields->speedhackViolations >=
            requiredViolations)
        {
            m_fields->isSpeedhack = true;

            log::warn(
                "Speedhack detected "
                "(actual: {:.3f}, expected: {:.3f}, diff: {:.3f})",
                actualTimeWarp,
                m_fields->expectedTimeWarp,
                difference);

            return;
        }
    }

    bool isLegal()
    {
        const bool speedhackDetectionEnabled =
            Mod::get()->getSettingValue<bool>(
                "enable-speedhack-detection");

        return !m_fields->isNoclip &&
               (!speedhackDetectionEnabled ||
                !m_fields->isSpeedhack) &&
               !m_isIgnoreDamageEnabled &&
               !m_ignoreDamage;
    }
};
