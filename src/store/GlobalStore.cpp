#include "GlobalStore.hpp"

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

int GlobalStore::checkRun(std::string profileId)
{
  auto currentProfile = getProfileById(profileId);

  if (currentProfile.id.empty())
    return -1;

  bool canPlaySound = false;
  bool isStageClosed = false;

  int totalStages = currentProfile.data.stages.size();

  for (auto &stage : currentProfile.data.stages)
  {
    if (stage.checked)
      continue;

    bool checkedRangeThisRun = false;

    std::vector<Range *> candidates;

    for (auto &range : stage.ranges)
      if (runStart <= range.from)
        candidates.push_back(&range);

    if (!candidates.empty())
    {
      auto *toCheck = *std::min_element(candidates.begin(), candidates.end(),
                                        [](Range *a, Range *b)
                                        { return a->from < b->from; });
      toCheck->attempts++;

      if (toCheck->bestRunTo < runEnd)
      {
        toCheck->bestRunFrom = runStart;
        toCheck->bestRunTo = runEnd;
      }

      if (runEnd < toCheck->to)
      {
        updateProfile(currentProfile);
        break;
      }

      if (toCheck->checked)
      {
        toCheck->completionCounter++;
        updateProfile(currentProfile);
        break;
      }

      geode::Notification::create(
          fmt::format("Passed {:.2f}-{:.2f} run", toCheck->from, toCheck->to),
          geode::NotificationIcon::Success,
          geode::NOTIFICATION_DEFAULT_TIME)
          ->show();

      toCheck->firstRunFrom = runStart;
      toCheck->firstRunTo = runEnd;
      toCheck->bestRunFrom = runStart;
      toCheck->bestRunTo = runEnd;
      toCheck->checked = true;
      toCheck->completionCounter++;
      canPlaySound = true;
      checkedRangeThisRun = true;
      break;
    }

    // Если закрыли range, проверяем закрыта ли теперь вся стадия
    if (checkedRangeThisRun)
    {
      bool allChecked = std::all_of(stage.ranges.begin(), stage.ranges.end(), [](const Range &r)
                                    { return r.checked; });

      if (allChecked && !stage.checked)
      {
        stage.checked = true;
        canPlaySound = true;
        isStageClosed = true;
      }

      break;
    }
    else
      break;
  }

  if (canPlaySound)
  {
    updateProfile(currentProfile);
    return isStageClosed;
  }

  return -1;
}

// ! --- Search API ---
Profile GlobalStore::getProfileById(std::string &profileId)
{
  auto it = std::find_if(m_profiles.begin(), m_profiles.end(),
                         [&](const Profile &p)
                         {
                           return p.id == profileId;
                         });

  if (it != m_profiles.end())
  {
    return *it;
  }

  return {};
}

Profile GlobalStore::getProfileByLevel(GJGameLevel *level)
{
  if (!level)
    return Profile{};

  std::string levelId = !level->getID().empty()
                            ? level->getID()
                            : geode::utils::numToString(EditorIDs::getID(level));

  return getProfileByLevel(levelId);
}

Profile GlobalStore::getProfileByLevel(std::string const &levelId)
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

Range GlobalStore::getCurrentRange(std::string &profileId)
{
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
        if (!range.checked && range.from <= (runStart + 0.1f))
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