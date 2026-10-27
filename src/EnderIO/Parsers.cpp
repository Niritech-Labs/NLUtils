#include "../../include/NLUtils/EnderIO/IO.hpp"
#include "../../include/NLUtils/TextUtils.hpp"
using namespace EnderIO;
////////////////////////////////////////////////////////////// Parser
//Blocks ############################################
PARSER_REALIZATION_COPY(FormatParsers::BlocksParser) 
Blocks FormatParsers::BlocksParser::Load() {
    return Blocks::FromStr(read(),Logger);
}
bool FormatParsers::BlocksParser::Save(Blocks& data) {
    data.Update();
    return write(data.toStr());
}



//JSON ############################################
FormatParsers::JSONParser::JSONParser() {
    conversionAllowed = {"JSON","XML","INI"};
}
PARSER_REALIZATION_COPY(FormatParsers::JSONParser) 
bool parseIndexSuffix_Parsers(const std::string& name, std::size_t& index) {
    if (name.size() < 3 || name.back() != ']') return false;

    const std::size_t open = name.find_last_of('[');
    if (open == std::string::npos) return false;

    const std::string digits = name.substr(open + 1, name.size() - open - 2);
    if (digits.empty() || digits.find_first_not_of("0123456789") != std::string::npos) return false;

    index = static_cast<std::size_t>(std::stoull(digits));
    return true;
}
nlohmann::json FormatParsers::JSONParser::stringToJsObj(std::string& value) {
    if (value == "null")  return nlohmann::json(nullptr);
    if (value == "true")  return nlohmann::json(true);
    if (value == "false") return nlohmann::json(false);

    char first = value.empty() ? '\0' : value.front();
    bool looksNumeric = (first >= '0' && first <= '9') || first == '-' || first == '+' || first == '.';

    if (looksNumeric) {
        std::size_t pos = 0;
        try { long long i = std::stoll(value, &pos); if (pos == value.size()) return nlohmann::json(i); } catch (...) {}
        try { double d = std::stod(value, &pos); if (pos == value.size()) return nlohmann::json(d); } catch (...) {}
    }

    return nlohmann::json(value);
}
nlohmann::json FormatParsers::JSONParser::blockToJs(Block& block) {
    std::vector<std::pair<std::string, std::string>> params = block.GetAllParams();
    std::vector<Block> children = block.GetAllBlocks();

    struct Entry { 
        std::size_t index; 
        bool isBlock; 
        std::size_t src; 
    };
    std::vector<Entry> entries;
    bool isArray = !params.empty() || !children.empty();

    for (std::size_t i = 0; isArray && i < params.size(); ++i) {
        std::size_t idx = 0;

        if (parseIndexSuffix_Parsers(params[i].first, idx)) entries.push_back({idx, false, i});
        else isArray = false;
    }
    for (std::size_t i = 0; isArray && i < children.size(); ++i) {
        std::size_t idx = 0;

        if (parseIndexSuffix_Parsers(children[i].GetName(), idx)) entries.push_back({idx, true, i});
        else isArray = false;
    }

    if (isArray) {
        std::sort(entries.begin(), entries.end(),
        [](Entry& a,Entry& b) { 
            return a.index < b.index; 
        });
        for (std::size_t k = 0; isArray && k < entries.size(); ++k) {
            if (entries[k].index != k) isArray = false;
        }
    }

    if (isArray) {
        nlohmann::json arr = nlohmann::json::array();
        for (Entry& e : entries) {
            if (e.isBlock)
                arr.push_back(blockToJs(children[e.src]));
            else
                arr.push_back(stringToJsObj(params[e.src].second));
        }
        return arr;
    }

    nlohmann::json obj = nlohmann::json::object();
    for (auto& p : params)
        obj[p.first] = stringToJsObj(p.second);
    for (auto& c : children)
        obj[c.GetName()] = blockToJs(c);

    return obj;
}
std::string FormatParsers::JSONParser::jsObjToString(nlohmann::json& value) {
    return value.is_string() ? value.get<std::string>() : value.dump();
}
Block FormatParsers::JSONParser::toBlock(std::string name,nlohmann::json& value) {
    Block block(name);

    if (value.is_object()) {
        for (auto it = value.begin(); it != value.end(); ++it) {
            nlohmann::json& v = it.value();

            if (v.is_object() || v.is_array()) {
                Block child = toBlock(it.key(), v);
                block.AddBlock(child);
            } 
            else {
                std::pair<std::string, std::string> param{it.key(), jsObjToString(v)};
                block.AddParam(param);
            }
        }
    }
    else if (value.is_array()) {
        for (std::size_t i = 0; i < value.size(); ++i) {
            nlohmann::json& v = value[i];
            std::string elementName = name + '[' + std::to_string(i) + ']';

            if (v.is_object() || v.is_array()) {
                Block child = toBlock(elementName, v);
                block.AddBlock(child);
            } 
            else {
                std::pair<std::string, std::string> param{elementName, jsObjToString(v)};
                block.AddParam(param);
            }
        }
    }

    return block;
}


