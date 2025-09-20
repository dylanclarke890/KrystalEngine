#pragma once

#include "Krystal.Lib/Detection.hpp"

#ifdef KRYS_DEBUG
  #ifdef KRYS_PLATFORM_WINDOWS
    #define KRYS_DEBUG_BREAK() __debugbreak()
  #elif defined(KRYS_PLATFORM_LINUX)
    #include <signal.h>
    #define KRYS_DEBUG_BREAK() raise(SIGTRAP)
  #else
    #error "KRYS_DEBUG_BREAK is not supported for your platform!"
  #endif
#else
  #define KRYS_DEBUG_BREAK()
#endif
