#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::dom
{
  /// @brief The state of an element's `popover` attribute.
  /// @see https://html.spec.whatwg.org/#attr-popover
  enum class PopoverState : uint8
  {
    None,
    Auto,
    Manual,
    Hint
  };

  /// @see https://html.spec.whatwg.org/#popover-visibility-state
  enum class PopoverVisibilityState : bool
  {
    Hidden,
    Showing
  };

  /// @brief Whether hiding a popover returns the focus to the element focused before it showed.
  enum class FocusPreviousElement : bool
  {
    No,
    Yes
  };

  /// @brief Whether showing or hiding a popover fires its `beforetoggle` and `toggle` events.
  enum class FireEvents : bool
  {
    No,
    Yes
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::dom::PopoverState, 4uz);
