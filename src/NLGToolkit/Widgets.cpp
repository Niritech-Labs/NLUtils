#include "../../include/NLUtils/NLGToolkit/Widgets.hpp"


using namespace NLGT::Widgets;
using namespace NLGT::DrawCommands;

uint64_t IDManager::next = 1;
std::vector<uint64_t> IDManager::free;
std::unordered_map<uint64_t,uint32_t> IDManager::refs;

uint64_t IDManager::Acquire() {
    uint64_t id;
    if (free.empty()) {
        id = next++;
    } else {
        id = free.back();
        free.pop_back();
    }
    refs[id] = 1;
    return id;
}
void IDManager::Ref(uint64_t id) {
    refs[id]++;
}
void IDManager::Unref(uint64_t id) {
    auto it = refs.find(id);
    if (it == refs.end()) return;
    if (--it->second == 0) {
        refs.erase(it);
        free.push_back(id);
    }
}



DrawStack::DrawStack(struct nk_context* nctx) {
    this->nctx = nctx;
    
}
void DrawStack::Add(Widget& widget) {
    auto it = Widgets.find(widget.id);
    if (it == Widgets.end()) {
        Widgets[widget.id] = widget.Get();
        Widgets[widget.id].get()->Setup(nctx);
        if (widget.parentid != UINT64_MAX) {
            auto pt_it = Widgets.find(widget.parentid);
            if (pt_it != Widgets.end()) {
                pt_it->second.get()->drawCmds.get()->AddCommand(widget.id,Widgets[widget.id].get()->drawCmds);
            }
            else {
                Logger.Warning("Parent ["+ std::to_string(widget.id) +"] is not found");
            }
        }
        else {
            this->drawCmds.AddCommand(widget.id,Widgets[widget.id].get()->drawCmds);
        }
    }
    else {
        Logger.Warning("Widget is already added");
    }
}
void DrawStack::Delete(Widget& widget) {
    auto it = Widgets.find(widget.id);
    auto itp = Widgets.find(widget.parentid);
    if (widget.parentid != UINT64_MAX) {
        if (itp != Widgets.end()) { 
            if (it != Widgets.end()) {
                itp->second.get()->drawCmds.get()->DelCommand(widget.id);
            }
            else {
                Logger.Warning("Widget is not found");
            }
        }
        else {
            Logger.Warning("Parent Widget is not found");
        }
    }
    else {
        if (it != Widgets.end()) {
            this->drawCmds.DelCommand(it->first);
        }
        else {
            Logger.Warning("Widget is not found");
        }
    }
    std::vector<uint64_t> toDelete;
    forel& [id,element] in(Widgets) {
        if (element.get()->__toDelete) {
            toDelete.push_back(id);
        }
    }
    forel& tdId in(toDelete) {
        Widgets.erase(tdId);
    }

}
void DrawStack::Draw() {
    this->drawCmds.Body();
}
std::shared_ptr<Widget> DrawStack::GetWidget(uint64_t id) {
    auto it = this->Widgets.find(id);
    if (it != Widgets.end()) {
        return it->second;
    }
    else {
        Logger.Warning("Widget is not found");
        return nullptr;
    }
}
std::shared_ptr<Widget> DrawStack::GetWidget(Widget& widget) {
    auto it = this->Widgets.find(widget.id);
    if (it != Widgets.end()) {
        return it->second;
    }
    else {
        Logger.Warning("Widget is not found");
        return nullptr;
    }
}



bool DrawCommand::AddCommand(uint64_t id, std::shared_ptr<DrawCommand> dc) {
    auto it = body.find(id);
    if (it == body.end()) {
        body[id] = dc;
        return true;
    }
    return false;
}
bool DrawCommand::DelCommand(uint64_t id) {
    auto it = body.find(id);
    if (it != body.end()) {
        it->second.get()->DeleteAll();
        body.erase(id); 
        return true;
    }
    return false;
}
void DrawCommand::DeleteAll() {
    forel& [id,el] in(body) {
        el.get()->DeleteAll();
    }
    body.clear();
    basePtr->__toDelete = true;
}
void DrawCommand::Body() {
    forel& [id,bodyCommands] in(body) {
        auto* cmds = bodyCommands.get();
        cmds->Begin();
        cmds->Body();
        cmds->End();
    }
}
void DrawCommand::Setup(Widget* base) {
    basePtr = base;
}