bool FormatParsers::JSONParser::Save(Blocks& data) {
    nlohmann::json jdata = blockToJs(data);
    if (nlfval data.GetName() invector(conversionAllowed)) {
        return write(jdata.dump(4, ' ', false, nlohmann::json::error_handler_t::replace));
    }
    else {
        if (Logger) Logger->Warning("Conversion not allowed",filepath);
    }
    return false;
    
}
Blocks FormatParsers::JSONParser::Load() {
    nlohmann::json data = read();
    Blocks root("JSON");

    if (data.is_discarded()) 
        return root;

    if (data.is_object()) {
        for (auto it = data.begin(); it != data.end(); ++it) {
            Block child = toBlock(it.key(), it.value());
            root.AddBlock(child);
        }
    }
    else if (data.is_array()) {
        for (std::size_t i = 0; i < data.size(); ++i) {
            Block child = toBlock("item[" + std::to_string(i) + ']', data[i]);
            root.AddBlock(child);
        }
    }
    else if (!data.is_null()) {
        std::pair<std::string, std::string> param{"value", jsObjToString(data)};
        root.AddParam(param);
    }

    return root;
}



//XML ############################################
FormatParsers::XMLParser::XMLParser() {
    conversionAllowed = {"JSON","XML","INI"};
}
PARSER_REALIZATION_COPY(FormatParsers::XMLParser) 
std::string stripIndexSuffix(const std::string& name) {
    std::string base = name;
    std::size_t index = 0;
    while (parseIndexSuffix_Parsers(base, index))
        base = base.substr(0, base.find_last_of('['));
    return base;
}
void FormatParsers::XMLParser::toBlock(pugi::xml_node node, std::string name, Block& parent) {
    const bool hasAttrs = (node.first_attribute() != nullptr);

    bool hasElems = false;
    for (pugi::xml_node c = node.first_child(); c; c = c.next_sibling()) {
        if (c.type() == pugi::node_element) {
            hasElems = true;
            break;
        }
    }

    std::string text = node.text().as_string();
    NLTextUtils::StripIn(text);


    if (!hasAttrs && !hasElems && !text.empty()) {
        std::pair<std::string, std::string> param{name, text};
        parent.AddParam(param);
        return;
    }

    Block block(name);

    for (pugi::xml_attribute a = node.first_attribute(); a; a = a.next_attribute()) {
        std::pair<std::string, std::string> param{a.name(), a.value()};
        block.AddParam(param);
    }

    if (!text.empty()) { 
        std::pair<std::string, std::string> textParam{"#text", text};
        block.AddParam(textParam);
    }

    std::unordered_map<std::string, std::size_t> tagCount, tagIndex;

    for (pugi::xml_node c = node.first_child(); c; c = c.next_sibling())
        if (c.type() == pugi::node_element)
            ++tagCount[c.name()];

    for (pugi::xml_node c = node.first_child(); c; c = c.next_sibling()) {
        if (c.type() != pugi::node_element) continue; 
        const std::string tag = c.name();

        if (tagCount[tag] > 1) toBlock(c, tag + '[' + std::to_string(tagIndex[tag]++) + ']', block);
        else toBlock(c, tag, block);
    }

    parent.AddBlock(block);
}
void FormatParsers::XMLParser::toNode(Block& block, pugi::xml_node parent) {
    pugi::xml_node node = parent.append_child(stripIndexSuffix(block.GetName()).c_str());

    std::size_t idx = 0;
    for (auto& p : block.GetAllParams()) {
        if (p.first == "#text") {
            node.text().set(p.second.c_str());
        }
        else if (parseIndexSuffix_Parsers(p.first, idx)) {
            node.append_child(stripIndexSuffix(p.first).c_str()).text().set(p.second.c_str());
        }
        else {
            node.append_attribute(p.first.c_str()) = p.second.c_str();
        }
    }

    for (Block& c : block.GetAllBlocks())
        toNode(c, node);
}


