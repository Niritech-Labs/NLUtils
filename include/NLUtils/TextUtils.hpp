#pragma once
#include "Types.hpp"
#include "Extra/Lang.hpp"
#include <string>

namespace NLTextUtils {

NLTypes::FixedList<std::string> Split(std::string& text, std::string splitter, int count = -1);

enum StripFor {
    ALL,
    LEFT,
    RIGHT
};
std::string Strip(const std::string& in, StripFor mode = ALL);
void StripIn(std::string& str, StripFor mode = ALL);
void ReplaceIn(std::string& str, std::string find, std::string replace, int count = -1);
std::string Replace(const std::string& in, std::string find, std::string replace, int count = -1);


}