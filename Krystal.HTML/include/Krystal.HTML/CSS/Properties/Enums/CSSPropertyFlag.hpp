#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class CSSPropertyFlag : uint8
  {
    None = 0,
    /// @brief Whether or not the property is important (i.e. has the `!important` flag).
    Important = 1 << 0,
    /// @brief Whether or not the property was implicitly set as the result of a shorthand.
    Implicit = 1 << 1,
    /// @brief Whether or not the property was set as the result of a shorthand.
    SetFromShorthand = 1 << 2,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(Krys::HTML::CSSPropertyFlag, 4uz);