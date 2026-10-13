#pragma once

#define DRAW_REALIZATION_STANDARD(baseClass)        \
class Draw##baseClass : public DrawCommand {        \
public:                                             \
    void Begin() override;                          \
    void Body() override;                           \
    void End() override;                            \
    void Setup(Widgets::Widget* base) override;     \
private:                                            \
    Widgets::baseClass* castedBasePtr = nullptr;    \
};

#define DRAW_REALIZATION_BEGIN_CHECK(baseClass)     \
class Draw##baseClass : public DrawCommand {        \
public:                                             \
    void Begin() override;                          \
    void Body() override;                           \
    void End() override;                            \
    void Setup(Widgets::Widget* base) override;     \
private:                                            \
    bool beginSuccesful = false;                    \
    Widgets::baseClass* castedBasePtr = nullptr;    \
};

#define DRAW_REALIZATION_COMPLEX(baseClass)          \
class Draw##baseClass : public DrawCommand {        \
public:                                             \
    void Begin() override;                          \
    void Body() override;                           \
    void End() override;                            \
    void Setup(Widgets::Widget* base) override;     \
private:                                            \
    int whatDrawRequest();                          \
    std::vector<uint64_t> toDraw;                   \
    Widgets::baseClass* castedBasePtr = nullptr;    \
};


namespace NLGT {

namespace Widgets {
    class Widget;
    
    class Window;
    class Layout;
    class TreeSubLayout;
    class GroupSubLayout;

    class Button;
    class Custom;
    class Label;
    class Text;
    class Image;
    class Value;
    class Checkbox;
    class Entry;
    class Slider;
    class Progress;
    class Property;
}

namespace DrawCommands {

class DrawCommand {
public:
    virtual bool AddCommand(uint64_t id, std::shared_ptr<DrawCommand> dc);
    virtual bool DelCommand(uint64_t id);
    virtual void DeleteAll();
    virtual void Begin() {}
    virtual void Body();
    virtual void End() {}

    virtual void Setup(Widgets::Widget* base);
    Widgets::Widget* basePtr = nullptr;
protected:
    std::unordered_map<uint64_t,std::shared_ptr<DrawCommand>> body;
};



DRAW_REALIZATION_BEGIN_CHECK(Window)

DRAW_REALIZATION_COMPLEX(Layout)
DRAW_REALIZATION_BEGIN_CHECK(TreeSubLayout)
DRAW_REALIZATION_BEGIN_CHECK(GroupSubLayout)

DRAW_REALIZATION_STANDARD(Button)
DRAW_REALIZATION_STANDARD(Custom)
DRAW_REALIZATION_STANDARD(Label)
DRAW_REALIZATION_STANDARD(Text)
DRAW_REALIZATION_STANDARD(Image)
DRAW_REALIZATION_STANDARD(Value)
DRAW_REALIZATION_STANDARD(Checkbox)
DRAW_REALIZATION_STANDARD(Entry)
DRAW_REALIZATION_STANDARD(Slider)
DRAW_REALIZATION_STANDARD(Progress)
DRAW_REALIZATION_STANDARD(Property)

}
}