/*
 * Copyright (C) 2020 Igalia S.L. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::dom
{
  /// @brief What focusing an element that has a selection of its own (a text control) does with it.
  enum class SelectionRestorationMode : uint8
  {
    RestoreOrSelectAll,
    SelectAll,
    PlaceCaretAtStart,
  };

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
