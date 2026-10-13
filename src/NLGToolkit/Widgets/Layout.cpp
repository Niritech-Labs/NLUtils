#include "../../../include/NLUtils/NLGToolkit/Widgets.hpp"
#include <map>

using namespace NLGT::Widgets;
using namespace NLGT::DrawCommands;

void DrawLayout::Begin() {
    int rs = whatDrawRequest();

    switch (castedBasePtr->layoutType) {
    case Layout::Type::SPACE:
        nk_layout_space_begin(castedBasePtr->nctx,castedBasePtr->posFormat,castedBasePtr->height,rs);
        break;
    case Layout::Type::GRID:
        break;
    case Layout::Type::ROW:
        nk_layout_row_template_begin(castedBasePtr->nctx,castedBasePtr->height);
        break;
    default:
        break;
    }
}
void DrawLayout::Body() {
    forel& id in(toDraw) {
        auto* cmds = body[id].get();
        glm::mat3x4 data = castedBasePtr->positions[id];
        switch (castedBasePtr->layoutType) {
            case Layout::Type::SPACE:
                nk_layout_space_push(castedBasePtr->nctx,nk_rect(data[0][0],data[0][1],data[1][0],data[1][1]));
                break;
            case Layout::Type::GRID:
                break;
            case Layout::Type::ROW:
                if (data[1][2] != 0) {
                    nk_layout_row_template_push_static(castedBasePtr->nctx,data[1][2]);
                }
                else {
                    nk_layout_row_template_push_dynamic(castedBasePtr->nctx);
                }
                break;
            default:
                break;
        }
        cmds->Begin();
        cmds->Body();
        cmds->End();
    }
}
void DrawLayout::End() {
    switch (castedBasePtr->layoutType) {
    case Layout::Type::SPACE:
        nk_layout_space_end(castedBasePtr->nctx);
        break;
    case Layout::Type::GRID:
        break;
    case Layout::Type::ROW:
        nk_layout_row_template_end(castedBasePtr->nctx);
        break;
    default:
        break;
    }
    toDraw.clear();
}
void DrawLayout::Setup(Widget* base) {
    castedBasePtr = dynamic_cast<Layout*>(base);
    basePtr = base;
}
int DrawLayout::whatDrawRequest() {
    std::map<int,uint64_t> rows;
    forel& [id,dcmd] in(this->body) {
        auto it = castedBasePtr->positions.find(id);
        if (it != castedBasePtr->positions.end()) {
            if (castedBasePtr->layoutType == Layout::Type::ROW && it->second[2][0] == 1) {
                if (!rows.contains((int)it->second[1][3])) rows[(int)it->second[1][3]] = id;
            }
            else if (castedBasePtr->layoutType == Layout::Type::GRID && it->second[2][1] == 1) {
                toDraw.push_back(id);
            }
            else if (castedBasePtr->layoutType == Layout::Type::SPACE && it->second[2][2] == 1) {
                toDraw.push_back(id);
            }
        }
    }
    forel [order,oid] in(rows) {
        toDraw.push_back(oid);
    }
    return this->toDraw.size();
}

