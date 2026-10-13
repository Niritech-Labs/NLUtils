#include "../../../include/NLUtils/NLGToolkit/Widgets.hpp"

using namespace NLGT::Widgets;
using namespace NLGT::DrawCommands;

void DrawValue::Begin() {}
void DrawValue::Body() {
    switch (castedBasePtr->valueType) {
        case Value::Type::BOOL:  
            nk_value_bool(castedBasePtr->nctx,  castedBasePtr->text.c_str(), castedBasePtr->boolValue);  
            break;
        case Value::Type::INT:   
            nk_value_int(castedBasePtr->nctx,   castedBasePtr->text.c_str(), castedBasePtr->intValue);   
            break;
        case Value::Type::FLOAT: 
            nk_value_float(castedBasePtr->nctx, castedBasePtr->text.c_str(), castedBasePtr->floatValue); 
            break;
    }
}
void DrawValue::End() {}
void DrawValue::Setup(Widget* base) {
    castedBasePtr = dynamic_cast<Value*>(base);
    basePtr = base;
}

WIDGET_SETUP_DEFAULT(Value)
WIDGET_GET_DEFAULT(Value)