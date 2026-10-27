#pragma once
#include <unordered_map>
#include <string>
#include <vector>

#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_STANDARD_BOOL
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_COMMAND_USERDATA
#include <Nuklear.h>

#include "../Extra/Lang.hpp"
#include "../Logger.hpp"
#include "../Extra/AssetsUtils.hpp"

namespace NLGT {

enum StyleParamType {
    IMAGE,
    ITEM,
    INT,
    FLOAT,
    VEC2,
    SYMBOL,
    COLOR,
    ALIGN,
    BOOL,
    TOOLTIP_POS,
    STRING
};
struct NGTSTheme {
    std::string defaultVariation = "fallback";
    struct Info {
        std::string Name;
        std::string Autor;
        std::string Description;
        std::string License;
    } info;
    std::unordered_map<std::string,nk_style> themeVariations;
};
struct TypeBinding {
    void* value = nullptr;
    StyleParamType type;
};


class Stylist {
public:
    void Setup(NLLogger* Logger,AssetManager* manager,nk_context* ctx) {
        this->Logger = Logger;
        this->GMan = manager;
        this->ctx = ctx;
    }
    NGTSTheme LoadTheme(std::string path) {
        std::string rawTheme = loadRawFile(path);

        NGTSTheme theme;
        loadInfoFromTheme(theme,rawTheme);
        forel variation inside(getVariations(rawTheme)) {
            theme.themeVariations[variation] = ctx->style;

            std::unordered_map<std::string,std::string> values = getValuesFromThemeVariation(variation,rawTheme);
            std::unordered_map<std::string,TypeBinding> bindings = getBindings(&theme.themeVariations[variation]);

            forel& [path,value] inside(values) {
                auto it = bindings.find(path);
                if (it != bindings.end()) {
                    auto result = setValue(it->second.value,value,it->second.type);
                    if(result != "ok") {
                        Logger->Warning(result+" on parametr "+path,"Stylist");
                    }
                }
                else {
                    Logger->Warning("Parametr "+path+" is not found in all parametrs","Stylist");
                }
            }
        }
        
        theme.themeVariations["fallback"] = ctx->style;

        
        return theme;
    }


private:
    nk_context* ctx = nullptr;
    AssetManager* GMan = nullptr;
    NLLogger* Logger;
    std::string loadRawFile(std::string path);
    void loadInfoFromTheme(NGTSTheme& theme,std::string themeRaw);
    std::unordered_map<std::string,std::string> getValuesFromThemeVariation(std::string themeVariation,std::string themeRaw);
    std::vector<std::string> getVariations(std::string themeRaw);

    std::pair<std::string, std::vector<std::string>> extractValue(const std::string& text, int& argsCount);
    std::vector<std::string> split(std::string rawFile);

    std::string setValue(void* param,std::string value,StyleParamType type);

    std::unordered_map<std::string,TypeBinding> getBindings(struct nk_style* style);
};


}

