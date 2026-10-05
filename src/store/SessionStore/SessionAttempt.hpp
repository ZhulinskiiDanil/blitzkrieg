#pragma once

#include <Geode/Geode.hpp>

#include <cstdint>
#include <ctime>
#include <string>
#include <vector>

#include "../../utils/getOr.hpp"

using namespace geode::prelude;

// What happened to an attempt
enum class AttemptOutcome
{
  // Stats went to an open run, it was not passed
  Counted,
  // Every touched run was done, stats went to one of them
  CountedChecked,
  // An open run was passed and closed
  RunPassed,
  // The passed run was the last open one of its stage
  StageClosed,
  // Not checked at all: practice, noclip, speedhack, no profile
  Ignored,
  // Checked, but no run could take it
  Dropped
};

// A run that the attempt touched in the open stage
struct AttemptCandidate
{
  std::string rangeId;
  float from = 0.f;
  float to = 0.f;
  // Percent of the level shared with the run, and its share of the run, 0..1
  float overlap = 0.f;
  float coverage = 0.f;
  bool checked = false;
  bool passable = false;
  bool selected = false;
};

// One attempt of the session log
struct SessionAttempt
{
  std::uint64_t id = 0;

  std::string levelId;
  std::string levelName;
  std::string profileId;
  std::string profileName;

  // Wall clock
  std::time_t startedAt = 0;
  std::time_t endedAt = 0;
  // Level time of the attempt, seconds
  float duration = 0.f;

  float from = 0.f;
  float to = 0.f;

  // Index among considered stages, -1 when no stage was checked
  int stageIndex = -1;

  // The run that received the stats
  std::string rangeId;
  float rangeFrom = 0.f;
  float rangeTo = 0.f;

  // Which runs were picked from, and the rule of RunWindow::selectStatsRange
  std::string pool;
  std::string rule;
  std::vector<AttemptCandidate> candidates;

  AttemptOutcome outcome = AttemptOutcome::Ignored;
  // Human readable why
  std::string reason;

  bool newBest = false;
  // Attempts of the run after this one was counted
  int attemptNumber = 0;

  bool hasRange() const { return !rangeId.empty(); }
};

const char *getAttemptOutcomeName(AttemptOutcome outcome);

// ! --- Serialization --- !

template <>
struct matjson::Serialize<AttemptCandidate>
{
  static geode::Result<AttemptCandidate> fromJson(matjson::Value const &value)
  {
    return geode::Ok(AttemptCandidate{
        .rangeId = getOr<std::string>(value, "rangeId", ""),
        .from = getOr<float>(value, "from", 0.f),
        .to = getOr<float>(value, "to", 0.f),
        .overlap = getOr<float>(value, "overlap", 0.f),
        .coverage = getOr<float>(value, "coverage", 0.f),
        .checked = getOr<bool>(value, "checked", false),
        .passable = getOr<bool>(value, "passable", false),
        .selected = getOr<bool>(value, "selected", false)});
  }

  static matjson::Value toJson(AttemptCandidate const &c)
  {
    auto obj = matjson::Value::object();
    obj["rangeId"] = c.rangeId;
    obj["from"] = c.from;
    obj["to"] = c.to;
    obj["overlap"] = c.overlap;
    obj["coverage"] = c.coverage;
    obj["checked"] = c.checked;
    obj["passable"] = c.passable;
    obj["selected"] = c.selected;
    return obj;
  }
};

template <>
struct matjson::Serialize<SessionAttempt>
{
  static geode::Result<SessionAttempt> fromJson(matjson::Value const &value)
  {
    SessionAttempt a;
    a.id = getOr<std::uint64_t>(value, "id", 0);
    a.levelId = getOr<std::string>(value, "levelId", "");
    a.levelName = getOr<std::string>(value, "levelName", "");
    a.profileId = getOr<std::string>(value, "profileId", "");
    a.profileName = getOr<std::string>(value, "profileName", "");
    a.startedAt = getOr<std::time_t>(value, "startedAt", 0);
    a.endedAt = getOr<std::time_t>(value, "endedAt", 0);
    a.duration = getOr<float>(value, "duration", 0.f);
    a.from = getOr<float>(value, "from", 0.f);
    a.to = getOr<float>(value, "to", 0.f);
    a.stageIndex = getOr<int>(value, "stageIndex", -1);
    a.rangeId = getOr<std::string>(value, "rangeId", "");
    a.rangeFrom = getOr<float>(value, "rangeFrom", 0.f);
    a.rangeTo = getOr<float>(value, "rangeTo", 0.f);
    a.pool = getOr<std::string>(value, "pool", "");
    a.rule = getOr<std::string>(value, "rule", "");
    a.outcome = static_cast<AttemptOutcome>(getOr<int>(value, "outcome", static_cast<int>(AttemptOutcome::Ignored)));
    a.reason = getOr<std::string>(value, "reason", "");
    a.newBest = getOr<bool>(value, "newBest", false);
    a.attemptNumber = getOr<int>(value, "attemptNumber", 0);

    if (auto candidates = value.get("candidates"); candidates && candidates.unwrap().isArray())
    {
      for (auto const &item : candidates.unwrap())
        a.candidates.push_back(item.as<AttemptCandidate>().unwrapOr(AttemptCandidate{}));
    }

    return geode::Ok(a);
  }

  static matjson::Value toJson(SessionAttempt const &a)
  {
    auto candidates = matjson::Value::array();

    for (auto const &c : a.candidates)
      candidates.push(c);

    auto obj = matjson::Value::object();
    obj["id"] = a.id;
    obj["levelId"] = a.levelId;
    obj["levelName"] = a.levelName;
    obj["profileId"] = a.profileId;
    obj["profileName"] = a.profileName;
    obj["startedAt"] = a.startedAt;
    obj["endedAt"] = a.endedAt;
    obj["duration"] = a.duration;
    obj["from"] = a.from;
    obj["to"] = a.to;
    obj["stageIndex"] = a.stageIndex;
    obj["rangeId"] = a.rangeId;
    obj["rangeFrom"] = a.rangeFrom;
    obj["rangeTo"] = a.rangeTo;
    obj["pool"] = a.pool;
    obj["rule"] = a.rule;
    obj["candidates"] = candidates;
    obj["outcome"] = static_cast<int>(a.outcome);
    obj["outcomeName"] = getAttemptOutcomeName(a.outcome);
    obj["reason"] = a.reason;
    obj["newBest"] = a.newBest;
    obj["attemptNumber"] = a.attemptNumber;
    return obj;
  }
};
