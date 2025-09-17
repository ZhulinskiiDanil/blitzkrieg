#pragma once

#include <vector>
#include <cvolton.level-id-api/include/EditorIDs.hpp>

#include "../serialization/profile/index.hpp"
#include "../utils/getSavedProfiles.hpp"

class GlobalStore
{
public:
  static GlobalStore *get();

  // --- Profiles API ---
  std::vector<Profile> &getProfiles();
  void addProfile(Profile const &profile);
  void addProfiles(std::vector<Profile> const &newProfiles, bool overwrite = false);
  void setProfiles(std::vector<Profile> const &profiles);
  void updateProfile(Profile const &profile);
  void upProfileById(const std::string &profileId);
  void removeProfileById(std::string const &id);
  void pinProfileById(std::string profileId, bool isPinned);
  bool isProfilePinned(std::string profileId);

  // --- Search API ---
  Profile getProfileByLevel(GJGameLevel *level) const;
  Profile getProfileByLevel(std::string const &levelId) const;

  // --- Persistence ---
  void saveProfiles() const;

private:
  GlobalStore();

  std::vector<Profile> m_profiles;
};
