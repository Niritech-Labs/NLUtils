#pragma once

#include <string>
#include <optional>
#include "../../libs/nlohmann/json.hpp"
#include "Logger.hpp"

class JSONManager {
public:
    void Setup(std::string path, bool production);

    nlohmann::json LoadConfig();
    nlohmann::json OpenRestricted(std::string path);
    void SaveRestricted(std::string path, const nlohmann::json& dataToSave);
    void SaveConfig(const nlohmann::json& dataToSave);

private:
    std::string expanduser(std::string path);
    NLLogger Logger;
    std::string configPath;
};

