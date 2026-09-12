
#include "../../include/NLUtils/NLGToolkit/NLGToolkit.hpp"
#include "../../include/NLUtils/Extra/Lang.hpp"

NLGTContext* NLGT::Init(NLGTConfig config) {
    Config = config;
    ctx.Logger.Setup(!Config.debug,Config.winName);
    ctx.Logger.Info("Init "+ Config.winName +" start",NLLogger::ConColors::VIOLET_B,false);
    ctx.Logger.Info("Init resolution",NLLogger::ConColors::GREEN,false);
    
    
    ctx.Logger.Info("Init GLFW",NLLogger::ConColors::GREEN,false);
    if (!glfwInit()) { 
        ctx.Logger.Error("GLFW init failed",true);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, Config.OGLC.Major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, Config.OGLC.Minor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, Config.OGLC.Profile);
    ctx.Logger.Info("Init window",NLLogger::ConColors::GREEN,false);
    ctx.window = glfwCreateWindow(Config.winSize.x, Config.winSize.y, Config.winName.c_str(), nullptr, nullptr);
    if (!ctx.window) { 
        glfwTerminate();
        ctx.Logger.Error("Window init failed",true);
    }
    ctx.Logger.Info("Making window context current ",NLLogger::ConColors::GREEN,false);
    glfwMakeContextCurrent(ctx.window);
    ctx.Logger.Info("Init GL GLAD",NLLogger::ConColors::GREEN,false);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { 
        ctx.Logger.Error("GL GLAD init failed",true);
    }
    glEnable(GL_DEPTH_TEST);
    glClearColor(Config.bgcolor.r, Config.bgcolor.g, Config.bgcolor.b, Config.bgcolor.a);
    if (Config.VSync.state == NLState::ENABLED) {
        glfwSwapInterval(Config.VSync.divisor); 
    }


    ctx.assets.Setup(!Config.debug,(GLADloadproc)glfwGetProcAddress);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
        

    ctx.glimpl.Init(ctx.window, config.calbacksConfig);
    struct nk_font_atlas* atlas;
    ctx.glimpl.FontStashBegin(&atlas);
    forel& [name,fontMeta] in(config.fonts) {
        struct nk_font* font = nk_font_atlas_add_from_file(atlas,fontMeta.first.c_str(),fontMeta.second,0);
        if (font) {
            fonts[name] = font;
        }
    }
    ctx.glimpl.FontStashEnd();
    //glimpl.SetDefaultFont(font);
    if (Config.NS.path == "internal") {
        ctx.glimpl.UseDefaultProgram(&ctx.assets,Config.NS.version);
    }
    else {
        GLuint shader = ctx.assets.LoadComplexShader(Config.NS.path, Config.NS.basedir, Config.NS.version);
        ctx.glimpl.UseProgram(shader);
    }
        

    

    glfwSetFramebufferSizeCallback(ctx.window, [](GLFWwindow* w, int width, int height) {
        glViewport(0, 0, width, height);
    });
    ctx.nkctx = ctx.glimpl.GetContext();

    return &ctx;
}

std::unordered_map<std::string,struct nk_font*> NLGT::GetFonts() {
    return fonts;
}
bool NLGT::ShouldClose() {
    return glfwWindowShouldClose(ctx.window);
}
void NLGT::BeginFrame() {
    ctx.glimpl.NewFrame();
}
void NLGT::EndFrame() {
    int x,y;
    glfwGetWindowSize(ctx.window,&x,&y);
    glClear(GL_COLOR_BUFFER_BIT);
    ctx.glimpl.Render(x, y, NK_ANTI_ALIASING_ON);
    glfwSwapBuffers(ctx.window);
    glfwPollEvents();
}
void NLGT::Shutdown() {
    glfwSetWindowShouldClose(ctx.window, GLFW_TRUE);
}