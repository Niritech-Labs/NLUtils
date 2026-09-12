#include "../include/NLUtils/Timer.hpp"

bool NLTimer::GetState() {
    if (!active) return false;
    auto currentTime = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - sourceTime).count();

    if (elapsed < durationMs) {return true;} 
    else {
        active = false; 
        return false;
    }
}
bool NLTimer::IsActive() {
    return active;
}
void NLTimer::SetOFF() {
    active = false;
    durationMs = 0;
}
void NLTimer::SetON(uint32_t deltaTimeMs) {
    sourceTime = std::chrono::steady_clock::now();
    durationMs = deltaTimeMs;  
    active = true;             
}