#include "../../include/NLUtils/NLGToolkit/NLGToolkit.hpp"
#include <string.h>
#include <stdlib.h>
#include <assert.h>



struct nk_glfw_vertex {
    float position[2];
    float uv[2];
    nk_byte col[4];
};

void NKGLI::DeviceCreateBuffers() {
    struct nk_glfw_device *dev = &NR_glfw.ogl;
    nk_buffer_init_default(&dev->cmds);

    glGenBuffers(1, &dev->vbo);
    glGenBuffers(1, &dev->ebo);
    glGenVertexArrays(1, &dev->vao);

    glBindVertexArray(dev->vao);
    glBindBuffer(GL_ARRAY_BUFFER, dev->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dev->ebo);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void NKGLI::DeviceUploadAtlas(const void *image, int width, int height) {
    struct nk_glfw_device *dev = &NR_glfw.ogl;
    glGenTextures(1, &dev->font_tex);
    glBindTexture(GL_TEXTURE_2D, dev->font_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)width, (GLsizei)height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, image);
}

void NKGLI::ClipboardPaste(nk_handle usr, struct nk_text_edit *edit) {
    NKGLI* self = (NKGLI*)usr.ptr;
    const char *text = glfwGetClipboardString(self->NR_glfw.win);
    if (text) nk_textedit_paste(edit, text, nk_strlen(text));
}

void NKGLI::ClipboardCopy(nk_handle usr, const char *text, int len) {
    NKGLI* self = (NKGLI*)usr.ptr;
    char *str = 0;
    if (!len) return;
    str = (char*)malloc((size_t)len+1);
    if (!str) return;
    memcpy(str, text, (size_t)len);
    str[len] = '\0';
    glfwSetClipboardString(self->NR_glfw.win, str);
    free(str);
}

struct nk_context* NKGLI::Init(GLFWwindow *win, GLFWCallbacksConfig config) {
    memset(&NR_glfw, 0, sizeof(NR_glfw));
    NR_glfw.win = win;
    if (config.state != CallbackState::INSTALL_NONE) {
        glfwSetScrollCallback(win, config.scrollFunc);
        glfwSetCharCallback(win, config.charFunc);
        glfwSetKeyCallback(win, config.keyFunc);
        glfwSetMouseButtonCallback(win, config.mousenbtnFunc);

        if (config.state == CallbackState::INSTALL_DEFAULT) glfwSetWindowUserPointer(win, this);
    }
    nk_init_default(&NR_glfw.ctx, 0);
    nk_buffer_init_default(&NR_glfw.ctx.memory); 
    NR_glfw.ctx.clip.copy = ClipboardCopy;
    NR_glfw.ctx.clip.paste = ClipboardPaste;
    NR_glfw.ctx.clip.userdata = nk_handle_ptr(this);
    NR_glfw.last_button_click = 0;
    NR_glfw.is_double_click_down = nk_false;
    NR_glfw.double_click_pos = nk_vec2(0, 0);
    NR_glfw.delta_time_seconds_last = (float)glfwGetTime();

    DeviceCreateBuffers();
    return &NR_glfw.ctx;
}

void NKGLI::Shutdown() {
    struct nk_glfw_device *dev = &NR_glfw.ogl;
    if (dev->font_tex) glDeleteTextures(1, &dev->font_tex);
    if (dev->vbo) glDeleteBuffers(1, &dev->vbo);
    if (dev->ebo) glDeleteBuffers(1, &dev->ebo);
    if (dev->vao) glDeleteVertexArrays(1, &dev->vao);
    nk_buffer_free(&dev->cmds);
    nk_font_atlas_clear(&NR_glfw.atlas);
    nk_free(&NR_glfw.ctx);
}

void NKGLI::FontStashBegin(struct nk_font_atlas **atlas) {
    nk_font_atlas_init_default(&NR_glfw.atlas);
    nk_font_atlas_begin(&NR_glfw.atlas);
    *atlas = &NR_glfw.atlas;
}
void NKGLI::SetDefaultFont(nk_font* font) {
    nk_style_set_font(&NR_glfw.ctx,&font->handle);
}

