#pragma once

#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/StronglyTypedValue.hpp"

namespace Krys::HTML
{
  constexpr inline char32 EOFMarker = U'\uFFFF';

  struct IsEOF : StronglyTypedBool<IsEOF>
  {
    using StronglyTypedBool::StronglyTypedBool;
  };
}