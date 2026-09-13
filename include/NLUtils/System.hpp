#pragma once
#include <iostream>
#include <cstdint>
#include <filesystem>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    #include <psapi.h>
#elif defined(__linux__)
    #include <fstream>
    #include <unistd.h>
    #include <limits.h>
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




std::filesystem::path GetExecutablePath() {
#if defined(_WIN32)
    std::vector<wchar_t> buffer(MAX_PATH);
    while (true) {
        DWORD size = GetModuleFileNameW(NULL, buffer.data(), buffer.size());
        if (size == 0) return "";
        if (size < buffer.size()) {
            return std::filesystem::path(buffer.data());
        }
        buffer.resize(buffer.size() * 2);
    }
#elif defined(__linux__)
    std::string buffer(PATH_MAX, '\0');
    while (true) {
        ssize_t len = readlink("/proc/self/exe", buffer.data(), buffer.size());
        if (len == -1) return ""; 
        if (static_cast<size_t>(len) < buffer.size()) {
            buffer.resize(len); 
            return std::filesystem::path(buffer);
        }
        buffer.resize(buffer.size() * 2);
    }
#endif
    return "";
}


