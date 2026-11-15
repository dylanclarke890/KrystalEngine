#pragma once

#include "Krystal.Core/Maths/Quaternion.hpp"
#include "Krystal.Core/Serialisation/Builtins.hpp"

namespace krys
{
  template <typename Archive, typename T>
  void Transfer(Archive &archive, Quaternion<T> &quat) noexcept
  {
    archive(serialisation::CreateNamedField("w", quat.w));
    archive(serialisation::CreateNamedField("x", quat.x));
    archive(serialisation::CreateNamedField("y", quat.y));
    archive(serialisation::CreateNamedField("z", quat.z));
  }
}
