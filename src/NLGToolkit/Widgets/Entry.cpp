#include "../../../include/NLUtils/NLGToolkit/Widgets.hpp"

using namespace NLGT::Widgets;
using namespace NLGT::DrawCommands;

void DrawEntry::Begin() {}
void DrawEntry::Body() {
    if ((int)castedBasePtr->text->size() < castedBasePtr->lenth) {
        castedBasePtr->text->resize(castedBasePtr->lenth);
    }

    nk_flags result = nk_edit_string_zero_terminated(castedBasePtr->nctx, castedBasePtr->editFlags, castedBasePtr->text->data(), castedBasePtr->lenth, castedBasePtr->filter);

    if ((result & NK_EDIT_ACTIVATED) && castedBasePtr->Active) castedBasePtr->Active(castedBasePtr);
    if ((result & NK_EDIT_DEACTIVATED) && castedBasePtr->Deactivated) castedBasePtr->Deactivated(castedBasePtr);
    if ((result & NK_EDIT_COMMITED) && castedBasePtr->Commited) castedBasePtr->Commited(castedBasePtr);
}
void DrawEntry::End() {}
void DrawEntry::Setup(Widget* base) {
    castedBasePtr = dynamic_cast<Entry*>(base);
    basePtr = base;
}

WIDGET_SETUP_DEFAULT(Entry)
WIDGET_GET_DEFAULT(Entry)