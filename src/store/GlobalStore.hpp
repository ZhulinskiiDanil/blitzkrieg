#pragma once

#include <Geode/Geode.hpp>
#include <cvolton.level-id-api/include/EditorIDs.hpp>
#include <vector>

#include "../serialization/profile/index.hpp"
#include "../utils/getSavedProfiles.hpp"

using namespace geode::prelude;

class GlobalStore
{
public:
  static GlobalStore *get();

  // ! --- Profiles API --- !
  std::vector<Profile> &getProfiles();

  void addProfile(Profile const &profile);
  void addProfiles(std::vector<Profile> const &newProfiles, bool overwrite = false);
  void setProfiles(std::vector<Profile> const &profiles);
  void updateProfile(Profile const &profile);
  void upProfileById(const std::string &profileId);
  void removeProfileById(std::string const &id);
  void pinProfileById(std::string profileId, bool isPinned);
  bool isProfilePinned(std::string profileId);

  // ! --- Current Run API --- !
  void setRunStart(float start);
  void setRunEnd(float end);
  void resetRun();
  int checkRun(std::string profileId);

  // ! --- Search API --- !
  Profile getProfileById(std::string &profileId);
  Profile getProfileByLevel(GJGameLevel *level);
  Profile getProfileByLevel(std::string const &levelId);
  Range getCurrentRange(std::string &profileId);

  // ! --- Persistence --- !
  void saveProfiles() const;

private:
  GlobalStore();

  std::vector<Profile> m_profiles;
  float runStart = 0.f;
  float runEnd = 0.f;
};
