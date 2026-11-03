#pragma once

#include "Krystal.Booey/DOM/Events/ToggleEventTask.hpp"
#include "Krystal.Booey/HTML/CloseWatcher/CloseWatcher.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::html
{
  class HTMLElement;
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

namespace krys::boo::dom
{
  class Element;

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
    WeakPtr<html::HTMLElement> _invoker;
    RefPtr<html::CloseWatcher> _closeWatcher;
    bool _isHidingPopover {false};

  public:
    PopoverData() noexcept = default;

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

    KRYS_NODISCARD Element *PreviouslyFocusedElement() const noexcept
    {
      return _previouslyFocusedElement.get();
    }

    void SetPreviouslyFocusedElement(Element *element) noexcept
    {
      _previouslyFocusedElement = CreateWeakPtr(element);
    }

    KRYS_NODISCARD Ref<ToggleEventTask> EnsureToggleEventTask(Element &element) noexcept;

    /// @brief The element that showed the popover (its popover trigger).
    /// @see https://html.spec.whatwg.org/#popover-trigger
    KRYS_NODISCARD html::HTMLElement *Invoker() const noexcept
    {
      return _invoker.get();
    }

    void SetInvoker(html::HTMLElement *element) noexcept
    {
      _invoker = CreateWeakPtr(element);
    }

    /// @see https://html.spec.whatwg.org/#popover-close-watcher
    KRYS_NODISCARD html::CloseWatcher *CloseWatcher() const noexcept
    {
      return _closeWatcher.get();
    }

    void SetCloseWatcher(RefPtr<html::CloseWatcher> &&closeWatcher) noexcept
    {
      _closeWatcher = krys::move(closeWatcher);
    }

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
