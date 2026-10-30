#pragma once

#include "Krystal.Core/Types/FixedVector.hpp"

namespace krys::boo
{
  using DashArrayElement = float;

  // DashArray is array of lengths along a path, with elements alternating on, off, on, off state.
  // The array is applied circularly mod path length.
  using DashArray = FixedVector<DashArrayElement>;
}
