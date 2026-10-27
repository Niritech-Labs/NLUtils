#include "../../../include/NLUtils/NLGToolkit/Widgets.hpp"

using namespace NLGT::Widgets;
using namespace NLGT::DrawCommands;

void DrawProgress::Begin() {}
void DrawProgress::Body() {
    nk_size current = *castedBasePtr->currentValue.get();
    nk_progress(castedBasePtr->nctx, &current, castedBasePtr->maxValue, castedBasePtr->modifyable);
    *castedBasePtr->currentValue.get() = (float)current;
}
void DrawProgress::End() {}
void DrawProgress::Setup(Widget* base) {
    castedBasePtr = dynamic_cast<Progress*>(base);
    basePtr = base;
}

WIDGET_SETUP_DEFAULT(Progress)
WIDGET_GET_DEFAULT(Progress)