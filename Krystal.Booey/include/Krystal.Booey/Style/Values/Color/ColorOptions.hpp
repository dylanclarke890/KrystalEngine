#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::style
{
  /// @brief Options for resolving system and keyword colours.
  enum class ColorOptions : uint8
  {
    None = 0,
    UseSystemAppearance = 1 << 0,
    UseDarkAppearance = 1 << 1,
    UseElevatedUserInterfaceLevel = 1 << 2
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::boo::style::ColorOptions, 3uz)
