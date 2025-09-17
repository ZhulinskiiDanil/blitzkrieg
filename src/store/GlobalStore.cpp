#include "GlobalStore.hpp"
#include <Geode/Geode.hpp>
#include <Geode/utils/file.hpp>

using namespace geode::prelude;

GlobalStore *GlobalStore::get()
{
  static GlobalStore instance;
  return &instance;
}

GlobalStore::GlobalStore()
    : m_profiles(getSavedProfiles()) {}

// ! --- Profiles API ---
std::vector<Profile> &GlobalStore::getProfiles()
{
  return m_profiles;
}

void GlobalStore::addProfile(Profile const &profile)
{
  m_profiles.push_back(profile);
  saveProfiles();
}

void GlobalStore::addProfiles(std::vector<Profile> const &newProfiles, bool overwrite)
{
  for (auto const &profile : newProfiles)
  {
    auto it = std::find_if(m_profiles.begin(), m_profiles.end(),
                           [&](Profile const &p)
                           { return p.id == profile.id; });

    if (it != m_profiles.end() && overwrite)
    {
      *it = profile;
    }
    else if (it == m_profiles.end())
    {
      m_profiles.push_back(profile);
    }
  }

  saveProfiles();
}

void GlobalStore::setProfiles(std::vector<Profile> const &profiles)
{
  m_profiles = profiles;
}

void GlobalStore::updateProfile(Profile const &profile)
{
  auto it = std::find_if(m_profiles.begin(), m_profiles.end(),
                         [&](Profile const &p)
                         { return p.id == profile.id; });

  if (it != m_profiles.end())
    *it = profile;
  else
    m_profiles.push_back(profile);

  saveProfiles();
}

void GlobalStore::removeProfileById(std::string const &id)
{
  m_profiles.erase(
      std::remove_if(m_profiles.begin(), m_profiles.end(),
                     [&](Profile const &p)
                     { return p.id == id; }),
      m_profiles.end());
}

void GlobalStore::upProfileById(const std::string &profileId)
{
  auto it = std::find_if(m_profiles.begin(), m_profiles.end(),
                         [&](const Profile &p)
                         {
                           return p.id == profileId;
                         });

  if (it != m_profiles.end() && it != m_profiles.begin())
    std::rotate(m_profiles.begin(), it, it + 1);

  saveProfiles();
}

void GlobalStore::pinProfileById(std::string profileId, bool isPinned)
{
  Mod::get()->setSavedValue<bool>(fmt::format("{}-pinned", profileId), isPinned);
}

bool GlobalStore::isProfilePinned(std::string profileId)
{
  return Mod::get()->getSavedValue<bool>(fmt::format("{}-pinned", profileId));
}

// ! --- Search API ---
Profile GlobalStore::getProfileByLevel(GJGameLevel *level) const
{
  if (!level)
    return Profile{};

  std::string levelId = !level->getID().empty()
                            ? level->getID()
                            : geode::utils::numToString(EditorIDs::getID(level));

  return getProfileByLevel(levelId);
}

Profile GlobalStore::getProfileByLevel(std::string const &levelId) const
{
  for (const auto &profile : m_profiles)
  {
    std::string key = levelId + "-" + profile.profileName;
    auto savedStr = Mod::get()->getSavedValue<std::string>(key);

    if (!savedStr.empty())
    {
      return profile;
    }
  }
  return Profile{};
}

// ! --- Persistence ---
void GlobalStore::saveProfiles() const
{
  matjson::Value j = m_profiles;

  if (j.isArray())
  {
    std::string jsonString = j.dump(matjson::NO_INDENTATION);
    Mod::get()->setSavedValue("profiles", jsonString);
  }
}