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




void paramIsMeta_Parsers(Block& block, std::string& name, std::string& value, bool& res) {
    res = false;
    std::string suffix = "-o-meta";
    if (name.size() <= suffix.size()) return;
    if (name.compare(name.size()-suffix.size(), suffix.size(), suffix) != 0) return;
    std::vector<std::string> metaWords = {"string","int","uint","float","bool","null","array","attribute"};
    if (nlfval value invector(metaWords)) {
        std::string partner = name.substr(0, name.size()-suffix.size());
        if (!block.FindParametr(partner).empty() || !block.FindBlock(partner).empty()) {
            res = true;
        }
    }
}
void paramMeta_Parsers(std::vector<std::pair<std::string,std::string>>& params, std::string& name, std::string& meta) {
    meta = "";
    std::string target = name+"-o-meta";
    forel& param inside(params) {
        if (param.first == target) {
            meta = param.second;
            return;
        }
    }
}

//JSON ############################################
void jsonValue_Parsers(std::string& value, std::string& meta, nlohmann::ordered_json& out) {
    if (meta == "int") {
        try {
            out = std::stoll(value);
        }
        catch (...) {
            out = value;
        }
    }
    else if (meta == "uint") {
        try {
            out = std::stoull(value);
        }
        catch (...) {
            out = value;
        }
    }
    else if (meta == "float") {
        try {
            out = std::stod(value);
        }
        catch (...) {
            out = value;
        }
    }
    else if (meta == "bool") {
        out = (value == "true");
    }
    else if (meta == "null") {
        out = nullptr;
    }
    else {
        out = value;
    }
}
void jsonToBlocks_Parsers(nlohmann::ordered_json& j, Block& parent, std::string key, NLLogger* Logger) {
    if (j.is_object()) {
        Block child(key);
        forel item inside(j.items()) {
            std::string memberKey = item.key();
            jsonToBlocks_Parsers(item.value(), child, memberKey, Logger);
        }
        parent.AddBlock(child);
        return;
    }
    if (j.is_array()) {
        Block child(key);
        size_t i = 0;
        forel item inside(j) {
            jsonToBlocks_Parsers(item, child, std::to_string(i), Logger);
            i++;
        }
        parent.AddBlock(child);
        std::string metaName = key+"-o-meta";
        if (parent.FindParametr(metaName).empty()) {
            std::pair<std::string,std::string> meta = {metaName, "array"};
            parent.AddParam(meta);
        }
        else {
            Logger->Warning("Meta name taken, array meta skipped: "+metaName);
        }
        return;
    }
    std::pair<std::string,std::string> param = {key, ""};
    std::string metaWord = "";
    if (j.is_string()) {
        param.second = j.get<std::string>();
    }
    else if (j.is_boolean()) {
        param.second = j.dump();
        metaWord = "bool";
    }
    else if (j.is_number_float()) {
        param.second = j.dump();
        metaWord = "float";
    }
    else if (j.is_number_unsigned()) {
        param.second = j.dump();
        metaWord = "uint";
    }
    else if (j.is_number_integer()) {
        param.second = j.dump();
        metaWord = "int";
    }
    else if (j.is_null()) {
        metaWord = "null";
    }
    parent.AddParam(param);
    if (!metaWord.empty()) {
        std::string metaName = key+"-o-meta";
        if (parent.FindParametr(metaName).empty()) {
            std::pair<std::string,std::string> meta = {metaName, metaWord};
            parent.AddParam(meta);
        }
        else {
            Logger->Warning("Meta name taken, meta skipped: "+metaName);
        }
    }
    bool isMeta = false;
    paramIsMeta_Parsers(parent, param.first, param.second, isMeta);
    if (isMeta) {
        Logger->Warning("Data key conflicts with meta namespace: "+key);
    }
}
void blocksToJson_Parsers(Block& block, nlohmann::ordered_json& out, bool asArray) {
    if (asArray) {
        out = nlohmann::ordered_json::array();
    }
    else {
        out = nlohmann::ordered_json::object();
    }
    std::vector<std::pair<std::string,std::string>> params = block.GetAllParams();
    std::vector<Block> children = block.GetAllBlocks();

    if (asArray) {
        //params and child blocks merged by numeric index, entries without a number go last
        std::vector<std::string> names;
        std::vector<bool> isBlk;
        std::vector<size_t> orig;
        size_t i = 0;
        forel& param inside(params) {
            bool isMeta = false;
            paramIsMeta_Parsers(block, param.first, param.second, isMeta);
            if (!isMeta) {
                names.push_back(param.first);
                isBlk.push_back(false);
                orig.push_back(i);
            }
            i++;
        }
        i = 0;
        forel& child inside(children) {
            names.push_back(child.GetName());
            isBlk.push_back(true);
            orig.push_back(i);
            i++;
        }
        std::vector<uint64_t> nums(names.size(), UINT64_MAX);
        size_t k = 0;
        forel& nm inside(names) {
            try {
                nums[k] = std::stoull(nm);
            }
            catch (...) {
            }
            k++;
        }
        std::vector<bool> used(names.size(), false);
        size_t count = names.size();
        for (size_t a = 0; a < count; a++) {
            size_t best = count;
            for (size_t b = 0; b < count; b++) {
                if (used[b]) continue;
                if (best == count || nums[b] < nums[best]) best = b;
            }
            used[best] = true;
            size_t e = best;
            if (isBlk[e]) {
                std::string elName = names[e];
                std::string meta = "";
                paramMeta_Parsers(params, elName, meta);
                nlohmann::ordered_json element;
                blocksToJson_Parsers(children[orig[e]], element, meta == "array");
                out.push_back(element);
            }
            else {
                std::pair<std::string,std::string>& param = params[orig[e]];
                std::string meta = "";
                paramMeta_Parsers(params, param.first, meta);
                nlohmann::ordered_json element;
                jsonValue_Parsers(param.second, meta, element);
                out.push_back(element);
            }
        }
        return;
    }

    forel& param inside(params) {
        bool isMeta = false;
        paramIsMeta_Parsers(block, param.first, param.second, isMeta);
        if (isMeta) continue;
        std::string meta = "";
        paramMeta_Parsers(params, param.first, meta);
        nlohmann::ordered_json value;
        jsonValue_Parsers(param.second, meta, value);
        if (out.find(param.first) != out.end()) {
            nlohmann::ordered_json prev = out[param.first];
            if (!prev.is_array()) {
                nlohmann::ordered_json arr = nlohmann::ordered_json::array();
                arr.push_back(prev);
                prev = arr;
            }
            prev.push_back(value);
            out[param.first] = prev;
        }
        else {
            out[param.first] = value;
        }
    }
    forel& child inside(children) {
        std::string childName = child.GetName();
        std::string meta = "";
        paramMeta_Parsers(params, childName, meta);
        nlohmann::ordered_json value;
        blocksToJson_Parsers(child, value, meta == "array");
        if (out.find(child.GetName()) != out.end()) {
            nlohmann::ordered_json prev = out[child.GetName()];
            if (!prev.is_array()) {
                nlohmann::ordered_json arr = nlohmann::ordered_json::array();
                arr.push_back(prev);
                prev = arr;
            }
            prev.push_back(value);
            out[child.GetName()] = prev;
        }
        else {
            out[child.GetName()] = value;
        }
    }
}
FormatParsers::JSONParser::JSONParser() {
    conversionAllowed = {"JSON","XML","INI"};
}
PARSER_REALIZATION_COPY(FormatParsers::JSONParser) 
bool FormatParsers::JSONParser::Save(Blocks& data) {
    if (nlfval data.GetName() invector(conversionAllowed)) {
        std::string rootname = data.GetName();
        std::vector<std::pair<std::string,std::string>> rootParams = data.GetAllParams();
        std::string rootMeta = "";
        paramMeta_Parsers(rootParams, rootname, rootMeta);
        nlohmann::ordered_json jdata;

        if (!rootMeta.empty()) {
            bool hasRootParam = false;
            forel& rp inside(rootParams) {
                if (rp.first == rootname) {
                    hasRootParam = true;
                }
            }
            auto found = data.FindBlock(rootname);
            if (!hasRootParam && !found.empty()) {
                Block* wrapped = found[found.size()-1];
                if (rootMeta == "array") {
                    blocksToJson_Parsers(*wrapped, jdata, true);
                }
                else {
                    std::vector<std::pair<std::string,std::string>> wparams = wrapped->GetAllParams();
                    std::string wvalue = "";
                    forel& wp inside(wparams) {
                        if (wp.first == rootname) {
                            wvalue = wp.second;
                        }
                    }
                    std::string wmeta = "";
                    paramMeta_Parsers(wparams, rootname, wmeta);
                    jsonValue_Parsers(wvalue, wmeta, jdata);
                }
            }
            else {
                if (!hasRootParam) {
                    Logger->Warning("Root meta without root block: "+rootname, filepath);
                }
                blocksToJson_Parsers(data, jdata, false);
            }
        }
        else {
            blocksToJson_Parsers(data, jdata, false);
        }
        return write(jdata.dump(4, ' ', false, nlohmann::ordered_json::error_handler_t::replace));
    }
    else {
        if (Logger) Logger->Warning("Conversion not allowed",filepath);
    }
    return false;
}
Blocks FormatParsers::JSONParser::Load() {
    Blocks root("JSON");
    nlohmann::ordered_json data;
    try {
        data = nlohmann::ordered_json::parse(read());
    }
    catch (...) {
        Logger->Warning("Invalid JSON in "+filepath.string(),filepath);
        return root;
    }

    if (data.is_object()) {
        forel item inside(data.items()) {
            std::string key = item.key();
            jsonToBlocks_Parsers(item.value(), root, key, Logger);
        }
    }
    else if (data.is_array()) {
        Block child(root.GetName());
        size_t i = 0;
        forel item inside(data) {
            jsonToBlocks_Parsers(item, child, std::to_string(i), Logger);
            i++;
        }
        root.AddBlock(child);
        std::pair<std::string,std::string> meta = {root.GetName()+"-o-meta", "array"};
        root.AddParam(meta);
    }
    else {
        Block child(root.GetName());
        jsonToBlocks_Parsers(data, child, root.GetName(), Logger);
        if (data.is_string()) {
            std::pair<std::string,std::string> meta = {root.GetName()+"-o-meta", "string"};
            child.AddParam(meta);
        }
        root.AddBlock(child);
        std::string rootname = root.GetName();
        std::vector<std::pair<std::string,std::string>> cparams = child.GetAllParams();
        std::string rootWord = "";
        paramMeta_Parsers(cparams, rootname, rootWord);
        std::pair<std::string,std::string> rmeta = {rootname+"-o-meta", rootWord};
        root.AddParam(rmeta);
    }

    return root;
}



