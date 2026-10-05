#pragma once

#include <Geode/Geode.hpp>

#include <ctime>
#include <filesystem>
#include <string>
#include <vector>

using namespace geode::prelude;

// Why a backup was made, automatic ones rotate, the rest stay until deleted
enum class BackupReason
{
  Automatic,
  Manual,
  BeforeDelete,
  BeforeImport,
  BeforeStartposChange,
  BeforeRestore
};

struct BackupInfo
{
  // Folder name, also the id
  std::string id;
  std::filesystem::path path;

  std::time_t createdAt = 0;
  BackupReason reason = BackupReason::Automatic;
  // What the backup is about, like the deleted profile name
  std::string note;

  std::vector<std::string> profileNames;
  std::uintmax_t size = 0;
};

const char *getBackupReasonName(BackupReason reason);

// Copies of the profiles folder in saveDir/backups/<id>/profiles
class BackupStore
{
public:
  static BackupStore *get();

  geode::Result<BackupInfo> create(BackupReason reason, std::string const &note = {});

  // Once per game start, when the last automatic backup is older than the interval
  void createAutomaticIfDue();

  // Newest first
  std::vector<BackupInfo> list() const;

  // Backs the current profiles up first, then replaces them and reloads the store
  geode::Result<> restore(std::string const &id);
  geode::Result<> remove(std::string const &id);

  std::filesystem::path getDir() const;

private:
  BackupStore() = default;

  // Keeps the newest automatic backups, the setting says how many
  void rotate();
};
