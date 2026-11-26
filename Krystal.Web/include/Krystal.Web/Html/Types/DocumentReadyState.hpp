#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::web::html
{
  /// @see https://html.spec.whatwg.org/multipage/dom.html#documentreadystate
  enum class DocumentReadyState : uint8
  {
    Loading,
    Interactive,
    Complete,
  };

  consteval DocumentReadyState MaxEnumValue(DocumentReadyState) noexcept
  {
    return DocumentReadyState::Complete;
  }
}
