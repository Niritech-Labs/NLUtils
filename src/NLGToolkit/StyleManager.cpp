#include "../../include/NLUtils/NLGToolkit/StyleManager.hpp"
#include "../../include/NLUtils/TextUtils.hpp"
using namespace NLGT;

#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <algorithm>
#include <utility> 
#include <fstream>
#include <sstream>
#include <utility>
#include <filesystem>


std::pair<std::string, std::vector<std::string>> Stylist::extractValue(const std::string& text, int& splittersCount) {
    splittersCount = 0;
    
    std::regex pattern(R"(=\s*([a-zA-Z0-9_]+)\((.*?)\);)");
    std::smatch match;

    if (std::regex_search(text, match, pattern)) {
        std::string typeName = match[1].str(); 
        std::string content = match[2].str(); 

        splittersCount = (int)std::count(content.begin(), content.end(), ',');

        std::vector<std::string> values;
        std::stringstream ss(content);
        std::string item;
        
        while (std::getline(ss, item, ',')) {
            values.push_back(NLTextUtils::Strip(item));
        }

        return {typeName, values};
    }

    return {"", std::vector<std::string>{""}};
}

std::vector<std::string> Stylist::split(std::string rawFile) {
    std::istringstream stream(rawFile);
    std::vector<std::string> lines;
    std::string line;

    std::unordered_map<std::string, std::string> defvalues;

    while (std::getline(stream, line)) {
        std::string buf = line;
        if (nlfval "#defval" instring(line)) {
            NLTextUtils::ReplaceIn(buf,"#defval","");
            auto data = NLTextUtils::Split(buf, "<-");
            if (data.size() == 2) {
                NLTextUtils::StripIn(data[0]);
                NLTextUtils::StripIn(data[1]);
                defvalues[data[0]] = data[1];
            }
            buf = line;
        }
        forel [key,value] inside(defvalues) {
            if (nlfval key instring(buf)) {
                NLTextUtils::ReplaceIn(buf,key,value,1);
            }
        }
        lines.push_back(buf);
    }
    return lines;
}

bool baseSintaxisOk(const std::string& str) {
    return std::count(str.begin(), str.end(), '{') == std::count(str.begin(), str.end(), '}');
}




