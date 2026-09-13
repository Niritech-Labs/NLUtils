# Copyright (C) 2026 Niritech Labs
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

import re
from NLUtils.Logger import NLLogger
EnumsRE = r'enum\s+(\w+)\s*\{([^}]*)\};'

def Enums(datainput, Logger):
    match = re.search(r'enum\s+(\w+)\s*\{(.*?)\}', datainput, re.DOTALL)
    if not match: return (False, "")
    
    name, body = match.groups()
    

    is_flag = ', nb::is_flag()' if '=' in body else ''

    defenum = f'    nb::enum_<enum {name}>(m, "{name}"{is_flag})\n'

    keys = re.findall(r'(?:^|,)\s*(NK_\w+)(?:\s*=[^,]*)?', body)

    for key in keys:
        defenum += f'        .value("{key}", {key})\n'

    defenum += '        .export_values();\n'
    return (True, defenum)



#registredStructs = []
PredefStructsRE = r"^struct\s+\w+\s*\{.*?\};"
def PredefStructs(dataInput,Logger):
    name = re.search(r'struct\s+(\w+)', dataInput).group(1)

    structpredef = f'    auto {name+'_var'} = nb::class_<struct {name}>(m, "{name}");\n        {name+'_var'}.def(nb::init<>());\n'
    return (True,structpredef)

StructsRE = r"^struct\s+\w+\s*\{.*?\};"
def Structs(inputData,Logger:NLLogger):
    name = re.search(r'struct\s+(\w+)', inputData).group(1)
    content = re.search(r'\{([\s\S]*)\}', inputData).group(1)
 
    skeleton = ''
    lines = [line.strip() for line in content.split(';') if line.strip()]
    
    for line in lines: 
        if '=' in line:
            pass
        elif '*' in line:
            defname = line.split()[-1].replace('*','')
        elif 'void' in line:
            pass
        elif 'nk_allocator' in line:
            pass
        elif 'nk_text_width_f' in line:
            pass
        elif 'nk_plugin_' in line:
            pass
        elif 'nk_query_font_glyph_f' in line:
            pass
        elif '[' in line:
            #defname = re.search(r'(\w+)\s*\[', line).group(1)
            #skeleton += f'    BIND_NK_ARRAY({name+'_var'}, {name}, {defname}); \n'
            pass
        elif ',' in line:
            defnames = line.split()[-1].split(',')
            for nameD in defnames:
                skeleton += f'    {name+'_var'}.def_rw("{nameD}", &{name}::{nameD});\n'
        else:
            defname = line.split()[-1]
            skeleton += f'    {name+'_var'}.def_rw("{defname}", &{name}::{defname});\n'
    
    return (True,skeleton)

    

MethodsRE = r"extern\s+.+?\s+\w+\s*\(.*?\);"
def Methods(inputData,Logger):
    match = re.search(r"extern\s+(?P<ret>.+?)\s+(?P<name>\w+)\((?P<first_arg>[^,)]+)(?P<others>.*?)\);", inputData)
    if not match: 
        filter = False
    else:
        filter = True
    
    ret = match.group('ret')
    name = match.group('name')

    if 'void*' in ret:
        ppSkeleton = f'    m.def("m{name}", as_capsule(&{name}),"{inputData}");\n'
    elif "*" in ret:
        ppSkeleton = f'    m.def("m{name}", &{name}, nb::rv_policy::reference,"{inputData}");\n'
    else:
        ppSkeleton = f'    m.def("m{name}", &{name},"{inputData}");\n'

    return (filter,ppSkeleton)