//XML ############################################
void xmlElementToBlocks_Parsers(pugi::xml_node element, Block& parent, bool forceBlock, NLLogger* Logger) {
    bool asBlock = forceBlock;
    if (!asBlock) {
        forel attr inside(element.attributes()) {
            asBlock = true;
            break;
        }
    }
    if (!asBlock) {
        forel child inside(element.children()) {
            if (child.type() == pugi::node_element) {
                asBlock = true;
                break;
            }
        }
    }
    if (!asBlock) {
        std::string text = "";
        forel child inside(element.children()) {
            if (child.type() == pugi::node_pcdata || child.type() == pugi::node_cdata) {
                text += child.value();
            }
        }
        std::pair<std::string,std::string> param = {element.name(), text};
        parent.AddParam(param);
        return;
    }

    Block childBlock(element.name());
    forel attr inside(element.attributes()) {
        std::pair<std::string,std::string> param = {attr.name(), attr.value()};
        childBlock.AddParam(param);
        std::pair<std::string,std::string> meta = {std::string(attr.name())+"-o-meta", "attribute"};
        childBlock.AddParam(meta);
    }
    std::string text = "";
    bool hasText = false;
    bool hasElements = false;
    forel child inside(element.children()) {
        if (child.type() == pugi::node_pcdata || child.type() == pugi::node_cdata) {
            text += child.value();
            hasText = true;
        }
        else if (child.type() == pugi::node_element) {
            hasElements = true;
            xmlElementToBlocks_Parsers(child, childBlock, false, Logger);
        }
    }
    if (hasText) {
        if (hasElements) {
            Logger->Warning("Mixed content in <"+std::string(element.name())+">: text order not preserved");
        }
        std::pair<std::string,std::string> param = {"#text", text};
        childBlock.AddParam(param);
    }
    parent.AddBlock(childBlock);
}
void blocksToXml_Parsers(Block& block, pugi::xml_node element, NLLogger* Logger) {
    std::vector<std::pair<std::string,std::string>> params = block.GetAllParams();
    std::vector<Block> children = block.GetAllBlocks();

    std::string text = "";
    forel& param inside(params) {
        bool isMeta = false;
        paramIsMeta_Parsers(block, param.first, param.second, isMeta);
        if (isMeta) continue;
        if (param.first == "#text") {
            text += param.second;
            continue;
        }
        std::string meta = "";
        paramMeta_Parsers(params, param.first, meta);
        if (meta == "attribute") {
            element.append_attribute(param.first.c_str()).set_value(param.second.c_str());
        }
        else {
            if (param.first.empty() || !(std::isalpha((unsigned char)param.first[0]) || param.first[0] == '_')) {
                Logger->Warning("Name skipped in XML: "+param.first);
                continue;
            }
            pugi::xml_node child = element.append_child(param.first.c_str());
            if (!param.second.empty()) {
                child.append_child(pugi::node_pcdata).set_value(param.second.c_str());
            }
        }
    }
    if (!text.empty()) {
        element.append_child(pugi::node_pcdata).set_value(text.c_str());
    }
    forel& child inside(children) {
        std::string elName = child.GetName();
        if (elName.empty() || !(std::isalpha((unsigned char)elName[0]) || elName[0] == '_')) {
            Logger->Warning("Block name skipped in XML: "+elName);
            continue;
        }
        pugi::xml_node childElement = element.append_child(elName.c_str());
        blocksToXml_Parsers(child, childElement, Logger);
    }
}
FormatParsers::XMLParser::XMLParser() {
    conversionAllowed = {"XML","INI"};
}
PARSER_REALIZATION_COPY(FormatParsers::XMLParser) 
Blocks FormatParsers::XMLParser::Load() {
    Blocks root("XML");
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(filepath.c_str());
    if (!result) {
        Logger->Warning("Invalid XML: "+std::string(result.description()), filepath);
        return root;
    }

    bool foundRoot = false;
    forel node inside(doc.children()) {
        if (node.type() == pugi::node_element) {
            if (foundRoot) {
                Logger->Warning("Multiple root elements, extra skipped: "+std::string(node.name()), filepath);
                continue;
            }
            xmlElementToBlocks_Parsers(node, root, true, Logger);
            foundRoot = true;
        }
    }

    return root;
}
bool FormatParsers::XMLParser::Save(Blocks& data) {
    if (nlfval data.GetName() invector(conversionAllowed)) {
        pugi::xml_document doc;
        std::vector<Block> children = data.GetAllBlocks();
        std::vector<std::pair<std::string,std::string>> rootParams = data.GetAllParams();

        pugi::xml_node rootElement;
        if (rootParams.empty() && children.size() == 1) {
            std::string elName = children[0].GetName();
            if (elName.empty() || !(std::isalpha((unsigned char)elName[0]) || elName[0] == '_')) {
                Logger->Warning("Block name skipped in XML: "+elName, filepath);
                elName = data.GetName();
            }
            rootElement = doc.append_child(elName.c_str());
            blocksToXml_Parsers(children[0], rootElement, Logger);
        }
        else {
            rootElement = doc.append_child(data.GetName().c_str());
            blocksToXml_Parsers(data, rootElement, Logger);
        }
        return doc.save_file(filepath.c_str(), "  ");
    }
    else {
        if (Logger) Logger->Warning("Conversion not allowed",filepath);
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


