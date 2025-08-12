#include "loadProfiles.hpp"
#include "loadExportData.hpp"

#include <Geode/Geode.hpp>
#include <nlohmann/json.hpp>

using namespace geode::prelude;
using json = nlohmann::json;

std::vector<Profile> loadProfiles() {
    // Проверяем, есть ли уже сохранённая строка с профилями
    auto savedProfiles = Mod::get()->getSavedValue<std::string>("profiles");
    if (!savedProfiles.empty()) {
        geode::log::debug("Profiles already loaded from saved value");
        return json::parse(savedProfiles).get<std::vector<Profile>>();
    }

    geode::utils::file::pick;

    // Путь к export.json внутри ресурсов
    auto resourcesDir = Mod::get()->getResourcesDir();
    auto fullPath = (resourcesDir / "export.json").string();

    // Читаем JSON из файла
    auto j = loadExportData(fullPath);

    // Сохраняем как строку в saved data
    Mod::get()->setSavedValue("profiles", j.dump());

    geode::log::debug("Profiles loaded from export.json and saved");

    // Возвращаем результат
    return j.get<std::vector<Profile>>();
}
