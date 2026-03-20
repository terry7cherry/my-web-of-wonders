#pragma once
#include <cstdint>

namespace SessionState
{
    // True if the most recently completed spin included a Free Spins launch that then completed within the same cycle.
    // Set during per-spin stats collection.
    bool getLastSpinIncludedFS();
    void setLastSpinIncludedFS(bool v);
}
