#include "../include/NLUtils/XMLUtils.hpp"
#include <fstream>
#include <filesystem>
#include <cstdlib>

std::string XMLManager::expanduser(std::string path) {
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
    } 
    catch (...) {
        return path;
    }
}

void XMLManager::Setup(std::string path, bool production) {
    Logger.Setup(production, "XMLManager");
    configPath = expanduser(path);
}

pugi::xml_document XMLManager::LoadConfig() {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(configPath.c_str());
    if (result) {
        return doc;  
    } 
    else {
        Logger.Info(std::string(result.description()) + ", Can't load saved config, creating new", NLLogger::ConColors::STANDART, false);
        doc.append_child("config");
        SaveConfig(doc);
        return doc;
    }
}

pugi::xml_document XMLManager::OpenRestricted(std::string path) {
    std::string resolved = expanduser(path);
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(resolved.c_str());
    if (!result) {
        Logger.Error(std::string(result.description()) + ", Can't load this config: " + resolved, false);
        doc.append_child("config");
    }
    return doc;  
}

void XMLManager::SaveRestricted(std::string path, const pugi::xml_document& doc) {
    std::string resolved = expanduser(path);
    try {
        std::filesystem::create_directories(
            std::filesystem::path(resolved).parent_path()
        );
        if (!doc.save_file(resolved.c_str(), "  ")) { 
            Logger.Error("Failed to save file: " + resolved, false);
        } 
        else {
            Logger.Info("Saved restricted config " + resolved,NLLogger::ConColors::VIOLET, false);
        }
    } 
    catch (const std::exception& E) {
        Logger.Error(std::string(E.what()) + ", Can't save this config: " + resolved, false);
    }
}

void XMLManager::SaveConfig(const pugi::xml_document& doc) {
    try {
        std::filesystem::create_directories(
            std::filesystem::path(configPath).parent_path()
        );
        if (!doc.save_file(configPath.c_str(), "  ")) {
            Logger.Error("Failed to save file: " + configPath, false);
        } 
        else {
            Logger.Info("Saved main config " + configPath,NLLogger::ConColors::VIOLET, false);
        }
    } catch (const std::exception& E) {
        Logger.Error(std::string(E.what()) + ", Can't save this config: " + configPath, false);
    }
}