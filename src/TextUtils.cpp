#include "../include/NLUtils/TextUtils.hpp"

namespace NLTextUtils {
    
NLTypes::FixedList<std::string> Split(std::string& text, std::string splitter, int count) {
    if (text.empty() || splitter.empty() || count == 0) {
        NLTypes::FixedList<std::string> emptyList(1);
        emptyList[0] = text;
        return emptyList;
    }

    uint64_t size = 0;
    size_t pos = 0;
    size_t lastPos = 0;
    
    while ((pos = text.find(splitter, lastPos)) != std::string::npos) {
        size++;
        lastPos = pos + splitter.length();
        if (count > 0 && size == static_cast<uint64_t>(count)) break;
    }
    
    if (count < 0 || size < static_cast<uint64_t>(count)) {
        size++;
    }

    NLTypes::FixedList<std::string> fList(size);
    pos = 0;
    lastPos = 0;
    uint64_t i = 0;

    while ((pos = text.find(splitter, lastPos)) != std::string::npos && i < size - 1) {
        fList[i++] = text.substr(lastPos, pos - lastPos);
        lastPos = pos + splitter.length();
    }
    
    if (i < size) {
        fList[i] = text.substr(lastPos);
    }

    return fList;
}
std::string Strip(const std::string& in, StripFor mode) {
    std::string str = in;
    constexpr const char* ws = " \t\n\r\f\v";
    if (mode == ALL || mode == RIGHT) str.erase(str.find_last_not_of(ws) + 1); 
    if (mode == ALL || mode == LEFT) str.erase(0, str.find_first_not_of(ws)); 
    return str;
} 
void StripIn(std::string& str, StripFor mode) {
    constexpr const char* ws = " \t\n\r\f\v";
    if (mode == ALL || mode == RIGHT) str.erase(str.find_last_not_of(ws) + 1); 
    if (mode == ALL || mode == LEFT) str.erase(0, str.find_first_not_of(ws)); 
} 
void ReplaceIn(std::string& str, std::string find, std::string replace, int count) {
    if (find.empty()) return; 
    size_t pos = str.find(find);

    while (pos != std::string::npos && count != 0) {
        str.replace(pos, find.length(), replace);
        
        pos = str.find(find, pos + replace.length());
        
        if (count > 0) {
            count--;
        }
    }
}
std::string Replace(const std::string& in, std::string find, std::string replace, int count) {
    std::string str = in;
    ReplaceIn(str, find, replace, count);
    return str;
}













}