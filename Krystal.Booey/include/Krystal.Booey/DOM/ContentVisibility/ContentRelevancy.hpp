#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::dom
{
  /// @brief Why an element with `content-visibility: auto` is relevant to the user.
  /// @see https://drafts.csswg.org/css-contain/#relevant-to-the-user
  enum class ContentRelevancy : uint8
  {
    /// @brief It's on or near the screen.
    OnScreen,
    /// @brief It or something in it has focus.
    Focused,
    /// @brief It or something in it is in the top layer.
    IsInTopLayer,
    /// @brief Some of it is selected.
    Selected,
  };

  consteval ContentRelevancy MaxEnumValue(ContentRelevancy) noexcept
  {
    return ContentRelevancy::Selected;
  }
}
