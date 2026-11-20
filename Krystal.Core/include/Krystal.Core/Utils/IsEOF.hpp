#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys
{
  /// @brief The code point a tokenizer's input stream returns once it's past its end.
  constexpr char32 EOFMarker = U'\uFFFF';

  /// @brief Whether an input stream is past its end.
  enum class IsEOF : bool
  {
    No,
    Yes,
  };
}