std::string Stylist::loadRawFile(std::string path) {
    std::error_code ec;
    auto normalizedPath = std::filesystem::weakly_canonical(path, ec);
    
    if (ec) {
        return "";
    }

    std::ifstream file(normalizedPath, std::ios::binary);
    if (!file.is_open()) {
        return ""; 
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}


void Stylist::loadInfoFromTheme(NGTSTheme& theme,std::string themeRaw) {
    auto preprocString = [&](std::string line, std::string* field) {
        NLTypes::FixedList list = NLTextUtils::Split(line,"=");
        if (list.size() == 2) {
            int splittersCount = 0;
            *field = NLTextUtils::Replace(extractValue(NLTextUtils::Strip("=" + list[1]),splittersCount).second[0],"\"","");
        }
        
    };
    std::vector<std::string> file = split(themeRaw);
    bool blockReached = false;
    forel& line inside(file) {
        if(nlfval "theme-info {" instring(line)) {blockReached = true;}
        else if (nlfval "theme-info{" instring(line)) {blockReached = true;}
        if (blockReached) {
            if(nlfval "Author" instring(line)) preprocString(line,&theme.info.Autor);
            if(nlfval "Name" instring(line)) preprocString(line,&theme.info.Name);
            if(nlfval "Description" instring(line)) preprocString(line,&theme.info.Description);
            if(nlfval "License" instring(line)) preprocString(line,&theme.info.License);

        }
        if(nlfval "theme-variation-default:" instring(line)) {
            NLTypes::FixedList list = NLTextUtils::Split(line,":");
            if (list.size() == 2) {
                NLTextUtils::ReplaceIn(list[1],";","");
                theme.defaultVariation = NLTextUtils::Strip(list[1]);
            }
        }
        if(nlfval "}" instring(line)) blockReached = false;
    }

}


std::unordered_map<std::string,std::string> Stylist::getValuesFromThemeVariation(std::string themeVariation,std::string themeRaw) {
    std::unordered_map<std::string,std::string> values;
    std::vector<std::string> file = split(themeRaw);

    std::vector<std::string> path;
    auto toPath = [&](std::string paramName) {
        std::string pathfull;
        forel& frag inside(path) {
            pathfull += (frag + ".");
        }
        pathfull += paramName;
        return pathfull;
    };

    bool blockEnd = false;
    bool blockReached = false;
    forel& line inside(file) {
        if(nlfval "theme-variation: "+themeVariation+" {" instring(line)) blockReached = true;
        else if (nlfval "theme-variation: "+themeVariation+"{" instring(line)) blockReached = true;
        if (blockReached) {
            if(nlfval "-> {" instring(line)) {
                auto linebuf = NLTextUtils::Split(line,"->");
                NLTextUtils::StripIn(linebuf[0]);
                if (linebuf.size() == 2) {
                    path.push_back(linebuf[0]);
                }
            }
            if(nlfval "=" instring(line)) {
                auto linebuf = NLTextUtils::Split(line,"=");
                NLTextUtils::StripIn(linebuf[0]);
                if (linebuf.size() == 2) {
                    values[toPath(linebuf[0])] = "=" + linebuf[1];
                }
            }
            if(nlfval "}" instring(line)) {
                if(path.size() != 0) path.pop_back();
                else blockEnd = true;
            }
        }
        if(blockEnd) return values;
    }
    return values;

}
std::vector<std::string> Stylist::getVariations(std::string themeRaw) {
    std::vector<std::string> file = split(themeRaw);
    std::vector<std::string> variations;
    forel& line inside(file) {
        if(nlfval "theme-variation:" instring(line)) {
            auto data = NLTextUtils::Split(line,":");
            if (data.size() == 2) {
                NLTextUtils::ReplaceIn(data[1],"{","");
                variations.push_back(NLTextUtils::Strip(data[1]));
            }
        }
    }
    return variations;
}

    
std::string Stylist::setValue(void* param,std::string valueRaw,StyleParamType type) {
    int splitters = 0;
    std::pair<std::string, std::vector<std::string>> data = extractValue(valueRaw,splitters);

try { 
    switch (type) {
    case StyleParamType::ITEM: {
            nk_style_item* item = static_cast<nk_style_item*>(param);
            if (data.first == "image" && splitters == 0) {
                item->type = NK_STYLE_ITEM_IMAGE;
                item->data.image = GMan->LoadImage(data.second[0]);
                return "ok";
            }
            if (data.first == "image9s" && splitters == 4) {
                item->type = NK_STYLE_ITEM_NINE_SLICE;
                item->data.slice = GMan->LoadImageNineSlice(data.second[0],std::stoi(data.second[1]),std::stoi(data.second[2]),std::stoi(data.second[3]),std::stoi(data.second[4]));
                return "ok";
            }
            if (data.first == "rgb" && splitters == 2) {
                item->type = NK_STYLE_ITEM_COLOR;
                item->data.color = nk_rgb(std::stoi(data.second[0]),std::stoi(data.second[1]),std::stoi(data.second[2]));
                return "ok";
            }
            if (data.first == "rgba" && splitters == 3) {
                item->type = NK_STYLE_ITEM_COLOR;
                item->data.color = nk_rgba(std::stoi(data.second[0]),std::stoi(data.second[1]),std::stoi(data.second[2]),std::stoi(data.second[3]));
                return "ok";
            }
            if (data.first == "hsv" && splitters == 2) {
                item->type = NK_STYLE_ITEM_COLOR;
                item->data.color = nk_hsv(std::stoi(data.second[0]),std::stoi(data.second[1]),std::stoi(data.second[2]));
                return "ok";
            }
            if (data.first == "hex" && splitters == 0) {
                item->type = NK_STYLE_ITEM_COLOR;
                item->data.color = nk_rgb_hex(data.second[0].c_str());
                return "ok";
            }
            return "invalid type or number of args";

        }
        break;
    case StyleParamType::COLOR: {
            nk_color* color = static_cast<nk_color*>(param);
            if (data.first == "rgb" && splitters == 2) {
                *color = nk_rgb(std::stoi(data.second[0]),std::stoi(data.second[1]),std::stoi(data.second[2]));
                return "ok";
            }
            if (data.first == "rgba" && splitters == 3) {
                *color = nk_rgba(std::stoi(data.second[0]),std::stoi(data.second[1]),std::stoi(data.second[2]),std::stoi(data.second[3]));
                return "ok";
            }
            if (data.first == "hsv" && splitters == 2) {
                *color = nk_hsv(std::stoi(data.second[0]),std::stoi(data.second[1]),std::stoi(data.second[2]));
                return "ok";
            }
            if (data.first == "hex" && splitters == 0) {
                *color = nk_rgb_hex(data.second[0].c_str());
                return "ok";
            }
            return "invalid type or number of args";
        }
        break;
    case StyleParamType::VEC2: {
            struct nk_vec2* vec2 = static_cast<struct nk_vec2*>(param);
            if (data.first == "vec2" && splitters == 1) {
                *vec2 = nk_vec2(std::stof(data.second[0]),std::stof(data.second[1]));
                return "ok";
            }
            return "invalid type or number of args";
        }
        break;
    case StyleParamType::FLOAT: {
            float* value = static_cast<float*>(param);
            if (data.first == "float" && splitters == 0) {
                *value = std::stof(data.second[0]);
                return "ok";
            }
            return "invalid type or number of args in value: "+valueRaw;
        }
        break;
    case StyleParamType::INT: {
            int* value = static_cast<int*>(param);
            if (data.first == "int" && splitters == 0) {
                *value = std::stoi(data.second[0]);
                return "ok";
            }
            return "invalid type or number of args in value: "+valueRaw;
        }
        break;
    case StyleParamType::BOOL: {
            int* value = static_cast<int*>(param);
            if (data.first == "bool" && splitters == 0) {
                *value = (data.second[0] == "true") ? 1 : 0;
                return "ok";
            }
            return "invalid type or number of args in value: "+valueRaw;
        }
        break;
    case StyleParamType::TOOLTIP_POS: {
            nk_tooltip_pos* value = static_cast<nk_tooltip_pos*>(param);
            if (data.first == "tooltipPos" && splitters == 0) {
                if ((data.second[0] == "TOP_LEFT")) *value = NK_TOP_LEFT;
                else if ((data.second[0] == "TOP_CENTER")) *value = NK_TOP_CENTER;
                else if ((data.second[0] == "TOP_RIGHT")) *value = NK_TOP_RIGHT;

                else if ((data.second[0] == "MIDDLE_LEFT")) *value = NK_MIDDLE_LEFT;
                else if ((data.second[0] == "MIDDLE_CENTER")) *value = NK_MIDDLE_CENTER;
                else if ((data.second[0] == "MIDDLE_RIGHT")) *value = NK_MIDDLE_RIGHT;

                else if ((data.second[0] == "BOTTOM_LEFT")) *value = NK_BOTTOM_LEFT;
                else if ((data.second[0] == "BOTTOM_CENTER")) *value = NK_BOTTOM_CENTER;
                else if ((data.second[0] == "BOTTOM_RIGHT")) *value = NK_BOTTOM_RIGHT;
                else return "invalid value:" +valueRaw;
                return "ok";
            }
            return "invalid type or number of args in value: "+valueRaw;
        }
        break;
    case StyleParamType::ALIGN: {
            nk_flags* value = static_cast<nk_flags*>(param);
            if (data.first == "align" && splitters == 0) {
                if ((data.second[0] == "TOP_LEFT")) *value = NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_TOP;
                else if ((data.second[0] == "TOP_CENTER")) *value = NK_TEXT_ALIGN_CENTERED | NK_TEXT_ALIGN_TOP;
                else if ((data.second[0] == "TOP_RIGHT")) *value = NK_TEXT_ALIGN_RIGHT | NK_TEXT_ALIGN_TOP;

                else if ((data.second[0] == "MIDDLE_LEFT")) *value = NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE;
                else if ((data.second[0] == "MIDDLE_CENTER")) *value = NK_TEXT_ALIGN_CENTERED | NK_TEXT_ALIGN_MIDDLE;
                else if ((data.second[0] == "MIDDLE_RIGHT")) *value = NK_TEXT_ALIGN_RIGHT | NK_TEXT_ALIGN_MIDDLE;

                else if ((data.second[0] == "BOTTOM_LEFT")) *value = NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_BOTTOM;
                else if ((data.second[0] == "BOTTOM_CENTER")) *value = NK_TEXT_ALIGN_CENTERED | NK_TEXT_ALIGN_BOTTOM;
                else if ((data.second[0] == "BOTTOM_RIGHT")) *value = NK_TEXT_ALIGN_RIGHT | NK_TEXT_ALIGN_BOTTOM;
                else return "invalid value:" +valueRaw;
                return "ok";
            }
            return "invalid type or number of args in value: "+valueRaw;
        }
        break;
    case StyleParamType::SYMBOL: {
            nk_flags* value = static_cast<nk_flags*>(param);
            if (data.first == "symbol" && splitters == 0) {
                if ((data.second[0] == "NONE")) *value = NK_SYMBOL_NONE;
                else if ((data.second[0] == "UNDERSCORE")) *value = NK_SYMBOL_UNDERSCORE;
                else if ((data.second[0] == "CIRCLE_SOLID")) *value = NK_SYMBOL_CIRCLE_SOLID;
                else if ((data.second[0] == "CIRCLE_OUTLINE")) *value = NK_SYMBOL_CIRCLE_OUTLINE;
                else if ((data.second[0] == "RECT_SOLID")) *value = NK_SYMBOL_RECT_SOLID;
                else if ((data.second[0] == "RECT_OUTLINE")) *value = NK_SYMBOL_RECT_OUTLINE;
                else if ((data.second[0] == "TRIANGLE_UP")) *value = NK_SYMBOL_TRIANGLE_UP;
                else if ((data.second[0] == "TRIANGLE_DOWN")) *value = NK_SYMBOL_TRIANGLE_DOWN;
                else if ((data.second[0] == "TRIANGLE_LEFT")) *value = NK_SYMBOL_TRIANGLE_LEFT;
                else if ((data.second[0] == "TRIANGLE_RIGHT")) *value = NK_SYMBOL_TRIANGLE_RIGHT;
                else if ((data.second[0] == "PLUS")) *value = NK_SYMBOL_PLUS;
                else if ((data.second[0] == "MINUS")) *value = NK_SYMBOL_MINUS;
                else if ((data.second[0] == "X")) *value = NK_SYMBOL_X;
                else if ((data.second[0] == "TRIANGLE_UP_OUTLINE")) *value = NK_SYMBOL_TRIANGLE_UP_OUTLINE;
                else if ((data.second[0] == "TRIANGLE_DOWN_OUTLINE")) *value = NK_SYMBOL_TRIANGLE_DOWN_OUTLINE;
                else if ((data.second[0] == "TRIANGLE_LEFT_OUTLINE")) *value = NK_SYMBOL_TRIANGLE_LEFT_OUTLINE;
                else if ((data.second[0] == "TRIANGLE_RIGHT_OUTLINE")) *value = NK_SYMBOL_TRIANGLE_RIGHT_OUTLINE;
                else if ((data.second[0] == "MAX")) *value = NK_SYMBOL_MAX;
                else return "invalid value";
                return "ok";
            }
            return "invalid type or number of args in value: "+valueRaw;
        }
        break;
    default:
        return "invalid type in value: " + valueRaw;
        break;
    }
}
catch (const std::exception& e) {
    return e.what();
}
}

std::unordered_map<std::string,TypeBinding> Stylist::getBindings(struct nk_style* style) {
    std::unordered_map<std::string,TypeBinding> bindings;


    auto bindButton = [&](const std::string& prefix, struct nk_style_button* b) {
        bindings[prefix + ".normal"]            = {&b->normal,                  StyleParamType::ITEM};
        bindings[prefix + ".hover"]             = {&b->hover,                   StyleParamType::ITEM};
        bindings[prefix + ".active"]            = {&b->active,                  StyleParamType::ITEM};
        bindings[prefix + ".border-color"]      = {&b->border_color,            StyleParamType::COLOR};
        bindings[prefix + ".bg-color-factor"]   = {&b->color_factor_background, StyleParamType::FLOAT};
        bindings[prefix + ".text-background"]   = {&b->text_background,         StyleParamType::COLOR};
        bindings[prefix + ".text-normal"]       = {&b->text_normal,             StyleParamType::COLOR};
        bindings[prefix + ".text-hover"]        = {&b->text_hover,              StyleParamType::COLOR};
        bindings[prefix + ".text-active"]       = {&b->text_active,             StyleParamType::COLOR};
        bindings[prefix + ".text-alignment"]    = {&b->text_alignment,          StyleParamType::ALIGN};
        bindings[prefix + ".text-color-factor"] = {&b->color_factor_text,       StyleParamType::FLOAT};
        bindings[prefix + ".border"]            = {&b->border,                  StyleParamType::FLOAT};
        bindings[prefix + ".rounding"]          = {&b->rounding,                StyleParamType::FLOAT};
        bindings[prefix + ".padding"]           = {&b->padding,                 StyleParamType::VEC2};
        bindings[prefix + ".image-padding"]     = {&b->image_padding,           StyleParamType::VEC2};
        bindings[prefix + ".touch-padding"]     = {&b->touch_padding,           StyleParamType::VEC2};
        bindings[prefix + ".disabled-factor"]   = {&b->disabled_factor,         StyleParamType::FLOAT};
    };

    auto bindToggle = [&](const std::string& prefix, struct nk_style_toggle* t) {
        bindings[prefix + ".normal"]          = {&t->normal,          StyleParamType::ITEM};
        bindings[prefix + ".hover"]           = {&t->hover,           StyleParamType::ITEM};
        bindings[prefix + ".active"]          = {&t->active,          StyleParamType::ITEM};
        bindings[prefix + ".border-color"]    = {&t->border_color,    StyleParamType::COLOR};
        bindings[prefix + ".cursor-normal"]   = {&t->cursor_normal,   StyleParamType::ITEM};
        bindings[prefix + ".cursor-hover"]    = {&t->cursor_hover,    StyleParamType::ITEM};
        bindings[prefix + ".text-background"] = {&t->text_background, StyleParamType::COLOR};
        bindings[prefix + ".text-normal"]     = {&t->text_normal,     StyleParamType::COLOR};
        bindings[prefix + ".text-hover"]      = {&t->text_hover,      StyleParamType::COLOR};
        bindings[prefix + ".text-active"]     = {&t->text_active,     StyleParamType::COLOR};
        bindings[prefix + ".text-alignment"]  = {&t->text_alignment,  StyleParamType::ALIGN};
        bindings[prefix + ".padding"]         = {&t->padding,         StyleParamType::VEC2};
        bindings[prefix + ".touch-padding"]   = {&t->touch_padding,   StyleParamType::VEC2};
        bindings[prefix + ".spacing"]         = {&t->spacing,         StyleParamType::FLOAT};
        bindings[prefix + ".border"]          = {&t->border,          StyleParamType::FLOAT};
        bindings[prefix + ".color-factor"]    = {&t->color_factor,    StyleParamType::FLOAT};
        bindings[prefix + ".disabled-factor"] = {&t->disabled_factor, StyleParamType::FLOAT};
    };

    auto bindScrollbar = [&](const std::string& prefix, struct nk_style_scrollbar* s) {
        bindings[prefix + ".normal"]              = {&s->normal,              StyleParamType::ITEM};
        bindings[prefix + ".hover"]               = {&s->hover,               StyleParamType::ITEM};
        bindings[prefix + ".active"]              = {&s->active,              StyleParamType::ITEM};
        bindings[prefix + ".border-color"]        = {&s->border_color,        StyleParamType::COLOR};
        bindings[prefix + ".cursor-normal"]       = {&s->cursor_normal,       StyleParamType::ITEM};
        bindings[prefix + ".cursor-hover"]        = {&s->cursor_hover,        StyleParamType::ITEM};
        bindings[prefix + ".cursor-active"]       = {&s->cursor_active,       StyleParamType::ITEM};
        bindings[prefix + ".cursor-border-color"] = {&s->cursor_border_color, StyleParamType::COLOR};
        bindings[prefix + ".border"]              = {&s->border,              StyleParamType::FLOAT};
        bindings[prefix + ".rounding"]            = {&s->rounding,            StyleParamType::FLOAT};
        bindings[prefix + ".border-cursor"]       = {&s->border_cursor,       StyleParamType::FLOAT};
        bindings[prefix + ".rounding-cursor"]     = {&s->rounding_cursor,     StyleParamType::FLOAT};
        bindings[prefix + ".padding"]             = {&s->padding,             StyleParamType::VEC2};
        bindings[prefix + ".color-factor"]        = {&s->color_factor,        StyleParamType::FLOAT};
        bindings[prefix + ".disabled-factor"]     = {&s->disabled_factor,     StyleParamType::FLOAT};
        bindings[prefix + ".show-buttons"]        = {&s->show_buttons,        StyleParamType::INT};
        bindButton(prefix + ".inc-button", &s->inc_button);
        bindButton(prefix + ".dec-button", &s->dec_button);
        bindings[prefix + ".inc-symbol"]          = {&s->inc_symbol,          StyleParamType::SYMBOL};
        bindings[prefix + ".dec-symbol"]          = {&s->dec_symbol,          StyleParamType::SYMBOL};
    };

    auto bindEdit = [&](const std::string& prefix, struct nk_style_edit* e) {
        bindings[prefix + ".normal"]               = {&e->normal,               StyleParamType::ITEM};
        bindings[prefix + ".hover"]                = {&e->hover,                StyleParamType::ITEM};
        bindings[prefix + ".active"]               = {&e->active,               StyleParamType::ITEM};
        bindings[prefix + ".border-color"]         = {&e->border_color,         StyleParamType::COLOR};
        bindScrollbar(prefix + ".scrollbar", &e->scrollbar);
        bindings[prefix + ".cursor-normal"]        = {&e->cursor_normal,        StyleParamType::COLOR};
        bindings[prefix + ".cursor-hover"]         = {&e->cursor_hover,         StyleParamType::COLOR};
        bindings[prefix + ".cursor-text-normal"]   = {&e->cursor_text_normal,   StyleParamType::COLOR};
        bindings[prefix + ".cursor-text-hover"]    = {&e->cursor_text_hover,    StyleParamType::COLOR};
        bindings[prefix + ".text-normal"]          = {&e->text_normal,          StyleParamType::COLOR};
        bindings[prefix + ".text-hover"]           = {&e->text_hover,           StyleParamType::COLOR};
        bindings[prefix + ".text-active"]          = {&e->text_active,          StyleParamType::COLOR};
        bindings[prefix + ".selected-normal"]      = {&e->selected_normal,      StyleParamType::COLOR};
        bindings[prefix + ".selected-hover"]       = {&e->selected_hover,       StyleParamType::COLOR};
        bindings[prefix + ".selected-text-normal"] = {&e->selected_text_normal, StyleParamType::COLOR};
        bindings[prefix + ".selected-text-hover"]  = {&e->selected_text_hover,  StyleParamType::COLOR};
        bindings[prefix + ".border"]               = {&e->border,               StyleParamType::FLOAT};
        bindings[prefix + ".rounding"]             = {&e->rounding,             StyleParamType::FLOAT};
        bindings[prefix + ".cursor-size"]          = {&e->cursor_size,          StyleParamType::FLOAT};
        bindings[prefix + ".scrollbar-size"]       = {&e->scrollbar_size,       StyleParamType::VEC2};
        bindings[prefix + ".padding"]              = {&e->padding,              StyleParamType::VEC2};
        bindings[prefix + ".row-padding"]          = {&e->row_padding,          StyleParamType::FLOAT};
        bindings[prefix + ".color-factor"]         = {&e->color_factor,         StyleParamType::FLOAT};
        bindings[prefix + ".disabled-factor"]      = {&e->disabled_factor,      StyleParamType::FLOAT};
    };

    //// Window

    bindings["Window.Header.normal"] = {&style->window.header.normal, StyleParamType::ITEM};
    bindings["Window.Header.hover"] = {&style->window.header.hover, StyleParamType::ITEM};
    bindings["Window.Header.active"] = {&style->window.header.active, StyleParamType::ITEM};

    bindButton("Window.Header.close", &style->window.header.close_button);
    bindButton("Window.Header.minimize", &style->window.header.minimize_button);

    bindings["Window.Header.close-symbol"]    = {&style->window.header.close_symbol,    StyleParamType::SYMBOL};
    bindings["Window.Header.minimize-symbol"] = {&style->window.header.minimize_symbol, StyleParamType::SYMBOL};
    bindings["Window.Header.maximize-symbol"] = {&style->window.header.maximize_symbol, StyleParamType::SYMBOL};

    bindings["Window.Header.label-normal"]  = {&style->window.header.label_normal,  StyleParamType::COLOR};
    bindings["Window.Header.label-hover"]   = {&style->window.header.label_hover,   StyleParamType::COLOR};
    bindings["Window.Header.label-active"]  = {&style->window.header.label_active,  StyleParamType::COLOR};

    bindings["Window.Header.header-align"]   = {&style->window.header.align,        StyleParamType::BOOL};
    bindings["Window.Header.padding"]        = {&style->window.header.padding,      StyleParamType::VEC2};
    bindings["Window.Header.label-padding"]  = {&style->window.header.label_padding,StyleParamType::VEC2};
    bindings["Window.Header.spacing"]        = {&style->window.header.spacing,      StyleParamType::VEC2};


    bindings["Window.background-fixed"]        = {&style->window.fixed_background,        StyleParamType::ITEM};
    bindings["Window.background"]              = {&style->window.background,              StyleParamType::COLOR};
    bindings["Window.border-color"]            = {&style->window.border_color,            StyleParamType::COLOR};
    bindings["Window.popup-border-color"]      = {&style->window.popup_border_color,      StyleParamType::COLOR};
    bindings["Window.combo-border-color"]      = {&style->window.combo_border_color,      StyleParamType::COLOR};
    bindings["Window.contextual-border-color"] = {&style->window.contextual_border_color, StyleParamType::COLOR};
    bindings["Window.menu-border-color"]       = {&style->window.menu_border_color,       StyleParamType::COLOR};
    bindings["Window.group-border-color"]      = {&style->window.group_border_color,      StyleParamType::COLOR};
    bindings["Window.tooltip-border-color"]    = {&style->window.tooltip_border_color,    StyleParamType::COLOR};
    bindings["Window.scaler"]                  = {&style->window.scaler,                  StyleParamType::ITEM};

    bindings["Window.border"]                 = {&style->window.border,                 StyleParamType::FLOAT};
    bindings["Window.combo-border"]           = {&style->window.combo_border,           StyleParamType::FLOAT};
    bindings["Window.contextual-border"]      = {&style->window.contextual_border,      StyleParamType::FLOAT};
    bindings["Window.menu-border"]            = {&style->window.menu_border,            StyleParamType::FLOAT};
    bindings["Window.group-border"]           = {&style->window.group_border,           StyleParamType::FLOAT};
    bindings["Window.tooltip-border"]         = {&style->window.tooltip_border,         StyleParamType::FLOAT};
    bindings["Window.popup-border"]           = {&style->window.popup_border,           StyleParamType::FLOAT};
    bindings["Window.min-row-height-padding"] = {&style->window.min_row_height_padding, StyleParamType::FLOAT};
    bindings["Window.rounding"]               = {&style->window.rounding,               StyleParamType::FLOAT};

    bindings["Window.spacing"]            = {&style->window.spacing,            StyleParamType::VEC2};
    bindings["Window.scrollbar_size"]     = {&style->window.scrollbar_size,     StyleParamType::VEC2};
    bindings["Window.min_size"]           = {&style->window.min_size,           StyleParamType::VEC2};
    bindings["Window.padding"]            = {&style->window.padding,            StyleParamType::VEC2};
    bindings["Window.group_padding"]      = {&style->window.group_padding,      StyleParamType::VEC2};
    bindings["Window.popup_padding"]      = {&style->window.popup_padding,      StyleParamType::VEC2};
    bindings["Window.combo_padding"]      = {&style->window.combo_padding,      StyleParamType::VEC2};
    bindings["Window.contextual_padding"] = {&style->window.contextual_padding, StyleParamType::VEC2};
    bindings["Window.menu_padding"]       = {&style->window.menu_padding,       StyleParamType::VEC2};
    bindings["Window.tooltip_padding"]    = {&style->window.tooltip_padding,    StyleParamType::VEC2};

    bindings["Window.tooltip-origin"] = {&style->window.tooltip_origin, StyleParamType::TOOLTIP_POS};
    bindings["Window.tooltip-offset"] = {&style->window.tooltip_offset, StyleParamType::VEC2};

    ////  Text

    bindings["Text.color"]          = {&style->text.color,           StyleParamType::COLOR};
    bindings["Text.padding"]        = {&style->text.padding,         StyleParamType::VEC2};
    bindings["Text.color-factor"]    = {&style->text.color_factor,    StyleParamType::FLOAT};
    bindings["Text.disabled-factor"] = {&style->text.disabled_factor, StyleParamType::FLOAT};

    //// Button / Contextual-Button / Menu-Button 

    bindButton("Button",            &style->button);
    bindButton("Contextual-Button", &style->contextual_button);
    bindButton("Menu-Button",       &style->menu_button);

    ////  Radio-Button / Checkbox 

    bindToggle("Radio-Button", &style->option);
    bindToggle("Checkbox",     &style->checkbox);

    //// Selectable 

    bindings["Selectable.normal"]              = {&style->selectable.normal,              StyleParamType::ITEM};
    bindings["Selectable.hover"]               = {&style->selectable.hover,               StyleParamType::ITEM};
    bindings["Selectable.pressed"]             = {&style->selectable.pressed,             StyleParamType::ITEM};
    bindings["Selectable.active-normal"]       = {&style->selectable.normal_active,       StyleParamType::ITEM};
    bindings["Selectable.active-hover"]        = {&style->selectable.hover_active,        StyleParamType::ITEM};
    bindings["Selectable.active-pressed"]      = {&style->selectable.pressed_active,      StyleParamType::ITEM};
    bindings["Selectable.text-normal"]         = {&style->selectable.text_normal,         StyleParamType::COLOR};
    bindings["Selectable.text-hover"]          = {&style->selectable.text_hover,          StyleParamType::COLOR};
    bindings["Selectable.text-pressed"]        = {&style->selectable.text_pressed,        StyleParamType::COLOR};
    bindings["Selectable.active-text-normal"]  = {&style->selectable.text_normal_active,  StyleParamType::COLOR};
    bindings["Selectable.active-text-hover"]   = {&style->selectable.text_hover_active,   StyleParamType::COLOR};
    bindings["Selectable.active-text-pressed"] = {&style->selectable.text_pressed_active, StyleParamType::COLOR};
    bindings["Selectable.bg-text"]             = {&style->selectable.text_background,     StyleParamType::COLOR};
    bindings["Selectable.text-alignment"]      = {&style->selectable.text_alignment,      StyleParamType::ALIGN};
    bindings["Selectable.rounding"]            = {&style->selectable.rounding,            StyleParamType::FLOAT};
    bindings["Selectable.padding"]             = {&style->selectable.padding,             StyleParamType::VEC2};
    bindings["Selectable.touch-padding"]       = {&style->selectable.touch_padding,       StyleParamType::VEC2};
    bindings["Selectable.image-padding"]       = {&style->selectable.image_padding,       StyleParamType::VEC2};
    bindings["Selectable.color-factor"]        = {&style->selectable.color_factor,        StyleParamType::FLOAT};
    bindings["Selectable.disabled-factor"]     = {&style->selectable.disabled_factor,     StyleParamType::FLOAT};

    ////  Slider 

    bindings["Slider.normal"]           = {&style->slider.normal,           StyleParamType::ITEM};
    bindings["Slider.hover"]            = {&style->slider.hover,            StyleParamType::ITEM};
    bindings["Slider.active"]           = {&style->slider.active,           StyleParamType::ITEM};
    bindings["Slider.border-color"]     = {&style->slider.border_color,     StyleParamType::COLOR};
    bindings["Slider.bar-normal"]       = {&style->slider.bar_normal,       StyleParamType::COLOR};
    bindings["Slider.bar-hover"]        = {&style->slider.bar_hover,        StyleParamType::COLOR};
    bindings["Slider.bar-active"]       = {&style->slider.bar_active,       StyleParamType::COLOR};
    bindings["Slider.bar-filled"]       = {&style->slider.bar_filled,       StyleParamType::COLOR};
    bindings["Slider.cursor-normal"]    = {&style->slider.cursor_normal,    StyleParamType::ITEM};
    bindings["Slider.cursor-hover"]     = {&style->slider.cursor_hover,     StyleParamType::ITEM};
    bindings["Slider.cursor-active"]    = {&style->slider.cursor_active,    StyleParamType::ITEM};
    bindings["Slider.border"]           = {&style->slider.border,           StyleParamType::FLOAT};
    bindings["Slider.rounding"]         = {&style->slider.rounding,         StyleParamType::FLOAT};
    bindings["Slider.bar-height"]       = {&style->slider.bar_height,       StyleParamType::FLOAT};
    bindings["Slider.padding"]          = {&style->slider.padding,          StyleParamType::VEC2};
    bindings["Slider.spacing"]          = {&style->slider.spacing,          StyleParamType::VEC2};
    bindings["Slider.cursor-size"]      = {&style->slider.cursor_size,      StyleParamType::VEC2};
    bindings["Slider.color-factor"]     = {&style->slider.color_factor,     StyleParamType::FLOAT};
    bindings["Slider.disabled-factor"]  = {&style->slider.disabled_factor,  StyleParamType::FLOAT};
    bindings["Slider.show-buttons"]     = {&style->slider.show_buttons,     StyleParamType::INT};
    bindButton("Slider.inc-button", &style->slider.inc_button);
    bindButton("Slider.dec-button", &style->slider.dec_button);
    bindings["Slider.inc-symbol"]       = {&style->slider.inc_symbol,       StyleParamType::SYMBOL};
    bindings["Slider.dec-symbol"]       = {&style->slider.dec_symbol,       StyleParamType::SYMBOL};

    //// Progress 

    bindings["Progress.normal"]              = {&style->progress.normal,              StyleParamType::ITEM};
    bindings["Progress.hover"]               = {&style->progress.hover,               StyleParamType::ITEM};
    bindings["Progress.active"]              = {&style->progress.active,              StyleParamType::ITEM};
    bindings["Progress.border-color"]        = {&style->progress.border_color,        StyleParamType::COLOR};
    bindings["Progress.cursor-normal"]       = {&style->progress.cursor_normal,       StyleParamType::ITEM};
    bindings["Progress.cursor-hover"]        = {&style->progress.cursor_hover,        StyleParamType::ITEM};
    bindings["Progress.cursor-active"]       = {&style->progress.cursor_active,       StyleParamType::ITEM};
    bindings["Progress.cursor-border-color"] = {&style->progress.cursor_border_color, StyleParamType::COLOR};
    bindings["Progress.rounding"]            = {&style->progress.rounding,            StyleParamType::FLOAT};
    bindings["Progress.border"]              = {&style->progress.border,              StyleParamType::FLOAT};
    bindings["Progress.cursor-border"]       = {&style->progress.cursor_border,       StyleParamType::FLOAT};
    bindings["Progress.cursor-rounding"]     = {&style->progress.cursor_rounding,     StyleParamType::FLOAT};
    bindings["Progress.padding"]             = {&style->progress.padding,             StyleParamType::VEC2};
    bindings["Progress.color-factor"]        = {&style->progress.color_factor,        StyleParamType::FLOAT};
    bindings["Progress.disabled-factor"]     = {&style->progress.disabled_factor,     StyleParamType::FLOAT};

    ////  Property 

    bindings["Property.normal"]          = {&style->property.normal,          StyleParamType::ITEM};
    bindings["Property.hover"]           = {&style->property.hover,           StyleParamType::ITEM};
    bindings["Property.active"]          = {&style->property.active,          StyleParamType::ITEM};
    bindings["Property.border-color"]    = {&style->property.border_color,    StyleParamType::COLOR};
    bindings["Property.label-normal"]    = {&style->property.label_normal,    StyleParamType::COLOR};
    bindings["Property.label-hover"]     = {&style->property.label_hover,     StyleParamType::COLOR};
    bindings["Property.label-active"]    = {&style->property.label_active,    StyleParamType::COLOR};
    bindings["Property.sym-left"]        = {&style->property.sym_left,        StyleParamType::SYMBOL};
    bindings["Property.sym-right"]       = {&style->property.sym_right,       StyleParamType::SYMBOL};
    bindings["Property.border"]          = {&style->property.border,          StyleParamType::FLOAT};
    bindings["Property.rounding"]        = {&style->property.rounding,        StyleParamType::FLOAT};
    bindings["Property.padding"]         = {&style->property.padding,         StyleParamType::VEC2};
    bindings["Property.color-factor"]    = {&style->property.color_factor,    StyleParamType::FLOAT};
    bindings["Property.disabled-factor"] = {&style->property.disabled_factor, StyleParamType::FLOAT};
    bindEdit("Property.edit-entry", &style->property.edit);
    bindButton("Property.inc-button", &style->property.inc_button);
    bindButton("Property.dec-button", &style->property.dec_button);

    ////  Edit

    bindEdit("Edit", &style->edit);

    ////  Chart 

    bindings["Chart.background"]      = {&style->chart.background,      StyleParamType::ITEM};
    bindings["Chart.border-color"]    = {&style->chart.border_color,    StyleParamType::COLOR};
    bindings["Chart.selected-color"]  = {&style->chart.selected_color,  StyleParamType::COLOR};
    bindings["Chart.color"]           = {&style->chart.color,           StyleParamType::COLOR};
    bindings["Chart.border"]          = {&style->chart.border,          StyleParamType::FLOAT};
    bindings["Chart.rounding"]        = {&style->chart.rounding,        StyleParamType::FLOAT};
    bindings["Chart.padding"]         = {&style->chart.padding,         StyleParamType::VEC2};
    bindings["Chart.color-factor"]    = {&style->chart.color_factor,    StyleParamType::FLOAT};
    bindings["Chart.disabled-factor"] = {&style->chart.disabled_factor, StyleParamType::FLOAT};
    bindings["Chart.show-markers"]    = {&style->chart.show_markers,    StyleParamType::BOOL};

    ////  Scrollbars 

    bindScrollbar("Scrollbar-Vertical",   &style->scrollv);
    bindScrollbar("Scrollbar-Horizontal", &style->scrollh);

    //// Tree 

    bindings["Tree.background"]      = {&style->tab.background,      StyleParamType::ITEM};
    bindings["Tree.border-color"]    = {&style->tab.border_color,    StyleParamType::COLOR};
    bindings["Tree.text"]            = {&style->tab.text,            StyleParamType::COLOR};
    bindButton("Tree.tab-maximize-button", &style->tab.tab_maximize_button);
    bindButton("Tree.tab-minimize-button", &style->tab.tab_minimize_button);
    bindButton("Tree.node-maximize-button", &style->tab.node_maximize_button);
    bindButton("Tree.node-minimize-button", &style->tab.node_minimize_button);
    bindings["Tree.sym-minimize"]    = {&style->tab.sym_minimize,    StyleParamType::SYMBOL};
    bindings["Tree.sym-maximize"]    = {&style->tab.sym_maximize,    StyleParamType::SYMBOL};
    bindings["Tree.border"]          = {&style->tab.border,          StyleParamType::FLOAT};
    bindings["Tree.rounding"]        = {&style->tab.rounding,        StyleParamType::FLOAT};
    bindings["Tree.indent"]          = {&style->tab.indent,          StyleParamType::FLOAT};
    bindings["Tree.padding"]         = {&style->tab.padding,         StyleParamType::VEC2};
    bindings["Tree.spacing"]         = {&style->tab.spacing,         StyleParamType::VEC2};
    bindings["Tree.color-factor"]    = {&style->tab.color_factor,    StyleParamType::FLOAT};
    bindings["Tree.disabled-factor"] = {&style->tab.disabled_factor, StyleParamType::FLOAT};

    //// Combobox 

    bindings["Combobox.normal"]         = {&style->combo.normal,         StyleParamType::ITEM};
    bindings["Combobox.hover"]          = {&style->combo.hover,          StyleParamType::ITEM};
    bindings["Combobox.active"]         = {&style->combo.active,         StyleParamType::ITEM};
    bindings["Combobox.border-color"]   = {&style->combo.border_color,   StyleParamType::COLOR};
    bindings["Combobox.label-normal"]   = {&style->combo.label_normal,   StyleParamType::COLOR};
    bindings["Combobox.label-hover"]    = {&style->combo.label_hover,    StyleParamType::COLOR};
    bindings["Combobox.label-active"]   = {&style->combo.label_active,   StyleParamType::COLOR};
    bindings["Combobox.symbol-normal"]  = {&style->combo.symbol_normal,  StyleParamType::COLOR};
    bindings["Combobox.symbol-hover"]   = {&style->combo.symbol_hover,   StyleParamType::COLOR};
    bindings["Combobox.symbol-active"]  = {&style->combo.symbol_active,  StyleParamType::COLOR};
    bindButton("Combobox.button", &style->combo.button);
    bindings["Combobox.sym-normal"]     = {&style->combo.sym_normal,     StyleParamType::SYMBOL};
    bindings["Combobox.sym-hover"]      = {&style->combo.sym_hover,      StyleParamType::SYMBOL};
    bindings["Combobox.sym-active"]     = {&style->combo.sym_active,     StyleParamType::SYMBOL};
    bindings["Combobox.border"]         = {&style->combo.border,         StyleParamType::FLOAT};
    bindings["Combobox.rounding"]       = {&style->combo.rounding,       StyleParamType::FLOAT};
    bindings["Combobox.content-padding"] = {&style->combo.content_padding, StyleParamType::VEC2};
    bindings["Combobox.button-padding"] = {&style->combo.button_padding, StyleParamType::VEC2};
    bindings["Combobox.spacing"]        = {&style->combo.spacing,        StyleParamType::VEC2};
    bindings["Combobox.color-factor"]   = {&style->combo.color_factor,   StyleParamType::FLOAT};
    bindings["Combobox.disabled-factor"] = {&style->combo.disabled_factor, StyleParamType::FLOAT};

    ////  knob 

    bindings["Knob.normal"]            = {&style->knob.normal,            StyleParamType::ITEM};
    bindings["Knob.hover"]             = {&style->knob.hover,             StyleParamType::ITEM};
    bindings["Knob.active"]            = {&style->knob.active,            StyleParamType::ITEM};
    bindings["Knob.border-color"]      = {&style->knob.border_color,      StyleParamType::COLOR};
    bindings["Knob.knob-normal"]       = {&style->knob.knob_normal,       StyleParamType::COLOR};
    bindings["Knob.knob-hover"]        = {&style->knob.knob_hover,        StyleParamType::COLOR};
    bindings["Knob.knob-active"]       = {&style->knob.knob_active,       StyleParamType::COLOR};
    bindings["Knob.knob-border-color"] = {&style->knob.knob_border_color, StyleParamType::COLOR};
    bindings["Knob.cursor-normal"]     = {&style->knob.cursor_normal,     StyleParamType::COLOR};
    bindings["Knob.cursor-hover"]      = {&style->knob.cursor_hover,      StyleParamType::COLOR};
    bindings["Knob.cursor-active"]     = {&style->knob.cursor_active,     StyleParamType::COLOR};
    bindings["Knob.border"]            = {&style->knob.border,            StyleParamType::FLOAT};
    bindings["Knob.knob-border"]       = {&style->knob.knob_border,       StyleParamType::FLOAT};
    bindings["Knob.padding"]           = {&style->knob.padding,           StyleParamType::VEC2};
    bindings["Knob.spacing"]           = {&style->knob.spacing,           StyleParamType::VEC2};
    bindings["Knob.cursor-width"]      = {&style->knob.cursor_width,      StyleParamType::FLOAT};
    bindings["Knob.color-factor"]      = {&style->knob.color_factor,      StyleParamType::FLOAT};
    bindings["Knob.disabled-factor"]   = {&style->knob.disabled_factor,   StyleParamType::FLOAT};

    return bindings;
}