Blocks FormatParsers::XMLParser::Load() {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(filepath.c_str());
    Blocks root("XML");

    if (!result) return root;

    
    for (pugi::xml_node c = doc.first_child(); c; c = c.next_sibling()) {
        if (c.type() == pugi::node_element) {
            toBlock(c, c.name(), root);
            break;
        }
    }

    return root;
}
bool FormatParsers::XMLParser::Save(Blocks& data) {
    pugi::xml_document doc;

    std::vector<Block> children = data.GetAllBlocks();

    
    if (children.size() == 1) {
        toNode(children[0], doc);
    }
    else if (!children.empty()) {
        pugi::xml_node wrapper = doc.append_child("root"); 
        for (Block& c : children) toNode(c, wrapper);
    }
    if (nlfval data.GetName() invector(conversionAllowed)) {
        return doc.save_file(filepath.c_str(), "  ");
    }
    return false;
}


//INI ############################################
FormatParsers::INIParser::INIParser() {
    conversionAllowed = {"INI"};
}
PARSER_REALIZATION_COPY(FormatParsers::INIParser) 
Blocks FormatParsers::INIParser::Load() {
    Blocks root("INI");

    std::string data = read();
    if (data.empty()) return root;

    std::istringstream stream(data);
    std::vector<std::string> lines;
    std::string line;
    bool ignore = false;
    bool inBlock = true;

    std::string comments = ";#";
    while (std::getline(stream, line)) {
        forel& comment inside(comments) {
            if (line[0] == comment) {
                ignore = true;
            }
        }
        if (!ignore) lines.push_back(line);
        else ignore = false;
    }

    
    std::string iniBlocks = "[]";

    Block child("");
    Block* childptr = nullptr;

    forel& line inside(lines) {
        if (line[0] == iniBlocks[0]) {
            if (nlfval iniBlocks[1] instring(line)) {
                std::string res = NLTextUtils::Split(line,"]")[0];
                NLTextUtils::ReplaceIn(res,"[","");
                
                Block newBlock(res);
                root.AddBlock(newBlock);
                auto blocksres = root.FindBlock(res); 
                childptr = blocksres[blocksres.size()-1];
            }
            else { 
                Logger->Warning("Invalid INI block: "+line,filepath);
            }
        }
        else if (nlfval "=" instring(line)) {
            auto pair = NLTextUtils::Split(line,"=");
            if (pair.size() == 2) {
                std::pair<std::string,std::string> param = {NLTextUtils::Strip(pair[0]),NLTextUtils::Strip(pair[1])};
                if (childptr) {
                    childptr->AddParam(param);
                }
                else {
                    root.AddParam(param);
                }
            }
            else {
                Logger->Warning("Invalid INI parametr: "+line,filepath);
            }
        }
        else if (nlfval ":" instring(line)) {
            auto pair = NLTextUtils::Split(line,"=");
            if (pair.size() == 2) {
                std::pair<std::string,std::string> param = {NLTextUtils::Strip(pair[0]),NLTextUtils::Strip(pair[1])};
                if (childptr) {
                    childptr->AddParam(param);
                }
                else {
                    root.AddParam(param);
                }
            }
            else {
                Logger->Warning("Invalid INI parametr: "+line,filepath);
            }
        }
        else if (!line.empty()) {
            Logger->Warning("Invalid INI line: "+line,filepath);
        }
    }



    return root;
}
bool FormatParsers::INIParser::Save(Blocks& data) {
    if (nlfval data.GetName() invector(conversionAllowed)) {
        std::string out;

        std::vector<std::pair<std::string,std::string>> rootParams = data.GetAllParams();
        if (!rootParams.empty()) {  
            forel& paramr inside(rootParams) out += NLTextUtils::Strip(paramr.first) + " = " + paramr.second + '\n';
            out += '\n';
        }

        std::vector<Block> children = data.GetAllBlocks();
        forel& child inside(children) {
            std::vector<std::pair<std::string,std::string>> params = child.GetAllParams();
            if (!params.empty()) {  
                out += "["+child.GetName()+"]\n";
                forel& param inside(params) {
                    out += NLTextUtils::Strip(param.first) + " = " + param.second + '\n';
                }
                out += '\n';

                forel& childOfChild inside(child.GetAllBlocks()) {
                    if (Logger) Logger->Warning("Data in block: " + childOfChild.GetName() + " not writed", filepath);
                }
            }
        }

        return write(out);
    }
    else {
        if (Logger) Logger->Warning("Conversion not allowed",filepath);
    }

    return false;
}


