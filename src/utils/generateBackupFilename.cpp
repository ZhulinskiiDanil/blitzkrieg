#include "generateBackupFilename.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>

namespace backup
{

  std::string generateBackupFilename()
  {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    std::ostringstream oss;
    oss << "backup-"
        << std::setw(2) << std::setfill('0') << tm.tm_mday << "-"
        << std::setw(2) << std::setfill('0') << (tm.tm_mon + 1) << "-"
        << (tm.tm_year + 1900) << "-"
        << std::setw(2) << std::setfill('0') << tm.tm_hour << "-"
        << std::setw(2) << std::setfill('0') << tm.tm_min
        << ".json";

    return oss.str();
  }

} // namespace backup
