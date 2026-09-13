#pragma once
#include <memory>
#include <initializer_list>



namespace NLTypes {

struct Version {
    uint major = 0;
    uint minor = 0;
    uint patch = 0;
    bool operator==(Version other) const {
        return major == other.major && minor == other.minor && patch == other.patch;
    }

    std::string ToString() {
        return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
    }

    void FromString(std::string s) {
        Version v;
        size_t p1 = s.find('.');
        size_t p2 = s.find('.', p1 + 1);
        if (p1 == std::string::npos || p2 == std::string::npos) {
            major = v.major;
            minor = v.minor;
            patch = v.patch;
            return;
        }
        major = (uint)std::stoul(s.substr(0, p1));
        minor = (uint)std::stoul(s.substr(p1 + 1, p2 - p1 - 1));
        patch = (uint)std::stoul(s.substr(p2 + 1));
    }
};

enum Condition {
    LESS,
    LESS_EQUAL,
    EQUAL,
    GREATER_EQUAL,
    GREATER,
    NOT_EQUAL,
};

class URL {
public:
    URL() = default;
    URL(std::string raw) : value(raw) {}

    URL& operator=(std::string raw) {
        value = raw;
        return *this;
    }

    operator std::string() { return value; }

    bool IsValid() {
        size_t sep = value.find("://");
        return sep != std::string::npos && sep > 0 && sep + 3 < value.size();
    }

private:
    std::string value;
};


template <typename valueType>
class ConstantFixedList {
private:
    std::unique_ptr<valueType[]> data; 
    size_t elementCount;               

public:
    ConstantFixedList(std::initializer_list<valueType> list) {
        elementCount = list.size();
        data = std::make_unique<valueType[]>(list.size());
        valueType* clPtr = const_cast<valueType*>(data.get());
        std::copy(list.begin(), list.end(), clPtr);
    }

    valueType operator[](size_t index) { 
        return data[index]; 
    }
    size_t size() { 
        return elementCount; 
    }

    ConstantFixedList(const ConstantFixedList&) = delete;
    ConstantFixedList& operator=(const ConstantFixedList&) = delete;
};

template <typename valueType>
class FixedList {
private:
    std::unique_ptr<valueType[]> data; 
    size_t elementCount = 0;           

public:
    FixedList(std::initializer_list<valueType> list) {
        elementCount = list.size();
        data = std::make_unique<valueType[]>(list.size());
        std::copy(list.begin(), list.end(), data.get());
    }

    FixedList(size_t size) {
        elementCount = size;
        data = std::make_unique<valueType[]>(size);
    }

    FixedList(const FixedList& other) {
        elementCount = other.elementCount;
        data = std::make_unique<valueType[]>(other.elementCount);
        std::copy(other.data.get(), other.data.get() + other.elementCount, data.get());
    }

    FixedList& operator=(const FixedList& other) {
        if (this != &other && elementCount == other.elementCount) {
            std::copy(other.data.get(), other.data.get() + other.elementCount, data.get());
        }
        return *this;
    }

    FixedList& operator=(std::initializer_list<valueType> list) {
        if (elementCount == list.size()) {
            std::copy(list.begin(), list.end(), data.get());
        }
        return *this;
    }

    valueType& operator[](size_t index) { 
        return data[index]; 
    }
    
    size_t size() { 
        return elementCount; 
    }
};
}

namespace std {
    template <>
    struct hash<NLTypes::Version> {
        size_t operator()(const NLTypes::Version& v) const {
            size_t h = hash<uint>()(v.major);
            h ^= hash<uint>()(v.minor) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= hash<uint>()(v.patch) + 0x9e3779b9 + (h << 6) + (h >> 2);
            return h;
        }
    };
}