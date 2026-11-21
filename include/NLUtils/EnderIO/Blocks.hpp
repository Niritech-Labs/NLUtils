#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <cinttypes>
#include "../Logger.hpp"

namespace EnderIO {


class Block {
public:
    Block(std::string name);

    bool operator==(const Block&) const = default; 

    std::vector<std::pair<std::string,std::string>*> FindParametr(std::string paramName,bool recursive = false);
    std::vector<Block*> FindBlock(std::string paramName,bool recursive = false);

    void AddBlock(Block& block);
    void AddParam(std::pair<std::string,std::string>& param);
    void SetBlocks(std::vector<Block>& blocks);
    void SetParams(std::vector<std::pair<std::string,std::string>>& params);

    void DeleteMarkedObjects();
    void DeleteAllBlocks();
    void DeleteAllParams();

    //Converts a list of parameters into a map. Warning: In the case of duplicate elements, the list will be trimmed down to unique elements.
    std::vector<std::pair<std::string,std::string>> GetAllParams();

    bool ParamsIsUnique();
    std::vector<Block> GetAllBlocks();

    std::unordered_map<std::string,std::string> GetParamsAsMap();

    static void markToDelete(std::pair<std::string,std::string>& param);
    static void markToDelete(Block& block);

    std::string GetName();
    std::string toStr();
    bool IsEmpty();

protected:
    void resetLevel();
    void updateLevel();

    std::string name;
    std::vector<Block> cBlocks;
    std::vector<std::pair<std::string,std::string>> Params;

    uint16_t level = 1;
    bool toDelete = false;
};

class Blocks : public Block {
public:
    using Block::Block;
    void Update();
    static Blocks FromStr(std::string data,NLLogger* Logger = nullptr);
    void AddNewRootBlock(std::string name);
};


}