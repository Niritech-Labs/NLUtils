#include "../include/NLUtils/Cmd.hpp"
#include <cerrno>
#include <filesystem>


#if defined(_WIN32) || defined(_WIN64)
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <windows.h>
#elif defined(__linux__)
    #include <unistd.h>
    #include <sys/wait.h>
    #include <poll.h>
#endif

using namespace NLSystem;

CommandsSession::~CommandsSession() {
    joinWorker();
}

bool CommandsSession::IsBusy() {
    return running.load();
}

int CommandsSession::Wait() {
    joinWorker();
    return lastExitCode;
}

void CommandsSession::joinWorker() {
    if (commandHandler.joinable()) {
        commandHandler.join();
    }
}



#if defined(__linux__)

void CommandsSession::openPipes(NL_COMMANDS_PIPE_HANDLE outFd, NL_COMMANDS_PIPE_HANDLE errFd, std::string& out, std::string& err) {
    struct pollfd pfd[2] = { {outFd, POLLIN, 0}, {errFd, POLLIN, 0} };
    int active = 2;
    char buf[4096];

    while (active > 0) {
        if (poll(pfd, 2, -1) < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }

        for (int i = 0; i < 2; ++i) {
            if (pfd[i].fd < 0) {
                continue;
            }
            if (!(pfd[i].revents & (POLLIN | POLLHUP | POLLERR))) {
                continue;
            }

            ssize_t r = read(pfd[i].fd, buf, sizeof(buf));

            if (r > 0) {
                if (i == 0) {
                    out.append(buf, static_cast<size_t>(r));
                }
                else {
                    err.append(buf, static_cast<size_t>(r));
                }
            }
            else if (r < 0 && errno == EINTR) {
                continue;
            }
            else {
                close(pfd[i].fd);
                pfd[i].fd = -1;
                pfd[i].events = 0;
                --active;
            }
        }
    }

    if (pfd[0].fd >= 0) {
        close(pfd[0].fd);
    }
    if (pfd[1].fd >= 0) {
        close(pfd[1].fd);
    }
}

void CommandsSession::waitCommand(NL_COMMANDS_PROCESS_HANDLE pid) {
    int status = 0;
    while (waitpid(pid, &status, 0) < 0) {
        if (errno != EINTR) {
            lastExitCode = -1;
            return;
        }
    }

    if (WIFEXITED(status)) {
        lastExitCode = WEXITSTATUS(status);
    }
    else if (WIFSIGNALED(status)) {
        lastExitCode = 128 + WTERMSIG(status);
    }
    else {
        lastExitCode = -1;
    }
}

#elif defined(_WIN32) || defined(_WIN64)

void CommandsSession::openPipes(NL_COMMANDS_PIPE_HANDLE outHandle, NL_COMMANDS_PIPE_HANDLE errHandle, std::string& out, std::string& err) {
    HANDLE hOut = static_cast<HANDLE>(outHandle);
    HANDLE hErr = static_cast<HANDLE>(errHandle);

    std::thread outThread([hOut, &out]() {
        char buf[4096];
        DWORD bytesRead = 0;
        while (ReadFile(hOut, buf, sizeof(buf), &bytesRead, NULL) && bytesRead > 0) {
            out.append(buf, bytesRead);
        }
        CloseHandle(hOut);
    });

    std::thread errThread([hErr, &err]() {
        char buf[4096];
        DWORD bytesRead = 0;
        while (ReadFile(hErr, buf, sizeof(buf), &bytesRead, NULL) && bytesRead > 0) {
            err.append(buf, bytesRead);
        }
        CloseHandle(hErr);
    });

    outThread.join();
    errThread.join();
}

void CommandsSession::waitCommand(NL_COMMANDS_PROCESS_HANDLE processHandle) {
    HANDLE hProcess = static_cast<HANDLE>(processHandle);

    WaitForSingleObject(hProcess, INFINITE);

    DWORD exitCode = 0;
    if (GetExitCodeProcess(hProcess, &exitCode)) {
        lastExitCode = static_cast<int>(exitCode);
    }
    else {
        lastExitCode = -1;
    }

    CloseHandle(hProcess);
}

#endif


