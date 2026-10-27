# Copyright (C) 2026 Niritech Labs
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
from NLUtils.TextUtils import GigaRE
from handleFunctions import *
gre = GigaRE(True)
file = ''
with open('python/tmp/NkPreproc.h','r') as f:
    file = f.read()
fl = []
enums = gre.FindAndWrap(file,EnumsRE,Enums,None)
fl.append('\n'.join(enums))


methodBlacklist = ['nk_command_custom','nk_allocator','nk_strto','nk_color_hex_rgb','nk_utf','nk_edit_string','nk_combo(','nk_combobox(','__',
                   "nk_edit_buffer","nk_plot_function","nk_combo_callback","nk_combobox_callback"]
structBlacklist = ['nk_allocator','nk_command_custom','__']

structsPre = gre.FindAndWrap(file,PredefStructsRE,PredefStructs,wrapSkeleton=None,blacklist=structBlacklist,flags=re.MULTILINE | re.DOTALL)
fl.append('\n'.join(structsPre))
structs = gre.FindAndWrap(file,StructsRE,Structs,None,blacklist=structBlacklist,flags=re.MULTILINE | re.DOTALL)
fl.append('\n'.join(structs))
methods = gre.FindAndWrap(file,MethodsRE,Methods,None,blacklist=methodBlacklist,flags=0)
fl.append('\n'.join(methods))