void Layout::SetPosition(Widget& positionedWidget,struct nk_rect pos) {
    auto it = positions.find(positionedWidget.id);
    if (it != positions.end()) {
        glm::mat3x4& data = it->second;
        data[2][2] = 1;
        data[0][0] = pos.x;
        data[0][1] = pos.y;
        data[1][0] = pos.w;
        data[1][1] = pos.h;
    }
    else {
        glm::mat3x4 data;
        data[2][2] = 1;
        data[0][0] = pos.x;
        data[0][1] = pos.y;
        data[1][0] = pos.w;
        data[1][1] = pos.h;
        positions[positionedWidget.id] = data;
    }

}
void Layout::SetPosition(Widget& positionedWidget,int order) {
    auto it = positions.find(positionedWidget.id);
    if (it != positions.end()) {
        glm::mat3x4& data = it->second;
        data[2][0] = 1;
        data[1][2] = 0;
        data[1][3] = order;
        
    }
    else {
        glm::mat3x4 data;
        data[2][0] = 1;
        data[1][2] = 0;
        data[1][3] = order;
        positions[positionedWidget.id] = data;
    }
}
void Layout::SetPosition(Widget& positionedWidget,float width,int order) {
    auto it = positions.find(positionedWidget.id);
    if (it != positions.end()) {
        glm::mat3x4& data = it->second;
        data[2][0] = 1;
        data[1][2] = width;
        data[1][3] = order;
        
    }
    else {
        glm::mat3x4 data;
        data[2][0] = 1;
        data[1][2] = width;
        data[1][3] = order;
        positions[positionedWidget.id] = data;
    }

}
void Layout::SetPosition(Widget& positionedWidget,glm::vec2 pos) {
    auto it = positions.find(positionedWidget.id);
    if (it != positions.end()) {
        glm::mat3x4& data = it->second;
        data[2][1] = 1;
        data[0][2] = pos.x;
        data[0][3] = pos.y;
        
    }
    else {
        glm::mat3x4 data;
        data[2][1] = 1;
        data[0][2] = pos.x;
        data[0][3] = pos.y;
        positions[positionedWidget.id] = data;
    }
}

void Layout::SetPosition(uint64_t positionedWidget,struct nk_rect pos) {
    auto it = positions.find(positionedWidget);
    if (it != positions.end()) {
        glm::mat3x4& data = it->second;
        data[2][2] = 1;
        data[0][0] = pos.x;
        data[0][1] = pos.y;
        data[1][0] = pos.w;
        data[1][1] = pos.h;
    }
    else {
        glm::mat3x4 data;
        data[2][2] = 1;
        data[0][0] = pos.x;
        data[0][1] = pos.y;
        data[1][0] = pos.w;
        data[1][1] = pos.h;
        positions[positionedWidget] = data;
    }
}
void Layout::SetPosition(uint64_t positionedWidget,int order) {
    auto it = positions.find(positionedWidget);
    if (it != positions.end()) {
        glm::mat3x4& data = it->second;
        data[2][0] = 1;
        data[1][2] = 0;
        data[1][3] = order;
        
    }
    else {
        glm::mat3x4 data;
        data[2][0] = 1;
        data[1][2] = 0;
        data[1][3] = order;
        positions[positionedWidget] = data;
    }
}
void Layout::SetPosition(uint64_t positionedWidget,float width,int order) {
    auto it = positions.find(positionedWidget);
    if (it != positions.end()) {
        glm::mat3x4& data = it->second;
        data[2][0] = 1;
        data[1][2] = width;
        data[1][3] = order;
        
    }
    else {
        glm::mat3x4 data;
        data[2][0] = 1;
        data[1][2] = width;
        data[1][3] = order;
        positions[positionedWidget] = data;
    }
}
void Layout::SetPosition(uint64_t positionedWidget,glm::vec2 pos) {
    auto it = positions.find(positionedWidget);
    if (it != positions.end()) {
        glm::mat3x4& data = it->second;
        data[2][1] = 1;
        data[0][2] = pos.x;
        data[0][3] = pos.y;
        
    }
    else {
        glm::mat3x4 data;
        data[2][1] = 1;
        data[0][2] = pos.x;
        data[0][3] = pos.y;
        positions[positionedWidget] = data;
    }
}

bool Layout::UnsetPosition(Widget& positionedWidget) {
    auto it = positions.find(positionedWidget.id);
    if (it != positions.end()) {
        positions.erase(positionedWidget.id);
        return true;
    }
    else {
        return false;
    }
}
bool Layout::UnsetPosition(uint64_t positionedWidget) {
    auto it = positions.find(positionedWidget);
    if (it != positions.end()) {
        positions.erase(positionedWidget);
        return true;
    }
    else {
        return false;
    }
}
void Layout::UnsetPositionAll() {
    this->positions.clear();
}

WIDGET_SETUP_DEFAULT(Layout)
WIDGET_GET_DEFAULT(Layout)