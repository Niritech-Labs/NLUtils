#include "../../../include/NLUtils/NLGToolkit/Widgets.hpp"

using namespace NLGT::Widgets;
using namespace NLGT::DrawCommands;
int nl_combo(struct nk_context *ctx, std::vector<std::string>& options, int count, int selected, int options_height, struct nk_vec2 size);

void DrawCombobox::Begin() {}
void DrawCombobox::Body() {
    castedBasePtr->currentElement = nl_combo(castedBasePtr->nctx, castedBasePtr->options, castedBasePtr->options.size(),castedBasePtr->currentElement,castedBasePtr->optionsHeight,nk_vec2(castedBasePtr->size.x,castedBasePtr->size.y));
}
void DrawCombobox::End() {}
void DrawCombobox::Setup(Widget* base) {
    castedBasePtr = dynamic_cast<Combobox*>(base);
    basePtr = base;
}

WIDGET_SETUP_DEFAULT(Combobox)
WIDGET_GET_DEFAULT(Combobox)

struct nk_vec2 getPadding(const struct nk_style *style, enum nk_panel_type type) {
    switch (type) {
        default:
        case NK_PANEL_WINDOW: return style->window.padding;
        case NK_PANEL_GROUP: return style->window.group_padding;
        case NK_PANEL_POPUP: return style->window.popup_padding;
        case NK_PANEL_CONTEXTUAL: return style->window.contextual_padding;
        case NK_PANEL_COMBO: return style->window.combo_padding;
        case NK_PANEL_MENU: return style->window.menu_padding;
        case NK_PANEL_TOOLTIP: return style->window.menu_padding;
    }
}


int nl_combo(struct nk_context *ctx, std::vector<std::string>& options, int count, int selected, int options_height, struct nk_vec2 size) {
    int i = 0;
    int max_height;
    struct nk_vec2 item_spacing;
    struct nk_vec2 window_padding;

    if (!ctx || options.empty() ||!count) return selected;

    item_spacing = ctx->style.window.spacing;
    window_padding = getPadding(&ctx->style, ctx->current->layout->type);
    max_height = count * options_height + count * (int)item_spacing.y;
    max_height += (int)item_spacing.y * 2 + (int)window_padding.y * 2;
    size.y = NK_MIN(size.y, (float)max_height);
    if (nk_combo_begin_label(ctx, options[selected].c_str(), size)) {
        nk_layout_row_dynamic(ctx, (float)options_height, 1);
        for (i = 0; i < count; ++i) {
            if (nk_combo_item_label(ctx, options[i].c_str(), NK_TEXT_LEFT))
                selected = i;
        }
        nk_combo_end(ctx);
    }
    return selected;
}