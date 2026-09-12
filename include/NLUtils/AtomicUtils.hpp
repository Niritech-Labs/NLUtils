#pragma once
#include <atomic>
#include <cstdint>
#include <any>
#include <string_view>
#include <string>
#include <cstring>
#include <cstddef> 
#include <any>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <source_location>
#include "Logger.hpp"

enum AccessModifier {
    R_,
    _W,
    RW,
};

template <typename LO>
class NLAtomic {
public:
    NLAtomic(int MaxWTicketsDifference = 8,int MaxRUnableUpdate = 8) { 
        SetBackbuf(false);
        SetFrontbuf(true);
        this->MaxRUnableUpdate = MaxRUnableUpdate;
        this->MaxWTicketsDifference = MaxWTicketsDifference;
    }

    NLAtomic(LO value,int MaxWTicketsDifference = 8,int MaxRUnableUpdate = 8) {
        SetBackbuf(false);
        SetFrontbuf(true);
        this->MaxWTicketsDifference = MaxWTicketsDifference;
        this->MaxRUnableUpdate = MaxRUnableUpdate;
        falseBuf0 = value;
        CopyTo(true);
    }

    LO* Get(AccessModifier am) {
        if (am == AccessModifier::R_) {
            return getR_();
        }
        else if (am == AccessModifier::RW) {
            return getRW();
        }
        else {
            return nullptr;
        }
    }
    void Release(AccessModifier am) {
        if (am == AccessModifier::R_) {
            R_Close();
        }
        else if (am == AccessModifier::RW) {
            W_Close();
        }
        else {
            
        }
    }

private:
    LO* getR_() {
        R_Order();
        if (BackbufModified.load(std::memory_order_acquire) && W_GetOpenTickets() > 0) {
            UnableRUpdateCounter.fetch_add(1,std::memory_order_release);
        }

        if (R_OrderedInWB.load(std::memory_order_acquire) && W_GetState(R_OrderedInW) && BackbufModified.load(std::memory_order_acquire)) {
            R_SetLockState(true);
            while (R_GetOpenTickets() != 1) {}
            UnableRUpdateCounter.store(0,std::memory_order_release);
            bool back = GetCurrentBackbuf();
            SetFrontbuf(back);
            SetBackbuf(!back);
            R_OrderedInWB.store(false,std::memory_order_release);
            BackbufModified.store(false,std::memory_order_release);
            W_Close();
            R_SetLockState(false);
        }

        if ((W_GetOpenTickets() == 0) && BackbufModified.load(std::memory_order_acquire)) {
            W_SetLockState(true);
            R_SetLockState(true);
            while (R_GetOpenTickets() != 1) {}
            while (W_GetOpenTickets() != 0) {}
            UnableRUpdateCounter.store(0,std::memory_order_release);
            bool back = GetCurrentBackbuf();
            SetFrontbuf(back);
            SetBackbuf(!back);
            BackbufModified.store(false,std::memory_order_release);
            W_SetLockState(false);
            R_SetLockState(false);
        }

        if (((W_GetOpenTickets() >= MaxWTicketsDifference) || (UnableRUpdateCounter.load(std::memory_order_acquire) >= MaxRUnableUpdate)) && !R_OrderedInWB.load(std::memory_order_acquire) && BackbufModified.load(std::memory_order_acquire)) {
            R_SetLockState(true);
            while (R_GetOpenTickets() != 1) {}
            if (!R_OrderedInWB.load(std::memory_order_acquire)) {
                R_OrderedInW = W_Order(false);
                R_OrderedInWB.store(true,std::memory_order_release);
            }
            R_SetLockState(false);
        }

        return frontPtr;
    }
    LO* getRW() {
        W_Order(true);  
        if (!BackbufModified.load(std::memory_order_acquire)) {
            CopyTo(GetCurrentBackbuf());
        }
        BackbufModified.store(true,std::memory_order_release);
        return backPtr;
    }

    alignas(64) std::atomic<uint64_t> UnableRUpdateCounter = 0;
    int MaxWTicketsDifference = 8;
    int MaxRUnableUpdate = 8;

    void SetBackbuf(bool buf);
    void SetFrontbuf(bool buf);
    LO* backPtr;
    LO* frontPtr;
    
    bool GetCurrentBackbuf();
    void CopyTo(bool buf);
    alignas(64) std::atomic<bool> CurrentBackbuf = false;
    alignas(64) std::atomic<bool> BackbufModified = false;
    LO trueBuf1;
    LO falseBuf0;

    uint64_t R_OrderedInW = 0;
    alignas(64) std::atomic<bool> R_OrderedInWB = false;
    
