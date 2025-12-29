#pragma once

#include "Krystal.Maths/Vector.hpp"
#include "Krystal.Serialisation/Builtins.hpp"

namespace Krys::Maths
{
  template <typename Archive, typename T, int N>
  void Transfer(Archive &archive, Vector<T, N> &vector) noexcept
  {
    archive(Serialisation::CreateNamedField("x", vector.x));

    if constexpr (N > 1)
    {
      archive(Serialisation::CreateNamedField("y", vector.y));
    }

    if constexpr (N > 2)
    {
      archive(Serialisation::CreateNamedField("z", vector.z));
    }

    if constexpr (N > 3)
    {
      archive(Serialisation::CreateNamedField("w", vector.w));
    }
  }
}