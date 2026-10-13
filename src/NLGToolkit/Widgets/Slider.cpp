#include "../../../include/NLUtils/NLGToolkit/Widgets.hpp"

using namespace NLGT::Widgets;
using namespace NLGT::DrawCommands;

void DrawSlider::Begin() {}
void DrawSlider::Body() {
    switch (castedBasePtr->sliderType) {
        case Slider::Type::INT:
            nk_slider_int(castedBasePtr->nctx, (int)castedBasePtr->minValue, castedBasePtr->intValue.get(), (int)castedBasePtr->maxValue, (int)castedBasePtr->valPerStep);
            break;
        case Slider::Type::FLOAT:
            nk_slider_float(castedBasePtr->nctx, castedBasePtr->minValue, castedBasePtr->floatValue.get(), castedBasePtr->maxValue, castedBasePtr->valPerStep);
            break;
    }
}
void DrawSlider::End() {}
void DrawSlider::Setup(Widget* base) {
    castedBasePtr = dynamic_cast<Slider*>(base);
    basePtr = base;
}

WIDGET_SETUP_DEFAULT(Slider)
WIDGET_GET_DEFAULT(Slider)