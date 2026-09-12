#pragma once
#include <string>

#define forel for (auto
#define in(obj) : obj)
typedef uint64_t NLID_int;
typedef std::string NLID_string;
#define in_map(value, map) ([](auto& v, auto& m) -> bool { return m.find(v) != m.end(); })(value, map)
