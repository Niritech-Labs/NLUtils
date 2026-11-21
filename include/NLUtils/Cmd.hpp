#pragma once
#include <string>
#include <thread>
#include <vector>
#include <atomic>

#if defined(_WIN32) || defined(_WIN64)
    #define NL_COMMANDS_PIPE_HANDLE void*
    #define NL_COMMANDS_PROCESS_HANDLE void*
    #define NL_COMMANDS_PROCESS_ID unsigned long
#elif defined(__linux__)
    #include <sys/types.h>
    #define NL_COMMANDS_PIPE_HANDLE int
    #define NL_COMMANDS_PROCESS_HANDLE pid_t
    #define NL_COMMANDS_PROCESS_ID pid_t
#else
    #error "this OS is not supported!"
#endif

namespace NLSystem {
class CommandsSession {
public:
    CommandsSession() = default;
    ~CommandsSession();

    CommandsSession(const CommandsSession&) = delete;
    CommandsSession& operator=(const CommandsSession&) = delete;

    int Run(const std::vector<std::string>& cmdLine, std::string& out, std::string& err, bool wait = true);
    int RunWithShell(const std::string& cmdLine, std::string& out, std::string& err, bool wait = true);

    bool IsBusy();
    int Wait();

private:
    void openPipes(NL_COMMANDS_PIPE_HANDLE outHandle, NL_COMMANDS_PIPE_HANDLE errHandle, std::string& out, std::string& err);
    void waitCommand(NL_COMMANDS_PROCESS_HANDLE processHandle);
    void joinWorker();

    std::atomic<bool> running{false};
    std::thread commandHandler;
    int lastExitCode = -1;
};

}