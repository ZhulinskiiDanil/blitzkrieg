#include "./generateDSMessageByClosedRun.hpp"

std::string generateDSMessageByClosedRun(
    float runStart,
    float runEnd,
    Profile *profile,
    Range *range,
    Stage *stage)
{
  if (!profile || !range)
    return "";

  bool isLevelCompleted = runStart <= 0.f && runEnd >= 100.f;
  std::string idShort = profile->id.substr(0, 7);
  std::string message;

  if (isLevelCompleted)
  {
    message += fmt::format(
        "## :tada: Level Completed!\n\n"
        "Completed the level from **0% to 100%** in profile **{}**\n",
        profile->profileName);
  }
  else
  {
    message += fmt::format("## :white_check_mark: Run: {:.2f}% → {:.2f}%\n\n", runStart, runEnd);
  }

  if (isLevelCompleted)
  {
    message += fmt::format(
        "Closed run **{:.2f}% – {:.2f}%**",
        range->from,
        range->to);
  }
  else
  {
    message += fmt::format(
        "Closed run **{:.2f}% – {:.2f}%** in the profile **{}**",
        range->from,
        range->to,
        profile->profileName);
  }

  if (stage && stage->checked)
  {
    auto meta = getMetaInfoFromStages(profile->data.stages);

    message += fmt::format(
        "\nStage **{}/{}** completed",
        stage->stage,
        meta.total);
  }

  message += fmt::format("\n\n-# ID: {}", idShort);

  return message;
}