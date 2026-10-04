#include "GlobalStore.hpp"

using namespace geode::prelude;

GlobalStore *GlobalStore::get()
{
  static GlobalStore instance;
  return &instance;
}

GlobalStore::GlobalStore()
{
  auto indexPath = getProfilesDir() / "index.json";

  if (std::filesystem::exists(indexPath))
  {
    m_profiles = loadProfiles();
    return;
  }

  // Migration
  m_profiles = getSavedProfiles();

  for (auto const &profile : m_profiles)
    saveProfile(profile);

  saveProfileIndex();
}
