#include <iostream>
#include <cstdio>
#include <algorithm>
#include <string>
#include <thread>
#include <cstdlib>
#include <cstdint>
#include <vector>
// ... existing code ...
#include <csignal>
#include <atomic>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#endif

// ... existing code ...

#ifdef FRONTEND_DEV

namespace {
    std::atomic<bool> g_shuttingDown{false};

    class NodeDevServer {
    public:
        NodeDevServer() = default;

        NodeDevServer(const NodeDevServer &) = delete;

        NodeDevServer &operator=(const NodeDevServer &) = delete;

        void start() {
            if (started_) return;
            started_ = true;

#ifdef _WIN32
            // npm is npm.cmd (batch script). CreateProcess can't execute .cmd directly,
            // so run it through cmd.exe.
            std::wstring npmCmd =
                    L"cmd.exe /c npm --prefix \"" + widen(std::string(FRONTEND_DIR)) + L"\" run dev";

            STARTUPINFOW si{};
            si.cb = sizeof(si);
            PROCESS_INFORMATION pi{};

            job_ = CreateJobObjectW(nullptr, nullptr);
            if (!job_) {
                std::cerr << "CreateJobObject failed\n";
                return;
            }
            JOBOBJECT_EXTENDED_LIMIT_INFORMATION info{};
            info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
            SetInformationJobObject(job_, JobObjectExtendedLimitInformation, &info, sizeof(info));

            std::vector<wchar_t> buf(npmCmd.begin(), npmCmd.end());
            buf.push_back(L'\0');

            BOOL ok = CreateProcessW(
                nullptr,
                buf.data(),
                nullptr, nullptr,
                FALSE,
                0, // keep it simple; if you want no window, use CREATE_NO_WINDOW
                nullptr,
                nullptr, // optionally set to widen(FRONTEND_DIR).c_str() as working dir
                &si, &pi
            );
            if (!ok) {
                DWORD err = GetLastError();
                std::wcerr << L"CreateProcessW failed to start: " << npmCmd << L"\n";

                LPWSTR msg = nullptr;
                FormatMessageW(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    nullptr, err,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPWSTR) &msg, 0, nullptr
                );
                if (msg) {
                    std::wcerr << L"Win32 error " << err << L": " << msg << L"\n";
                    LocalFree(msg);
                } else {
                    std::wcerr << L"Win32 error " << err << L"\n";
                }

                CloseHandle(job_);
                job_ = nullptr;
                return;
            }

            AssignProcessToJobObject(job_, pi.hProcess);

            process_ = pi.hProcess;
            thread_ = pi.hThread;

#else
            // ... existing code ...
            pid_ = fork();
            if (pid_ == 0) {
                setpgid(0, 0);
                std::string prefix = std::string(FRONTEND_DIR);

                execlp("npm", "npm", "--prefix", prefix.c_str(), "run", "dev", (char *) nullptr);
                _exit(127);
            }
            if (pid_ < 0) {
                std::cerr << "fork failed starting npm\n";
                pid_ = -1;
                return;
            }
            setpgid(pid_, pid_);
#endif
        }

        void stop() {
            // ... existing code ...
#ifdef _WIN32
            if (job_) {
                CloseHandle(job_);
                job_ = nullptr;
            }
            if (thread_) {
                CloseHandle(thread_);
                thread_ = nullptr;
            }
            if (process_) {
                CloseHandle(process_);
                process_ = nullptr;
            }
#else
            // ... existing code ...
            if (pid_ > 0) {
                kill(-pid_, SIGTERM);
                int status = 0;
                waitpid(pid_, &status, 0);
                pid_ = -1;
            }
#endif
        }

        ~NodeDevServer() { stop(); }

    private:
#ifdef _WIN32
        static std::wstring widen(const std::string &s) {
            if (s.empty()) return {};
            int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
            std::wstring w(len ? len - 1 : 0, L'\0');
            if (len > 1) MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, w.data(), len);
            return w;
        }

        HANDLE job_ = nullptr;
        HANDLE process_ = nullptr;
        HANDLE thread_ = nullptr;
#else
        pid_t pid_ = -1;
#endif
        bool started_ = false;
    };

    NodeDevServer *g_nodeServer = nullptr;

    void shutdownHook() {
        if (g_shuttingDown.exchange(true)) return;
        if (g_nodeServer) g_nodeServer->stop();
    }

    void signalHandler(int) {
        shutdownHook();
        std::_Exit(1);
    }
}

#endif // FRONTEND_DEV
