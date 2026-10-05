#include "SessionStore.hpp"

#include <fmt/chrono.h>

#include "../../events/SessionChangedEvent.hpp"

using namespace geode::prelude;

namespace
{
  // format is a strftime pattern like "%Y-%m-%d"
  std::string formatTime(std::time_t time, const char *format)
  {
    if (time <= 0)
      return "-";

    return fmt::format(fmt::runtime(fmt::format("{{:{}}}", format)), geode::localtime(time));
  }
}

const char *getAttemptOutcomeName(AttemptOutcome outcome)
{
  switch (outcome)
  {
  case AttemptOutcome::Counted:
    return "Counted";
  case AttemptOutcome::CountedChecked:
    return "Counted to a done run";
  case AttemptOutcome::RunPassed:
    return "Run passed";
  case AttemptOutcome::StageClosed:
    return "Stage closed";
  case AttemptOutcome::Dropped:
    return "Dropped";
  default:
    return "Ignored";
  }
}

SessionStore *SessionStore::get()
{
  static SessionStore instance;
  return &instance;
}

SessionStore::SessionStore()
{
  load();

  if (m_startedAt <= 0)
    m_startedAt = std::time(nullptr);
}

std::filesystem::path SessionStore::getPath() const
{
  return Mod::get()->getSaveDir() / "session.json";
}

// ! --- Storage --- !

void SessionStore::load()
{
  auto result = file::readJson(getPath());

  // No log yet
  if (result.isErr())
    return;

  auto json = result.unwrap();

  m_startedAt = getOr<std::time_t>(json, "startedAt", 0);
  m_nextId = getOr<std::uint64_t>(json, "nextId", 1);

  if (auto attempts = json.get("attempts"); attempts && attempts.unwrap().isArray())
  {
    for (auto const &item : attempts.unwrap())
    {
      if (auto attempt = item.as<SessionAttempt>(); attempt.isOk())
        m_attempts.push_back(attempt.unwrap());
    }
  }

  while (m_attempts.size() > MAX_ATTEMPTS)
    m_attempts.pop_front();

  if (!m_attempts.empty())
    m_nextId = std::max(m_nextId, m_attempts.back().id + 1);
}

void SessionStore::save() const
{
  auto attempts = matjson::Value::array();

  for (auto const &attempt : m_attempts)
    attempts.push(attempt);

  auto json = matjson::Value::object();
  json["startedAt"] = m_startedAt;
  json["nextId"] = m_nextId;
  json["attempts"] = attempts;

  auto result = file::writeStringSafe(getPath(), json.dump(matjson::NO_INDENTATION));

  if (result.isErr())
    log::error("Failed to save the session log: {}", result.unwrapErr());
}

void SessionStore::queueSave()
{
  if (m_saveQueued)
    return;

  m_saveQueued = true;

  geode::queueInMainThread([this]()
                           {
                             m_saveQueued = false;
                             save(); });
}

// ! --- Log --- !

void SessionStore::add(SessionAttempt attempt)
{
  attempt.id = m_nextId++;
  m_attempts.push_back(std::move(attempt));

  while (m_attempts.size() > MAX_ATTEMPTS)
    m_attempts.pop_front();

  queueSave();
  SessionChangedEvent().send();
}

void SessionStore::reset()
{
  m_attempts.clear();
  m_nextId = 1;
  m_startedAt = std::time(nullptr);

  queueSave();
  SessionChangedEvent().send();
}

// ! --- Export --- !

geode::Result<std::filesystem::path> SessionStore::exportToFile() const
{
  const auto dir = Mod::get()->getSaveDir() / "session-logs";

  std::error_code ec;
  std::filesystem::create_directories(dir, ec);

  if (ec)
    return geode::Err(fmt::format("Failed to create {}: {}", geode::utils::string::pathToString(dir), ec.message()));

  const auto now = std::time(nullptr);
  const auto name = "session-" + formatTime(now, "%Y%m%d-%H%M%S");

  // ! --- Readable text --- !
  std::string text;
  auto line = [&text](std::string const &value)
  {
    text += value;
    text += '\n';
  };

  line("Blitzkrieg session log");
  line(fmt::format("Started: {}", formatTime(m_startedAt, "%Y-%m-%d %H:%M:%S")));
  line(fmt::format("Exported: {}", formatTime(now, "%Y-%m-%d %H:%M:%S")));
  line(fmt::format("Attempts: {}", m_attempts.size()));
  line("");

  for (auto const &a : m_attempts)
  {
    line(fmt::format(
        "#{}  {} -> {}  ({:.2f}s)",
        a.id,
        formatTime(a.startedAt, "%Y-%m-%d %H:%M:%S"),
        formatTime(a.endedAt, "%H:%M:%S"),
        a.duration));
    line(fmt::format(
        "  Level: {} ({})   Profile: {}",
        a.levelName.empty() ? "-" : a.levelName,
        a.levelId,
        a.profileName.empty() ? "-" : a.profileName));
    line(fmt::format("  Attempt: {:.2f}% -> {:.2f}%", a.from, a.to));
    line(fmt::format("  Outcome: {}", getAttemptOutcomeName(a.outcome)));

    if (a.stageIndex >= 0)
      line(fmt::format("  Stage: {}", a.stageIndex + 1));

    if (a.hasRange())
    {
      line(fmt::format(
          "  Run: {:.2f}% - {:.2f}%   attempt #{} of the run{}",
          a.rangeFrom,
          a.rangeTo,
          a.attemptNumber,
          a.newBest ? "   new best" : ""));
    }

    if (!a.pool.empty() || !a.rule.empty())
      line(fmt::format("  Picked from: {}   Rule: {}", a.pool, a.rule.empty() ? "-" : a.rule));

    line(fmt::format("  Reason: {}", a.reason));

    if (!a.candidates.empty())
    {
      line("  Candidates:");

      for (auto const &c : a.candidates)
      {
        line(fmt::format(
            "    {} {:.2f}-{:.2f}  overlap {:.2f}  coverage {:.1f}%  {}  {}",
            c.selected ? "*" : " ",
            c.from,
            c.to,
            c.overlap,
            c.coverage * 100.f,
            c.passable ? "passable" : "not passable",
            c.checked ? "done" : "open"));
      }
    }

    line("");
  }

  const auto textPath = dir / (name + ".txt");

  if (auto result = file::writeStringSafe(textPath, text); result.isErr())
    return geode::Err(result.unwrapErr());

  // ! --- JSON with everything --- !
  auto attempts = matjson::Value::array();

  for (auto const &attempt : m_attempts)
    attempts.push(attempt);

  auto json = matjson::Value::object();
  json["startedAt"] = m_startedAt;
  json["exportedAt"] = now;
  json["attempts"] = attempts;

  if (auto result = file::writeStringSafe(dir / (name + ".json"), json.dump()); result.isErr())
    return geode::Err(result.unwrapErr());

  return geode::Ok(textPath);
}
