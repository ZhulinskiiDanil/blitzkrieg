#include "GlobalStore.hpp"

using namespace geode::prelude;

GlobalStore *GlobalStore::get()
{
  static GlobalStore instance;
  return &instance;
}

GlobalStore::GlobalStore()
    : m_profiles(getSavedProfiles()) {}

// ! --- Profiles API --- !
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
    m_profiles.insert(m_profiles.begin(), profile);

  saveProfiles();
}

void GlobalStore::removeProfileById(std::string const &id)
{
  m_profiles.erase(
      std::remove_if(m_profiles.begin(), m_profiles.end(),
                     [&](Profile const &p)
                     { return p.id == id; }),
      m_profiles.end());

  saveProfiles();
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

int GlobalStore::checkRun(std::string profileId, float timePlayed)
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

  updateProfile(*currentProfile);

  if (progressHasChecked)
    return isStageClosed ? 1 : 0;
  else
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

Range GlobalStore::getCurrentRange(std::string &profileId)
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

// ! --- Persistence --- !
void GlobalStore::saveProfiles() const
{
  matjson::Value j = m_profiles;

  if (j.isArray())
  {
    std::string jsonString = j.dump(matjson::NO_INDENTATION);
    Mod::get()->setSavedValue("profiles", jsonString);
    (void)Mod::get()->saveData();
  }
}
