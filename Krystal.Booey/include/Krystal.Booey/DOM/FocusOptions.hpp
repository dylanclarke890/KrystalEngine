#pragma once

#include "Krystal.Booey/DOM/Enums/SelectionRestorationMode.hpp"
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

  /// @brief Which way focus moves: sequentially (Tab and Shift+Tab) or spatially.
  enum class FocusDirection : uint8
  {
    None,
    Forward,
    Backward,
    Up,
    Down,
    Left,
    Right
  };

  /// @brief Whether a focused element matches `:focus-visible`.
  enum class FocusVisibility : uint8
  {
    Invisible,
    Visible,
    ForceInvisible
  };

  /// @brief Where the sequential focus navigation starting point is relative to its node.
  enum class SequentialFocusNavigationStartingPointPosition : uint8
  {
    /// @brief At the node: navigation goes to the areas after it, or before it.
    At,
    /// @brief Just after the node and its descendants (where a removed next sibling was).
    After,
    /// @brief At the start of the node's children (where a removed first child was).
    Inside
  };

  /// @brief How to focus an element: the `FocusOptions` dictionary and the engine's own options.
  /// @see https://html.spec.whatwg.org/#focusoptions
  struct FocusOptions
  {
    /// @brief What focusing an element with a selection of its own (a text control) does with it.
    SelectionRestorationMode SelectionRestoration {SelectionRestorationMode::RestoreOrSelectAll};
    FocusRemovalEventsMode RemovalEventsMode {FocusRemovalEventsMode::Dispatch};
    FocusDirection Direction {FocusDirection::None};
    FocusTrigger Trigger {FocusTrigger::Other};
    FocusVisibility Visibility {FocusVisibility::Invisible};
    bool PreventScroll {false};
    Maybe<bool> FocusVisible;
  };
}
