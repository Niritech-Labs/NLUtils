#include "../../../include/NLUtils/NLGToolkit/Widgets.hpp"

using namespace NLGT::Widgets;
using namespace NLGT::DrawCommands;

void DrawKnob::Begin() {}
void DrawKnob::Body() {
    switch (castedBasePtr->knobType) {
        case Knob::Type::INT:
            nk_knob_int(castedBasePtr->nctx,(int)castedBasePtr->minValue,castedBasePtr->intKnob.get(),(int)castedBasePtr->maxValue,(int)castedBasePtr->step,castedBasePtr->heading,castedBasePtr->deadZone_degrees);
            break;
        case Knob::Type::FLOAT:
            nk_knob_float(castedBasePtr->nctx,castedBasePtr->minValue,castedBasePtr->floatKnob.get(),castedBasePtr->maxValue,castedBasePtr->step,castedBasePtr->heading,castedBasePtr->deadZone_degrees);
            break;
    }
}
void DrawKnob::End() {}
void DrawKnob::Setup(Widget* base) {
    castedBasePtr = dynamic_cast<Knob*>(base);
    basePtr = base;
}

WIDGET_SETUP_DEFAULT(Knob)
WIDGET_GET_DEFAULT(Knob)