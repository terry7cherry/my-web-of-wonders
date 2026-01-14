#pragma once

#include <string>
#include <atomic>

#ifdef _WIN32
#  include <windows.h> // for HANDLE
#else
#  include <sys/types.h> // for pid_t
#endif

#ifdef FRONTEND_DEV
// Exposed so users can coordinate shutdown across TUs
extern std::atomic<bool> g_shuttingDown;

class NodeDevServer
{
public:
    NodeDevServer() = default;
    NodeDevServer(const NodeDevServer&) = delete;

    NodeDevServer& operator=(const NodeDevServer&) = delete;

    ~NodeDevServer() { stop(); }

    void start();

    void stop();

    inline static NodeDevServer* g_nodeServer = nullptr;

    static void shutdownHook()
    {
        if (g_shuttingDown.exchange(true)) return;
        if (g_nodeServer) g_nodeServer->stop();
    }

    static void signalHandler(int)
    {
        shutdownHook();
        std::_Exit(1);
    }

private:
#ifdef _WIN32
    static std::wstring widen(const std::string& s);
    HANDLE job_ = nullptr;
    HANDLE process_ = nullptr;
    HANDLE thread_ = nullptr;
#else
    pid_t pid_ = -1;
#endif
    bool started_ = false;
};
#endif
