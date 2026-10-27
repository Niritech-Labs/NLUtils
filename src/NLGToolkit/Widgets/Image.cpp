#include "../../../include/NLUtils/NLGToolkit/Widgets.hpp"

using namespace NLGT::Widgets;
using namespace NLGT::DrawCommands;

void DrawImage::Begin() {}
void DrawImage::Body() {
    nk_image(castedBasePtr->nctx, castedBasePtr->image);
}
void DrawImage::End() {}
void DrawImage::Setup(Widget* base) {
    castedBasePtr = dynamic_cast<Image*>(base);
    basePtr = base;
}

WIDGET_SETUP_DEFAULT(Image)
WIDGET_GET_DEFAULT(Image)