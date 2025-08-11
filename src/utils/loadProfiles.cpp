#include "loadProfiles.hpp"
#include "loadExportData.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

using json = nlohmann::json;

void from_json(const json& j, Range& r) {
    j.at("id").get_to(r.id);
    j.at("from").get_to(r.from);
    j.at("to").get_to(r.to);
    j.at("checked").get_to(r.checked);
    j.at("note").get_to(r.note);
    j.at("attempts").get_to(r.attempts);
    j.at("completionCounter").get_to(r.completionCounter);
}

void from_json(const json& j, Stage& s) {
    j.at("id").get_to(s.id);
    j.at("stage").get_to(s.stage);
    j.at("checked").get_to(s.checked);
    j.at("note").get_to(s.note);
    j.at("attempts").get_to(s.attempts);
    j.at("completionCounter").get_to(s.completionCounter);
    j.at("ranges").get_to(s.ranges);
}

void from_json(const json& j, ProfileData& pd) {
    j.at("tags").get_to(pd.tags);
    j.at("stages").get_to(pd.stages);
}

void from_json(const json& j, Profile& p) {
    j.at("id").get_to(p.id);
    j.at("profileName").get_to(p.profileName);
    j.at("data").get_to(p.data);
}

std::vector<Profile> loadProfiles(const std::string& path) {
    auto j = loadExportData(path);
    return j.get<std::vector<Profile>>();
}