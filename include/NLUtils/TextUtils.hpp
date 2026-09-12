#pragma once
#include "Types.hpp"
#include "Extra/Lang.hpp"
#include <string>

namespace NLTextUtils {

NLTypes::FixedList<std::string> Split(std::string& text, std::string& splitter, int count = -1);
NLTypes::FixedList<std::string> Split(std::string& text, const char* splitter_c, int count = -1);
NLTypes::FixedList<std::string> Split(const char* text_c, std::string& splitter, int count = -1);
NLTypes::FixedList<std::string> Split(const char* text_c, const char* splitter_c, int count = -1);

}