    void R_SetLockState(bool lock);
    uint64_t R_Order();
    uint64_t R_GetOpenTickets();
    void R_Close();
    alignas(64) std::atomic<bool> R_Lock = false;
    alignas(64) std::atomic<uint64_t> R_CurrentTicket = 0;
    alignas(64) std::atomic<uint64_t> R_OrderedTickets = 0;
    
    uint64_t W_Order(bool wait);
    uint64_t W_GetOpenTickets();
    bool W_GetState(uint64_t ticket);
    void W_Close();
    void W_SetLockState(bool lock);
    alignas(64) std::atomic<bool> W_Lock = false;
    alignas(64) std::atomic<uint64_t> W_CurrentTicket = 0;
    alignas(64) std::atomic<uint64_t> W_OrderedTickets = 0;
}; 


template <typename LO>
void NLAtomic<LO>::W_SetLockState(bool lock) {
    W_Lock.store(lock,std::memory_order_release);
}
template <typename LO>
void NLAtomic<LO>::R_SetLockState(bool lock) {
    R_Lock.store(lock,std::memory_order_release);
}

template <typename LO>
void NLAtomic<LO>::SetBackbuf(bool buf) {
    CurrentBackbuf.store(buf,std::memory_order_release);
	if (buf) backPtr = &trueBuf1;
	else backPtr = &falseBuf0;
}
template <typename LO>
void NLAtomic<LO>::SetFrontbuf(bool buf) {
	if (buf) frontPtr = &trueBuf1;
	else frontPtr = &falseBuf0;
}
    
template <typename LO>
void NLAtomic<LO>::CopyTo(bool buf) { 
	if (buf) trueBuf1 = falseBuf0;
	else falseBuf0 = trueBuf1;
}

template <typename LO>
bool NLAtomic<LO>::GetCurrentBackbuf() {
    return CurrentBackbuf.load(std::memory_order_acquire);
}

template <typename LO>
uint64_t NLAtomic<LO>::R_Order() {
    while (R_Lock.load(std::memory_order_acquire)) {}
    return R_OrderedTickets.fetch_add(1, std::memory_order_acquire);
}
template <typename LO>
uint64_t NLAtomic<LO>::R_GetOpenTickets() {
    uint64_t ordered = R_OrderedTickets.load(std::memory_order_acquire);
    uint64_t current = R_CurrentTicket.load(std::memory_order_acquire);
    return ordered - current;
}
template <typename LO>
void NLAtomic<LO>::R_Close() {
    R_CurrentTicket.fetch_add(1, std::memory_order_release);
}

template <typename LO>
uint64_t NLAtomic<LO>::W_Order(bool wait) {
    while (W_Lock.load(std::memory_order_acquire)) {}
	if (wait) {
		uint64_t ot = W_OrderedTickets.fetch_add(1, std::memory_order_acquire);
		while (!W_GetState(ot)) {}
		return ot;
	}
	else {
	    return W_OrderedTickets.fetch_add(1, std::memory_order_acquire);
	}
}
template <typename LO>
uint64_t NLAtomic<LO>::W_GetOpenTickets() {
    uint64_t ordered = W_OrderedTickets.load(std::memory_order_acquire);
    uint64_t current = W_CurrentTicket.load(std::memory_order_acquire);
    return ordered - current;
}
template <typename LO>
bool NLAtomic<LO>::W_GetState(uint64_t ticket) {
    return ticket == W_CurrentTicket.load(std::memory_order_acquire);
}
template <typename LO>
void NLAtomic<LO>::W_Close() {
    W_CurrentTicket.fetch_add(1, std::memory_order_release);
    
}






#define GetFieldCasted(Type,Search) getFieldCasted<Type>(Search,#Type)
#define GetCasted(Type) getCasted<Type>(#Type)

class NLOpaqueObject {
public:
    NLLogger* Logger = nullptr;
    int size;
    char* fields;
    void* obj;
    void* FDelete;
    void* FCopy;

    void Delete() {
        ((void(*)(void*))FDelete)((void*)this);
    }

    void AddLogger(NLLogger* logger) {
        Logger = logger;
    }

    NLOpaqueObject* Copy() {
        void* data = ((void*(*)(void*))FCopy)((void*)this);
        return reinterpret_cast<NLOpaqueObject*>(data);
    }

