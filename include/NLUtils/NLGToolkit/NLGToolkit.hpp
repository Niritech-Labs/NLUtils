#include <glm/glm.hpp>
#include <string>
#include "NKGLImplementation.hpp"
#include <cstdint>
#include "../Logger.hpp"
#include "../Extra/AssetsUtils.hpp"

struct OPENGLConfig {
    int Major = 4;
    int Minor = 3;
    uint32_t Profile = GLFW_OPENGL_CORE_PROFILE;
};

enum NLState {
    ENABLED,
    DISABLED
};

struct NuklearShader {
    std::string path = "internal";
    std::string basedir = "./";
    std::string version = "330 core";
    bool smoothing = false;
};

struct NLGTFont {
    std::string path;
    int size;
};
struct NLGTVSync {
    NLState state = NLState::ENABLED;
    uint divisor = 1;
};

struct NLGTConfig {
    glm::vec2 winSize;
    std::string winName;
    bool debug = false;

    OPENGLConfig OGLC;
    NuklearShader NS;
    GLFWCallbacksConfig calbacksConfig;
    std::unordered_map<std::string,std::pair<std::string,int>> fonts;
    struct nk_color bgcolor;

    NLGTVSync VSync;

};

struct NLGTContext {
    NKGLI glimpl;
    NLLogger Logger;
    GLFWwindow* window;
    AssetManager assets;
    nk_context* nkctx;
};

class NLGT {
public:
    NLGTContext* Init(NLGTConfig config);

    bool ShouldClose();
    void BeginFrame();
    void EndFrame();
    void Shutdown();
    std::unordered_map<std::string,struct nk_font*> GetFonts();
private:
    NLGTConfig Config;
    NLGTContext ctx;

    std::unordered_map<std::string,struct nk_font*> fonts;
};