#include "../GlobalStore.hpp"
#include "../../utils/debugLog.hpp"

using namespace geode::prelude;

std::vector<Profile> const &GlobalStore::getProfiles() const
{
  return m_profiles;
}

void GlobalStore::addProfile(Profile const &profile)
{
  if (getProfileById(profile.id))
  {
    debugLog::warn("Profile {} already exists", profile.id);
    return;
  }

  m_profiles.push_back(profile);

  saveProfile(m_profiles.back());
  saveProfileIndex();
}

std::size_t GlobalStore::addProfiles(
    std::vector<Profile> const &newProfiles,
    bool overwrite)
{
  std::size_t added = 0;

  for (auto const &profile : newProfiles)
  {
    auto it = std::find_if(
        m_profiles.begin(),
        m_profiles.end(),
        [&](Profile const &existingProfile)
        {
          return existingProfile.id == profile.id;
        });

    if (it != m_profiles.end())
    {
      if (!overwrite)
        continue;

      *it = profile;
      saveProfile(*it);
      continue;
    }

    m_profiles.push_back(profile);
    saveProfile(m_profiles.back());
    added++;
  }

  if (added > 0)
    saveProfileIndex();

  return added;
}

void GlobalStore::updateProfile(Profile const &profile)
{
  auto it = std::find_if(
      m_profiles.begin(),
      m_profiles.end(),
      [&](Profile const &existingProfile)
      {
        return existingProfile.id == profile.id;
      });

  if (it != m_profiles.end())
  {
    if (&(*it) != &profile)
      *it = profile;

    saveProfile(*it);
    return;
  }

  m_profiles.insert(m_profiles.begin(), profile);

  saveProfile(m_profiles.front());
  saveProfileIndex();
}

void GlobalStore::removeProfileById(std::string const &id)
{
  m_profiles.erase(
      std::remove_if(
          m_profiles.begin(),
          m_profiles.end(),
          [&](Profile const &profile)
          {
            return profile.id == id;
          }),
      m_profiles.end());

  std::error_code ec;
  std::filesystem::remove(getProfilePath(id), ec);

  if (ec)
    log::error("Failed to remove profile {}: {}", id, ec.message());

  saveProfileIndex();
}

void GlobalStore::upProfileById(std::string const &profileId)
{
  auto it = std::find_if(
      m_profiles.begin(),
      m_profiles.end(),
      [&](Profile const &profile)
      {
        return profile.id == profileId;
      });

  if (it != m_profiles.end() && it != m_profiles.begin())
  {
    std::rotate(m_profiles.begin(), it, it + 1);
    saveProfileIndex();
  }
}

void GlobalStore::pinProfileById(std::string const &profileId, bool isPinned)
{
  Mod::get()->setSavedValue<bool>(fmt::format("{}-pinned", profileId), isPinned);
}

bool GlobalStore::isProfilePinned(std::string const &profileId)
{
  return Mod::get()->getSavedValue<bool>(fmt::format("{}-pinned", profileId));
}
