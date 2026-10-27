#pragma once
#include <glm/glm.hpp>
#include <string>
#include "Widgets.hpp"
#include "NKGLImplementation.hpp"
#include <cstdint>
#include "../Logger.hpp"
#include "../Extra/AssetsUtils.hpp"
#include "../Types.hpp"
#include <functional>
#include "StyleManager.hpp"


namespace NLGT {

struct OPENGLConfig {
    int Major = 4;
    int Minor = 3;
    uint32_t Profile = GLFW_OPENGL_CORE_PROFILE;
};


struct NuklearShader {
    std::string path = "internal";
    std::string basedir = "./";
    std::string version = "330 core";
    bool smoothing = false;
};

struct Font {
    std::string path;
    int size;
    std::function<const nk_rune* ()> glyph; 
};
struct VSync {
    NLTypes::State state = NLTypes::State::ENABLED;
    uint64_t divisor = 1;
};

struct AppWindowConfig {
    glm::vec2 winSize;
    std::string winName;
    bool debug = false;

    OPENGLConfig OGLC;
    NuklearShader NS;
    GLFWCallbacksConfig calbacksConfig;
    
    struct nk_color bgcolor;

    VSync Vsync;

    std::unordered_map<std::string,Font> fonts;

    void AddFont(std::string path,std::string name,int height,std::function<const nk_rune* ()> glyph = nk_font_cyrillic_glyph_ranges) {
        auto it = fonts.find(name);
        if (it == fonts.end()) {
            Font font;
            font.path = path;
            font.glyph = glyph;
            font.size = height;
            fonts[name] = font;
        }
    }
    void ClearFont(std::string name) {
        auto it = fonts.find(name);
        if (it != fonts.end()) {
            fonts.erase(name);
        }
    }
    void ClearAllFonts() {
        fonts.clear();
    }
};

struct AppWindowContext {
    NKGLI glimpl;
    NLLogger Logger;
    GLFWwindow* window;
    AssetManager assets;
    nk_context* nkctx;
    Stylist stylist;
};

class AppWindow {
public:
    AppWindowContext* Init(AppWindowConfig config);
    NL_LOGGER_METHODS_PTR
    glm::vec2 GetCurrentWinSize();
    bool ShouldClose();
    void BeginFrame();
    void EndFrame();
    void Shutdown();
    std::unordered_map<std::string,struct nk_font*> GetFonts();
private:
    AppWindowConfig Config;
    AppWindowContext ctx;
    NLLogger* Logger = &ctx.Logger;
    std::unordered_map<std::string,struct nk_font*> fonts;
};

}