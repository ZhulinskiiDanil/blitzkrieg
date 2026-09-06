#include <fstream>
#include <sstream>
#include <utility>

#include <Geode/utils/file.hpp>
#include <Geode/utils/async.hpp>
#include <Geode/loader/Mod.hpp>

#include "selectJsonFile.hpp"

using namespace geode;

static std::string readFileToString(const std::filesystem::path &path)
{
    std::ifstream file(path, std::ios::binary);

    if (!file.is_open())
        return {};

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

void selectJsonFile(std::function<void(std::string)> callback)
{
    geode::utils::file::FilePickOptions options;

    options.filters = {
        {"JSON files",
         {"*.json"}}};

    geode::async::spawn(
        geode::utils::file::pick(
            geode::utils::file::PickMode::OpenFile,
            options),
        [callback = std::move(callback)](
            geode::utils::file::PickResult result) mutable
        {
            if (result.isErr())
            {
                log::error(
                    "Failed to select JSON file: {}",
                    result.unwrapErr());

                return;
            }

            auto path = result.unwrap();

            // User closed the file picker.
            if (!path)
                return;

            auto content = readFileToString(*path);

            if (content.empty())
            {
                log::error(
                    "Failed to read selected JSON file: {}",
                    path->string());

                return;
            }

            callback(std::move(content));
        });
}