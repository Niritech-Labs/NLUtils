#pragma once

#include <string>
#include <optional>
#include <pugixml.hpp>
#include "../Logger.hpp"
#include "IO.hpp"

class Pugi_XMLParser : EnderIO::ParserRealization {
public:
    pugi::xml_document Read(NLPath::Path filepath) {
        this->filepath = filepath.expanduser();
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_file(this->filepath.c_str());
        return doc;
    }
    bool Write(const pugi::xml_document& doc,NLPath::Path filepath) {
        this->filepath = filepath.expanduser();
        return doc.save_file(this->filepath.c_str(), "  ");
    }
};


