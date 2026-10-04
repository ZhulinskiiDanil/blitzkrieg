#include "../GlobalStore.hpp"

using namespace geode::prelude;

std::filesystem::path GlobalStore::getProfilesDir() const
{
  return Mod::get()->getSaveDir() / "profiles";
}

std::filesystem::path GlobalStore::getProfilePath(
    std::string const &profileId) const
{
  return getProfilesDir() / fmt::format("{}.json", profileId);
}

void GlobalStore::saveProfile(Profile const &profile) const
{
  if (profile.id.empty())
  {
    log::error("Cannot save profile with empty ID");
    return;
  }

  std::error_code ec;
  std::filesystem::create_directories(getProfilesDir(), ec);

  if (ec)
  {
    log::error("Failed to create profiles directory: {}", ec.message());
    return;
  }

  matjson::Value json = profile;
  auto result = geode::utils::file::writeStringSafe(
      getProfilePath(profile.id),
      json.dump(matjson::NO_INDENTATION));

  if (result.isErr())
    log::error(
        "Failed to save profile {}: {}",
        profile.id,
        result.unwrapErr());
}

void GlobalStore::saveProfileIndex() const
{
  std::error_code ec;
  std::filesystem::create_directories(getProfilesDir(), ec);

  if (ec)
  {
    log::error(
        "Failed to create profiles directory: {}",
        ec.message());
    return;
  }

  std::vector<std::string> profileIds;
  profileIds.reserve(m_profiles.size());

  for (auto const &profile : m_profiles)
    profileIds.push_back(profile.id);

  matjson::Value json = profileIds;

  auto result = geode::utils::file::writeStringSafe(
      getProfilesDir() / "index.json",
      json.dump(matjson::NO_INDENTATION));

  if (result.isErr())
  {
    log::error(
        "Failed to save profile index: {}",
        result.unwrapErr());
  }
}

std::vector<Profile>
GlobalStore::loadProfiles() const
{
  std::vector<Profile> profiles;

  auto indexResult =
      geode::utils::file::readJson(
          getProfilesDir() /
          "index.json");

  if (indexResult.isErr())
  {
    log::error(
        "Failed to read profile index: {}",
        indexResult.unwrapErr());

    return {};
  }

  auto idsResult =
      indexResult
          .unwrap()
          .as<std::vector<std::string>>();

  if (idsResult.isErr())
  {
    log::error(
        "Failed to parse profile index: {}",
        idsResult.unwrapErr());

    return {};
  }

  auto ids = idsResult.unwrap();

  profiles.reserve(ids.size());

  for (auto const &id : ids)
  {
    auto profileJsonResult =
        geode::utils::file::readJson(
            getProfilePath(id));

    if (profileJsonResult.isErr())
    {
      log::error(
          "Failed to read profile {}: {}",
          id,
          profileJsonResult.unwrapErr());

      continue;
    }

    auto parsedProfile =
        profileJsonResult
            .unwrap()
            .as<Profile>();

    if (parsedProfile.isErr())
    {
      log::error(
          "Failed to parse profile {}: {}",
          id,
          parsedProfile.unwrapErr());

      continue;
    }

    auto profile =
        parsedProfile.unwrap();

    if (profile.id.empty())
    {
      log::error(
          "Profile file {} has an empty ID",
          id);

      continue;
    }

    if (profile.id != id)
    {
      log::error(
          "Profile ID mismatch: index={}, file={}",
          id,
          profile.id);

      continue;
    }

    profiles.push_back(
        std::move(profile));
  }

  return profiles;
}
