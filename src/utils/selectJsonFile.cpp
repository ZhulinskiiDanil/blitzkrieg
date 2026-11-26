#include <fstream>
#include <sstream>
#include <utility>

#include <Geode/utils/file.hpp>
#include <Geode/utils/async.hpp>
#include <Geode/loader/Mod.hpp>

#include "selectJsonFile.hpp"

using namespace geode;

static std::string readFileToString(const asp::fs::Path &path)
{
    auto data = utils::file::readString(path);
    if (data.isErr()) {
        log::error("Failed to read file: {}", data.err());
        return {};
    }

    return data.unwrap();
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
            if (!path.has_value()) return;

            auto content = readFileToString(path.value());

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