#include "../include/NLUtils/JSONUtils.hpp"
#include <fstream>
#include <filesystem>
#include <cstdlib>


std::string JSONManager::expanduser(std::string path) {
    if (!path.empty() && path[0] == '~') {
        const char* home = nullptr;
        #if defined(_WIN32)
        home = std::getenv("USERPROFILE");
        #else
        home = std::getenv("HOME");
        #endif
        if (home) {
            path.replace(0, 1, home);
        }
    }
    try {
        return std::filesystem::weakly_canonical(
            std::filesystem::path(path)
        ).string();
    } catch (...) {
        return path;
    }
}


void JSONManager::Setup(std::string path, bool production) {
    Logger.Setup(production, "ConfigManager");
    configPath = expanduser(path);
}


nlohmann::json JSONManager::LoadConfig() {
    try {
        std::ifstream file(configPath);
        nlohmann::json data;
        file >> data;          
        return data;
    } catch (const std::exception& E) {
        Logger.Info(std::string(E.what()) + ", Can't load saved config, creating new", NLLogger::ConColors::STANDART, false);
        nlohmann::json defconf = nlohmann::json::object();
        SaveConfig(defconf);
        return defconf;
    }
}


nlohmann::json JSONManager::OpenRestricted(std::string path) {
    std::string resolved = expanduser(path);
    try {
        std::ifstream file(resolved);
        nlohmann::json data;
        file >> data;
        return data;
    } catch (const std::exception& E) {
        Logger.Error(std::string(E.what()) + ", Can't load this config: " + resolved, false);
        return nlohmann::json::object();
    }
}

void JSONManager::SaveRestricted(std::string path, const nlohmann::json& dataToSave) {
    std::string resolved = expanduser(path);
    try {
        auto parent = std::filesystem::path(resolved).parent_path();
        if (!parent.empty()) {
            std::filesystem::create_directories(parent);
        }
        std::ofstream file(resolved);
        file << dataToSave.dump(4, ' ', false, nlohmann::json::error_handler_t::replace);
        Logger.Info("Saved restricted config " + resolved,
                 NLLogger::ConColors::VIOLET, false);
    } catch (const std::exception& E) {
        Logger.Error(std::string(E.what()) + ", Can't save this config: " + resolved, false);
    }
}

// ------------------------------------------------------------
// SaveConfig
// ------------------------------------------------------------
void JSONManager::SaveConfig(const nlohmann::json& dataToSave) {
    try {
        auto parent = std::filesystem::path(configPath).parent_path();
        if (!parent.empty()) {
            std::filesystem::create_directories(parent);
        }
        std::ofstream file(configPath);
        file << dataToSave.dump(4, ' ', false, nlohmann::json::error_handler_t::replace);
        Logger.Info("Saved main config " + configPath, NLLogger::ConColors::VIOLET, false);
    } catch (const std::exception& E) {
        Logger.Error(std::string(E.what()) + ", Can't save this config: " + configPath, false);
    }
}