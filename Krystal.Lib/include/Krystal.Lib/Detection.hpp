#pragma once

#ifdef _DEBUG
  #define KRYS_DEBUG
#else
  #define KRYS_RELEASE
#endif

#pragma region Compiler Detection

#if defined(_MSC_VER)
  #define KRYS_COMPILER_VISUAL_STUDIO
#elif defined(__clang__)
  #define KRYS_COMPILER_CLANG
#elif defined(__GNUC__)
  #define KRYS_COMPILER_GCC
#else
  #error "Unknown Compiler!"
#endif

#pragma endregion

#pragma region Platform Detection

#if defined(_WIN32) // Windows platform detection
  #if defined(_WIN64)
    #define KRYS_PLATFORM_WINDOWS
  #else
    #error "x86 Builds are not supported!"
  #endif
#elif defined(__APPLE__) || defined(__MACH__) // Apple platform detection
  #include <TargetConditionals.h>
  // TARGET_OS_MAC exists on all platforms, not just on MacOS.
  // Use this order of checks to determine the specific platform.
  #if defined(TARGET_IPHONE_SIMULATOR)
    #error "IOS simulator is not supported!"
  #elif defined(TARGET_OS_IPHONE)
    #define KRYS_PLATFORM_IOS
    #error "IOS is not supported!"
  #elif defined(TARGET_OS_MAC)
    #define KRYS_PLATFORM_MACOS
    #error "MacOS is not supported!"
  #else
    #error "Unknown Apple platform!"
  #endif
// Linux platform detection
#elif defined(__linux__) // Apple platform detection
  // __linux__ exists on both android and linux.
  // Check for __ANDROID__ to determine the platform.
  #if defined(__ANDROID__)
    #define KRYS_PLATFORM_ANDROID
    #error "Android is not supported!"
  #else
    #define KRYS_PLATFORM_LINUX
    #error "Linux is not supported!"
  #endif
#else
  #error "Unknown platform!"
#endif

#pragma endregion

#ifdef __INTELLISENSE__
constexpr auto KRYS_INTELLISENSE_RUNNING = true;
#else
constexpr auto KRYS_INTELLISENSE_RUNNING = false;
#endif