inner = '''
//Copyright (C) 2026 Niritech Labs
//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

#define NK_IMPLEMENTATION
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_STANDARD_BOOL
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_STANDARD_VARARGS


#include <nanobind/nanobind.h>
#include <stdint.h>
#include <algorithm>           
#include <type_traits>   
#include "../nuklear/nuklear.h"
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/tuple.h>
#include <unordered_map>

namespace nb = nanobind;
using namespace nanobind::literals;

template <typename Ret, typename... Args>
auto as_capsule(Ret (*func)(Args...)) {
    return [func](Args... args) {
        return nb::capsule((void*)func(args...));
    };
}

namespace {
    thread_local std::unordered_map<const void*, nb::callable> filter_map;

    nk_bool filter_trampoline(const nk_text_edit* edit, nk_rune unicode) {
        auto it = filter_map.find(static_cast<const void*>(edit));
        if (it != filter_map.end())
            return nb::cast<bool>(it->second(edit, unicode));
        return nk_true;  
    }

    float plot_value_getter_trampoline(void* user, int index) {
        return nb::cast<float>((*static_cast<nb::callable*>(user))(user, index));
    }

    void combo_item_getter_trampoline(void* user, int index, const char** item) {
        static thread_local std::string result;
        result = nb::cast<std::string>((*static_cast<nb::callable*>(user))(user, index));
        *item = result.c_str();
    }
}

NB_MODULE(Nuklear, m) {
    m.doc() = "Niritech Labs port Nuklear to python with nanobind"; 
'''
outer = """
// NOT AUTOMATIC REALIZATION


    m.def("mnk_combobox", [](nk_context* ctx, std::vector<std::string> items, int selected, int item_height, struct nk_vec2 size) {
        std::vector<const char*> ptrs;
        for (const auto& s : items) ptrs.push_back(s.c_str());
        nk_combobox(ctx, ptrs.data(), (int)ptrs.size(), &selected, item_height, size);
        return selected; 
    });


    m.def("mnk_combo", [](nk_context* ctx, std::vector<std::string> items, int selected, int item_height, struct nk_vec2 size) {
        std::vector<const char*> ptrs;
        for (const auto& s : items) ptrs.push_back(s.c_str());
        return nk_combo(ctx, ptrs.data(), (int)ptrs.size(), selected, item_height, size);
    });


    m.def("mnk_edit_buffer", [](nk_context* ctx, nk_flags flags, nk_text_edit* edit, nb::callable filter) {
        nk_plugin_filter c_filter = nullptr;
        if (!filter.is_none()) {
            filter_map[static_cast<const void*>(edit)] = filter;   
            c_filter = filter_trampoline;
        }
        nk_flags res = nk_edit_buffer(ctx, flags, edit, c_filter);
        filter_map.erase(static_cast<const void*>(edit));           
        return res;
    }, "ctx"_a, "flags"_a, "edit"_a, "filter"_a = nb::none());

    m.def("mnk_plot_function", [](nk_context* ctx, nk_chart_type chart_type, nb::callable value_getter, int count, int offset) {
        if (value_getter.is_none()) {
            nk_plot_function(ctx, chart_type, nullptr, nullptr, count, offset);
        } else {
            nb::callable* cb_ptr = &value_getter;              
            nk_plot_function(ctx, chart_type, cb_ptr, plot_value_getter_trampoline,
                             count, offset);
        }
    }, "ctx"_a, "chart_type"_a, "value_getter"_a = nb::none(), "count"_a, "offset"_a);

    m.def("mnk_combo_callback", [](nk_context* ctx, nb::callable item_getter, void* userdata, int selected, int count, int item_height, struct nk_vec2 size) {
        if (item_getter.is_none()) return nk_combo_callback(ctx, nullptr, nullptr, selected, count, item_height, size);

        nb::callable* cb_ptr = &item_getter;

        return nk_combo_callback(ctx, combo_item_getter_trampoline, cb_ptr, selected, count, item_height, size);

    }, "ctx"_a, "item_getter"_a = nb::none(), "userdata"_a = nb::none(), "selected"_a, "count"_a, "item_height"_a, "size"_a);

    m.def("mnk_combobox_callback", [](nk_context* ctx, nb::callable item_getter, void* userdata, int* selected, int count, int item_height, struct nk_vec2 size) {
        if (item_getter.is_none())
            return nk_combobox_callback(ctx, nullptr, nullptr, selected, count, item_height, size);
        nb::callable* cb_ptr = &item_getter;
        return nk_combobox_callback(ctx, combo_item_getter_trampoline, cb_ptr, selected, count, item_height, size);
    }, "ctx"_a, "item_getter"_a = nb::none(), "userdata"_a = nb::none(), "selected"_a, "count"_a, "item_height"_a, "size"_a);

    m.def("mnk_edit_string_zero_terminated", [](nk_context* ctx, nk_flags flags, std::string buffer, int max, nb::callable filter) {
        buffer.resize(max);
        nk_plugin_filter c_filter = nullptr;
        const void* key = static_cast<const void*>(&filter);  
        if (!filter.is_none()) {
            filter_map[key] = filter;
            c_filter = filter_trampoline;
        }
        nk_flags res = nk_edit_string_zero_terminated(ctx, flags, buffer.data(),
                                                      max, c_filter);
        filter_map.erase(key);
        buffer.resize(strlen(buffer.c_str()));
        return std::make_tuple(res, buffer);
    }, "ctx"_a, "flags"_a, "buffer"_a, "max"_a, "filter"_a = nb::none());

    
    m.def("mnk_edit_string", [](nk_context* ctx, nk_flags flags, std::string buffer, int max, nb::callable filter) {
        buffer.resize(max);
        int current_len = (int)strlen(buffer.c_str());
        nk_plugin_filter c_filter = nullptr;
        const void* key = static_cast<const void*>(&filter);
        if (!filter.is_none()) {
            filter_map[key] = filter;
            c_filter = filter_trampoline;
        }
        nk_flags res = nk_edit_string(ctx, flags, buffer.data(), &current_len, max, c_filter);
        filter_map.erase(key);
        buffer.resize(current_len);
        return std::make_tuple(res, buffer);
    }, "ctx"_a, "flags"_a, "buffer"_a, "max"_a, "filter"_a = nb::none());

    nk_mouse_var.def("__getitem__", [](nk_mouse &m, size_t i) -> nk_mouse_button& {
            if (i >= NK_BUTTON_MAX) throw nb::index_error();
            return m.buttons[i];
        }, nb::rv_policy::reference_internal);
        
    nk_mouse_var.def("__len__", [](nk_mouse &) { return NK_BUTTON_MAX; });

    nb::class_<nk_style_item_data>(m, "nk_style_item_data")
        .def(nb::init<>())
        .def_rw("color", &nk_style_item_data::color)
        .def_rw("image", &nk_style_item_data::image)
        .def_rw("slice", &nk_style_item_data::slice);

}
"""


body = '\n'.join(fl)
res = inner + body + outer

with open('python/src/Nuklear.cpp','w') as f:
    f.write(res)

