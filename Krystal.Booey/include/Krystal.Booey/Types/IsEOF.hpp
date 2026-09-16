#pragma once

#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/StronglyTypedValue.hpp"

namespace krys::boo
{
  constexpr char32 EOFMarker = U'\uFFFF';

  struct IsEOF : StronglyTypedBool<IsEOF>
  {
    using StronglyTypedBool::StronglyTypedBool;
  };
}