#pragma once

#include "Krystal.Maths/Quaternion.hpp"
#include "Krystal.Serialisation/Builtins.hpp"

namespace Krys::Maths
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