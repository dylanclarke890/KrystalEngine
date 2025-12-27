#pragma once

#include "Krystal.Lib/Core/Detection.hpp"

#if KRYS_ENV(DEV)
  #if KRYS_PLATFORM(WINDOWS)
    #define KRYS_DEBUG_BREAK() __debugbreak()
  #elif KRYS_PLATFORM(LINUX)
    #include <signal.h>
    #define KRYS_DEBUG_BREAK() raise(SIGTRAP)
  #else
    #error "KRYS_DEBUG_BREAK is not supported for your platform!"
  #endif
#else
  #define KRYS_DEBUG_BREAK()
#endif
