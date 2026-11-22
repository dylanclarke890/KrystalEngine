#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#documentvisibilitystate
  enum class DocumentVisibilityState : uint8
  {
    Visible,
    Hidden,
  };

  consteval DocumentVisibilityState MaxEnumValue(DocumentVisibilityState) noexcept
  {
    return DocumentVisibilityState::Hidden;
  }
}
