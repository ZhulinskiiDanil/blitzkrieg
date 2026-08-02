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

// ! --- Profiles API --- !
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

std::vector<Profile> const &GlobalStore::getProfiles() const
{
  return m_profiles;
}

void GlobalStore::addProfile(Profile const &profile)
{
  if (getProfileById(profile.id))
  {
    log::warn("Profile {} already exists", profile.id);
    return;
  }

  m_profiles.push_back(profile);

  saveProfile(m_profiles.back());
  saveProfileIndex();
}

void GlobalStore::addProfiles(
    std::vector<Profile> const &newProfiles,
    bool overwrite)
{
  bool indexChanged = false;

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
    indexChanged = true;
  }

  if (indexChanged)
    saveProfileIndex();
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

// ! --- Current Run API --- !
void GlobalStore::setRunStart(float val)
{
  if (val >= 0 && val <= 100)
    runStart = val;
}

void GlobalStore::setRunEnd(float val)
{
  if (val >= 0 && val <= 100)
    runEnd = val;
}

void GlobalStore::resetRun()
{
  runStart = 0.f;
  runEnd = 0.f;
}

int GlobalStore::checkRun(std::string const &profileId, float timePlayed)
{
  const float eps = 0.01f;
  const float runDiff = std::abs(runEnd - runStart);

  auto *currentProfile = getProfileById(profileId);

  if (!currentProfile)
  {
    log::error("Profile not found!");
    return -1;
  }

  Stage *targetStage = nullptr;
  Range *targetRange = nullptr;
  bool progressHasChecked = false;
  bool isStageClosed = false;

  for (auto &stage : currentProfile->data.stages)
  {
    if (isStageDeepChecked(stage))
      continue;

    targetStage = &stage;
    std::vector<Range *> candidates;

    for (auto &range : stage.ranges)
    {
      if (runStart <= range.from && range.consider)
        candidates.push_back(&range);
    }

    if (!candidates.empty())
    {
      auto *toCheck = *std::min_element(candidates.begin(), candidates.end(),
                                        [eps](Range *a, Range *b)
                                        {
                                          if (std::fabs(a->from - b->from) <= eps)
                                            return a->to < b->to;

                                          return a->from < b->from;
                                        });

      Range *toCheckActualRange = nullptr;

      for (auto *r : candidates)
      {
        if (r->consider && !r->checked && runEnd >= r->to)
        {
          toCheckActualRange = r;
          break;
        }
      }

      if (!toCheckActualRange)
      {
        toCheck->attempts++;
        toCheck->timePlayed += timePlayed;

        auto bestRunDiff = std::abs(toCheck->bestRunFrom - toCheck->bestRunTo);

        if (bestRunDiff < runDiff)
        {
          toCheck->bestRunFrom = runStart;
          toCheck->bestRunTo = runEnd;
        }

        if (toCheck->firstRunTo <= 0 && runEnd >= toCheck->to)
        {
          toCheck->firstRunFrom = runStart;
          toCheck->firstRunTo = runEnd;
        }

        if (toCheck->checked && runEnd >= toCheck->to)
          toCheck->completionCounter++;

        break;
      }

      toCheckActualRange->timePlayed += timePlayed;
      toCheckActualRange->attempts++;

      if (toCheckActualRange->checked)
        break;

      auto bestRunDiff = std::abs(toCheckActualRange->bestRunFrom - toCheckActualRange->bestRunTo);

      if (bestRunDiff < runDiff)
      {
        toCheckActualRange->bestRunFrom = runStart;
        toCheckActualRange->bestRunTo = runEnd;
      }

      if (runEnd < toCheckActualRange->to)
        break;

      if (!Mod::get()->getSettingValue<bool>("disable-run-notifications"))
      {
        geode::Notification::create(
            fmt::format("Passed {:.2f}-{:.2f} run", toCheckActualRange->from, toCheckActualRange->to),
            geode::NotificationIcon::Success,
            geode::NOTIFICATION_DEFAULT_TIME)
            ->show();
      }

      targetRange = toCheckActualRange;
      toCheckActualRange->checked = true;
      toCheckActualRange->firstRunFrom = runStart;
      toCheckActualRange->firstRunTo = runEnd;
      toCheckActualRange->completedAt = std::time(nullptr);
      toCheckActualRange->attemptsToComplete = toCheckActualRange->attempts;
      toCheckActualRange->completionCounter++;
      progressHasChecked = true;
      break;
    }

    break;
  }

  if (targetStage)
  {
    bool allChecked = std::all_of(targetStage->ranges.begin(), targetStage->ranges.end(),
                                  [](const Range &r)
                                  {
                                    return r.checked || !r.consider;
                                  });

    if (allChecked)
    {
      targetStage->checked = true;
      isStageClosed = true;
    }
  }

  if (targetRange)
    RunClosedEvent().send(runStart, runEnd, currentProfile, targetRange, isStageClosed ? targetStage : nullptr);

  saveProfile(*currentProfile);

  if (progressHasChecked)
    return isStageClosed ? 1 : 0;

  return -1;
}

// ! --- Search API --- !
Profile *GlobalStore::getProfileById(const std::string &profileId)
{
  auto it = std::find_if(m_profiles.begin(), m_profiles.end(),
                         [&](const Profile &p)
                         {
                           return p.id == profileId;
                         });

  if (it != m_profiles.end())
  {
    return &(*it);
  }

  return nullptr;
}

Profile *GlobalStore::getProfileByLevel(GJGameLevel *level)
{
  if (!level)
    return {};

  std::string levelId = level->m_levelID ? utils::numToString(level->m_levelID.value()) : utils::numToString(EditorIDs::getID(level));
  return getProfileByLevel(levelId);
}

Profile *GlobalStore::getProfileByLevel(std::string const &levelId)
{
  for (auto &profile : m_profiles)
  {
    std::string key = levelId + "-" + profile.id;
    auto savedStr = Mod::get()->getSavedValue<std::string>(key);

    if (!savedStr.empty())
      return &profile;
  }

  return nullptr;
}

Range GlobalStore::getCurrentRange(std::string const &profileId)
{
  const float eps = 0.01f;
  Range *maxRange = nullptr;
  int currentStage = 0;

  for (auto &profile : m_profiles)
  {
    if (profile.id != profileId)
      continue;

    for (auto &stage : profile.data.stages)
    {
      if (stage.checked)
        continue;

      if (currentStage == 0)
        currentStage = stage.stage;
      else
        break;

      for (auto &range : stage.ranges)
      {
        if (range.consider && !range.checked && std::abs(range.from - runStart) < eps)
        {
          if (!maxRange || range.from > maxRange->from)
          {
            maxRange = &range;
          }
        }
      }
    }
  }

  if (maxRange)
    return *maxRange;

  return {};
}
