#pragma once

#include "Krystal.Lib/Core/Detection.hpp"

#if KRYS_ENV(DEV)
  #if KRYS_OS(WINDOWS)
    #define KRYS_DEBUG_BREAK() __debugbreak()
  #elif KRYS_OS(LINUX)
    #include <signal.h>
    #define KRYS_DEBUG_BREAK() raise(SIGTRAP)
  #else
    #error "KRYS_DEBUG_BREAK is not supported for your platform!"
  #endif
#else
  #define KRYS_DEBUG_BREAK()
#endif
