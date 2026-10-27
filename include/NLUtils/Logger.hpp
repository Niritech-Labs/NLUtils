#pragma once

#include <string>
#include <stack>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <functional>

#define NL_LOGGER_METHODS                               \
    void SetBlacklist(std::vector<std::string> bl) {    \
        Logger.SetBlacklist(bl);                        \
    }                                                   \
    void SetWhitelist(std::vector<std::string> wl) {    \
        Logger.SetWhitelist(wl);                        \
    }                                                   \
    void SetOutCallbacks(NLLogger_OutCallbacks cb) {    \
        Logger.SetOutCallbacks(cb);                     \
    }                                                   \
    void UnsetOutCallbacks() {                          \
        Logger.UnsetOutCallbacks();                     \
    }

#define NL_LOGGER_METHODS_PTR                           \
    void SetBlacklist(std::vector<std::string> bl) {    \
        Logger->SetBlacklist(bl);                       \
    }                                                   \
    void SetWhitelist(std::vector<std::string> wl) {    \
        Logger->SetWhitelist(wl);                       \
    }                                                   \
    void SetOutCallbacks(NLLogger_OutCallbacks cb) {    \
        Logger->SetOutCallbacks(cb);                    \
    }                                                   \
    void UnsetOutCallbacks() {                          \
        Logger->UnsetOutCallbacks();                    \
    }

struct NLLogger_OutCallbacks {
   std::function<void(std::string)> Info;
   std::function<void(std::string)> Error;
   std::function<void(std::string)> Warning;
};

class NLLogger {
public:
    enum ConColors {
        RED, GREEN, YELLOW, BLUE, VIOLET, STANDART, LIGHTBLUE, WHITE, SANDY, GRAY,
        RED_B, GREEN_B, YELLOW_B, BLUE_B, VIOLET_B, WHITE_B, LIGHTBLUE_B, SANDY_B, GRAY_B
    };

    void Setup(bool production, std::string ComponentName = "");

    void SetOutCallbacks(NLLogger_OutCallbacks cb);
    void UnsetOutCallbacks();
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
    std::string name = "";
    std::vector<std::string> blacklist;
    std::vector<std::string> whitelist;
    NLLogger_OutCallbacks cb;
    bool usingCb = false;
};

