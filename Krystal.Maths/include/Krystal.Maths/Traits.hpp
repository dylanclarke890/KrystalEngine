#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"

namespace Krys::Maths
{
  template <typename T>
  struct Traits;

  template <typename T>
  struct Traits
  {
    using ComponentType = T;

    static constexpr bool IsVector = false;
    static constexpr bool IsMatrix = false;

    static constexpr int TotalComponents = 0;
    static constexpr int Rows = 0;
    static constexpr int Columns = 0;
  };
}