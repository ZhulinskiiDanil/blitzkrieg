#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct Range {
    std::string id;
    int from;
    int to;
    bool checked;
    std::string note;
    int attempts;
    int completionCounter;
};

struct Stage {
    std::string id;
    int stage;
    bool checked;
    std::string note;
    int attempts;
    int completionCounter;
    std::vector<Range> ranges;
};

struct ProfileData {
    std::vector<int> tags;
    std::vector<Stage> stages;
};

struct Profile {
    std::string id;
    std::string profileName;
    ProfileData data;
};

// Только объявления:
nlohmann::json loadExportData(const std::string& path);
std::vector<Profile> loadProfiles(const std::string& path);