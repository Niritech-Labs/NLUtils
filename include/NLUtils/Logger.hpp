#pragma once

#include <string>
#include <stack>
#include <iostream>
#include <cstdlib>
#include <vector>

class NLLogger {
public:
    enum ConColors {
        RED, GREEN, YELLOW, BLUE, VIOLET, STANDART, LIGHTBLUE, WHITE, SANDY, GRAY,
        RED_B, GREEN_B, YELLOW_B, BLUE_B, VIOLET_B, WHITE_B, LIGHTBLUE_B, SANDY_B, GRAY_B
    };

    void Setup(bool production, std::string ComponentName = "", std::stack<std::string>* logStack = nullptr);

    void Warning(std::string warn);
    void Error(std::string err, bool critical);
    void Warning(std::string warn, std::string layer);
    void Error(std::string err, bool critical, std::string layer);
    void Info(std::string inf, ConColors color, bool productionLatency);
    void Info(std::string inf, ConColors color, bool productionLatency, std::string layer);
    void SetBlacklist(std::vector<std::string> bl);
    void SetWhitelist(std::vector<std::string> wl);
    std::string ansiCode(ConColors c);

private: 
    bool production;
    std::string name;
    std::stack<std::string>* logStack;
    std::vector<std::string> blacklist;
    std::vector<std::string> whitelist;
    bool toLogStack;
};

