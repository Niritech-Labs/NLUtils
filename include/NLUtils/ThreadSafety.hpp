#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>

class NLThreadMonitor {
private:
    std::atomic<uint16_t> counter{0};
    uint16_t lastValue{0};
    
    std::chrono::steady_clock::time_point lastCheckTime;
    std::chrono::milliseconds timeout{1000};

public:
    NLThreadMonitor() {
        lastCheckTime = std::chrono::steady_clock::now();
    }

    void Setup(std::chrono::milliseconds checkTimeout) noexcept {
        timeout = checkTimeout;
    }

    void SetAlive() noexcept {
        counter.fetch_add(1, std::memory_order_relaxed);
    }

    bool GetStatus() noexcept {
        auto now = std::chrono::steady_clock::now();
        uint16_t currentValue = counter.load(std::memory_order_relaxed);

        if (now - lastCheckTime >= timeout) {
            lastCheckTime = now;

            if (currentValue == lastValue) {
                return false; 
            }
            lastValue = currentValue;
        }
        
        return true; 
    }
};