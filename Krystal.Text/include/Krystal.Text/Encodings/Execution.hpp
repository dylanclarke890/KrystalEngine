#pragma once

#include "Krystal.Lib/Detection/Compiler.hpp"
#include "Krystal.Lib/Detection/OS.hpp"
#include "Krystal.Text/Encodings/Impl/ExecutionCuchar.hpp"
#include "Krystal.Text/Encodings/NoEncoding.hpp"

namespace Krys
{
  /// @brief The Encoding that represents the "Execution" (narrow locale-based) encoding. The encoding is
  /// typically associated with the locale, which is tied to the C standard library's setlocale function.
  /// @remarks Use of this type is subject to the C Standard Library or platform defaults. Some locales
  /// (such as the Big5 Hong King Supplementary Character Set (Big5-HKSCS)) are broken due to fundamental
  /// design issues in the C Standard Library and bugs in glibc/musl libc's current locale encoding support.
  /// On Apple, this is cuurrently assumed to be UTF-8 since they do not support the @c \<cuchar\> or @c
  /// \<uchar.h\> headers.
  class execution_t : public
#if KRYS_COMPILER_INCLUDE(<cuchar>)
                      Impl::ExecutionCuChar
#else
                      no_encoding_t
  #warning "Unsupported platform for using the execution encoing, sorry! Falling back to no encoding."
#endif
  {
  };

  /// @brief An instance of the execution_t type for ease of use.
  inline constexpr execution_t execution = {};
}
