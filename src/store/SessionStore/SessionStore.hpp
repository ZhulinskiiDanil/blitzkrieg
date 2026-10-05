#pragma once

#include <Geode/Geode.hpp>

#include <cstdint>
#include <deque>
#include <filesystem>

#include "SessionAttempt.hpp"

using namespace geode::prelude;

// Log of every attempt since the last reset, saved to session.json.
// It survives restarts of the game, Reset in the Session tab clears it.
class SessionStore
{
public:
  static constexpr std::size_t MAX_ATTEMPTS = 5000;

  static SessionStore *get();

  // Gives the attempt its id, saves on the next frame and sends SessionChangedEvent
  void add(SessionAttempt attempt);
  void reset();

  // Oldest first
  std::deque<SessionAttempt> const &getAttempts() const { return m_attempts; }
  // When the log was started or last reset
  std::time_t getStartedAt() const { return m_startedAt; }

  // Writes a readable .txt and a .json next to it, returns the .txt path
  geode::Result<std::filesystem::path> exportToFile() const;

private:
  SessionStore();

  std::filesystem::path getPath() const;
  void load();
  void save() const;
  // Many attempts in a row are saved once
  void queueSave();

  std::deque<SessionAttempt> m_attempts;
  std::uint64_t m_nextId = 1;
  std::time_t m_startedAt = 0;
  bool m_saveQueued = false;
};
