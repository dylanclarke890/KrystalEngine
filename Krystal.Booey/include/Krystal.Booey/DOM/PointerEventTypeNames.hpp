#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::dom
{
  /// @brief The `pointerType` of a mouse's pointer events: `mouse`.
  KRYS_NODISCARD const DOMString &MousePointerEventType() noexcept;
  /// @brief The `pointerType` of a pen's pointer events: `pen`.
  KRYS_NODISCARD const DOMString &PenPointerEventType() noexcept;
  /// @brief The `pointerType` of a touch's pointer events: `touch`.
  KRYS_NODISCARD const DOMString &TouchPointerEventType() noexcept;
}