    template <typename TRC>
    TRC* getFieldCasted(std::string search,std::string type) { 
        if (!fields || !obj) return nullptr;

        std::string sFields(fields);
        
        std::string pattern = search + "~";
        size_t foundPos = sFields.find(pattern);
        
        if (foundPos == std::string::npos) {
            pattern = ";" + search + "~";
            foundPos = sFields.find(pattern);
            if (foundPos == std::string::npos) return nullptr;
            foundPos += 1; 
        }

        std::string sub = sFields.substr(foundPos);

        size_t firstTilde = sub.find('~');
        size_t semicolon = sub.find(';');  
        if (firstTilde == std::string::npos || semicolon == std::string::npos) return nullptr;
        size_t lastTilde = sub.find_last_of('~', semicolon); 
        if (lastTilde == std::string::npos || lastTilde == firstTilde) return nullptr;
        std::string parsedType = sub.substr(firstTilde + 1, lastTilde - (firstTilde + 1));


        if (parsedType != type) {
            if (Logger) Logger->Error("Field " + type + " not found in " + GetName(), false);
            return nullptr;
        }
        std::string offsetStr = sub.substr(lastTilde + 1, semicolon - (lastTilde + 1));
        int ByteOffset = std::stoi(offsetStr);
        char* fieldAddr = reinterpret_cast<char*>(obj) + ByteOffset;
        return reinterpret_cast<TRC*>(fieldAddr);
    }

    void* GetRawPtr() {
        return obj;
    }

    std::string GetName() {
        if (!fields) return "";
        std::string_view vFields(fields);
        return std::string(vFields.substr(0, vFields.find(';')));
    }

    template <typename TSC>
    TSC* getCasted(std::string type) {
        if (GetName() == type){
            return static_cast<TSC*>(obj);
        }
        else {
            if (Logger) Logger->Error("Cast fault",false);
            return nullptr;
        }
    }
};



template <>
class NLAtomic<NLOpaqueObject> {
public:
    NLAtomic(NLOpaqueObject* value, int MaxWTicketsDifference = 8,int MaxRUnableUpdate = 8) { 
        falseBuf0 = value;
        trueBuf1 = falseBuf0->Copy();
        SetBackbuf(false);
        SetFrontbuf(true);
        this->MaxRUnableUpdate = MaxRUnableUpdate;
        this->MaxWTicketsDifference = MaxWTicketsDifference;
    }

    NLAtomic(const NLAtomic&) = delete;
    NLAtomic& operator=(const NLAtomic&) = delete;
    ~NLAtomic() {
        if (trueBuf1) {
            trueBuf1->Delete();
        }
        if (falseBuf0) {
            falseBuf0->Delete();
        }
    }

    NLOpaqueObject* Get(AccessModifier am) {
        if (am == AccessModifier::R_) {
            return *getR_();
        }
        else if (am == AccessModifier::RW) {
            return *getRW();
        }
        else {
            return nullptr;
        }
    }
    void Release(AccessModifier am) {
        if (am == AccessModifier::R_) {
            R_Close();
        }
        else if (am == AccessModifier::RW) {
            W_Close();
        }
        else {
            
        }
    }

private:
    NLOpaqueObject** getR_() {
        R_Order();
        if (BackbufModified.load(std::memory_order_acquire) && W_GetOpenTickets() > 0) {
            UnableRUpdateCounter.fetch_add(1,std::memory_order_release);
        }

        if (R_OrderedInWB.load(std::memory_order_acquire) && W_GetState(R_OrderedInW) && BackbufModified.load(std::memory_order_acquire)) {
            R_SetLockState(true);
            while (R_GetOpenTickets() != 1) {}
            UnableRUpdateCounter.store(0,std::memory_order_release);
            bool back = GetCurrentBackbuf();
            SetFrontbuf(back);
            SetBackbuf(!back);
            R_OrderedInWB.store(false,std::memory_order_release);
            BackbufModified.store(false,std::memory_order_release);
            W_Close();
            R_SetLockState(false);
        }

        if ((W_GetOpenTickets() == 0) && BackbufModified.load(std::memory_order_acquire)) {
            W_SetLockState(true);
            R_SetLockState(true);
            while (R_GetOpenTickets() != 1) {}
            while (W_GetOpenTickets() != 0) {}
            UnableRUpdateCounter.store(0,std::memory_order_release);
            bool back = GetCurrentBackbuf();
            SetFrontbuf(back);
            SetBackbuf(!back);
            BackbufModified.store(false,std::memory_order_release);
            W_SetLockState(false);
            R_SetLockState(false);
        }

        if (((W_GetOpenTickets() >= MaxWTicketsDifference) || (UnableRUpdateCounter.load(std::memory_order_acquire) >= MaxRUnableUpdate)) && !R_OrderedInWB.load(std::memory_order_acquire) && BackbufModified.load(std::memory_order_acquire)) {
            R_SetLockState(true);
            while (R_GetOpenTickets() != 1) {}
            if (!R_OrderedInWB.load(std::memory_order_acquire)) {
                R_OrderedInW = W_Order(false);
                R_OrderedInWB.store(true,std::memory_order_release);
            }
            R_SetLockState(false);
        }

        return frontPtr;
    }
    NLOpaqueObject** getRW() {
        W_Order(true);  
        if (!BackbufModified.load(std::memory_order_acquire)) {
            CopyTo(GetCurrentBackbuf());
        }
        BackbufModified.store(true,std::memory_order_release);
        return backPtr;
    }

