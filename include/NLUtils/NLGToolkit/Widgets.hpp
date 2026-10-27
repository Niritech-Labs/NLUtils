#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include <functional>

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

#include "../Logger.hpp"
#include "../Extra/Lang.hpp"
#include "Widgets/DrawCommands.hpp"
#include <glm/glm.hpp>



#define WIDGET_SETUP_DEFAULT(Class)             \
void Class::Setup(struct nk_context* nctx) {    \
    drawCmds = std::make_shared<Draw##Class>(); \
    drawCmds.get()->Setup(this);                \
    this->nctx = nctx;                          \
}

#define WIDGET_GET_DEFAULT(Class)               \
std::shared_ptr<Widget> Class::Get() {          \
    return std::make_shared<Class>(*this);      \
}

namespace NLGT {

namespace Widgets {

class IDManager {
public:
    static uint64_t Acquire();
    static void Ref(uint64_t id);
    static void Unref(uint64_t id);
private:
    static uint64_t next;
    static std::vector<uint64_t> free;
    static std::unordered_map<uint64_t,uint32_t> refs;
};


class DrawStack;
class Widget;

class DrawStack {
public:
    NL_LOGGER_METHODS 
    DrawStack(struct nk_context* nctx);
    void Add(Widget& widget);
    void Delete(Widget& widget);
    void Draw();
    template <typename Wgti>
    std::shared_ptr<Wgti> GetWidget(uint64_t id);
    template <typename Wgt>
    std::shared_ptr<Wgt> GetWidget(Widget& widget);

private:
    NLLogger Logger;
    DrawCommands::DrawCommand drawCmds;
    struct nk_context* nctx = nullptr;
    std::unordered_map<uint64_t,std::shared_ptr<Widget>> Widgets;
};



class Widget {
public:
    Widget() {
        id = IDManager::Acquire();
    }
    Widget(uint64_t id) {
        this->id = id;
        IDManager::Ref(id);
    }
    Widget(Widget& parent) {
        id = IDManager::Acquire();
        parentid = parent.id;
    }
    Widget(Widget& parent, uint64_t id) {
        this->id = id;
        parentid = parent.id;
        IDManager::Ref(id);
    }
    Widget(uint64_t parentid, uint64_t id) {
        this->id = id;
        this->parentid = parentid;
        IDManager::Ref(id);
    }
    Widget(const Widget& other) {
        id = other.id;
        parentid = other.parentid;
        __toDelete = false;
        IDManager::Ref(id);
    }
    ~Widget() {
        IDManager::Unref(id);
    }

    uint64_t id;
    uint64_t parentid = UINT64_MAX;

    std::shared_ptr<DrawCommands::DrawCommand> drawCmds;

    struct nk_context* nctx = nullptr;

    virtual void Setup(struct nk_context* nctx) {
        drawCmds = std::make_shared<DrawCommands::DrawCommand>();
        drawCmds.get()->Setup(this);
        this->nctx = nctx;
    }

    virtual std::shared_ptr<Widget> Get() {
        return std::make_shared<Widget>(*this);
    }

