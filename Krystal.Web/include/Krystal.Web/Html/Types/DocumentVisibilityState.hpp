#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::web::html
{
  /// @see https://html.spec.whatwg.org/multipage/dom.html#documentvisibilitystate
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
