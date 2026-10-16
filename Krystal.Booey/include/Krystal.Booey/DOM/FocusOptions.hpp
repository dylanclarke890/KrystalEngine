#pragma once

#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::dom
{
  enum class FocusRemovalEventsMode : bool
  {
    Dispatch,
    DoNotDispatch
  };

  /// @brief What caused a focus change.
  enum class FocusTrigger : uint8
  {
    Other,
    Click,
    Bindings
  };

  /// @brief Whether a focused element matches `:focus-visible`.
  enum class FocusVisibility : uint8
  {
    Invisible,
    Visible,
    ForceInvisible
  };

  /// @brief How to focus an element: the `FocusOptions` dictionary and the engine's own options.
  /// @see https://html.spec.whatwg.org/#focusoptions
  // FIXME: The selection restoration mode and focus direction, once there's editing and sequential focus
  // navigation.
  struct FocusOptions
  {
    FocusRemovalEventsMode RemovalEventsMode {FocusRemovalEventsMode::Dispatch};
    FocusTrigger Trigger {FocusTrigger::Other};
    FocusVisibility Visibility {FocusVisibility::Invisible};
    bool PreventScroll {false};
    Maybe<bool> FocusVisible;
  };
}
