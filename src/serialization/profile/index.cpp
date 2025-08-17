#include "index.hpp"

// --- to_json ---
json serializeRange(const Range &range)
{
  return {
      {"id", range.id},
      {"from", range.from},
      {"to", range.to},
      {"checked", range.checked},
      {"note", range.note},
      {"attempts", range.attempts},
      {"completionCounter", range.completionCounter}};
}

json serializeStage(const Stage &stage)
{
  json jRanges = json::array();

  for (const auto &range : stage.ranges)
  {
    jRanges.push_back(serializeRange(range));
  }

  return {
      {"id", stage.id},
      {"stage", stage.stage},
      {"checked", stage.checked},
      {"note", stage.note},
      {"attempts", stage.attempts},
      {"completionCounter", stage.completionCounter},
      {"ranges", jRanges}};
}

json serializeProfileData(const ProfileData &data)
{
  json jStages = json::array();

  for (const auto &stage : data.stages)
  {
    jStages.push_back(serializeStage(stage));
  }

  return {
      {"tags", data.tags},
      {"stages", jStages}};
}

json serializeProfile(const Profile &profile)
{
  return {
      {"id", profile.id},
      {"profileName", profile.profileName},
      {"data", serializeProfileData(profile.data)}};
}

// --- from_json ---
void from_json(const json &j, Range &r)
{
  j.at("id").get_to(r.id);                                         // string
  r.from = safe::number<int>(j, "from");                           // number
  r.to = safe::number<int>(j, "to");                               // number
  j.at("checked").get_to(r.checked);                               // boolean
  j.at("note").get_to(r.note);                                     // string
  r.attempts = safe::number<int>(j, "attempts");                   // number
  r.completionCounter = safe::number<int>(j, "completionCounter"); // number
}

void from_json(const json &j, Stage &s)
{
  j.at("id").get_to(s.id);                                         // string
  s.stage = safe::number<int>(j, "stage");                         // number
  j.at("checked").get_to(s.checked);                               // boolean
  j.at("note").get_to(s.note);                                     // string
  s.attempts = safe::number<int>(j, "attempts");                   // number
  s.completionCounter = safe::number<int>(j, "completionCounter"); // number
  if (j.contains("ranges") && j["ranges"].is_array())
    j.at("ranges").get_to(s.ranges); // Range[]
  else
    s.ranges.clear();
}

void from_json(const json &j, ProfileData &pd)
{
  pd.tags.clear();
  if (j.contains("tags") && j["tags"].is_array())
  {
    for (const auto &t : j["tags"])
    {
      if (t.is_number_integer())
        pd.tags.push_back(t.get<int>());
      else if (t.is_string())
        pd.tags.push_back(geode::utils::numFromString<int>(t.get<std::string>()).unwrapOr(0));
      else
        pd.tags.push_back(0);
    }
  }

  pd.stages.clear();
  if (j.contains("stages") && j["stages"].is_array())
    j.at("stages").get_to(pd.stages); // Stage[]
}

void from_json(const json &j, Profile &p)
{
  j.at("id").get_to(p.id);                   // string
  j.at("profileName").get_to(p.profileName); // string
  if (j.contains("data"))
    j.at("data").get_to(p.data); // ProfileData
}