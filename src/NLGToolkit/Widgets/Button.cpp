#include "../../../include/NLUtils/NLGToolkit/Widgets.hpp"
#include <map>

using namespace NLGT::Widgets;
using namespace NLGT::DrawCommands;


void DrawButton::Begin() {}
void DrawButton::Body() {
    struct nk_rect bounds = nk_widget_bounds(castedBasePtr->nctx);
    glm::vec2 pos(bounds.x, bounds.y);

    bool clicked = false;

    switch (castedBasePtr->buttonType) {
        case Button::Type::RADIO:
            clicked = nk_option_label(castedBasePtr->nctx, castedBasePtr->text.c_str(), castedBasePtr->toggleState);
            break;

        case Button::Type::SYMBOL:
            clicked = nk_button_symbol(castedBasePtr->nctx, castedBasePtr->symbolType);
            break;

        case Button::Type::BASE:
            if (castedBasePtr->img.handle.id != 0 && !castedBasePtr->text.empty()) {
                clicked = nk_button_image_text(castedBasePtr->nctx, castedBasePtr->img, castedBasePtr->text.c_str(),(int)castedBasePtr->text.size(), castedBasePtr->alignment);
            }
            else if (castedBasePtr->img.handle.id != 0) {
                clicked = nk_button_image(castedBasePtr->nctx, castedBasePtr->img);
            }
            else {
                clicked = nk_button_label(castedBasePtr->nctx, castedBasePtr->text.c_str());
            }
            break;
        default:
            break;
    }

    

    if (clicked) {
        if (castedBasePtr->group && castedBasePtr->buttonType == Button::Type::RADIO) {
            castedBasePtr->group->SetActive(castedBasePtr->id);
            castedBasePtr->toggleState = castedBasePtr->group->GetState(castedBasePtr->id);
        }
        else if (castedBasePtr->buttonType == Button::Type::RADIO) {
            castedBasePtr->toggleState = !castedBasePtr->toggleState;
        }

        if (castedBasePtr->buttonType != Button::Type::RADIO || castedBasePtr->toggleMode == true) {
            castedBasePtr->toggleState = !castedBasePtr->toggleState;
        }

        if (castedBasePtr->LeftClick) castedBasePtr->LeftClick(pos,castedBasePtr->toggleState);
    }

    if (castedBasePtr->RightClick && nk_input_is_mouse_hovering_rect(&castedBasePtr->nctx->input, bounds) && nk_input_mouse_clicked(&castedBasePtr->nctx->input, NK_BUTTON_RIGHT, bounds)) {
        castedBasePtr->RightClick(pos);
    }
}
void DrawButton::End() {}
void DrawButton::Setup(Widget* base) {
    castedBasePtr = dynamic_cast<Button*>(base);
    basePtr = base;
}

WIDGET_SETUP_DEFAULT(Button)
WIDGET_GET_DEFAULT(Button)