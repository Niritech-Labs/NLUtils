#pragma once
#include <string>
#include <algorithm>

#define forel for (auto
#define inside(obj) : obj)
typedef uint64_t NLID_int;
typedef std::string NLID_string;
#define nlfval auto nlvalue_nvfal =
#define inmap(map) ;([](auto& v, auto& m) -> bool { return m.find(v) != m.end(); })(nlvalue_nvfal, map)
#define instring(cstring) ;([](auto& v, std::string s) -> bool { return s.find(v) != std::string::npos; })(nlvalue_nvfal, cstring)
#define invector(vec) ;([](auto& v, auto& cont) -> bool { return std::find(cont.begin(), cont.end(), v) != cont.end(); })(nlvalue_nvfal, vec)