void NKGLI::FontStashEnd() {
    const void *image; int w, h;
    image = nk_font_atlas_bake(&NR_glfw.atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
    DeviceUploadAtlas(image, w, h);
    nk_font_atlas_end(&NR_glfw.atlas, nk_handle_id((int)NR_glfw.ogl.font_tex), &NR_glfw.ogl.tex_null);
    if (NR_glfw.atlas.default_font)
        nk_style_set_font(&NR_glfw.ctx, &NR_glfw.atlas.default_font->handle);
}

void NKGLI::NewFrame() {
    int i;
    double x, y;
    struct nk_context *ctx = &NR_glfw.ctx;
    GLFWwindow *win = NR_glfw.win;
    nk_char* k_state = NR_glfw.key_events;

    float delta_time_now = (float)glfwGetTime();
    ctx->delta_time_seconds = delta_time_now - NR_glfw.delta_time_seconds_last;
    NR_glfw.delta_time_seconds_last = delta_time_now;

    glfwGetWindowSize(win, &NR_glfw.width, &NR_glfw.height);
    glfwGetFramebufferSize(win, &NR_glfw.display_width, &NR_glfw.display_height);
    NR_glfw.fb_scale.x = (float)NR_glfw.display_width / (float)NR_glfw.width;
    NR_glfw.fb_scale.y = (float)NR_glfw.display_height / (float)NR_glfw.height;

    nk_input_begin(ctx);
    for (i = 0; i < NR_glfw.text_len; ++i)
        nk_input_unicode(ctx, NR_glfw.text[i]);


    if (k_state[NK_KEY_DEL] >= 0) nk_input_key(ctx, NK_KEY_DEL, k_state[NK_KEY_DEL]);
    if (k_state[NK_KEY_ENTER] >= 0) nk_input_key(ctx, NK_KEY_ENTER, k_state[NK_KEY_ENTER]);
    if (k_state[NK_KEY_TEXT_RESET_MODE] >= 0) nk_input_key(ctx, NK_KEY_TEXT_RESET_MODE, k_state[NK_KEY_TEXT_RESET_MODE]);
    if (k_state[NK_KEY_TAB] >= 0) nk_input_key(ctx, NK_KEY_TAB, k_state[NK_KEY_TAB]);
    if (k_state[NK_KEY_BACKSPACE] >= 0) nk_input_key(ctx, NK_KEY_BACKSPACE, k_state[NK_KEY_BACKSPACE]);
    if (k_state[NK_KEY_UP] >= 0) nk_input_key(ctx, NK_KEY_UP, k_state[NK_KEY_UP]);
    if (k_state[NK_KEY_DOWN] >= 0) nk_input_key(ctx, NK_KEY_DOWN, k_state[NK_KEY_DOWN]);
    if (k_state[NK_KEY_SCROLL_UP] >= 0) nk_input_key(ctx, NK_KEY_SCROLL_UP, k_state[NK_KEY_SCROLL_UP]);
    if (k_state[NK_KEY_SCROLL_DOWN] >= 0) nk_input_key(ctx, NK_KEY_SCROLL_DOWN, k_state[NK_KEY_SCROLL_DOWN]);
    if (k_state[NK_KEY_TEXT_INSERT_MODE] >= 0) nk_input_key(ctx, NK_KEY_TEXT_INSERT_MODE, k_state[NK_KEY_TEXT_INSERT_MODE]);
    if (k_state[NK_KEY_TEXT_REPLACE_MODE] >= 0) nk_input_key(ctx, NK_KEY_TEXT_REPLACE_MODE, k_state[NK_KEY_TEXT_REPLACE_MODE]);

    nk_input_key(ctx, NK_KEY_TEXT_START, glfwGetKey(win, GLFW_KEY_HOME) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_TEXT_END, glfwGetKey(win, GLFW_KEY_END) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_SCROLL_START, glfwGetKey(win, GLFW_KEY_HOME) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_SCROLL_END, glfwGetKey(win, GLFW_KEY_END) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_SHIFT, glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS||
                                    glfwGetKey(win, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);

    if (glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(win, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) {
        if (k_state[NK_KEY_COPY] >= 0) nk_input_key(ctx, NK_KEY_COPY, k_state[NK_KEY_COPY]);
        if (k_state[NK_KEY_PASTE] >= 0) nk_input_key(ctx, NK_KEY_PASTE, k_state[NK_KEY_PASTE]);
        if (k_state[NK_KEY_CUT] >= 0) nk_input_key(ctx, NK_KEY_CUT, k_state[NK_KEY_CUT]);
        if (k_state[NK_KEY_TEXT_UNDO] >= 0) nk_input_key(ctx, NK_KEY_TEXT_UNDO, k_state[NK_KEY_TEXT_UNDO]);
        if (k_state[NK_KEY_TEXT_REDO] >= 0) nk_input_key(ctx, NK_KEY_TEXT_REDO, k_state[NK_KEY_TEXT_REDO]);
        if (k_state[NK_KEY_TEXT_LINE_START] >= 0) nk_input_key(ctx, NK_KEY_TEXT_LINE_START, k_state[NK_KEY_TEXT_LINE_START]);
        if (k_state[NK_KEY_TEXT_LINE_END] >= 0) nk_input_key(ctx, NK_KEY_TEXT_LINE_END, k_state[NK_KEY_TEXT_LINE_END]);
        if (k_state[NK_KEY_TEXT_SELECT_ALL] >= 0) nk_input_key(ctx, NK_KEY_TEXT_SELECT_ALL, k_state[NK_KEY_TEXT_SELECT_ALL]);
        if (k_state[NK_KEY_LEFT] >= 0) nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, k_state[NK_KEY_LEFT]);
        if (k_state[NK_KEY_RIGHT] >= 0) nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, k_state[NK_KEY_RIGHT]);
    } else {
        if (k_state[NK_KEY_LEFT] >= 0) nk_input_key(ctx, NK_KEY_LEFT, k_state[NK_KEY_LEFT]);
        if (k_state[NK_KEY_RIGHT] >= 0) nk_input_key(ctx, NK_KEY_RIGHT, k_state[NK_KEY_RIGHT]);
        nk_input_key(ctx, NK_KEY_COPY, 0);
        nk_input_key(ctx, NK_KEY_PASTE, 0);
        nk_input_key(ctx, NK_KEY_CUT, 0);
    }

    glfwGetCursorPos(win, &x, &y);
    nk_input_motion(ctx, (int)x, (int)y);
    nk_input_button(ctx, NK_BUTTON_LEFT, (int)x, (int)y, glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    nk_input_button(ctx, NK_BUTTON_MIDDLE, (int)x, (int)y, glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS);
    nk_input_button(ctx, NK_BUTTON_RIGHT, (int)x, (int)y, glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
    nk_input_button(ctx, NK_BUTTON_DOUBLE, (int)NR_glfw.double_click_pos.x, (int)NR_glfw.double_click_pos.y, NR_glfw.is_double_click_down);
    nk_input_scroll(ctx, NR_glfw.scroll);
    nk_input_end(ctx);

    memset(NR_glfw.key_events, -1, sizeof(NR_glfw.key_events));
    NR_glfw.text_len = 0;
    NR_glfw.scroll = nk_vec2(0,0);
}

void NKGLI::UseProgram(GLuint prog) {
    struct nk_glfw_device *dev = &NR_glfw.ogl;
    dev->prog = prog;
    dev->uniform_tex = glGetUniformLocation(prog, "Texture");
    dev->uniform_proj = glGetUniformLocation(prog, "ProjMtx");
    dev->attrib_pos = glGetAttribLocation(prog, "Position");
    dev->attrib_uv = glGetAttribLocation(prog, "TexCoord");
    dev->attrib_col = glGetAttribLocation(prog, "Color");

    glBindVertexArray(dev->vao);
    glBindBuffer(GL_ARRAY_BUFFER, dev->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dev->ebo);

    GLsizei vs = sizeof(struct nk_glfw_vertex);
    size_t vp = offsetof(struct nk_glfw_vertex, position);
    size_t vt = offsetof(struct nk_glfw_vertex, uv);
    size_t vc = offsetof(struct nk_glfw_vertex, col);

    glEnableVertexAttribArray((GLuint)dev->attrib_pos);
    glEnableVertexAttribArray((GLuint)dev->attrib_uv);
    glEnableVertexAttribArray((GLuint)dev->attrib_col);
    glVertexAttribPointer((GLuint)dev->attrib_pos, 2, GL_FLOAT, GL_FALSE, vs, (void*)vp);
    glVertexAttribPointer((GLuint)dev->attrib_uv, 2, GL_FLOAT, GL_FALSE, vs, (void*)vt);
    glVertexAttribPointer((GLuint)dev->attrib_col, 4, GL_UNSIGNED_BYTE, GL_TRUE, vs, (void*)vc);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void NKGLI::UseDefaultProgram(AssetManager* manager,std::string version) {
    struct nk_glfw_device *dev = &NR_glfw.ogl;

    GLuint prog =  manager->LoadComplexShaderFromString(
R"(
#ifdef VSH
uniform mat4 ProjMtx;
in vec2 Position;
in vec2 TexCoord;
in vec4 Color;
out vec2 Frag_UV;
out vec4 Frag_Color;
void main() {
    Frag_UV = TexCoord;
    Frag_Color = Color;
    gl_Position = ProjMtx * vec4(Position, 0.0, 1.0);
}
#endif

#ifdef FSH
precision mediump float;
uniform sampler2D Texture;
in vec2 Frag_UV;
in vec4 Frag_Color;
out vec4 Out_Color;
void main() {
    Out_Color = Frag_Color * texture(Texture, Frag_UV);
}
#endif
)"
    ,version);

    dev->prog = prog;
    dev->uniform_tex = glGetUniformLocation(prog, "Texture");
    dev->uniform_proj = glGetUniformLocation(prog, "ProjMtx");
    dev->attrib_pos = glGetAttribLocation(prog, "Position");
    dev->attrib_uv = glGetAttribLocation(prog, "TexCoord");
    dev->attrib_col = glGetAttribLocation(prog, "Color");

    glBindVertexArray(dev->vao);
    glBindBuffer(GL_ARRAY_BUFFER, dev->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dev->ebo);

    GLsizei vs = sizeof(struct nk_glfw_vertex);
    size_t vp = offsetof(struct nk_glfw_vertex, position);
    size_t vt = offsetof(struct nk_glfw_vertex, uv);
    size_t vc = offsetof(struct nk_glfw_vertex, col);

    glEnableVertexAttribArray((GLuint)dev->attrib_pos);
    glEnableVertexAttribArray((GLuint)dev->attrib_uv);
    glEnableVertexAttribArray((GLuint)dev->attrib_col);
    glVertexAttribPointer((GLuint)dev->attrib_pos, 2, GL_FLOAT, GL_FALSE, vs, (void*)vp);
    glVertexAttribPointer((GLuint)dev->attrib_uv, 2, GL_FLOAT, GL_FALSE, vs, (void*)vt);
    glVertexAttribPointer((GLuint)dev->attrib_col, 4, GL_UNSIGNED_BYTE, GL_TRUE, vs, (void*)vc);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


void NKGLI::Render(int width, int height, enum nk_anti_aliasing AA) {
    struct nk_glfw_device *dev = &NR_glfw.ogl;
    struct nk_buffer vbuf, ebuf;
    GLfloat ortho[4][4] = {
        {2.0f, 0.0f, 0.0f, 0.0f},
        {0.0f,-2.0f, 0.0f, 0.0f},
        {0.0f, 0.0f,-1.0f, 0.0f},
        {-1.0f, 1.0f, 0.0f, 1.0f},
    };
    ortho[0][0] /= (GLfloat)width;
    ortho[1][1] /= (GLfloat)height;

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glActiveTexture(GL_TEXTURE0);

    glUseProgram(dev->prog);
    glUniform1i(dev->uniform_tex, 0);
    glUniformMatrix4fv(dev->uniform_proj, 1, GL_FALSE, &ortho[0][0]);

    glBindVertexArray(dev->vao);
    glBindBuffer(GL_ARRAY_BUFFER, dev->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dev->ebo);

    struct nk_convert_config config;
    static const struct nk_draw_vertex_layout_element vertex_layout[] = {
        {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_glfw_vertex, position)},
        {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_glfw_vertex, uv)},
        {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct nk_glfw_vertex, col)},
        {NK_VERTEX_LAYOUT_END}
    };
    memset(&config, 0, sizeof(config));
    config.vertex_layout = vertex_layout;
    config.vertex_size = sizeof(struct nk_glfw_vertex);
    config.vertex_alignment = NK_ALIGNOF(struct nk_glfw_vertex);
    config.tex_null = dev->tex_null;
    config.circle_segment_count = 22;
    config.curve_segment_count = 22;
    config.arc_segment_count = 22;
    config.global_alpha = 1.0f;
    config.shape_AA = AA;
    config.line_AA = AA;

    nk_buffer_init_default(&vbuf);   
    nk_buffer_init_default(&ebuf);   
    nk_convert(&NR_glfw.ctx, &dev->cmds, &vbuf, &ebuf, &config);

    glBufferData(GL_ARRAY_BUFFER, vbuf.size, vbuf.memory.ptr, GL_STREAM_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ebuf.size, ebuf.memory.ptr, GL_STREAM_DRAW);

    const struct nk_draw_command *cmd;
    nk_size offset = 0;
    nk_draw_foreach(cmd, &NR_glfw.ctx, &dev->cmds) {
        if (!cmd->elem_count) continue;
        glBindTexture(GL_TEXTURE_2D, (GLuint)cmd->texture.id);
        glScissor((GLint)cmd->clip_rect.x,
                  (GLint)(height - (cmd->clip_rect.y + cmd->clip_rect.h)),
                  (GLint)cmd->clip_rect.w,
                  (GLint)cmd->clip_rect.h);
        glDrawElements(GL_TRIANGLES, (GLsizei)cmd->elem_count, GL_UNSIGNED_SHORT, (const void*)offset);
        offset += cmd->elem_count * sizeof(nk_draw_index);
    }

    nk_clear(&NR_glfw.ctx);
    nk_buffer_clear(&dev->cmds);
    nk_buffer_free(&vbuf); 
    nk_buffer_free(&ebuf);

    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);
}


