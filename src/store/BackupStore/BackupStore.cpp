#include "BackupStore.hpp"

#include <algorithm>
#include <fmt/chrono.h>

#include "../GlobalStore.hpp"
#include "../../events/ProfilesChangedEvent.hpp"
#include "../../utils/getOr.hpp"

using namespace geode::prelude;

namespace
{
  const char *META_FILE = "meta.json";
  const char *PROFILES_FOLDER = "profiles";

  std::filesystem::path getProfilesDir()
  {
    return Mod::get()->getSaveDir() / "profiles";
  }

  std::string toString(std::filesystem::path const &path)
  {
    return geode::utils::string::pathToString(path);
  }

  // Copies the files of a folder, not its subfolders
  geode::Result<> copyFiles(std::filesystem::path const &from, std::filesystem::path const &to)
  {
    std::error_code ec;
    std::filesystem::create_directories(to, ec);

    if (ec)
      return geode::Err(fmt::format("Failed to create {}: {}", toString(to), ec.message()));

    for (auto const &entry : std::filesystem::directory_iterator(from, ec))
    {
      if (!entry.is_regular_file())
        continue;

      std::filesystem::copy_file(
          entry.path(),
          to / entry.path().filename(),
          std::filesystem::copy_options::overwrite_existing,
          ec);

      if (ec)
        return geode::Err(fmt::format("Failed to copy {}: {}", toString(entry.path().filename()), ec.message()));
    }

    if (ec)
      return geode::Err(fmt::format("Failed to read {}: {}", toString(from), ec.message()));

    return geode::Ok();
  }

  std::uintmax_t getFolderSize(std::filesystem::path const &path)
  {
    std::error_code ec;
    std::uintmax_t size = 0;

    for (auto const &entry : std::filesystem::recursive_directory_iterator(path, ec))
    {
      if (entry.is_regular_file(ec))
        size += entry.file_size(ec);
    }

    return size;
  }

  // Backup ids are folder names made by create(), nothing else is touched
  bool isValidId(std::string const &id)
  {
    return !id.empty() &&
           id.find("..") == std::string::npos &&
           id.find('/') == std::string::npos &&
           id.find('\\') == std::string::npos;
  }
}

const char *getBackupReasonName(BackupReason reason)
{
  switch (reason)
  {
  case BackupReason::Manual:
    return "Manual";
  case BackupReason::BeforeDelete:
    return "Before delete";
  case BackupReason::BeforeImport:
    return "Before import";
  case BackupReason::BeforeStartposChange:
    return "Before start pos change";
  case BackupReason::BeforeRestore:
    return "Before restore";
  default:
    return "Automatic";
  }
}

BackupStore *BackupStore::get()
{
  static BackupStore instance;
  return &instance;
}

std::filesystem::path BackupStore::getDir() const
{
  return Mod::get()->getSaveDir() / "backups";
}

// ! --- Create --- !

geode::Result<BackupInfo> BackupStore::create(BackupReason reason, std::string const &note)
{
  const auto profilesDir = getProfilesDir();

  std::error_code ec;

  if (!std::filesystem::exists(profilesDir, ec) || std::filesystem::is_empty(profilesDir, ec))
    return geode::Err("There are no profiles to back up");

  // ! --- A free folder name, two backups can land in one second --- !
  const auto now = std::time(nullptr);
  const auto baseId = fmt::format("{:%Y%m%d-%H%M%S}", geode::localtime(now));

  std::string id = baseId;

  for (int i = 2; std::filesystem::exists(getDir() / id, ec); ++i)
    id = fmt::format("{}-{}", baseId, i);

  const auto path = getDir() / id;

  if (auto result = copyFiles(profilesDir, path / PROFILES_FOLDER); result.isErr())
  {
    std::filesystem::remove_all(path, ec);
    return geode::Err(result.unwrapErr());
  }

  // ! --- Meta --- !
  BackupInfo info;
  info.id = id;
  info.path = path;
  info.createdAt = now;
  info.reason = reason;
  info.note = note;

  for (auto const &profile : GlobalStore::get()->getProfiles())
    info.profileNames.push_back(profile.profileName);

  auto names = matjson::Value::array();

  for (auto const &name : info.profileNames)
    names.push(name);

  auto meta = matjson::Value::object();
  meta["createdAt"] = info.createdAt;
  meta["reason"] = static_cast<int>(reason);
  meta["reasonName"] = getBackupReasonName(reason);
  meta["note"] = note;
  meta["profileNames"] = names;

  if (auto result = file::writeStringSafe(path / META_FILE, meta.dump()); result.isErr())
    log::warn("Failed to write backup meta {}: {}", id, result.unwrapErr());

  info.size = getFolderSize(path);

  log::info("Profiles backed up to {} ({})", id, getBackupReasonName(reason));

  rotate();

  return geode::Ok(info);
}

