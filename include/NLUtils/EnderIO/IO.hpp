#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cinttypes>
#include "Blocks.hpp"
#include "PathUtils.hpp"
#include <memory>
#include <functional>
#include <json.hpp>
#include <pugixml.hpp>
#include "../Logger.hpp"

namespace EnderIO {

#define PARSER_REALIZATION_COPY(Class)              \
std::shared_ptr<ParserRealization> Class::Copy() {  \
    return std::make_shared<Class>(*this);          \
}

class NLParser;
    
class ParserRealization {
public:
    NLPath::Path filepath;
    virtual Blocks Load() {return Blocks("bro wtf you doing?");}
    virtual bool Save(Blocks& data) {return false;}
    virtual ~ParserRealization() {}
protected:
    friend class NLParser;
    NLLogger* Logger = nullptr;
    std::string read();

    std::vector<std::string> conversionAllowed;

    virtual std::shared_ptr<ParserRealization> Copy() {
        if (Logger) Logger->Error("Invalid Parser",false);
        return nullptr;
    }
    bool write(std::string data);
};

class RAWParser : ParserRealization {
public:
    std::string Read(NLPath::Path filepath) {
        this->filepath = filepath.expanduser();
        return read();
    }
    bool Write(std::string data,NLPath::Path filepath) {
        this->filepath = filepath.expanduser();
        return write(data);
    }
};

class NLParser {
public:
    NL_LOGGER_METHODS 
    void Setup(bool production = true);
    void AddParserRealization(std::string key,std::shared_ptr<ParserRealization> realization);
    std::shared_ptr<ParserRealization> BindParser(std::string key,NLPath::Path filepath);
private:
    NLLogger Logger;
    std::unordered_map<std::string,std::shared_ptr<ParserRealization>> rls;
};


namespace FormatParsers {

    class BlocksParser : public ParserRealization{
    public:
        Blocks Load() override;
        bool Save(Blocks& data) override;
        std::shared_ptr<ParserRealization> Copy() override;
    };

    class JSONParser : public ParserRealization{
    public:
        JSONParser();
        Blocks Load() override;
        bool Save(Blocks& data) override;
    private:
        std::shared_ptr<ParserRealization> Copy() override;
        std::string jsObjToString(nlohmann::json& value);
        nlohmann::json blockToJs(Block& block);
        Block toBlock(std::string name, nlohmann::json& value);
        nlohmann::json stringToJsObj(std::string& value);
    };

    class XMLParser : public ParserRealization{
    public:
        XMLParser();
        Blocks Load() override;
        bool Save(Blocks& data) override;
    private:
        std::shared_ptr<ParserRealization> Copy() override;
        void toBlock(pugi::xml_node node, std::string name, Block& parent);
        void toNode(Block& block, pugi::xml_node parent);
    };

    class INIParser : public ParserRealization{
    public:
        INIParser();
        Blocks Load() override;
        bool Save(Blocks& data) override;
    private:
        std::shared_ptr<ParserRealization> Copy() override;
    };
    

}

}