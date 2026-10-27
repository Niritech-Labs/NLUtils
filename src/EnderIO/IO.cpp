#include "../../include/NLUtils/EnderIO/IO.hpp"
#include <sstream>
#include <iostream>
#include <fstream> 
#include <cerrno>
#include <cstring>
using namespace EnderIO;



std::string ParserRealization::read() {
    std::ifstream file(filepath, std::ios::binary);
    errno = 0;
    if (!file.is_open()) {
        if (Logger) Logger->Error("Cannot read file - "+std::string(std::strerror(errno)),false,filepath.string());
        return ""; 
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
bool ParserRealization::write(std::string data) {
    std::ofstream out(filepath); 
    errno = 0;
    if (out.is_open()) {
        out << data;
        return true;
    } 
    if (Logger) Logger->Error("Cannot write file - "+std::string(std::strerror(errno)),false,filepath.string());
    return false;
    
}

void NLParser::Setup(bool production) {
    Logger.Setup(production,"NLParser");
}
void NLParser::AddParserRealization(std::string key,std::shared_ptr<ParserRealization> realization) {
    auto it = this->rls.find(key);
    if (it == rls.end()) {
        rls[key] = realization;
    }
    else {
        Logger.Error("Realization: "+key+" already added",false);
    }
}
std::shared_ptr<ParserRealization> NLParser::BindParser(std::string key,NLPath::Path filepath) {
    auto it = this->rls.find(key);
    if (it != rls.end()) {
        rls[key]->filepath = filepath.expanduser();
        rls[key]->Logger = &Logger;
        return rls[key]->Copy();
    }
    else {
        Logger.Error("Realization: "+key+" not found",false);
        return nullptr;
    }

}