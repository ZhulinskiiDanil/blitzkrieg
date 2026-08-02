#pragma once

#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <cvolton.level-id-api/include/EditorIDs.hpp>

#include <filesystem>
#include <string>
#include <vector>

#include "../utils/getSavedProfiles.hpp"
#include "../utils/linkProfileWithLevel.hpp"
#include "../utils/isStageDeepChecked.hpp"
#include "../events/RunClosedEvent.hpp"

#include "../serialization/profile/index.hpp"

using namespace geode::prelude;

class GlobalStore
{
public:
    static GlobalStore *get();

    // ! --- Profiles API --- !
    std::vector<Profile> const &getProfiles() const;

    void addProfile(Profile const &profile);

    void addProfiles(
        std::vector<Profile> const &newProfiles,
        bool overwrite = false);

    void updateProfile(Profile const &profile);
    void upProfileById(std::string const &profileId);
    void removeProfileById(std::string const &id);

    void pinProfileById(
        std::string const &profileId,
        bool isPinned);

    bool isProfilePinned(std::string const &profileId);

    // ! --- Current Run API --- !
    void setRunStart(float start);
    void setRunEnd(float end);
    void resetRun();

    int checkRun(
        std::string const &profileId,
        float timePlayed = 0.f);

    // ! --- Search API --- !
    Profile *getProfileById(std::string const &profileId);
    Profile *getProfileByLevel(GJGameLevel *level);
    Profile *getProfileByLevel(std::string const &levelId);

    Range getCurrentRange(std::string const &profileId);

private:
    GlobalStore();

    std::vector<Profile> loadProfiles() const;

    void saveProfile(Profile const &profile) const;
    void saveProfileIndex() const;

    std::filesystem::path getProfilesDir() const;

    std::filesystem::path getProfilePath(
        std::string const &profileId) const;

    std::vector<Profile> m_profiles;

    float runStart = 0.f;
    float runEnd = 0.f;
};