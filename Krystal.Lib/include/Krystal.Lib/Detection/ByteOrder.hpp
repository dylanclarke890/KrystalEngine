#pragma once

#include "Krystal.Lib/Detection/CPU.hpp"

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  #define KRYS_CPU_BIG_ENDIAN 1
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  #define KRYS_CPU_LITTLE_ENDIAN 1
#elif __BYTE_ORDER__ == __ORDER_PDP_ENDIAN__
  #define KRYS_CPU_MIDDLE_ENDIAN 1
#else
  #error "Unknown endian"
#endif

#if !KRYS_CPU(BIG_ENDIAN) && !KRYS_CPU(LITTLE_ENDIAN)
  #error "Unsupported endian"
#endif