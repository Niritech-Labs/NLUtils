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

struct NuklearShader {
    std::string path = "internal";
    std::string basedir = "./";
    std::string version = "330 core";

};

struct NLGTFont {
    std::string path;
    int size;
};

struct NLGTConfig {
    glm::vec2 winSize;
    std::string winName;
    bool debug = false;

    OPENGLConfig OGLC;
    NuklearShader NS;
    GLFWCallbacksConfig calbacksConfig;
    std::unordered_map<std::string,std::pair<std::string,int>> fonts;

};


class NLGT {
public:
    nk_context* Init(NLGTConfig config);

    bool ShouldClose();
    void BeginFrame();
    void EndFrame();
    void Shutdown();
    std::unordered_map<std::string,struct nk_font*> GetFonts();
private:
    AssetManager assets;
    NLGTConfig Config;
    NKGLI glimpl;
    NLLogger Logger;
    GLFWwindow* window;

    std::unordered_map<std::string,struct nk_font*> fonts;
};