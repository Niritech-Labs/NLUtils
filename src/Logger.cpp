#include "../include/NLUtils/Logger.hpp"


std::string NLLogger::ansiCode(ConColors c) {
    switch (c) {
        case RED: return "\033[91m";
        case GREEN: return "\033[92m";
        case YELLOW: return "\033[93m";
        case BLUE: return "\033[94m";
        case VIOLET: return "\033[95m";
        case STANDART: return "\033[0m";
        case LIGHTBLUE: return "\u001b[36m";
        case WHITE: return "\u001b[37m";
        case SANDY: return "\u001b[38;5;223m";
        case GRAY: return "\u001b[38;5;246m";


        case RED_B: return "\u001b[31;1m";
        case GREEN_B: return "\u001b[32;1m";
        case YELLOW_B: return "\u001b[33;1m";
        case BLUE_B: return "\u001b[34;1m";
        case VIOLET_B: return "\u001b[35;1m";
        case WHITE_B: return "\u001b[37;1m";
        case LIGHTBLUE_B: return "\u001b[36;1m";
        case SANDY_B: return "\u001b[38;5;223;1m";
        case GRAY_B: return "\u001b[38;5;246;1m";

    }
    return "\033[0m";
}


void NLLogger::Setup(bool production, std::string ComponentName, std::stack<std::string>* logStack)
{
    this->production = production;
    this->toLogStack = false;
    if (logStack != nullptr) {
        this->logStack = logStack;
        this->toLogStack = true;
    }
    this->name = "[" + ComponentName + "]";
}

void NLLogger::Warning(std::string warning) {
    std::string message = ansiCode(GRAY_B) + name + ansiCode(YELLOW_B) + " [Warning]: " + ansiCode(STANDART) + ansiCode(YELLOW) + warning + ansiCode(STANDART);
    if (toLogStack) {
        logStack->push(message);
    }
    std::cout << message << std::endl;
}
void NLLogger::Error(std::string error, bool critical) {
    std::string message;
    if (critical) {
        message = ansiCode(GRAY_B) + name + ansiCode(RED_B) + " [Critical Error]: " + error + ansiCode(STANDART);
        if (toLogStack) {
            logStack->push(message);
        }
        std::cout << message << std::endl;
        std::exit(1);
    } else {
        message = ansiCode(GRAY_B) + name + ansiCode(RED_B) + " [Error]: " + ansiCode(STANDART) + ansiCode(RED) + error + ansiCode(STANDART);
        if (toLogStack) {
            logStack->push(message);
        }
        std::cout << message << std::endl;
    }
}
void NLLogger::Warning(std::string warning, std::string layer) {
    std::string message = ansiCode(GRAY_B) + name + ansiCode(YELLOW_B) + " [Warning] " + ansiCode(WHITE_B) + "[" + layer + "]: " + ansiCode(STANDART) + ansiCode(YELLOW) + warning + ansiCode(STANDART);
    if (toLogStack) {
        logStack->push(message);
    }
    std::cout << message << std::endl;
}
void NLLogger::Error(std::string error, bool critical, std::string layer) {
    std::string message;
    if (critical) {
        message = ansiCode(GRAY_B) + name + ansiCode(RED_B) + " [Critical Error] "  + ansiCode(WHITE_B) + "[" + layer + "]: " + ansiCode(RED_B) + error + ansiCode(STANDART);
        if (toLogStack) {
            logStack->push(message);
        }
        std::cout << message << std::endl;
        std::exit(1);
    } else {
        message = ansiCode(GRAY_B) + name + ansiCode(RED_B) + " [Error] " + ansiCode(WHITE_B) + "[" + layer + "]: " + ansiCode(STANDART) + ansiCode(RED) + error + ansiCode(STANDART);
        if (toLogStack) {
            logStack->push(message);
        }
        std::cout << message << std::endl;
    }
}

void NLLogger::Info(std::string info, ConColors color, bool productionLatency) {
    std::string message = ansiCode(GRAY_B) + name + ansiCode(SANDY_B) + " [Info]: " + ansiCode(STANDART) + ansiCode(color) + info + ansiCode(STANDART);
    if (production) {
        if (productionLatency) {
            if (toLogStack) {
                logStack->push(message);
            }
            std::cout << message << std::endl;
        }
    } else {
        if (toLogStack) {
            logStack->push(message);
        }
        std::cout << message << std::endl;
    }
}
void NLLogger::SetBlacklist(std::vector<std::string> bl) {
    blacklist = bl;
}
void NLLogger::SetWhitelist(std::vector<std::string> wl) {
    whitelist = wl;
}
void NLLogger::Info(std::string info, ConColors color, bool productionLatency, std::string layer) {
    for (std::string black : blacklist) {if (layer == black) return;}
    if (!whitelist.empty()) {
        bool find = false;
        for (std::string white : whitelist) {if (layer == white) find = true;}
        if (!find) return;
    }

    std::string message = ansiCode(GRAY_B) + name + ansiCode(SANDY_B) + " [Info] " + ansiCode(WHITE_B) + "["+layer+"]: " + ansiCode(STANDART) + ansiCode(color) + info + ansiCode(STANDART);
    if (production) {
        if (productionLatency) {
            if (toLogStack) {
                logStack->push(message);
            }
            std::cout << message << std::endl;
        }
    } else {
        if (toLogStack) {
            logStack->push(message);
        }
        std::cout << message << std::endl;
    }
}