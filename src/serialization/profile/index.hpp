#pragma once

#include <Geode/Geode.hpp>

#include <string>
#include <vector>
#include <utility>

#include "./Range.hpp"
#include "./Stage.hpp"
#include "./ProfileData.hpp"
#include "./Profile.hpp"
#include "../../utils/getOr.hpp"

using namespace geode::prelude;

// ! --- Range ---- !
template <>
struct matjson::Serialize<Range>
{
  static geode::Result<Range> fromJson(matjson::Value const &value)
  {
    return geode::Ok(Range{
        .id = getOr<std::string>(value, "id", ""),
        .from = getOr<float>(value, "from", 0.f),
        .to = getOr<float>(value, "to", 0.f),
        .firstRunFrom = getOr<float>(value, "firstRunFrom", 0.f),
        .firstRunTo = getOr<float>(value, "firstRunTo", 0.f),
        .bestRunFrom = getOr<float>(value, "bestRunFrom", 0.f),
        .bestRunTo = getOr<float>(value, "bestRunTo", 0.f),
        .checked = getOr<bool>(value, "checked", false),
        .consider = getOr<bool>(value, "consider", true),
        .automaticallyClosed = getOr<bool>(value, "automaticallyClosed", false),
        .attempts = getOr<int>(value, "attempts", 0),
        .timePlayed = getOr<float>(value, "timePlayed", 0.f),
        .note = getOr<std::string>(value, "note", ""),
        .completedAt = getOr<std::time_t>(value, "completedAt", 0),
        .attemptsToComplete = getOr<int>(value, "attemptsToComplete", 0),
        .completionCounter = getOr<int>(value, "completionCounter", 0)});
  }

  static matjson::Value toJson(Range const &r)
  {
    auto obj = matjson::Value::object();
    obj["id"] = r.id;
    obj["from"] = r.from;
    obj["to"] = r.to;
    obj["firstRunFrom"] = r.firstRunFrom;
    obj["firstRunTo"] = r.firstRunTo;
    obj["bestRunFrom"] = r.bestRunFrom;
    obj["bestRunTo"] = r.bestRunTo;
    obj["checked"] = r.checked;
    obj["consider"] = r.consider;
    obj["automaticallyClosed"] = r.automaticallyClosed;
    obj["note"] = r.note;
    obj["attempts"] = r.attempts;
    obj["timePlayed"] = r.timePlayed;
    obj["completedAt"] = r.completedAt;
    obj["attemptsToComplete"] = r.attemptsToComplete;
    obj["completionCounter"] = r.completionCounter;
    return obj;
  }
};

template <>
struct matjson::Serialize<std::vector<Range>>
{
  static geode::Result<std::vector<Range>> fromJson(matjson::Value const &value)
  {
    std::vector<Range> result;
    if (value.isArray())
    {
      for (auto const &item : value)
      {
        result.push_back(item.as<Range>().unwrapOr(Range{}));
      }
    }
    return geode::Ok(result);
  }

  static matjson::Value toJson(std::vector<Range> const &ranges)
  {
    auto arr = matjson::Value::array();
    for (auto const &r : ranges)
      arr.push(r);
    return arr;
  }
};

// ! --- Stage ---- !
template <>
struct matjson::Serialize<Stage>
{
  static geode::Result<Stage> fromJson(matjson::Value const &value)
  {
    Stage s;
    s.id = getOr<std::string>(value, "id", "");
    s.stage = getOr<int>(value, "stage", 0);
    s.checked = getOr<bool>(value, "checked", false);
    s.note = getOr<std::string>(value, "note", "");
    s.completionCounter = getOr<int>(value, "completionCounter", 0);

    if (auto arr = value.get("ranges"))
      s.ranges = arr.unwrap().as<std::vector<Range>>().unwrap();

    return geode::Ok(s);
  }

  static matjson::Value toJson(Stage const &s)
  {
    auto obj = matjson::Value::object();
    obj["id"] = s.id;
    obj["stage"] = s.stage;
    obj["checked"] = s.checked;
    obj["note"] = s.note;
    obj["completionCounter"] = s.completionCounter;
    obj["ranges"] = s.ranges; // Serialize<std::vector<Range>>
    return obj;
  }
};

template <>
struct matjson::Serialize<std::vector<Stage>>
{
  static geode::Result<std::vector<Stage>> fromJson(matjson::Value const &value)
  {
    std::vector<Stage> result;
    if (value.isArray())
    {
      for (auto const &item : value)
        result.push_back(item.as<Stage>().unwrap());
    }
    return geode::Ok(result);
  }

  static matjson::Value toJson(std::vector<Stage> const &stages)
  {
    auto arr = matjson::Value::array();
    for (auto const &s : stages)
      arr.push(s);
    return arr;
  }
};

// ! --- Tags ---- !
template <>
struct matjson::Serialize<std::vector<float>>
{
  static geode::Result<std::vector<float>> fromJson(matjson::Value const &value)
  {
    std::vector<float> result;
    if (value.isArray())
    {
      for (auto const &t : value)
      {
        if (t.isNumber())
          result.push_back(t.asDouble().unwrapOr(0.f));
        else if (t.isString())
          result.push_back(geode::utils::numFromString<float>(t.asString().unwrapOr("")).unwrapOr(0.f));
        else
          result.push_back(0.f);
      }
    }
    return geode::Ok(result);
  }

