#include "../../../include/NLUtils/NLGToolkit/Widgets.hpp"

using namespace NLGT::Widgets;
using namespace NLGT::DrawCommands;

void DrawText::Begin() {}
void DrawText::Body() {
    switch (castedBasePtr->textType) {
        case Text::Type::BASE:
            if (castedBasePtr->autowrap)
                nk_text_wrap(castedBasePtr->nctx, castedBasePtr->text.c_str(), castedBasePtr->length);
            else
                nk_text(castedBasePtr->nctx, castedBasePtr->text.c_str(), castedBasePtr->length, castedBasePtr->alignment);
            break;
        case Text::Type::COLORED:
            if (castedBasePtr->autowrap)
                nk_text_wrap_colored(castedBasePtr->nctx, castedBasePtr->text.c_str(), castedBasePtr->length, castedBasePtr->color);
            else
                nk_text_colored(castedBasePtr->nctx, castedBasePtr->text.c_str(), castedBasePtr->length, castedBasePtr->alignment, castedBasePtr->color);
            break;
    }
}
void DrawText::End() {}
void DrawText::Setup(Widget* base) {
    castedBasePtr = dynamic_cast<Text*>(base);
    basePtr = base;
}

WIDGET_SETUP_DEFAULT(Text)
WIDGET_GET_DEFAULT(Text)