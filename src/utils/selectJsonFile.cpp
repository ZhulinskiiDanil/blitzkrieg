#include <fstream>
#include <sstream>

#include <Geode/utils/file.hpp>
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
    using FileEvent = geode::Task<geode::Result<std::filesystem::path>>;
    static geode::EventListener<FileEvent> listener;

    geode::utils::file::FilePickOptions::Filter filter;
    filter.description = "JSON files";
    filter.files.insert("*.json");

    listener.bind([callback](FileEvent::Event *event)
                  {
        if (auto value = event->getValue()) {
            auto path = value->unwrapOr("");
            if (path.empty()) {
                callback({});
                return;
            }
            callback(readFileToString(path));
        } else {
            callback({});
        } });

    listener.setFilter(
        geode::utils::file::pick(
            geode::utils::file::PickMode::OpenFile,
            {geode::Mod::get()->getSaveDir(), {filter}}));
}