  static matjson::Value toJson(std::vector<float> const &vec)
  {
    auto arr = matjson::Value::array();
    for (auto const &i : vec)
      arr.push(i);
    return arr;
  }
};

// ! --- History ---- !
// An array of days, older profiles have none
template <>
struct matjson::Serialize<std::map<std::string, DayStats>>
{
  static geode::Result<std::map<std::string, DayStats>> fromJson(matjson::Value const &value)
  {
    std::map<std::string, DayStats> result;

    if (!value.isArray())
      return geode::Ok(result);

    for (auto const &item : value)
    {
      const auto date = getOr<std::string>(item, "date", "");

      if (date.empty())
        continue;

      result[date] = DayStats{
          .attempts = getOr<int>(item, "attempts", 0),
          .timePlayed = getOr<float>(item, "timePlayed", 0.f),
          .runsPassed = getOr<int>(item, "runsPassed", 0),
          .stagesClosed = getOr<int>(item, "stagesClosed", 0),
          .bestFromZero = getOr<float>(item, "bestFromZero", 0.f),
      };
    }

    return geode::Ok(result);
  }

  static matjson::Value toJson(std::map<std::string, DayStats> const &history)
  {
    auto arr = matjson::Value::array();

    for (auto const &[date, day] : history)
    {
      auto obj = matjson::Value::object();
      obj["date"] = date;
      obj["attempts"] = day.attempts;
      obj["timePlayed"] = day.timePlayed;
      obj["runsPassed"] = day.runsPassed;
      obj["stagesClosed"] = day.stagesClosed;
      obj["bestFromZero"] = day.bestFromZero;
      arr.push(obj);
    }

    return arr;
  }
};

// ! --- ProfileData ---- !
template <>
struct matjson::Serialize<ProfileData>
{
  static geode::Result<ProfileData> fromJson(matjson::Value const &value)
  {
    ProfileData pd;

    if (auto arr = value.get("tags"))
      pd.tags = arr.unwrap().as<std::vector<float>>().unwrap();
    if (auto arr = value.get("stages"))
      pd.stages = arr.unwrap().as<std::vector<Stage>>().unwrap();
    if (auto arr = value.get("history"))
      pd.history = arr.unwrap().as<std::map<std::string, DayStats>>().unwrapOr(std::map<std::string, DayStats>{});

    return geode::Ok(pd);
  }

  static matjson::Value toJson(ProfileData const &pd)
  {
    auto obj = matjson::Value::object();
    obj["tags"] = pd.tags;       // Serialize<std::vector<int>>
    obj["stages"] = pd.stages;   // Serialize<std::vector<Stage>>
    obj["history"] = pd.history; // Serialize<std::map<std::string, DayStats>>
    return obj;
  }
};

// ! --- Profile ---- !
template <>
struct matjson::Serialize<Profile>
{
  static geode::Result<Profile> fromJson(matjson::Value const &value)
  {
    Profile p;
    p.id = getOr<std::string>(value, "id", "");
    p.profileName = getOr<std::string>(value, "profileName", "");
    p.discordWebhookForRunNotifications = getOr<std::string>(value, "discordWebhookForRunNotifications", "");
    p.discordWebhookForRunNotificationsEnabled = getOr<bool>(value, "discordWebhookForRunNotificationsEnabled", false);

    if (auto data = value.get("data"))
      p.data = data.unwrap().as<ProfileData>().unwrapOr(ProfileData{});
    return geode::Ok(p);
  }

  static matjson::Value toJson(Profile const &p)
  {
    auto obj = matjson::Value::object();
    obj["id"] = p.id;
    obj["profileName"] = p.profileName;
    obj["discordWebhookForRunNotifications"] = p.discordWebhookForRunNotifications;
    obj["discordWebhookForRunNotificationsEnabled"] = p.discordWebhookForRunNotificationsEnabled;
    obj["data"] = p.data;
    return obj;
  }
};

// ! --- Profiles Array ---- !
template <>
struct matjson::Serialize<std::vector<Profile>>
{
  static geode::Result<std::vector<Profile>> fromJson(
      matjson::Value const &value)
  {
    std::vector<Profile> result;

    if (!value.isArray())
    {
      geode::log::error("Profiles JSON is not an array");
      return geode::Ok(result);
    }

    for (auto const &item : value)
    {
      auto profileResult = item.as<Profile>();

      if (profileResult.isErr())
      {
        geode::log::error(
            "Failed to parse profile: {}",
            profileResult.unwrapErr());

        continue;
      }

      auto profile = profileResult.unwrap();

      if (profile.id.empty())
      {
        geode::log::error(
            "Skipped profile with empty ID");

        continue;
      }

      result.push_back(std::move(profile));
    }

    return geode::Ok(std::move(result));
  }

  static matjson::Value toJson(
      std::vector<Profile> const &profiles)
  {
    auto array = matjson::Value::array();

    for (auto const &profile : profiles)
      array.push(profile);

    return array;
  }
};