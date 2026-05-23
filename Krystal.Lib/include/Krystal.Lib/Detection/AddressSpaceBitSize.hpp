#pragma once

#include "Krystal.Lib/Detection/Compiler.hpp"

/// @brief Check if compiling with a particular address space bit size.
#define KRYS_ADDRESS_SPACE(BITSIZE) (defined KRYS_ADDRESS_SPACE_##BITSIZE && KRYS_ADDRESS_SPACE_##BITSIZE)

#if (KRYS_COMPILER(GCC) && __SIZEOF_POINTER__ == 8) || (KRYS_COMPILER(MSVC) && defined(_M_X64))
  #define KRYS_ADDRESS_SPACE_64 1
  #define KRYS_EFFECTIVE_ADDRESS_WIDTH 48
#else
  #define KRYS_ADDRESS_SPACE_32 1
  #define KRYS_EFFECTIVE_ADDRESS_WIDTH 32
#endif