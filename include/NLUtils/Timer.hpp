#include <chrono>

class NLTimer {
public:
    void SetON(uint32_t deltaTimeMs);
    void SetOFF();
    bool IsActive();
    bool GetState();
private:
    std::chrono::time_point<std::chrono::steady_clock> sourceTime;
    uint32_t durationMs = 0;   
    bool active = false;       
};


