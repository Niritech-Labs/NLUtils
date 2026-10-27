#include "../../../include/NLUtils/NLGToolkit/Widgets.hpp"

using namespace NLGT::Widgets;
using namespace NLGT::DrawCommands;

void DrawWindow::Begin() {
    if (nk_begin(this->castedBasePtr->nctx,this->castedBasePtr->name.c_str(),this->castedBasePtr->position,this->castedBasePtr->flags)) {
        this->beginSuccesful = true;
    }
}
void DrawWindow::Body() {
    forel [id,bodyCommands] inside(body) {
        auto* cmds = bodyCommands.get();
        cmds->Begin();
        if (this->beginSuccesful) cmds->Body();
        cmds->End();
    }
}
void DrawWindow::End() {
    this->beginSuccesful = false;
    nk_end(this->castedBasePtr->nctx);
}
void DrawWindow::Setup(Widget* base) {
    castedBasePtr = dynamic_cast<Window*>(base);
    basePtr = base;
}

void Window::Setup(struct nk_context* nctx) {
    drawCmds = std::make_shared<DrawWindow>();
    drawCmds.get()->Setup(this);
    this->nctx = nctx;
}
std::shared_ptr<Widget> Window::Get() {
    return std::make_shared<Window>(*this);
}


