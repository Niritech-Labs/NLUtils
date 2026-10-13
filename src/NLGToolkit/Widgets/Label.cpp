#include "../../../include/NLUtils/NLGToolkit/Widgets.hpp"

using namespace NLGT::Widgets;
using namespace NLGT::DrawCommands;

void DrawLabel::Begin() {}

void DrawLabel::Body() {
    switch (castedBasePtr->labelType) {
        case Label::Type::BASE:
            if (castedBasePtr->autowrap)
                nk_label_wrap(castedBasePtr->nctx, castedBasePtr->text.c_str());
            else
                nk_label(castedBasePtr->nctx, castedBasePtr->text.c_str(), castedBasePtr->alignment);
            break;
        case Label::Type::COLORED:
            if (castedBasePtr->autowrap)
                nk_label_colored_wrap(castedBasePtr->nctx, castedBasePtr->text.c_str(), castedBasePtr->color);
            else
                nk_label_colored(castedBasePtr->nctx, castedBasePtr->text.c_str(), castedBasePtr->alignment, castedBasePtr->color);
            break;
    }
}

void DrawLabel::End() {}

void DrawLabel::Setup(Widget* base) {
    castedBasePtr = dynamic_cast<Label*>(base);
    basePtr = base;
}

WIDGET_SETUP_DEFAULT(Label)
WIDGET_GET_DEFAULT(Label)