    bool __toDelete = false;
};


template <typename Wgti>
std::shared_ptr<Wgti> DrawStack::GetWidget(uint64_t id) {
    auto it = this->Widgets.find(id);
    if (it != Widgets.end()) {
        std::shared_ptr<Wgti> dc = std::dynamic_pointer_cast<Wgti>(it->second);
        if (dc) {
            return dc;
        }
        else {
            Logger.Warning("different Widget ["+std::to_string(id)+"] type");
            return nullptr;
        }
    }
    else {
        Logger.Warning("Widget ["+std::to_string(id)+"] is not found");
        return nullptr;
    }
}
template <typename Wgt>
std::shared_ptr<Wgt> DrawStack::GetWidget(Widget& widget) {
    auto it = this->Widgets.find(widget.id);
    if (it != Widgets.end()) {
        std::shared_ptr<Wgt> dc = std::dynamic_pointer_cast<Wgt>(it->second);
        if (dc) {
            return dc;
        }
        else {
            Logger.Warning("different Widget ["+std::to_string(widget.id)+"] type");
            return nullptr;
        }
    }
    else {
        Logger.Warning("Widget ["+std::to_string(widget.id)+"] is not found");
        return nullptr;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///WIDGETS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////// Windods /////////////////////////////////////////////////
/// Window 
class Window : public Widget {
public:
    using Widget::Widget;
    void Setup(struct nk_context* nctx) override;
    std::shared_ptr<Widget> Get() override;
    std::string name;
    nk_flags flags;
    struct nk_rect position;
};

//////////////////////////////// Layouts /////////////////////////////////////////////////
/// Layout
class Layout : public Widget {
public:
    using Widget::Widget;
    void Setup(struct nk_context* nctx) override;
    std::shared_ptr<Widget> Get() override;
    enum class Type : int {
        GRID,
        SPACE,
        ROW
    } layoutType;
    void SetPosition(Widget& positionedWidget,struct nk_rect pos);
    void SetPosition(Widget& positionedWidget,int order);
    void SetPosition(Widget& positionedWidget,float width,int order);
    void SetPosition(Widget& positionedWidget,glm::vec2 pos);

    void SetPosition(uint64_t positionedWidget,struct nk_rect pos);
    void SetPosition(uint64_t positionedWidget,int order);
    void SetPosition(uint64_t positionedWidget,float width,int order);
    void SetPosition(uint64_t positionedWidget,glm::vec2 pos);

    bool UnsetPosition(Widget& positionedWidget);
    bool UnsetPosition(uint64_t positionedWidget);
    void UnsetPositionAll();

    std::unordered_map<uint64_t,glm::mat3x4> positions;
    float height;
    nk_layout_format posFormat;
};

/// TreeSubLayout
class TreeSubLayout : public Widget {
public:
    using Widget::Widget;
    void Setup(struct nk_context* nctx) override;
    std::shared_ptr<Widget> Get() override;

    struct nk_image image = nk_image_id(0);

    nk_tree_type type = NK_TREE_TAB;
    nk_collapse_states state = NK_MINIMIZED;
    
    std::string name = "";
};

/// GroupSubLayout 
class GroupSubLayout : public Widget {
public:
    using Widget::Widget;
    void Setup(struct nk_context* nctx) override;
    std::shared_ptr<Widget> Get() override;
    std::string name = "";
    nk_flags flags = 0;
};



//////////////////////////////// Elements /////////////////////////////////////////////////
/// Custom 
class Custom : public Widget {
public:
    using Widget::Widget;
    void Setup(struct nk_context* nctx) override;
    std::shared_ptr<Widget> Get() override;

    std::shared_ptr<void> data;
    std::function<void(struct nk_context*,std::shared_ptr<void>)> functional;
};

/// Button 
struct RadioGroup {
    std::unordered_map<uint64_t,bool> states;

    void SetActive(uint64_t button) {
        for (auto& [id, active] : states) {
            active = false;
        }
        states[button] = true;
    }

    bool GetState(uint64_t button) {
        auto it = states.find(button);
        if (it != states.end()) return it->second;
        return false;
    }

    uint64_t GetActive() {
        for (auto& [id, active] : states) {
            if (active) return id;
        }
        return UINT64_MAX;
    }
};
class Button : public Widget {
public:
    using Widget::Widget;
    void Setup(struct nk_context* nctx) override;
    std::shared_ptr<Widget> Get() override;

    enum class Type : int {
        RADIO,
        BASE,
        SYMBOL
    } buttonType;

    
    //RADIO
    std::shared_ptr<RadioGroup> group;

    //SYMBOL
    nk_symbol_type symbolType = NK_SYMBOL_NONE;

    //BASE
    struct nk_image img = nk_image_id(0);
    //BASE
    std::string text = "";
    //BASE
    nk_text_align alignment = NK_TEXT_ALIGN_CENTERED;
    //BASE
    bool toggleMode = false;
    //If Toggle mode 
    bool toggleState = false;

    std::function<void(glm::vec2)> RightClick;
    std::function<void(glm::vec2,bool)> LeftClick;
};

/// Label 
class Label : public Widget {
public:
    using Widget::Widget;
    void Setup(struct nk_context* nctx) override;
    std::shared_ptr<Widget> Get() override;


    enum class Type : int {
        BASE,
        COLORED
    } labelType;

    std::string text = "";
    nk_text_align alignment = NK_TEXT_ALIGN_CENTERED;
    bool autowrap = false;
    nk_color color;
};

/// Text 
class Text : public Widget {
public:
    using Widget::Widget;
    void Setup(struct nk_context* nctx) override;
    std::shared_ptr<Widget> Get() override;

    enum class Type : int {
        BASE,
        COLORED
    } textType;

    std::string text = "";
    int length = 0;
    nk_text_align alignment = NK_TEXT_ALIGN_CENTERED;
    bool autowrap = false;
    nk_color color = nk_rgb(50,50,50);
};

/// Image 
class Image : public Widget {
public:
    using Widget::Widget;
    void Setup(struct nk_context* nctx) override;
    std::shared_ptr<Widget> Get() override;

    struct nk_image image = nk_image_id(0);
};

/// Value 
class Value : public Widget {
public:
    using Widget::Widget;
    void Setup(struct nk_context* nctx) override;
    std::shared_ptr<Widget> Get() override;

    enum class Type : int {
        BOOL,
        INT,
        FLOAT
    } valueType;

    std::string text = ""; 
    int intValue = 0;
    bool boolValue = 0;
    float floatValue = 0;

};

/// Checkbox
class Checkbox : public Widget {
public:
    using Widget::Widget;
    void Setup(struct nk_context* nctx) override;
    std::shared_ptr<Widget> Get() override;

    std::string text = ""; 
    std::shared_ptr<bool> state;
};

/// Entry 
class Entry : public Widget {
public:
    using Widget::Widget;
    void Setup(struct nk_context* nctx) override;
    std::shared_ptr<Widget> Get() override;

    int lenth = 512;
    std::shared_ptr<std::string> text;

    nk_plugin_filter filter = nk_filter_default;
    nk_flags editFlags = 0;

    std::function<void(Entry*)> Active;
    std::function<void(Entry*)> Deactivated;
    std::function<void(Entry*)> Commited;
};

/// Slider
class Slider : public Widget {
public:
    using Widget::Widget;
    void Setup(struct nk_context* nctx) override;
    std::shared_ptr<Widget> Get() override;

    enum class Type : int {
        INT,
        FLOAT
    } sliderType;

    float valPerStep = 1;
    float maxValue = 100;
    float minValue = 0;

    std::shared_ptr<int> intValue;
    std::shared_ptr<float> floatValue;

};

/// Progress
class Progress : public Widget {
public:
    using Widget::Widget;
    void Setup(struct nk_context* nctx) override;
    std::shared_ptr<Widget> Get() override;

    bool modifyable = false;
    float maxValue = 100;
    std::shared_ptr<float> currentValue;

};

/// Property 
class Property : public Widget {
public:
    using Widget::Widget;
    void Setup(struct nk_context* nctx) override;
    std::shared_ptr<Widget> Get() override;

    enum class Type : int {
        INT,
        FLOAT
    } propertyType;

    float valPerStep = 1;
    float incPerPixel = 2;
    float maxValue = 100;
    float minValue = 0;

    std::string text = ""; 

    std::shared_ptr<int> intProperty;
    std::shared_ptr<float> floatProperty;
};


//ListWiew

//Menubar

//Combobox
class Combobox : public Widget {
public:
    using Widget::Widget;
    void Setup(struct nk_context* nctx) override;
    std::shared_ptr<Widget> Get() override;

    int currentElement = 0;
    std::vector<std::string> options;
    float optionsHeight = 20;
    glm::vec2 size = glm::vec2(40);
};



//Knob
class Knob : public Widget {
public:
    using Widget::Widget;
    void Setup(struct nk_context* nctx) override;
    std::shared_ptr<Widget> Get() override;

    enum class Type : int {
        INT,
        FLOAT
    } knobType;

    float step = 1;
    float maxValue = 100;
    float minValue = 0;
    float deadZone_degrees = 45;
    nk_heading heading = NK_DOWN;

    std::shared_ptr<int> intKnob;
    std::shared_ptr<float> floatKnob;
};


//ColorPicker
class ColorPicker : public Widget {
public:
    using Widget::Widget;
    void Setup(struct nk_context* nctx) override;
    std::shared_ptr<Widget> Get() override;

    nk_colorf color = {0,0,0,0};
    nk_color_format format = NK_RGBA;
};


}
}


