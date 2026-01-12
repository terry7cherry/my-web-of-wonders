#pragma once

#include "../console/Console.h"
#include <cstdint>

#define MILLION (1000000)
#define BILLION (1000000000)


#ifdef _WIN32
typedef signed __int64 int64;
typedef unsigned __int64 uint64;
#else
typedef std::int64_t int64;
typedef std::uint64_t uint64;
#endif

typedef std::int8_t int8;
typedef std::uint8_t uint8;
typedef std::int16_t int16;
typedef std::uint16_t uint16;
typedef std::int32_t int32;
typedef std::uint32_t uint32;
typedef std::int64_t int64;
typedef std::uint64_t uint64;

typedef struct {
    std::string szName;
    std::string szSimDigits;
    std::string szMathsDigits;
    std::string szRtp;
} VERSION_DATA;

//extern VERSION_DATA gameVersion;

// Global sentinels
// Sentinel to represent "no symbol" for grids storing symbols as uint8_t
// Use UINT8_MAX (255) so it safely fits in uint8_t and can be compared/promoted to int
constexpr uint8_t SYMBOL_NONE = UINT8_MAX;
