cmake_minimum_required(VERSION 3.30) 

project(Krystal)

set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_VS_JUST_MY_CODE_DEBUGGING ON CACHE BOOL "Enable Just My Code Debugging")
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "ProgramDatabase" CACHE STRING "MSVC debug information format")

# Leave unreferenced inline functions and data out of object files (the generated projects don't ask for it):
# without it, Krystal.Booey's static library passes the 4 GB a library can hold.
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/Zc:inline>)

# MSBuild's CL task only parallelises (/MP) within a batch of files with identical settings, and files that share a
# basename get their own object path, so each one ends up in a batch of its own and compiles serially. The multi tool
# task schedules every file independently instead.
set(CMAKE_VS_GLOBALS
  "UseMultiToolTask=true"
  "EnforceProcessCountAcrossBuilds=true"
)

# Third-party - stb
add_subdirectory(third-party/stb)

# Third-party - llvm (only enough for SmallVector)
add_subdirectory(third-party/llvm)

# Third-party - Catch2
find_package(catch2 CONFIG REQUIRED)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/third_party/Catch2/extras")
include(Catch)
enable_testing()

add_subdirectory(Krystal.Core)
add_subdirectory(Krystal.Text)

# Krystal Engine modules
add_subdirectory(Krystal.Booey)
add_subdirectory(Krystal.Gfx)
add_subdirectory(Krystal.PAL)
add_subdirectory(Krystal.UI)

# Krystal Editor
add_subdirectory(Krystal.Editor)

# Krystal Engine tests
add_subdirectory(Krystal.Core.Tests)
add_subdirectory(Krystal.Text.Tests)
add_subdirectory(Krystal.Booey.Tests)
add_subdirectory(Krystal.Gfx.Tests)
add_subdirectory(Krystal.UI.Tests)
