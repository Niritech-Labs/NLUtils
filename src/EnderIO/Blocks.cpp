#include "../../include/NLUtils/EnderIO/Blocks.hpp"
#include "../../include/NLUtils/Extra/Lang.hpp"
#include "../../include/NLUtils/EnderIO/IO.hpp"
#include <vector>
#include <memory>
#include <sstream>
#include "../../include/NLUtils/TextUtils.hpp"

using namespace EnderIO;
using namespace NLTextUtils;


Block::Block(std::string name) {
    this->name = name;
}
std::vector<std::pair<std::string,std::string>*> Block::FindParametr(std::string paramName,bool recursive) {
    std::vector<std::pair<std::string,std::string>*> paramsFinded;
    forel& param inside(Params) {
        if (param.first == paramName) paramsFinded.push_back(&param);
    }
    if (recursive) {
        forel& block inside(cBlocks) {
            std::vector<std::pair<std::string,std::string>*> paramsFindedRcs = block.FindParametr(paramName,true);
            paramsFinded.insert(paramsFinded.end(),paramsFindedRcs.begin(),paramsFindedRcs.end());
        }
    }
    

    return paramsFinded;
}
std::vector<Block*> Block::FindBlock(std::string blockName,bool recursive) {
    std::vector<Block*> blocksFinded;
    forel& block inside(cBlocks) {
        if (block.name == blockName) blocksFinded.push_back(&block);
        if (recursive) {
            std::vector<Block*> blocksFindedRc = block.FindBlock(blockName,true);
            blocksFinded.insert(blocksFinded.end(),blocksFindedRc.begin(),blocksFindedRc.end());
        }
    }

    return blocksFinded;
}

void Block::AddBlock(Block& block) {
    this->cBlocks.push_back(block);
}
void Block::AddParam(std::pair<std::string,std::string>& param) {
    this->Params.push_back(param);
}
void Block::SetBlocks(std::vector<Block>& blocks) {
    this->cBlocks = blocks;
}
void Block::SetParams(std::vector<std::pair<std::string,std::string>>& params) {
    this->Params = params;
}

void Block::DeleteMarkedObjects() {
    std::vector<std::pair<std::string,std::string>> toDeletePair;
    std::vector<Block> toDeleteBlocks;
    forel& param inside(Params) {
        if (param.first == "-o-toDelete-o-") {
            toDeletePair.push_back(param);
        }
    }
    forel& tdp inside(toDeletePair) {
        std::erase(Params,tdp);
    }

    forel& block inside(cBlocks) {
        block.DeleteMarkedObjects();
        if (block.toDelete) {
            toDeleteBlocks.push_back(block);
        }
    }
    forel& tdb inside(toDeleteBlocks) {
        std::erase(cBlocks,tdb);
    }
}
void Block::DeleteAllBlocks() {
    this->cBlocks.clear();
}
void Block::DeleteAllParams() {
    this->Params.clear();
}

std::vector<std::pair<std::string,std::string>> Block::GetAllParams() {
    return this->Params;
}
std::vector<Block> Block::GetAllBlocks() {
    return this->cBlocks;
}

void Block::markToDelete(std::pair<std::string,std::string>& param) {
    param.first = "-o-toDelete-o-";
}
void Block::markToDelete(Block& block) {
    block.toDelete = true;
}

std::string Block::GetName() {
    return this->name;
}
std::string Block::toStr() {
    std::string blockPadding = "";
    if (level != 1) {
        blockPadding = std::string(8 * (level - 1), ' ');
    }

    std::string blocks = blockPadding+"name: "+name+"\n";
    if (!Params.empty()) {
        blocks += blockPadding + "    params:\n";
        forel param inside(this->Params) {
            blocks += blockPadding + "        "+param.first+" = "+param.second+"\n";
        }
    }

    if (!this->cBlocks.empty()) {
        blocks += blockPadding + "    blocks:\n";
        forel block inside(this->cBlocks) {
            blocks += block.toStr();
        }
        blocks += "\n";
    }

    return blocks;
}


void Block::resetLevel() {
    this->level = 1;
    forel& block inside(this->cBlocks) {
        block.resetLevel();
    }
}
void Block::updateLevel() {
    forel& block inside(this->cBlocks) {
        block.level += this->level;
        block.updateLevel();
    }
}
void Blocks::Update() {
    resetLevel();
    updateLevel();
}



