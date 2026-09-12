#pragma once
#include <iostream>
#include <cstdint>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    #include <psapi.h>
#elif defined(__linux__)
    #include <fstream>
    #include <unistd.h>
#else
    #error "this OS is not supported!"
#endif


uint64_t GetProcessVirtualMemory() {
#if defined(_WIN32) || defined(_WIN64)
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        return static_cast<uint32_t>(pmc.PrivateUsage / 1024 / 1024);
    }
    return 0;
#elif defined(__linux__)
    std::ifstream statm("/proc/self/statm");
    if (statm.is_open()) {
        uint64_t virtual_pages = 0;
        statm >> virtual_pages;
        
        long page_size = sysconf(_SC_PAGESIZE); 
        if (page_size > 0) {
            uint64_t totalBytes = virtual_pages * static_cast<uint64_t>(page_size);
            return totalBytes / 1024 / 1024;
        }
    }
    return 0;
#endif
}


uint64_t GetTotalSystemMemory() {
#if defined(_WIN32) || defined(_WIN64)
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    
    if (GlobalMemoryStatusEx(&status)) {
        return static_cast<uint64_t>(status.ullTotalPhys / 1024 / 1024);
    }
    return 0;
#elif defined(__linux__)
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGESIZE); 
    
    if (pages > 0 && page_size > 0) {
        uint64_t totalBytes = static_cast<uint64_t>(pages) * static_cast<uint64_t>(page_size);
        return totalBytes / 1024 / 1024; 
    }
    return 0;
#endif
}