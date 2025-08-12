#include "loadExportData.hpp"
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

json loadExportData(const std::string &path)
{
    std::ifstream f(path);
    if (!f.is_open())
    {
        throw std::runtime_error("Cannot open " + path);
    }
    json j;
    f >> j;
    return j;
}
