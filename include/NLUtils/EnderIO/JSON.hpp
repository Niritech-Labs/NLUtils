#pragma once

#include <string>
#include <optional>
#include "IO.hpp"
#include "../../../libs/nlohmann/json.hpp"
#include "../Logger.hpp"

class Nlohman_JSONParser : EnderIO::ParserRealization {
public:
    nlohmann::json Read(NLPath::Path filepath) {
        this->filepath = filepath.expanduser();
        return nlohmann::json::parse(read());
    }
    bool Write(nlohmann::json data,NLPath::Path filepath) {
        this->filepath = filepath.expanduser();
        return write(data.dump(4, ' ', false, nlohmann::json::error_handler_t::replace));
    }
};


