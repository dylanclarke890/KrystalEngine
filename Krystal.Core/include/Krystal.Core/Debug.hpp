#pragma once

#include "Krystal.Core/Detection/Compiler.hpp"
#include "Krystal.Core/Detection/OS.hpp"

#if !defined(KRYS_DEBUG)
  #if !defined(NDEBUG)
    #define KRYS_DEBUG 1
  #else
    #define KRYS_DEBUG 0
  #endif
#endif

#if KRYS_DEBUG
  #if KRYS_OS(WINDOWS)
    #include <intrin.h>
    #define KRYS_DEBUG_BREAK() __debugbreak()
  #elif KRYS_OS(LINUX)
    #include <signal.h>
    #define KRYS_DEBUG_BREAK() raise(SIGTRAP)
  #else
    #warning "KRYS_DEBUG_BREAK is not supported for your platform!"
    #define KRYS_DEBUG_BREAK()
  #endif
#else
  #define KRYS_DEBUG_BREAK()
#endif
