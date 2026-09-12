#pragma once

#include <string>
#include <optional>
#include <pugixml.hpp>
#include "Logger.hpp"
#pragma once
#include <string>
#include "Logger.hpp"

class XMLManager {
public:
    void Setup(std::string path, bool production);

    pugi::xml_document LoadConfig();
    pugi::xml_document OpenRestricted(std::string path);
    void SaveRestricted(std::string path, const pugi::xml_document& doc);
    void SaveConfig(const pugi::xml_document& doc);

private:
    std::string expanduser(std::string path);
    NLLogger Logger;
    std::string configPath;
};