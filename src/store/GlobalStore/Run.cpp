#include "../GlobalStore.hpp"
#include "RunWindow.hpp"
#include "../../utils/debugLog.hpp"

using namespace geode::prelude;

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

int GlobalStore::checkRun(
    std::string const &profileId,
    float timePlayed)
{
  // A single id shared by all logs produced for one checkRun call.
  // checkRun is expected to run on the game's main thread.
  static unsigned long long nextRunLogId = 0;
  const unsigned long long runLogId = ++nextRunLogId;

  const float runDiff = std::abs(runEnd - runStart);

  debugLog::info(
      "[Run #{}][START] profile='{}', run={:.2f}-{:.2f}, length={:.2f}, timePlayed={:.2f}",
      runLogId,
      profileId,
      runStart,
      runEnd,
      runDiff,
      timePlayed);

  auto *currentProfile = getProfileById(profileId);

  if (!currentProfile)
  {
    log::error(
        "[Run #{}][DROP] Profile '{}' was not found; the attempt cannot be assigned",
        runLogId,
        profileId);
    return -1;
  }

  Stage *targetStage = nullptr;
  Range *targetRange = nullptr;

  bool progressHasChecked = false;
  bool isStageClosed = false;

  const RunWindow run{runStart, runEnd};

  std::size_t stageIndex = 0;

  for (auto &stage : currentProfile->data.stages)
  {
    if (isStageDeepChecked(stage))
    {
      debugLog::info(
          "[Run #{}][STAGE {}] skipped: stage is deep-checked",
          runLogId,
          stageIndex);
      ++stageIndex;
      continue;
    }

    targetStage = &stage;

    debugLog::info(
        "[Run #{}][STAGE {}] checking first open stage ({} ranges)",
        runLogId,
        stageIndex,
        stage.ranges.size());

    std::vector<Range *> touchedRanges;
    std::vector<Range *> uncheckedRanges;

    std::size_t rangeIndex = 0;

    for (auto &range : stage.ranges)
    {
      if (!range.consider)
      {
        debugLog::info(
            "[Run #{}][STAGE {}][RANGE {} {:.2f}-{:.2f}] ignored: consider=false",
            runLogId,
            stageIndex,
            rangeIndex,
            range.from,
            range.to);
        ++rangeIndex;
        continue;
      }

      const float overlap = run.overlap(&range);
      const float coverage = run.coverage(&range);
      const bool touched = run.touches(&range);
      const bool passable = run.passes(&range);

      debugLog::info(
          "[Run #{}][STAGE {}][RANGE {} {:.2f}-{:.2f}] checked={}, touched={}, passable={}, overlap={:.2f}, coverage={:.1f}%",
          runLogId,
          stageIndex,
          rangeIndex,
          range.from,
          range.to,
          range.checked,
          touched,
          passable,
          overlap,
          coverage * 100.0f);

      if (!touched)
      {
        ++rangeIndex;
        continue;
      }

      touchedRanges.push_back(&range);

      if (!range.checked)
      {
        uncheckedRanges.push_back(
            &range);
      }

      ++rangeIndex;
    }

    debugLog::info(
        "[Run #{}][STAGE {}][CANDIDATES] touched={}, uncheckedTouched={}",
        runLogId,
        stageIndex,
        touchedRanges.size(),
        uncheckedRanges.size());

    if (touchedRanges.empty())
    {
      debugLog::warn(
          "[Run #{}][DROP] No range was touched in first open stage {}; later stages are not checked",
          runLogId,
          stageIndex);
      break;
    }

    const bool selectFromChecked = uncheckedRanges.empty();

    const char *selectionPool = selectFromChecked
                                    ? "already checked touched ranges"
                                    : "unchecked touched ranges";
    const char *selectionRule = nullptr;

    Range *statsRange = run.selectStatsRange(
        selectFromChecked ? touchedRanges : uncheckedRanges,
        selectFromChecked,
        selectionRule);

    if (!statsRange)
    {
      debugLog::warn(
          "[Run #{}][DROP] Candidate lists were non-empty, but no stats range was selected in stage {}",
          runLogId,
          stageIndex);
      break;
    }

    const auto selectedRangeIndex =
        static_cast<std::size_t>(
            statsRange - stage.ranges.data());

    debugLog::info(
        "[Run #{}][SELECT] stage={}, range={} ({:.2f}-{:.2f}), pool='{}', rule='{}', previouslyChecked={}",
        runLogId,
        stageIndex,
        selectedRangeIndex,
        statsRange->from,
        statsRange->to,
        selectionPool,
        selectionRule,
        statsRange->checked);

    const auto previousAttempts = statsRange->attempts;
    const float previousTimePlayed = statsRange->timePlayed;

    statsRange->attempts++;
    statsRange->timePlayed += timePlayed;

    debugLog::info(
        "[Run #{}][COUNT] range {:.2f}-{:.2f}: attempt {} -> {}, timePlayed {:.2f} -> {:.2f}",
        runLogId,
        statsRange->from,
        statsRange->to,
        previousAttempts,
        statsRange->attempts,
        previousTimePlayed,
        statsRange->timePlayed);

    const float bestRunDiff =
        std::abs(
            statsRange->bestRunFrom -
            statsRange->bestRunTo);

    if (bestRunDiff < runDiff)
    {
      debugLog::info(
          "[Run #{}][BEST] range {:.2f}-{:.2f}: best run {:.2f}-{:.2f} replaced with {:.2f}-{:.2f}",
          runLogId,
          statsRange->from,
          statsRange->to,
          statsRange->bestRunFrom,
          statsRange->bestRunTo,
          runStart,
          runEnd);

      statsRange->bestRunFrom = runStart;
      statsRange->bestRunTo = runEnd;
    }

    const bool passed =
        run.passes(statsRange);

    debugLog::info(
        "[Run #{}][PASS] range {:.2f}-{:.2f}: passed={}, previouslyChecked={}",
        runLogId,
        statsRange->from,
        statsRange->to,
        passed,
        statsRange->checked);

    if (statsRange->checked)
    {
      if (passed)
      {
        statsRange->completionCounter++;
        debugLog::info(
            "[Run #{}][COMPLETE] already checked range passed again; completionCounter={}",
            runLogId,
            statsRange->completionCounter);
      }
      else
      {
        debugLog::info(
            "[Run #{}][STOP] attempt counted for an already checked range, but it did not pass",
            runLogId);
      }

      break;
    }

    if (!passed)
    {
      debugLog::info(
          "[Run #{}][STOP] attempt {} counted for range {:.2f}-{:.2f}, but the range is not completed",
          runLogId,
          statsRange->attempts,
          statsRange->from,
          statsRange->to);
      break;
    }

    if (
        !Mod::get()->getSettingValue<bool>(
            "disable-run-notifications"))
    {
      geode::Notification::create(
          fmt::format(
              "Passed {:.2f}-{:.2f} run",
              statsRange->from,
              statsRange->to),
          geode::NotificationIcon::Success,
          geode::NOTIFICATION_DEFAULT_TIME)
          ->show();
    }

    statsRange->checked = true;

    statsRange->firstRunFrom = runStart;
    statsRange->firstRunTo = runEnd;

    statsRange->completedAt = std::time(nullptr);

    statsRange->attemptsToComplete =
        statsRange->attempts;

    statsRange->completionCounter++;

    debugLog::info(
        "[Run #{}][COMPLETE] range {:.2f}-{:.2f} completed on attempt {}; completionCounter={}",
        runLogId,
        statsRange->from,
        statsRange->to,
        statsRange->attemptsToComplete,
        statsRange->completionCounter);

    targetRange = statsRange;
    progressHasChecked = true;

    break;
  }

  if (targetStage)
  {
    const bool allChecked =
        std::all_of(
            targetStage->ranges.begin(),
            targetStage->ranges.end(),
            [](const Range &range)
            {
              return range.checked ||
                     !range.consider;
            });

    debugLog::info(
        "[Run #{}][STAGE RESULT] all considered ranges checked={}",
        runLogId,
        allChecked);

    if (allChecked)
    {
      targetStage->checked = true;
      isStageClosed = true;
    }
  }

  if (targetRange)
  {
    debugLog::info(
        "[Run #{}][EVENT] sending RunClosedEvent for {:.2f}-{:.2f}, stageClosed={}",
        runLogId,
        targetRange->from,
        targetRange->to,
        isStageClosed);

    RunClosedEvent().send(
        runStart,
        runEnd,
        currentProfile,
        targetRange,
        isStageClosed
            ? targetStage
            : nullptr);
  }
  else
  {
    debugLog::info(
        "[Run #{}][EVENT] RunClosedEvent not sent because no new range was completed",
        runLogId);
  }

  debugLog::info(
      "[Run #{}][SAVE] saving profile '{}'",
      runLogId,
      profileId);

  saveProfile(*currentProfile);

  if (progressHasChecked)
  {
    const int result = isStageClosed ? 1 : 0;
    debugLog::info(
        "[Run #{}][RESULT] return {} (new range completed, stageClosed={})",
        runLogId,
        result,
        isStageClosed);
    return result;
  }

  debugLog::info(
      "[Run #{}][RESULT] return -1 (no new range completed)",
      runLogId);
  return -1;
}