void NKGLI::CharCallback(GLFWwindow *win, unsigned int codepoint) {
    auto* ctx = static_cast<NKGLI*>(glfwGetWindowUserPointer(win));
    ctx->OnChar(codepoint);
}

void NKGLI::KeyCallback(GLFWwindow *win, int key, int scancode, int action, int mods) {
    auto* ctx = static_cast<NKGLI*>(glfwGetWindowUserPointer(win));
    ctx->OnKey(key, scancode, action, mods);
}

void NKGLI::ScrollCallback(GLFWwindow *win, double xoff, double yoff) {
    auto* ctx = static_cast<NKGLI*>(glfwGetWindowUserPointer(win));
    ctx->OnScroll(xoff, yoff);
}

void NKGLI::MouseButtonCallback(GLFWwindow *win, int button, int action, int mods) {
    auto* ctx = static_cast<NKGLI*>(glfwGetWindowUserPointer(win));
    ctx->OnMouseButton(button, action, mods);
}

void NKGLI::OnChar(unsigned int codepoint) {
    if (NR_glfw.text_len < NK_GLFW_TEXT_MAX)
        NR_glfw.text[NR_glfw.text_len++] = codepoint;
}

void NKGLI::OnKey(int key, int scancode, int action, int mods) {
    static int insert_toggle = 0;
    nk_char a = (action == GLFW_RELEASE) ? nk_false : nk_true;
    NK_UNUSED(scancode);
    NK_UNUSED(mods);

    switch (key) {
    case GLFW_KEY_DELETE:    NR_glfw.key_events[NK_KEY_DEL] = a; break;
    case GLFW_KEY_TAB:       NR_glfw.key_events[NK_KEY_TAB] = a; break;
    case GLFW_KEY_BACKSPACE: NR_glfw.key_events[NK_KEY_BACKSPACE] = a; break;
    case GLFW_KEY_UP:        NR_glfw.key_events[NK_KEY_UP] = a; break;
    case GLFW_KEY_DOWN:      NR_glfw.key_events[NK_KEY_DOWN] = a; break;
    case GLFW_KEY_LEFT:      NR_glfw.key_events[NK_KEY_LEFT] = a; break;
    case GLFW_KEY_RIGHT:     NR_glfw.key_events[NK_KEY_RIGHT] = a; break;
    case GLFW_KEY_ESCAPE:    NR_glfw.key_events[NK_KEY_TEXT_RESET_MODE] = a; break;
    case GLFW_KEY_PAGE_UP:   NR_glfw.key_events[NK_KEY_SCROLL_UP] = a; break;
    case GLFW_KEY_PAGE_DOWN: NR_glfw.key_events[NK_KEY_SCROLL_DOWN] = a; break;
    case GLFW_KEY_C:         NR_glfw.key_events[NK_KEY_COPY] = a; break;
    case GLFW_KEY_V:         NR_glfw.key_events[NK_KEY_PASTE] = a; break;
    case GLFW_KEY_X:         NR_glfw.key_events[NK_KEY_CUT] = a; break;
    case GLFW_KEY_Z:         NR_glfw.key_events[NK_KEY_TEXT_UNDO] = a; break;
    case GLFW_KEY_R:         NR_glfw.key_events[NK_KEY_TEXT_REDO] = a; break;
    case GLFW_KEY_B:         NR_glfw.key_events[NK_KEY_TEXT_LINE_START] = a; break;
    case GLFW_KEY_E:         NR_glfw.key_events[NK_KEY_TEXT_LINE_END] = a; break;
    case GLFW_KEY_A:         NR_glfw.key_events[NK_KEY_TEXT_SELECT_ALL] = a; break;
    case GLFW_KEY_ENTER:
    case GLFW_KEY_KP_ENTER:
        NR_glfw.key_events[NK_KEY_ENTER] = a;
        break;
    case GLFW_KEY_INSERT:
        if (!a) {
            insert_toggle = !insert_toggle;
            if (insert_toggle) {
                NR_glfw.key_events[NK_KEY_TEXT_INSERT_MODE] = !a;
            } else {
                NR_glfw.key_events[NK_KEY_TEXT_REPLACE_MODE] = !a;
            }
        }
        break;
    default: ;
    }
}

void NKGLI::OnScroll(double xoff, double yoff) {
    NR_glfw.scroll.x += (float)xoff;
    NR_glfw.scroll.y += (float)yoff;
}

void NKGLI::OnMouseButton(int button, int action, int mods) {
    double x, y;
    NK_UNUSED(mods);
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    glfwGetCursorPos(NR_glfw.win, &x, &y);
    if (action == GLFW_PRESS)  {
        double dt = glfwGetTime() - NR_glfw.last_button_click;
        if (dt > NK_GLFW_DOUBLE_CLICK_LO && dt < NK_GLFW_DOUBLE_CLICK_HI) {
            NR_glfw.is_double_click_down = nk_true;
            NR_glfw.double_click_pos = nk_vec2((float)x, (float)y);
        }
        NR_glfw.last_button_click = glfwGetTime();
    } else {
        NR_glfw.is_double_click_down = nk_false;
    }
}


struct nk_context* NKGLI::GetContext() {
    return const_cast<struct nk_context*>(&NR_glfw.ctx);
}