struct BlockLineMeta {
    std::string content;   
    uint32_t indent = 0;   
    uint32_t number = 0;   
};
void fail_blocks(uint32_t number, std::string what,NLLogger* Logger) {
    if (Logger) Logger->Error(what,false,"Line "+std::to_string(number));
}
bool isBlank_blocks(const std::string& content) {
    return NLTextUtils::Strip(content).empty();
}
bool isNameLine_blocks(const std::string& content) {
    if (content == "name:")
        return true;
    return content.compare(0, 6, "name: ") == 0;
}
std::string extractName_blocks(const BlockLineMeta& meta,NLLogger* Logger) {
    if (!isNameLine_blocks(meta.content)) { 
        fail_blocks(meta.number, "expected block name line 'name: <name>'",Logger); 
        return std::string(); 
    }
    if (meta.content == "name:") return std::string();
    std::string name = meta.content.substr(6);
    NLTextUtils::StripIn(name);
    return name;
}
std::vector<BlockLineMeta> analize_blocks(std::string& data,NLLogger* Logger) {
    std::vector<BlockLineMeta> lines;
    uint32_t number = 0;
    forel part inside(NLTextUtils::Split(data, "\n")) {
        number++;
        BlockLineMeta meta;
        meta.number = number;
        uint32_t indent = 0;
        while (indent < part.size() && part[indent] == ' ') indent++;
        if (indent < part.size() && part[indent] == '\t') { 
            fail_blocks(meta.number, "tab in indentation (spaces only)",Logger); 
            continue; 
        }
        meta.indent = indent;
        meta.content = part.substr(indent);
        lines.push_back(meta);
    }
    return lines;
}
void parseBlock_blocks(std::vector<BlockLineMeta>& lines, size_t& i, uint32_t myIndent, Block& target,NLLogger* Logger) {
    bool paramsHeader = false;
    bool blocksHeader = false;
    size_t paramCount = 0;
    size_t blockCount = 0;
    uint32_t paramsHeaderNumber = 0;
    uint32_t blocksHeaderNumber = 0;

    while (i < lines.size()) {
        BlockLineMeta& meta = lines[i];

        if (isBlank_blocks(meta.content)) {
            i++;
            continue;
        }
        if (meta.indent <= myIndent) break;  

        if (meta.indent == myIndent + 4) {
            std::string header = meta.content;
            NLTextUtils::StripIn(header);
            if (header == "params:") {
                if (paramsHeader) fail_blocks(meta.number, "duplicate 'params:' header",Logger);
                if (blocksHeader) fail_blocks(meta.number, "'params:' header after 'blocks:' header",Logger);
                paramsHeader = true;
                paramsHeaderNumber = meta.number;
            } 
            else if (header == "blocks:") {
                if (blocksHeader) fail_blocks(meta.number, "duplicate 'blocks:' header",Logger);
                if (paramsHeader && paramCount == 0) fail_blocks(meta.number, "'params:' header without parameters",Logger);
                blocksHeader = true;
                blocksHeaderNumber = meta.number;
            } 
            else {
                fail_blocks(meta.number, "expected 'params:' or 'blocks:' header",Logger);
            }

            i++;

            continue;
        }

        if (meta.indent == myIndent + 8) {
            if (isNameLine_blocks(meta.content)) {
                if (!blocksHeader) fail_blocks(meta.number, "child block without 'blocks:' header",Logger);
                uint32_t childIndent = meta.indent;
                std::string childName = extractName_blocks(meta,Logger);
                i++;
                Block child(childName);
                parseBlock_blocks(lines, i, childIndent, child,Logger);
                target.AddBlock(child);
                blockCount++;
            } 
            else if (nlfval " = " instring(meta.content)) {
                if (blocksHeader) fail_blocks(meta.number, "parameter after 'blocks:' header",Logger);
                if (!paramsHeader) fail_blocks(meta.number, "parameter without 'params:' header",Logger);
                
                NLTypes::FixedList<std::string> parts = NLTextUtils::Split(meta.content, " = ", 2);
                std::pair<std::string, std::string> param(parts[0], parts[1]);
                NLTextUtils::StripIn(param.first);
                NLTextUtils::StripIn(param.second);
                target.AddParam(param);
                paramCount++;
                i++;
            } 
            else { 
                fail_blocks(meta.number, "expected 'name: <name>' or 'key = value' line",Logger);
                i++; 
            }
            
            continue;
        }

        fail_blocks(meta.number, "unexpected indentation (a header goes at +4, its content at +8)",Logger);
        i++;
    }

    if (paramsHeader && paramCount == 0) fail_blocks(paramsHeaderNumber, "'params:' header without parameters",Logger);
    if (blocksHeader && blockCount == 0) fail_blocks(blocksHeaderNumber, "'blocks:' header without child blocks",Logger);
}

Blocks Blocks::FromStr(std::string data, NLLogger* Logger) {
    Blocks result("");
    if (data.empty()) return result;
    data = NLTextUtils::Replace(data, "\r\n", "\n");

    std::vector<BlockLineMeta> lines = analize_blocks(data,Logger);

    size_t i = 0;
    while (i < lines.size() && isBlank_blocks(lines[i].content)) i++;
    if (i == lines.size()) fail_blocks(lines.empty() ? 1 : lines.back().number, "text contains no blocks",Logger);

    BlockLineMeta& first = lines[i];
    if (first.indent != 0) fail_blocks(first.number, "root block must have zero indentation",Logger);

    Blocks parsed(extractName_blocks(first,Logger));
    i++;
    parseBlock_blocks(lines, i, 0, parsed,Logger);

    while (i < lines.size()) {
        if (!isBlank_blocks(lines[i].content))
            fail_blocks(lines[i].number, "unexpected content after the root block (exactly one root allowed)",Logger);
        i++;
    }

    result = parsed;
    result.Update();
    return result;
}


void Blocks::AddNewRootBlock(std::string name) {
    auto block = Block(this->name);
    block.SetBlocks(this->cBlocks); 
    block.SetParams(this->Params);
    this->DeleteAllBlocks();
    this->DeleteAllParams();
    this->name = name;
    this->AddBlock(block);
    this->Update();

}




