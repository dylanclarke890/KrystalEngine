cmake_minimum_required(VERSION 3.30) 

project(Krystal)

set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

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

# Krystal Engine modules
add_subdirectory(Krystal.Booey)
add_subdirectory(Krystal.Gfx)
add_subdirectory(Krystal.Gfx.D3D11)
add_subdirectory(Krystal.Gfx.OpenGL)
add_subdirectory(Krystal.Gfx.Vulkan)
add_subdirectory(Krystal.PAL)

# Krystal Editor
add_subdirectory(Krystal.Editor)

# Krystal Engine tests
add_subdirectory(Krystal.Core.Tests)
add_subdirectory(Krystal.Booey.Tests)
