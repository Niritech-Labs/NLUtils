#include "../../../include/NLUtils/NLGToolkit/Widgets.hpp"

using namespace NLGT::Widgets;
using namespace NLGT::DrawCommands;


void DrawColorPicker::Begin() {}
void DrawColorPicker::Body() {
    castedBasePtr->color = nk_color_picker(castedBasePtr->nctx,castedBasePtr->color,castedBasePtr->format);
}
void DrawColorPicker::End() {}
void DrawColorPicker::Setup(Widget* base) {
    castedBasePtr = dynamic_cast<ColorPicker*>(base);
    basePtr = base;
}

WIDGET_SETUP_DEFAULT(ColorPicker)
WIDGET_GET_DEFAULT(ColorPicker)