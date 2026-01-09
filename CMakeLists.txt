cmake_minimum_required(VERSION 3.21)
project(cpp-game LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Feature flags -------------------------------------------------
option(STRICT_PORTABLE_WARNINGS "Treat warnings as errors in a portable-safe way" OFF)
option(ENABLE_NATIVE_OPT "Enable native CPU-specific optimisations" OFF)
option(ENABLE_SANITIZERS "Enable Address/UB sanitizers on non-MSVC builds" OFF)

# --------------------------------------------------------------------
# Warnings library
# --------------------------------------------------------------------
add_library(project_warnings INTERFACE)

# Compiler-specific warning options
if (MSVC)
    # MSVC / clang-cl: high warnings + conformance
    target_compile_options(project_warnings INTERFACE
            /W4            # High warning level
            /permissive-   # Stricter C++ conformance
            /Zc:preprocessor
            /Zc:__cplusplus
    )

    if (STRICT_PORTABLE_WARNINGS)
        # Treat *all* warnings as errors on MSVC in strict mode
        target_compile_options(project_warnings INTERFACE /WX)
    endif ()

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
        OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")

    # GCC, Clang, AppleClang: crank it up
    target_compile_options(project_warnings INTERFACE
            -Wall -Wextra -Wpedantic
            -Wconversion
            -Wsign-conversion
            -Wimplicit-fallthrough
            -Wunreachable-code
            -Wnon-virtual-dtor
            -Wnull-dereference
            -Wdouble-promotion
            -Wmissing-include-dirs
            $<$<COMPILE_LANGUAGE:C>:-Werror=implicit-int>
            -Werror=return-type
    )

    if (STRICT_PORTABLE_WARNINGS)
        # macOS/Linux strict builds: warnings => errors
        target_compile_options(project_warnings INTERFACE -Werror)
    endif ()

endif ()


# --------------------------------------------------------------------
# Dependencies
# --------------------------------------------------------------------
find_package(Drogon CONFIG REQUIRED)
find_package(Xlnt CONFIG REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)
find_package(absl CONFIG REQUIRED)

# --------------------------------------------------------------------
# Sources & main target
# --------------------------------------------------------------------
file(GLOB_RECURSE CPP_GAME_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/backend/*.cpp"
)

add_executable(cpp-game ${CPP_GAME_SOURCES})
add_compile_definitions(NOMINMAX)

# --------------------------------------------------------------------
# Excel files
# --------------------------------------------------------------------

target_compile_definitions(cpp-game PRIVATE
        GAME_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

# --------------------------------------------------------------------
# Link libraries (inherits warnings from project_warnings)
# --------------------------------------------------------------------
target_link_libraries(cpp-game
        PRIVATE
        Drogon::Drogon
        xlnt::xlnt
        project_warnings
        JsonCpp::JsonCpp
        absl::flat_hash_map

)

# --------------------------------------------------------------------
# Per-compiler optimisation flags for cpp-game
# --------------------------------------------------------------------
if (MSVC)
    target_compile_options(cpp-game PRIVATE
            $<$<CONFIG:Release,RelWithDebInfo>:/O2>
            $<$<CONFIG:Release,RelWithDebInfo>:/Oi>
            $<$<CONFIG:Release,RelWithDebInfo>:/Ot>
            $<$<CONFIG:Release,RelWithDebInfo>:/GL>
    )
    target_link_options(cpp-game PRIVATE
            $<$<CONFIG:Release,RelWithDebInfo>:/LTCG>
    )

    if (ENABLE_NATIVE_OPT)
        target_compile_options(cpp-game PRIVATE
                $<$<CONFIG:Release,RelWithDebInfo>:/arch:AVX2>
        )
    endif ()

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
        OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")

    target_compile_options(cpp-game PRIVATE
            $<$<CONFIG:Release,RelWithDebInfo>:-O3>
            $<$<CONFIG:Release,RelWithDebInfo>:-ffast-math>
            $<$<CONFIG:Release,RelWithDebInfo>:-funroll-loops>
    )

    if (ENABLE_NATIVE_OPT)
        target_compile_options(cpp-game PRIVATE
                $<$<CONFIG:Release,RelWithDebInfo>:-march=native>
        )
    endif ()

endif ()

# --------------------------------------------------------------------
# Sanitizers (non-MSVC, typically macOS/Linux)
# --------------------------------------------------------------------
if (ENABLE_SANITIZERS AND NOT MSVC)
    message(STATUS "Enabling sanitizers (Address + Undefined)")
    target_compile_options(cpp-game PRIVATE
            $<$<CONFIG:Debug>:-fsanitize=address,undefined>
            $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
    )
    target_link_options(cpp-game PRIVATE
            $<$<CONFIG:Debug>:-fsanitize=address,undefined>
    )
endif ()


# --------------------------------------------------------------------
# Frontend options
# --------------------------------------------------------------------
option(FRONTEND_DEV "Enable frontend dev server" OFF)
option(FRONTEND "Enable frontend release server" OFF)

if (FRONTEND AND NOT FRONTEND_DEV)
    message(STATUS "Frontend release build is enabled (npm run build).")

    add_custom_target(frontend_build
            COMMAND npm run build
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/frontend
            COMMENT "Building frontend with vite"
            VERBATIM
    )

    add_dependencies(cpp-game frontend_build)
else ()
    message(STATUS "Frontend release build is disabled (no npm run build).")
endif ()


if (FRONTEND_DEV)
    target_compile_definitions(cpp-game PUBLIC
            FRONTEND_DEV=1
            FRONTEND_DIR=\"${CMAKE_SOURCE_DIR}/frontend\"
    )
endif ()

if (FRONTEND)
    target_compile_definitions(cpp-game PUBLIC
            FRONTEND=1
            FRONTEND_DIST_DIR=\"${CMAKE_SOURCE_DIR}/frontend/dist\"
    )
endif ()
