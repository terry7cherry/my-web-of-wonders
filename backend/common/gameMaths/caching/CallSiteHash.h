#pragma once

#include <functional>
#include "CallSite.h"

struct CallSiteHash
{
    std::size_t operator()(const CallSite& cs) const noexcept
    {
        return std::hash<const char*>{}(cs.file) ^
            (std::hash<std::uint32_t>{}(cs.line) << 1);
    }
};
