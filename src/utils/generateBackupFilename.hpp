#pragma once

#include <string>

namespace backup
{
  // Generates a backup filename, e.g. backup-15-08-2025-14-30.json
  std::string generateBackupFilename();
}