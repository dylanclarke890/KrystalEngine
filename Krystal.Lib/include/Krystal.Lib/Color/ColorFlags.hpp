#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys
{
  enum class ColorFlags : uint8
  {
    None = 0,
    Semantic = 1 << 0,
    UseColorFunctionSerialization = 1 << 1,
  };

  enum class ColorFlagsIncludingPrivate : uint8
  {
    None = 0,
    Semantic = static_cast<uint8>(ColorFlags::Semantic),
    UseColorFunctionSerialization = static_cast<uint8>(ColorFlags::UseColorFunctionSerialization),
    Valid = 1 << 2,
    OutOfLine = 1 << 3,
    HashTableEmptyValue = 1 << 4,
    HashTableDeletedValue = 1 << 5,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(Krys::ColorFlags, 3uz)
KRYS_DEFINE_FLAGS_ENUM_TRAITS(Krys::ColorFlagsIncludingPrivate, 7uz)