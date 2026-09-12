#pragma once
#include <memory>
#include <initializer_list>

namespace NLTypes {

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