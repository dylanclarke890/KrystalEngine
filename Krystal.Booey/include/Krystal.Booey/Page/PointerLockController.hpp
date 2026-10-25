#pragma once

#include "Krystal.Booey/DOM/Dicts/PointerLockOptions.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/Exception.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Document;
  class Element;
}

namespace krys::boo
{
  class Page;
  class PlatformMouseEvent;
  class PlatformWheelEvent;

  /// @brief The host's answer to a request to lock the pointer.
  enum class PointerLockRequestResult : uint8
  {
    Success,
    Failure,
    Unsupported
  };

  /// @brief The promise `requestPointerLock()` returns, as a callback: called once, as a microtask, with
  /// nothing when the lock is applied, or with the exception the request failed with.
  using PointerLockCallback = Func<void(dom::ExceptionOr<void> &&)>;

  /// @brief A page's pointer lock: the element the mouse's events go to while it's locked
  /// (`requestPointerLock()`), with the cursor hidden and the mouse's movement as its moves' `movementX` and
  /// `movementY`, the `pointerlockchange` and `pointerlockerror` events, and unlocking (`exitPointerLock()`,
  /// Escape, the element going away). The host locks and unlocks the pointer (the chrome client's
  /// `RequestPointerLock()` and `RequestPointerUnlock()`).
  /// @see https://w3c.github.io/pointerlock/
  class PointerLockController
  {
    KRYS_NON_COPY_MOVABLE(PointerLockController);

  private:
    WeakPtr<Page> _page;
    bool _lockPending {false};
    bool _unlockPending {false};
    Maybe<dom::PointerLockOptions> _options;
    RefPtr<dom::Element> _element;
    List<PointerLockCallback> _promises;
    WeakPtr<dom::Document> _documentOfRemovedElementWhileWaitingForUnlock;
    WeakPtr<dom::Document> _documentAllowedToRelockWithoutUserGesture;

  public:
    explicit PointerLockController(Page &page) noexcept;
    ~PointerLockController() noexcept;

    /// @brief Asks to lock the pointer to @p target (or moves the lock there, if the pointer is locked to an
    /// element of its document); @p promise, if given, settles once the host has answered.
    /// @see https://w3c.github.io/pointerlock/#dom-element-requestpointerlock
    void RequestPointerLock(dom::Element *target, Maybe<dom::PointerLockOptions> &&options = null,
                            PointerLockCallback &&promise = {}) noexcept;

    /// @brief Asks the host to unlock the pointer, if it's locked.
    void RequestPointerUnlock() noexcept;
    /// @brief Asks the host to unlock the pointer, as the user does (Escape): the document must have a user
    /// gesture to lock it again.
    void RequestPointerUnlockAndForceCursorVisible() noexcept;

    /// @brief Unlocks the pointer if @p element has it.
    void ElementWasRemoved(dom::Element &element) noexcept
    {
      if (_element.get() == &element)
      {
        ElementWasRemovedInternal();
      }
    }

    /// @brief Unlocks the pointer if an element of @p document has it.
    void DocumentDetached(dom::Document &document) noexcept;

    /// @brief Whether the pointer is locked to an element (and the host has said so).
    KRYS_NODISCARD bool IsLocked() const noexcept;
    /// @brief Whether a lock is waiting for the host's answer.
    KRYS_NODISCARD bool LockPending() const noexcept;
    /// @brief The element the pointer is locked (or being locked) to.
    KRYS_NODISCARD dom::Element *GetElement() const noexcept;

    /// @brief The host has locked the pointer.
    void DidAcquirePointerLock() noexcept;
    /// @brief The host didn't lock the pointer (or didn't unlock it).
    void DidNotAcquirePointerLock() noexcept;
    /// @brief The host has unlocked the pointer (or lost it, as the window lost focus).
    void DidLosePointerLock() noexcept;

    /// @brief Dispatches the mouse event of type @p eventType from @p event at the locked element (and a
    /// `click` after a `mouseup`).
    void DispatchLockedMouseEvent(const PlatformMouseEvent &event, dom::DOMStringAtom eventType) noexcept;
    /// @brief Dispatches the `wheel` event from @p event at the locked element.
    void DispatchLockedWheelEvent(const PlatformWheelEvent &event) noexcept;

  private:
    void ClearElement() noexcept;
    void EnqueueEvent(dom::DOMStringAtom type, dom::Element *element) noexcept;
    void EnqueueEvent(dom::DOMStringAtom type, dom::Document *document) noexcept;
    void ResolvePromises() noexcept;
    void RejectPromises(dom::ExceptionCode code, utf8_stringview reason) noexcept;
    void ElementWasRemovedInternal() noexcept;
    KRYS_NODISCARD bool SupportsUnadjustedMovement() const noexcept;
  };
}
