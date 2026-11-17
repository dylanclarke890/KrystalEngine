#pragma once

#include "Krystal.Core/Maths/Vector.hpp"
#include "Krystal.Core/Serialisation/Builtins.hpp"

namespace krys
{
  template <typename Archive, typename T, int N>
  void Transfer(Archive &archive, Vector<T, N> &vector) noexcept
  {
    archive(serialisation::CreateNamedField("x", vector.x));

    if constexpr (N > 1)
    {
      archive(serialisation::CreateNamedField("y", vector.y));
    }

    if constexpr (N > 2)
    {
      archive(serialisation::CreateNamedField("z", vector.z));
    }

    if constexpr (N > 3)
    {
      archive(serialisation::CreateNamedField("w", vector.w));
    }
  }
}
