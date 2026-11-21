/*
 * Copyright (C) 2023-2024 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"

namespace krys::boo::dom
{
  class ToggleEventTask;
}

namespace krys::boo
{
  class Element;
  class HTMLElement;
}

namespace krys::boo::html
{
  class CloseWatcher;
}

namespace krys::boo::dom
{
  /// @brief The state of an element's `popover` attribute.
  /// @see https://html.spec.whatwg.org/#attr-popover
  enum class PopoverState : uint8
  {
    None,
    Auto,
    Manual,
    Hint,
  };

  /// @see https://html.spec.whatwg.org/#popover-visibility-state
  enum class PopoverVisibilityState : bool
  {
    Hidden,
    Showing,
  };

  /// @brief Whether hiding a popover returns the focus to the element focused before it showed.
  enum class FocusPreviousElement : bool
  {
    No,
    Yes,
  };

  /// @brief Whether showing or hiding a popover fires its `beforetoggle` and `toggle` events.
  enum class FireEvents : bool
  {
    No,
    Yes,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::dom::PopoverState, 4uz);

namespace krys::boo::dom
{
  /// @brief The popover state of an element with a `popover` attribute.
  /// @see https://html.spec.whatwg.org/#the-popover-attribute
  class PopoverData
  {
    KRYS_NON_COPY_MOVABLE(PopoverData);

  private:
    PopoverState _popoverState {PopoverState::None};
    PopoverVisibilityState _visibilityState {PopoverVisibilityState::Hidden};
    PopoverState _openedInPopoverMode {PopoverState::None};
    WeakPtr<Element> _previouslyFocusedElement;
    RefPtr<ToggleEventTask> _toggleEventTask;
    WeakPtr<HTMLElement> _invoker;
    RefPtr<html::CloseWatcher> _closeWatcher;
    bool _isHidingPopover {false};

  public:
    PopoverData() noexcept = default;

    ~PopoverData() noexcept;

    KRYS_NODISCARD PopoverState GetPopoverState() const noexcept
    {
      return _popoverState;
    }

    void SetPopoverState(PopoverState state) noexcept
    {
      _popoverState = state;
    }

    KRYS_NODISCARD PopoverVisibilityState VisibilityState() const noexcept
    {
      return _visibilityState;
    }

    void SetVisibilityState(PopoverVisibilityState visibilityState) noexcept
    {
      _visibilityState = visibilityState;
    }

    /// @brief The stack the popover opened in: `Auto` or `Hint` while it's showing in one, `None` otherwise.
    /// @see https://html.spec.whatwg.org/#opened-in-popover-mode
    KRYS_NODISCARD PopoverState OpenedInPopoverMode() const noexcept
    {
      return _openedInPopoverMode;
    }

    void SetOpenedInPopoverMode(PopoverState mode) noexcept
    {
      _openedInPopoverMode = mode;
    }

    KRYS_NODISCARD Element *PreviouslyFocusedElement() const noexcept;

    void SetPreviouslyFocusedElement(Element *element) noexcept;

    KRYS_NODISCARD Ref<ToggleEventTask> EnsureToggleEventTask(Element &element) noexcept;

    /// @brief The element that showed the popover (its popover trigger).
    /// @see https://html.spec.whatwg.org/#popover-trigger
    KRYS_NODISCARD HTMLElement *Invoker() const noexcept;

    void SetInvoker(HTMLElement *element) noexcept;

    /// @see https://html.spec.whatwg.org/#popover-close-watcher
    KRYS_NODISCARD html::CloseWatcher *CloseWatcher() const noexcept
    {
      return _closeWatcher.get();
    }

    void SetCloseWatcher(RefPtr<html::CloseWatcher> &&closeWatcher) noexcept;

    /// @see https://html.spec.whatwg.org/#popover-hiding
    KRYS_NODISCARD bool IsHidingPopover() const noexcept
    {
      return _isHidingPopover;
    }

    void SetIsHidingPopover(bool value) noexcept
    {
      _isHidingPopover = value;
    }
  };
}
