cmake_minimum_required(VERSION 3.30) 

project(Krystal)

set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_VS_JUST_MY_CODE_DEBUGGING ON CACHE BOOL "Enable Just My Code Debugging")
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "ProgramDatabase" CACHE STRING "MSVC debug information format")

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
add_subdirectory(Krystal.I18n)
add_subdirectory(Krystal.Net)
add_subdirectory(Krystal.Image)

# Krystal Engine modules
add_subdirectory(Krystal.Web)
add_subdirectory(Krystal.Gfx)
add_subdirectory(Krystal.PAL)
add_subdirectory(Krystal.Booey)

# Krystal Editor
add_subdirectory(Krystal.Editor)

# Krystal Engine tests, grouped in a Tests folder in Visual Studio's solution
set(CMAKE_FOLDER Tests)
add_subdirectory(tests/Krystal.Core.Tests Krystal.Core.Tests)
add_subdirectory(tests/Krystal.Text.Tests Krystal.Text.Tests)
add_subdirectory(tests/Krystal.I18n.Tests Krystal.I18n.Tests)
add_subdirectory(tests/Krystal.Net.Tests Krystal.Net.Tests)
add_subdirectory(tests/Krystal.Image.Tests Krystal.Image.Tests)
add_subdirectory(tests/Krystal.Web.Tests Krystal.Web.Tests)
add_subdirectory(tests/Krystal.Gfx.Tests Krystal.Gfx.Tests)
add_subdirectory(tests/Krystal.Booey.Tests Krystal.Booey.Tests)
unset(CMAKE_FOLDER)
