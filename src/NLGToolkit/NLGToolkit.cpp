
#include "../../include/NLUtils/NLGToolkit/NLGToolkit.hpp"
#include "../../include/NLUtils/Extra/Lang.hpp"

nk_context* NLGT::Init(NLGTConfig config) {
    Config = config;
    Logger.Setup(!Config.debug);
    Logger.Info("Init "+ Config.winName +" start",NLLogger::ConColors::VIOLET_B,false);
    Logger.Info("Init resolution",NLLogger::ConColors::GREEN,false);
    
    
    Logger.Info("Init GLFW",NLLogger::ConColors::GREEN,false);
    if (!glfwInit()) { 
        Logger.Error("GLFW init failed",true);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, Config.OGLC.Major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, Config.OGLC.Minor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, Config.OGLC.Profile);
    Logger.Info("Init window",NLLogger::ConColors::GREEN,false);
    window = glfwCreateWindow(Config.winSize.x, Config.winSize.y, Config.winName.c_str(), nullptr, nullptr);
    if (!window) { 
        glfwTerminate();
        Logger.Error("Window init failed",true);
    }
    Logger.Info("Making window context current ",NLLogger::ConColors::GREEN,false);
    glfwMakeContextCurrent(window);
    Logger.Info("Init GL GLAD",NLLogger::ConColors::GREEN,false);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { 
        Logger.Error("GL GLAD init failed",true);
    }
    glEnable(GL_DEPTH_TEST);


    assets.Setup(!Config.debug,(GLADloadproc)glfwGetProcAddress);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
        

    glimpl.Init(window, config.calbacksConfig);
    struct nk_font_atlas* atlas;
    glimpl.FontStashBegin(&atlas);
    forel& [name,fontMeta] in(config.fonts) {
        struct nk_font* font = nk_font_atlas_add_from_file(atlas,fontMeta.first.c_str(),fontMeta.second,0);
        if (font) {
            fonts[name] = font;
        }
    }
    glimpl.FontStashEnd();
    //glimpl.SetDefaultFont(font);
    if (Config.NS.path == "internal") {
        glimpl.UseDefaultProgram(&assets,Config.NS.version);
    }
    else {
        GLuint shader = assets.LoadComplexShader(Config.NS.path, Config.NS.basedir, Config.NS.version);
        glimpl.UseProgram(shader);
    }
        

    

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* w, int width, int height) {
        glViewport(0, 0, width, height);
    });
        

    return glimpl.GetContext();
}

std::unordered_map<std::string,struct nk_font*> NLGT::GetFonts() {
    return fonts;
}
bool NLGT::ShouldClose() {
    return glfwWindowShouldClose(window);
}
void NLGT::BeginFrame() {
    glimpl.NewFrame();
}
void NLGT::EndFrame() {
    int x,y;
    glfwGetWindowSize(window,&x,&y);
    glimpl.Render(x, y, NK_ANTI_ALIASING_ON);
    glfwSwapBuffers(window);
    glfwPollEvents();
}
void NLGT::Shutdown() {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}