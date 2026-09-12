#pragma once
#include <glad.h>
#include <GLFW/glfw3.h>

#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_STANDARD_BOOL
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_STANDARD_VARARGS

#include <Nuklear.h>
#include "../Extra/AssetsUtils.hpp"

#ifndef NK_GLFW_TEXT_MAX
#define NK_GLFW_TEXT_MAX 256
#endif
#ifndef NK_GLFW_DOUBLE_CLICK_LO
#define NK_GLFW_DOUBLE_CLICK_LO 0.02
#endif
#ifndef NK_GLFW_DOUBLE_CLICK_HI
#define NK_GLFW_DOUBLE_CLICK_HI 0.2
#endif

#include <unordered_map>
struct GLFWCallbacksConfig;

enum CallbackState {
    INSTALL_DEFAULT = 0,
    INSTALL_CUSTOM,
    INSTALL_NONE
};

struct nk_glfw_device {
    struct nk_buffer cmds;
    struct nk_draw_null_texture tex_null;
    GLuint vbo, vao, ebo;
    GLuint font_tex;
    GLuint prog;
    GLint attrib_pos, attrib_uv, attrib_col;
    GLint uniform_tex, uniform_proj;
};

struct nk_glfw {
    GLFWwindow *win;
    int width, height;
    int display_width, display_height;
    struct nk_glfw_device ogl;
    struct nk_context ctx;
    struct nk_font_atlas atlas;
    struct nk_vec2 fb_scale;
    unsigned int text[NK_GLFW_TEXT_MAX];
    nk_char key_events[NK_KEY_MAX];
    int text_len;
    struct nk_vec2 scroll;
    double last_button_click;
    int is_double_click_down;
    struct nk_vec2 double_click_pos;
    float delta_time_seconds_last;
};



class NKGLI {
public:
    struct nk_context* Init(GLFWwindow *win, GLFWCallbacksConfig config);
    void Shutdown();
    void FontStashBegin(struct nk_font_atlas **atlas);
    void SetDefaultFont(nk_font* font);
    void FontStashEnd();
    void NewFrame();
    void UseProgram(GLuint prog);
    void UseDefaultProgram(AssetManager* manager,std::string version);
    void Render(int width, int height, enum nk_anti_aliasing AA);

    static void CharCallback(GLFWwindow *win, unsigned int codepoint);
    static void KeyCallback(GLFWwindow *win, int key, int scancode, int action, int mods);
    static void ScrollCallback(GLFWwindow *win, double xoff, double yoff);
    static void MouseButtonCallback(GLFWwindow *win, int button, int action, int mods);
    struct nk_context* GetContext();  

    void OnChar(unsigned int codepoint);
    void OnKey(int key, int scancode, int action, int mods);
    void OnScroll(double xoff, double yoff);
    void OnMouseButton(int button, int action, int mods);

private:
    struct nk_glfw NR_glfw;

    void DeviceCreateBuffers();
    void DeviceUploadAtlas(const void *image, int width, int height);

    static void ClipboardPaste(nk_handle usr, struct nk_text_edit *edit);
    static void ClipboardCopy(nk_handle usr, const char *text, int len);
    
};


struct GLFWCallbacksConfig {
    CallbackState state = CallbackState::INSTALL_DEFAULT;
    GLFWscrollfun scrollFunc = NKGLI::ScrollCallback;
    GLFWcharfun charFunc = NKGLI::CharCallback;
    GLFWmousebuttonfun mousenbtnFunc = NKGLI::MouseButtonCallback;
    GLFWkeyfun keyFunc = NKGLI::KeyCallback;
};