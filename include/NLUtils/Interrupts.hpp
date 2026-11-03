#pragma once
#include <vector>
#include <unordered_map>
#include "AtomicUtils.hpp"
#include "Logger.hpp"
#include <type_traits>
#include <cstring>

using InterruptFunc = void(*)(); 

#define INTERRUPT_ARGS(...) (__VA_ARGS__)
#define ARG_TO_CSTR(a) #a

#define ADD_INTERRUPT_INTERFACE(manager,interfaceSlot, argsType) {                  \
    std::string I_ARGS_TYPE_ = ARG_TO_CSTR(argsType);                               \
    if constexpr (std::is_pointer_v<std::remove_reference_t<decltype(manager)>>) {  \
        manager->Addinterface(interfaceSlot,I_ARGS_TYPE_);                          \
    }                                                                               \
    else {                                                                          \
        manager.Addinterface(interfaceSlot,I_ARGS_TYPE_);                           \
    }                                                                               \
}

#define INTERRUPT_REQUEST(manager,interfaceSlot,method, argsType) {                 \
    std::string I_ARGS_TYPE_ = ARG_TO_CSTR(argsType);                               \
    if constexpr (std::is_pointer_v<std::remove_reference_t<decltype(manager)>>) {  \
        manager->SetRequest(interfaceSlot,method,I_ARGS_TYPE_);                     \
    }                                                                               \
    else {                                                                          \
        manager.SetRequest(interfaceSlot,method,I_ARGS_TYPE_);                      \
    }                                                                               \
}

#define INTERRUPT_HANDLE(manager,interfaceSlot, argsType, ...) {                    \
    std::string I_ARGS_TYPE_ = ARG_TO_CSTR(argsType);                               \
    InterruptFunc request;                                                          \
    if constexpr (std::is_pointer_v<std::remove_reference_t<decltype(manager)>>) {  \
        request = manager->GetRequest(interfaceSlot,I_ARGS_TYPE_);                  \
    }                                                                               \
    else {                                                                          \
        request = manager.GetRequest(interfaceSlot,I_ARGS_TYPE_);                   \
    }                                                                               \
    if (request) {                                                                  \
        (void(*)argsType)(__VA_ARGS__);                                             \
    }                                                                               \
}


struct InterruptInterface {
    const char* args;
    std::vector<InterruptFunc> fifo;
};


class NLInterruptManager {
public:
    NL_LOGGER_METHODS
    NLInterruptManager() {
        Logger.Setup(true,"NLInterruptManager");
    }
    void SetRequest(uint16_t interfaceSlot, InterruptFunc interrupt,const char* args) {
        auto it = interrupts.find(interfaceSlot);
        const char* iargs;
        if (it != interrupts.end()) {
            auto interface = it->second.Get(AccessModifier::RW);
                iargs = interface->args;
                if (std::strcmp(iargs, args) == 0) {
                    interface->fifo.push_back(interrupt);
                }
                else {
                    Logger.Error("Args error on slot: " + std::to_string(interfaceSlot),false,"Get Request");
                }
            it->second.Release(AccessModifier::RW);
        }
        else {
            Logger.Error("Unknown interface on slot: " + std::to_string(interfaceSlot),false,"Set Request");
        }
    }
    InterruptFunc GetRequest(uint16_t interfaceSlot,char* args) {
        auto it = interrupts.find(interfaceSlot);
        if (it != interrupts.end()) {

            InterruptFunc interrupt;
            const char* iargs;

            auto interface = it->second.Get(AccessModifier::RW);
                interrupt = interface->fifo.back();
                interface->fifo.pop_back();
                iargs = interface->args;
            it->second.Release(AccessModifier::RW);

            if (std::strcmp(iargs, args) == 0) {
                return interrupt;
            }
            else {
                Logger.Error("Args error on slot: " + std::to_string(interfaceSlot),false,"Get Request");
                return nullptr;
            }
            
        }
        else {
            Logger.Error("Unknown interface on slot: " + std::to_string(interfaceSlot),false,"Get Request");
        }
    }

    void Addinterface(uint16_t interfaceSlot,const char* args) {
        auto it = interrupts.find(interfaceSlot);
        if (it == interrupts.end()) {
            interrupts.try_emplace(interfaceSlot);
            auto interface = interrupts[interfaceSlot].Get(AccessModifier::RW);
                interface->args = args;
            interrupts[interfaceSlot].Release(AccessModifier::RW);
        }
        else {
            Logger.Error("Interface already added on slot: " + std::to_string(interfaceSlot),false,"Add Request");
        }
    }

private:
    NLLogger Logger;
    std::unordered_map<uint16_t,NLAtomic<InterruptInterface>> interrupts;
};