#include "../../../include/NLUtils/NLGToolkit/Widgets.hpp"
#include <map>

using namespace NLGT::Widgets;
using namespace NLGT::DrawCommands;

void DrawTreeSubLayout::Begin() {
    if (castedBasePtr->image.handle.id == 0) {
        if (nk_tree_push_id(castedBasePtr->nctx,castedBasePtr->type,castedBasePtr->name.c_str(),castedBasePtr->state,castedBasePtr->id)) {
            this->beginSuccesful = true;
        }
    }
    else {
        if (nk_tree_image_push_id(castedBasePtr->nctx,castedBasePtr->type,castedBasePtr->image,castedBasePtr->name.c_str(),castedBasePtr->state,castedBasePtr->id)) {
            this->beginSuccesful = true;
        }
    }
    
}
void DrawTreeSubLayout::Body() {
    forel [id,bodyCommands] in(body) {
        auto* cmds = bodyCommands.get();
        cmds->Begin();
        if (this->beginSuccesful) cmds->Body();
        cmds->End();
    }
}
void DrawTreeSubLayout::End() {
    if (beginSuccesful) {
        nk_tree_pop(castedBasePtr->nctx);
        this->beginSuccesful = false;
    }
   
}
void DrawTreeSubLayout::Setup(Widget* base) {
    castedBasePtr = dynamic_cast<TreeSubLayout*>(base);
    basePtr = base;
}
WIDGET_SETUP_DEFAULT(TreeSubLayout)
WIDGET_GET_DEFAULT(TreeSubLayout)


void DrawGroupSubLayout::Begin() {
    if (nk_group_begin(castedBasePtr->nctx,castedBasePtr->name.c_str(),castedBasePtr->flags)) {
        this->beginSuccesful = true;
    }
}
void DrawGroupSubLayout::Body() {
    forel [id,bodyCommands] in(body) {
        auto* cmds = bodyCommands.get();
        cmds->Begin();
        if (this->beginSuccesful) cmds->Body();
        cmds->End();
    }
}
void DrawGroupSubLayout::End() {
    if (beginSuccesful) {
        nk_group_end(castedBasePtr->nctx);
        this->beginSuccesful = false;
    }
   
}
void DrawGroupSubLayout::Setup(Widget* base) {
    castedBasePtr = dynamic_cast<GroupSubLayout*>(base);
    basePtr = base;
}
WIDGET_SETUP_DEFAULT(GroupSubLayout)
WIDGET_GET_DEFAULT(GroupSubLayout)