int CommandsSession::Run(const std::vector<std::string>& cmdLine, std::string& out, std::string& err, bool wait,std::string cwd) {
    joinWorker();

    out.clear();
    err.clear();

    if (cmdLine.empty()) {
        errno = EINVAL;
        return -1;
    }

    NL_COMMANDS_PIPE_HANDLE outRead;
    NL_COMMANDS_PIPE_HANDLE errRead;
    NL_COMMANDS_PROCESS_HANDLE processHandle;
    NL_COMMANDS_PROCESS_ID processId;

#if defined(__linux__)
    std::vector<char*> cArgs;
    cArgs.reserve(cmdLine.size() + 1);
    for (auto& p : cmdLine) {
        cArgs.push_back(const_cast<char*>(p.c_str()));
    }
    cArgs.push_back(nullptr);

    int op[2];
    int ep[2];

    if (pipe(op) < 0) {
        return -1;
    }

    if (pipe(ep) < 0) {
        int e = errno;
        close(op[0]);
        close(op[1]);
        errno = e;
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        int e = errno;
        close(op[0]);
        close(op[1]);
        close(ep[0]);
        close(ep[1]);
        errno = e;
        return -1;
    }

    if (pid == 0) {
        if (dup2(op[1], STDOUT_FILENO) < 0) {
            _exit(126);
        }
        if (dup2(ep[1], STDERR_FILENO) < 0) {
            _exit(126);
        }
        if (op[0] > STDERR_FILENO) {
            close(op[0]);
        }
        if (ep[0] > STDERR_FILENO) {
            close(ep[0]);
        }
        if (op[1] > STDERR_FILENO) {
            close(op[1]);
        }
        if (ep[1] > STDERR_FILENO) {
            close(ep[1]);
        }
        if (!cwd.empty()) {
            std::filesystem::path cwdPath(cwd);
            if (chdir(cwdPath.c_str()) < 0) {
                _exit(126);
            }
        }
        execvp(cArgs[0], cArgs.data());
        _exit(127);
    }

    close(op[1]);
    close(ep[1]);

    outRead = op[0];
    errRead = ep[0];
    processHandle = pid;
    processId = pid;

#elif defined(_WIN32) || defined(_WIN64)
    std::string winCmdLine;
    for (size_t i = 0; i < cmdLine.size(); ++i) {
        if (i > 0) {
            winCmdLine += ' ';
        }

        const std::string& arg = cmdLine[i];

        bool needQuote = arg.empty();
        if (!needQuote) {
            for (char c : arg) {
                if (c == ' ' || c == '\t' || c == '"') {
                    needQuote = true;
                    break;
                }
            }
        }

        if (!needQuote) {
            winCmdLine += arg;
            continue;
        }

        winCmdLine += '"';
        size_t backslashes = 0;
        for (char c : arg) {
            if (c == '\\') {
                ++backslashes;
                continue;
            }
            if (c == '"') {
                winCmdLine.append(backslashes * 2 + 1, '\\');
                winCmdLine += '"';
                backslashes = 0;
                continue;
            }
            winCmdLine.append(backslashes, '\\');
            winCmdLine += c;
            backslashes = 0;
        }
        winCmdLine.append(backslashes * 2, '\\');
        winCmdLine += '"';
    }

    SECURITY_ATTRIBUTES sa = {};
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    HANDLE hOutRead = NULL;
    HANDLE hOutWrite = NULL;
    HANDLE hErrRead = NULL;
    HANDLE hErrWrite = NULL;

    if (!CreatePipe(&hOutRead, &hOutWrite, &sa, 0)) {
        return -1;
    }

    if (!CreatePipe(&hErrRead, &hErrWrite, &sa, 0)) {
        CloseHandle(hOutRead);
        CloseHandle(hOutWrite);
        return -1;
    }

    if (!SetHandleInformation(hOutRead, HANDLE_FLAG_INHERIT, 0) ||
        !SetHandleInformation(hErrRead, HANDLE_FLAG_INHERIT, 0)) {
        CloseHandle(hOutRead);
        CloseHandle(hOutWrite);
        CloseHandle(hErrRead);
        CloseHandle(hErrWrite);
        return -1;
    }

    STARTUPINFOA si = {};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = hOutWrite;
    si.hStdError = hErrWrite;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

    PROCESS_INFORMATION pi = {};

    std::vector<char> cmdBuf(winCmdLine.begin(), winCmdLine.end());
    cmdBuf.push_back('\0');

    std::string cwdStr;
    if (!cwd.empty()) {
        cwdStr = std::filesystem::path(cwd).string();
    }

    BOOL created = CreateProcessA(NULL, cmdBuf.data(), NULL, NULL, TRUE, 0, NULL,
        cwdStr.empty() ? NULL : cwdStr.c_str(), &si, &pi);

    CloseHandle(hOutWrite);
    CloseHandle(hErrWrite);

    if (!created) {
        CloseHandle(hOutRead);
        CloseHandle(hErrRead);
        return -1;
    }

    CloseHandle(pi.hThread);

    outRead = hOutRead;
    errRead = hErrRead;
    processHandle = pi.hProcess;
    processId = pi.dwProcessId;
#endif

    if (wait) {
        openPipes(outRead, errRead, out, err);
        waitCommand(processHandle);
        return lastExitCode;
    }

    running = true;
    commandHandler = std::thread([this, processHandle, outRead, errRead, &out, &err] {openPipes(outRead, errRead, out, err); waitCommand(processHandle); running = false;});
    return static_cast<int>(processId);
}

int CommandsSession::RunWithShell(const std::string& cmdLine, std::string& out, std::string& err, bool wait,std::string cwd) {
#if defined(__linux__)
    return Run({"sh", "-c", cmdLine}, out, err, wait, cwd);
#elif defined(_WIN32) || defined(_WIN64)
    return Run({"cmd.exe", "/c", cmdLine}, out, err, wait, cwd);
#endif
}