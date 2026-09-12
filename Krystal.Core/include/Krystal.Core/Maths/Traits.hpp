#pragma once

#include "Krystal.Core/Concepts.hpp"

namespace krys
{
  template <typename T>
  struct Traits;

  template <typename T>
  struct Traits
  {
    using ComponentType = T;

    constexpr static bool IsVector = false;
    constexpr static bool IsMatrix = false;

    constexpr static int TotalComponents = 0;
    constexpr static int Rows = 0;
    constexpr static int Columns = 0;
  };
}