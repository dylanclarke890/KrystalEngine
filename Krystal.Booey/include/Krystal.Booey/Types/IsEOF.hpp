#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  constexpr char32 EOFMarker = U'\uFFFF';

  enum class IsEOF : bool
  {
    No,
    Yes,
  };
}
