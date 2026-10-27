#include "../../../include/NLUtils/NLGToolkit/Widgets.hpp"

using namespace NLGT::Widgets;
using namespace NLGT::DrawCommands;

void DrawProperty::Begin() {}
void DrawProperty::Body() {
    switch (castedBasePtr->propertyType) {
        case Property::Type::INT:
            nk_property_int(castedBasePtr->nctx, castedBasePtr->text.c_str(), (int)castedBasePtr->minValue, castedBasePtr->intProperty.get(), (int)castedBasePtr->maxValue, (int)castedBasePtr->valPerStep, castedBasePtr->incPerPixel);
            break;
        case Property::Type::FLOAT:
            nk_property_float(castedBasePtr->nctx, castedBasePtr->text.c_str(), castedBasePtr->minValue, castedBasePtr->floatProperty.get(), castedBasePtr->maxValue, castedBasePtr->valPerStep, castedBasePtr->incPerPixel);
            break;
    }
}
void DrawProperty::End() {}
void DrawProperty::Setup(Widget* base) {
    castedBasePtr = dynamic_cast<Property*>(base);
    basePtr = base;
}

WIDGET_SETUP_DEFAULT(Property)
WIDGET_GET_DEFAULT(Property)