#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(MenuLayer) {
    bool init() {
        if (!MenuLayer::init())
            return false;

        return true;
    }
};


class $modify(PlayLayer) {
    struct Run {
        int start = 0;  // Процент, с которого начался забег
        int end   = 0;  // Процент, на котором закончился забег
    };

    struct Fields {
        bool hasRespawned = false;
        Run currentRun;
    };

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
    }

    void levelComplete() {
        PlayLayer::levelComplete();

        // same as PlayLayer::destroyPlayer()
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
        }
    }

    void destroyPlayer(PlayerObject* player, GameObject* p1) {
        PlayLayer::destroyPlayer(player, p1);

        if (!player->m_isDead) return;

        // just in case some mod accidentally calls
        // PlayLayer::destroyPlayer() twice
        if (!m_fields->hasRespawned) return;
        m_fields->hasRespawned = false;

        if (!m_level->isPlatformer()) {
            m_fields->currentRun.end = this->getCurrentPercent();
    
            geode::log::debug(
                "PLAYER DESTROYED, RUN: start = {}, end = {}",
                m_fields->currentRun.start,
                m_fields->currentRun.end
            );
        }
    }
};