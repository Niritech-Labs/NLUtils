#include "../../../include/NLUtils/NLGToolkit/Widgets.hpp"

using namespace NLGT::Widgets;
using namespace NLGT::DrawCommands;

void DrawCheckbox::Begin() {}
void DrawCheckbox::Body() {
    nk_checkbox_label(castedBasePtr->nctx, castedBasePtr->text.c_str(), castedBasePtr->state.get());
}
void DrawCheckbox::End() {}
void DrawCheckbox::Setup(Widget* base) {
    castedBasePtr = dynamic_cast<Checkbox*>(base);
    basePtr = base;
}

WIDGET_SETUP_DEFAULT(Checkbox)
WIDGET_GET_DEFAULT(Checkbox)


