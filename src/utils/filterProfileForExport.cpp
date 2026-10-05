#include "./filterProfileForExport.hpp"

Profile filterProfileForExport(
    Profile const &profile,
    bool includeSecrets,
    bool includeProgression,
    bool includeAttempts,
    bool includeNotes)
{
  Profile filtered = profile;

  if (!includeSecrets)
  {
    filtered.discordWebhookForRunNotifications.clear();
    filtered.discordWebhookForRunNotificationsEnabled = false;
  }

  // The days are made of attempts
  if (!includeAttempts)
    filtered.data.history.clear();

  for (auto &stage : filtered.data.stages)
  {
    if (!includeProgression)
    {
      stage.checked = false;
      stage.completionCounter = 0;
    }

    if (!includeNotes)
      stage.note.clear();

    for (auto &range : stage.ranges)
    {
      if (!includeProgression)
      {
        range.checked = false;
        range.automaticallyClosed = false;
        range.completedAt = 0;
        range.completionCounter = 0;
      }

      if (!includeAttempts)
      {
        range.attempts = 0;
        range.timePlayed = 0.f;
        range.firstRunFrom = 0.f;
        range.firstRunTo = 0.f;
        range.bestRunFrom = 0.f;
        range.bestRunTo = 0.f;
        range.attemptsToComplete = 0;
      }

      if (!includeNotes)
        range.note.clear();
    }
  }

  return filtered;
}
