#pragma once
#include <string>

namespace backup
{
  // Генерирует имя файла бекапа, например: backup-15-08-2025-14-30.json
  std::string generateBackupFilename();
}
