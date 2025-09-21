#pragma once

#include <string>
#include <vector>
#include <Geode/Geode.hpp>

#include "Range.hpp"
#include "Stage.hpp"
#include "ProfileData.hpp"
#include "Profile.hpp"

using namespace geode::prelude;

// Вспомогательная функция для безопасного получения значения
template <class T>
T getOr(matjson::Value const &value, std::string_view key, T fallback)
{
  if (auto val = value.get(key))
  {
    auto v = val.unwrap().as<T>();
    return v.unwrapOr(fallback);
  }
  return fallback;
}

// ------------------ Range ------------------
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
        .note = getOr<std::string>(value, "note", ""),
        .attempts = getOr<int>(value, "attempts", 0),
        .completionCounter = getOr<int>(value, "completionCounter", 0)});
  }

  static matjson::Value toJson(Range const &r)
  {
    auto obj = matjson::Value::object();
    obj["id"] = r.id;
    obj["from"] = r.from;
    obj["to"] = r.to;
    obj["checked"] = r.checked;
    obj["note"] = r.note;
    obj["attempts"] = r.attempts;
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

// ------------------ Stage ------------------
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
    s.attempts = getOr<int>(value, "attempts", 0);
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
    obj["attempts"] = s.attempts;
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

// ------------------ Tags ------------------
template <>
struct matjson::Serialize<std::vector<int>>
{
  static geode::Result<std::vector<int>> fromJson(matjson::Value const &value)
  {
    std::vector<int> result;
    if (value.isArray())
    {
      for (auto const &t : value)
      {
        if (t.isNumber())
          result.push_back(t.asInt().unwrapOr(0));
        else if (t.isString())
          result.push_back(geode::utils::numFromString<int>(t.asString().unwrapOr("")).unwrapOr(0));
        else
          result.push_back(0);
      }
    }
    return geode::Ok(result);
  }

  static matjson::Value toJson(std::vector<int> const &vec)
  {
    auto arr = matjson::Value::array();
    for (auto const &i : vec)
      arr.push(i);
    return arr;
  }
};

// ------------------ ProfileData ------------------
template <>
struct matjson::Serialize<ProfileData>
{
  static geode::Result<ProfileData> fromJson(matjson::Value const &value)
  {
    ProfileData pd;

    if (auto arr = value.get("tags"))
      pd.tags = arr.unwrap().as<std::vector<int>>().unwrap();
    if (auto arr = value.get("stages"))
      pd.stages = arr.unwrap().as<std::vector<Stage>>().unwrap();

    return geode::Ok(pd);
  }

  static matjson::Value toJson(ProfileData const &pd)
  {
    auto obj = matjson::Value::object();
    obj["tags"] = pd.tags;     // Serialize<std::vector<int>>
    obj["stages"] = pd.stages; // Serialize<std::vector<Stage>>
    return obj;
  }
};

// ------------------ Profile ------------------
template <>
struct matjson::Serialize<Profile>
{
  static geode::Result<Profile> fromJson(matjson::Value const &value)
  {
    Profile p;
    p.id = getOr<std::string>(value, "id", "");
    p.profileName = getOr<std::string>(value, "profileName", "");
    if (auto data = value.get("data"))
      p.data = data.unwrap().as<ProfileData>().unwrapOr(ProfileData{});
    return geode::Ok(p);
  }

  static matjson::Value toJson(Profile const &p)
  {
    auto obj = matjson::Value::object();
    obj["id"] = p.id;
    obj["profileName"] = p.profileName;
    obj["data"] = p.data; // Serialize<ProfileData>
    return obj;
  }
};

// --------------- Profiles Array ---------------
template <>
struct matjson::Serialize<std::vector<Profile>>
{
  static geode::Result<std::vector<Profile>> fromJson(matjson::Value const &value)
  {
    std::vector<Profile> result;
    if (value.isArray())
    {
      for (auto const &item : value)
        result.push_back(item.as<Profile>().unwrap());
    }
    return geode::Ok(result);
  }

  static matjson::Value toJson(std::vector<Profile> const &profiles)
  {
    auto arr = matjson::Value::array();
    for (auto const &p : profiles)
      arr.push(p);
    return arr;
  }
};