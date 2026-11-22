#pragma once

#include "Krystal.Core/Containers/EnumSet.hpp"
#include "Krystal.Core/Enum.hpp"

namespace krys::boo
{
  enum class PropertyFlag : uint8
  {
    /// @brief Whether or not the property is important (i.e. has the `!important` flag).
    Important,
    /// @brief Whether or not the property was implicitly set as the result of a shorthand.
    Implicit,
    /// @brief Whether or not the property was set as the result of a shorthand.
    SetFromShorthand,
  };

  consteval PropertyFlag MaxEnumValue(PropertyFlag) noexcept
  {
    return PropertyFlag::SetFromShorthand;
  }
}