    alignas(64) std::atomic<uint64_t> UnableRUpdateCounter = 0;
    int MaxWTicketsDifference = 8;
    int MaxRUnableUpdate = 8;

    void SetBackbuf(bool buf);
    void SetFrontbuf(bool buf);
    NLOpaqueObject** backPtr;
    NLOpaqueObject** frontPtr;
    
    bool GetCurrentBackbuf();
    void CopyTo(bool buf);
    alignas(64) std::atomic<bool> CurrentBackbuf = false;
    alignas(64) std::atomic<bool> BackbufModified = false;
    NLOpaqueObject* trueBuf1;
    NLOpaqueObject* falseBuf0;

    uint64_t R_OrderedInW = 0;
    alignas(64) std::atomic<bool> R_OrderedInWB = false;
    
    void R_SetLockState(bool lock);
    uint64_t R_Order();
    uint64_t R_GetOpenTickets();
    void R_Close();
    alignas(64) std::atomic<bool> R_Lock = false;
    alignas(64) std::atomic<uint64_t> R_CurrentTicket = 0;
    alignas(64) std::atomic<uint64_t> R_OrderedTickets = 0;
    
    uint64_t W_Order(bool wait);
    uint64_t W_GetOpenTickets();
    bool W_GetState(uint64_t ticket);
    void W_Close();
    void W_SetLockState(bool lock);
    alignas(64) std::atomic<bool> W_Lock = false;
    alignas(64) std::atomic<uint64_t> W_CurrentTicket = 0;
    alignas(64) std::atomic<uint64_t> W_OrderedTickets = 0;
}; 

void NLAtomic<NLOpaqueObject>::SetBackbuf(bool buf) {
    CurrentBackbuf.store(buf,std::memory_order_release);
	if (buf) backPtr = &trueBuf1;
	else backPtr = &falseBuf0;
}
void NLAtomic<NLOpaqueObject>::SetFrontbuf(bool buf) {
	if (buf) frontPtr = &trueBuf1;
	else frontPtr = &falseBuf0;
}
void NLAtomic<NLOpaqueObject>::CopyTo(bool buf) { 
	if (buf) {
        trueBuf1->Delete();
        trueBuf1 = nullptr;
        trueBuf1 = falseBuf0->Copy();
    }
	else {
        falseBuf0->Delete();
        falseBuf0 = nullptr;
        falseBuf0 = trueBuf1->Copy();
    }
}
bool NLAtomic<NLOpaqueObject>::GetCurrentBackbuf() {
    return CurrentBackbuf.load(std::memory_order_acquire);
}
uint64_t NLAtomic<NLOpaqueObject>::R_Order() {
    while (R_Lock.load(std::memory_order_acquire)) {}
    return R_OrderedTickets.fetch_add(1, std::memory_order_acquire);
}
uint64_t NLAtomic<NLOpaqueObject>::R_GetOpenTickets() {
    uint64_t ordered = R_OrderedTickets.load(std::memory_order_acquire);
    uint64_t current = R_CurrentTicket.load(std::memory_order_acquire);
    return ordered - current;
}
void NLAtomic<NLOpaqueObject>::R_Close() {
    R_CurrentTicket.fetch_add(1, std::memory_order_release);
}
uint64_t NLAtomic<NLOpaqueObject>::W_Order(bool wait) {
    while (W_Lock.load(std::memory_order_acquire)) {}
	if (wait) {
		uint64_t ot = W_OrderedTickets.fetch_add(1, std::memory_order_acquire);
		while (!W_GetState(ot)) {}
		return ot;
	}
	else {
	    return W_OrderedTickets.fetch_add(1, std::memory_order_acquire);
	}
}
uint64_t NLAtomic<NLOpaqueObject>::W_GetOpenTickets() {
    uint64_t ordered = W_OrderedTickets.load(std::memory_order_acquire);
    uint64_t current = W_CurrentTicket.load(std::memory_order_acquire);
    return ordered - current;
}
bool NLAtomic<NLOpaqueObject>::W_GetState(uint64_t ticket) {
    return ticket == W_CurrentTicket.load(std::memory_order_acquire);
}
void NLAtomic<NLOpaqueObject>::W_Close() {
    W_CurrentTicket.fetch_add(1, std::memory_order_release);
    
}


#define NLAtomicOpaque NLAtomic<NLOpaqueObject>