void BackupStore::createAutomaticIfDue()
{
  if (!Mod::get()->getSettingValue<bool>("auto-backups"))
    return;

  const auto intervalHours = Mod::get()->getSettingValue<int64_t>("backups-interval");
  std::time_t lastAutomatic = 0;

  for (auto const &backup : list())
  {
    if (backup.reason == BackupReason::Automatic)
    {
      lastAutomatic = backup.createdAt;
      break;
    }
  }

  if (lastAutomatic > 0 && std::time(nullptr) - lastAutomatic < intervalHours * 60 * 60)
    return;

  if (auto result = create(BackupReason::Automatic); result.isErr())
    log::info("Automatic backup skipped: {}", result.unwrapErr());
}

// ! --- List --- !

std::vector<BackupInfo> BackupStore::list() const
{
  std::vector<BackupInfo> backups;
  std::error_code ec;

  for (auto const &entry : std::filesystem::directory_iterator(getDir(), ec))
  {
    if (!entry.is_directory())
      continue;

    BackupInfo info;
    info.path = entry.path();
    info.id = toString(entry.path().filename());

    if (auto meta = file::readJson(info.path / META_FILE); meta.isOk())
    {
      auto json = meta.unwrap();

      info.createdAt = getOr<std::time_t>(json, "createdAt", 0);
      info.reason = static_cast<BackupReason>(getOr<int>(json, "reason", 0));
      info.note = getOr<std::string>(json, "note", "");
      info.profileNames = getOr<std::vector<std::string>>(json, "profileNames", {});
    }

    // A folder without meta still shows up, dated by the folder itself
    if (info.createdAt <= 0)
    {
      // clock_cast is missing in libc++ (macOS, iOS, Android), shift by the gap between the clocks
      const auto writeTime = std::filesystem::last_write_time(info.path, ec);
      const auto systemTime = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
          writeTime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());

      info.createdAt = std::chrono::system_clock::to_time_t(systemTime);
    }

    info.size = getFolderSize(info.path);
    backups.push_back(std::move(info));
  }

  std::sort(backups.begin(), backups.end(), [](BackupInfo const &a, BackupInfo const &b)
            { return a.createdAt != b.createdAt ? a.createdAt > b.createdAt : a.id > b.id; });

  return backups;
}

// ! --- Restore and remove --- !

geode::Result<> BackupStore::restore(std::string const &id)
{
  if (!isValidId(id))
    return geode::Err("Invalid backup");

  const auto source = getDir() / id / PROFILES_FOLDER;
  std::error_code ec;

  if (!std::filesystem::exists(source, ec))
    return geode::Err("The backup has no profiles folder");

  // ! The current state can be brought back too
  if (auto result = create(BackupReason::BeforeRestore, id); result.isErr())
    log::info("No backup before restore: {}", result.unwrapErr());

  // ! Replace the files, the folder itself stays
  const auto profilesDir = getProfilesDir();

  for (auto const &entry : std::filesystem::directory_iterator(profilesDir, ec))
  {
    if (entry.is_regular_file())
      std::filesystem::remove(entry.path(), ec);
  }

  if (auto result = copyFiles(source, profilesDir); result.isErr())
    return result;

  GlobalStore::get()->reloadProfiles();
  ProfilesChangedEvent().send();

  return geode::Ok();
}

geode::Result<> BackupStore::remove(std::string const &id)
{
  if (!isValidId(id))
    return geode::Err("Invalid backup");

  std::error_code ec;
  std::filesystem::remove_all(getDir() / id, ec);

  if (ec)
    return geode::Err(fmt::format("Failed to delete the backup: {}", ec.message()));

  return geode::Ok();
}

void BackupStore::rotate()
{
  const auto keep = std::max<int64_t>(1, Mod::get()->getSettingValue<int64_t>("backups-keep"));
  int64_t automatic = 0;

  for (auto const &backup : list())
  {
    if (backup.reason != BackupReason::Automatic)
      continue;

    if (++automatic > keep)
      (void)remove(backup.id);
  }
}
