#include "../include/NLUtils/TextUtils.hpp"

namespace NLTextUtils {
NLTypes::FixedList<std::string> Split(std::string& text, std::string& splitter, int count) {
    if (text.empty() || splitter.empty() || count == 0) {
        NLTypes::FixedList<std::string> emptyList(1);
        emptyList[0] = text;
        return emptyList;
    }

    uint size = 0;
    size_t pos = 0;
    size_t lastPos = 0;
    
    while ((pos = text.find(splitter, lastPos)) != std::string::npos) {
        size++;
        lastPos = pos + splitter.length();
        if (count > 0 && size == static_cast<uint>(count)) break;
    }
    
    if (count < 0 || size < static_cast<uint>(count)) {
        size++;
    }

    NLTypes::FixedList<std::string> fList(size);
    pos = 0;
    lastPos = 0;
    uint i = 0;

    while ((pos = text.find(splitter, lastPos)) != std::string::npos && i < size - 1) {
        fList[i++] = text.substr(lastPos, pos - lastPos);
        lastPos = pos + splitter.length();
    }
    
    if (i < size) {
        fList[i] = text.substr(lastPos);
    }

    return fList;
}
NLTypes::FixedList<std::string> Split(std::string& text, const char* splitter_c, int count) {
    std::string splitter = splitter_c;
    if (text.empty() || splitter.empty() || count == 0) {
        NLTypes::FixedList<std::string> emptyList(1);
        emptyList[0] = text;
        return emptyList;
    }

    uint size = 0;
    size_t pos = 0;
    size_t lastPos = 0;
    
    while ((pos = text.find(splitter, lastPos)) != std::string::npos) {
        size++;
        lastPos = pos + splitter.length();
        if (count > 0 && size == static_cast<uint>(count)) break;
    }
    
    if (count < 0 || size < static_cast<uint>(count)) {
        size++;
    }

    NLTypes::FixedList<std::string> fList(size);
    pos = 0;
    lastPos = 0;
    uint i = 0;

    while ((pos = text.find(splitter, lastPos)) != std::string::npos && i < size - 1) {
        fList[i++] = text.substr(lastPos, pos - lastPos);
        lastPos = pos + splitter.length();
    }
    
    if (i < size) {
        fList[i] = text.substr(lastPos);
    }

    return fList;
}
NLTypes::FixedList<std::string> Split(const char* text_c, std::string& splitter, int count) {
    std::string text = text_c;
    if (text.empty() || splitter.empty() || count == 0) {
        NLTypes::FixedList<std::string> emptyList(1);
        emptyList[0] = text;
        return emptyList;
    }

    uint size = 0;
    size_t pos = 0;
    size_t lastPos = 0;
    
    while ((pos = text.find(splitter, lastPos)) != std::string::npos) {
        size++;
        lastPos = pos + splitter.length();
        if (count > 0 && size == static_cast<uint>(count)) break;
    }
    
    if (count < 0 || size < static_cast<uint>(count)) {
        size++;
    }

    NLTypes::FixedList<std::string> fList(size);
    pos = 0;
    lastPos = 0;
    uint i = 0;

    while ((pos = text.find(splitter, lastPos)) != std::string::npos && i < size - 1) {
        fList[i++] = text.substr(lastPos, pos - lastPos);
        lastPos = pos + splitter.length();
    }
    
    if (i < size) {
        fList[i] = text.substr(lastPos);
    }

    return fList;
}
NLTypes::FixedList<std::string> Split(const char* text_c, const char* splitter_c, int count) {
    std::string splitter = splitter_c;
    std::string text = text_c;
    if (text.empty() || splitter.empty() || count == 0) {
        NLTypes::FixedList<std::string> emptyList(1);
        emptyList[0] = text;
        return emptyList;
    }

    uint size = 0;
    size_t pos = 0;
    size_t lastPos = 0;
    
    while ((pos = text.find(splitter, lastPos)) != std::string::npos) {
        size++;
        lastPos = pos + splitter.length();
        if (count > 0 && size == static_cast<uint>(count)) break;
    }
    
    if (count < 0 || size < static_cast<uint>(count)) {
        size++;
    }

    NLTypes::FixedList<std::string> fList(size);
    pos = 0;
    lastPos = 0;
    uint i = 0;

    while ((pos = text.find(splitter, lastPos)) != std::string::npos && i < size - 1) {
        fList[i++] = text.substr(lastPos, pos - lastPos);
        lastPos = pos + splitter.length();
    }
    
    if (i < size) {
        fList[i] = text.substr(lastPos);
    }

    return fList;
}

}