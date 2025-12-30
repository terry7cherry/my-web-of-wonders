#pragma once

#include <source_location>
#include <cstdint>

struct CallSite
{
    const char* file;
    std::uint32_t line;

    bool operator==(const CallSite& other) const noexcept
    {
        return file == other.file && line == other.line;
    }

    static CallSite current(std::source_location loc =
        std::source_location::current()) noexcept
    {
        return {loc.file_name(), loc.line()};
    }
};
