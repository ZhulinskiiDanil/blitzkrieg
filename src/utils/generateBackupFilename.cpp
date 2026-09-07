#include <string>

#include <Geode/Geode.hpp>

#include "generateBackupFilename.hpp"

namespace backup
{
    std::string generateBackupFilename()
    {
        const auto now =
            std::chrono::system_clock::now();

        const auto timestamp =
            std::chrono::system_clock::to_time_t(now);

        const auto timeInfo =
            geode::localtime(timestamp);

        return fmt::format(
            "backup-{:%d-%m-%Y-%H-%M}.json",
            timeInfo);
    }
}