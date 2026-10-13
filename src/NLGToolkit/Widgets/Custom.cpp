#include "../../../include/NLUtils/NLGToolkit/Widgets.hpp"

using namespace NLGT::Widgets;
using namespace NLGT::DrawCommands;

void DrawCustom::Begin() {}
void DrawCustom::Body() {
    if (castedBasePtr->functional) {
        castedBasePtr->functional(castedBasePtr->nctx, castedBasePtr->data);
    }
    forel [id,bodyCommands] in(body) {
        auto* cmds = bodyCommands.get();
        cmds->Begin();
        cmds->Body();
        cmds->End();
    }
}
void DrawCustom::End() {}
void DrawCustom::Setup(Widget* base) {
    castedBasePtr = dynamic_cast<Custom*>(base);
    basePtr = base;
}

WIDGET_SETUP_DEFAULT(Custom)
WIDGET_GET_DEFAULT(Custom)