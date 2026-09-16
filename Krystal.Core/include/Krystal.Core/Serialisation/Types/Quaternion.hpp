#pragma once

#include "Krystal.Core/Maths/Quaternion.hpp"
#include "Krystal.Core/Serialisation/Builtins.hpp"

namespace krys
{
  template <typename Archive, typename T>
  void Transfer(Archive &archive, Quaternion<T> &quat) noexcept
  {
    archive(Serialisation::CreateNamedField("w", quat.w));
    archive(Serialisation::CreateNamedField("x", quat.x));
    archive(Serialisation::CreateNamedField("y", quat.y));
    archive(Serialisation::CreateNamedField("z", quat.z));
  }
}