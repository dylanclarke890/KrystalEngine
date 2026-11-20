/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Input/PointerLockController.hpp"
#include "Krystal.Booey/DOM/Events/Event.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/HTML/EventLoop/TaskSource.hpp"
#include "Krystal.Booey/HTML/Window.hpp"
#include "Krystal.Booey/Input/PlatformMouseEvent.hpp"
#include "Krystal.Booey/Input/PlatformWheelEvent.hpp"
#include "Krystal.Booey/Input/PointerCaptureController.hpp"
#include "Krystal.Booey/Page/Clients/ChromeClient.hpp"
#include "Krystal.Booey/Page/Frame.hpp"
#include <utility>

namespace krys::boo
{
  namespace
  {
    class UnadjustedMovementPlatformMouseEvent : public PlatformMouseEvent
    {
    public:
      explicit UnadjustedMovementPlatformMouseEvent(const PlatformMouseEvent &src) noexcept
          : PlatformMouseEvent(src)
      {
        _movementDelta = src.UnadjustedMovementDelta();
      }
    };

    /// @brief Whether @p document's window has transient activation: the request started from a user
    /// gesture.
    KRYS_NODISCARD bool HasTransientActivation(Document &document) noexcept
    {
      auto *window = document.Window();
      return window && window->HasTransientActivation();
    }

    /// @brief Settles @p promise with @p result, as a microtask.
    void Settle(PointerLockCallback &&promise, ExceptionOr<void> &&result) noexcept
    {
      if (!promise)
      {
        return;
      }
      html::EventLoop::Current().QueueMicrotask(
        [promise = krys::move(promise), result = krys::move(result)]() mutable
        { promise(krys::move(result)); });
    }
  }

  PointerLockController::PointerLockController(Frame &frame) noexcept : _frame(CreateWeakPtr(&frame))
  {
  }

  PointerLockController::~PointerLockController() noexcept = default;

  void PointerLockController::RequestPointerLock(Element *target, Maybe<dom::PointerLockOptions> &&options,
                                                 PointerLockCallback &&promise) noexcept
  {
    if (!target || !target->IsConnected() || _documentOfRemovedElementWhileWaitingForUnlock)
    {
      EnqueueEvent(event_names::PointerLockError, target);
      Settle(krys::move(promise), Exception {ExceptionCode::WrongDocumentError,
                                             u8"Pointer lock target must be in an active document."});
      return;
    }

    if (_documentAllowedToRelockWithoutUserGesture.get() != &target->NodeDocument()
        && !HasTransientActivation(target->NodeDocument()))
    {
      EnqueueEvent(event_names::PointerLockError, target);
      // If the request was not started from an engagement gesture and the Document has not previously
      // released a successful Pointer Lock with exitPointerLock():
      Settle(krys::move(promise),
             Exception {ExceptionCode::NotAllowedError, u8"Pointer lock requires a user gesture."});
      return;
    }

    if (options && options->UnadjustedMovement && !SupportsUnadjustedMovement())
    {
      // If options["unadjustedMovement"] is true and the platform does not support unadjustedMovement:
      EnqueueEvent(event_names::PointerLockError, target);
      Settle(krys::move(promise),
             Exception {ExceptionCode::NotSupportedError, u8"Unadjusted movement is unavailable."});
      return;
    }

    if (_element)
    {
      if (&_element->NodeDocument() != &target->NodeDocument())
      {
        EnqueueEvent(event_names::PointerLockError, target);
        // If the user agent's pointer-lock target is an element whose shadow-including root is not equal to
        // this's shadow-including root, then:
        Settle(krys::move(promise),
               Exception {ExceptionCode::InvalidStateError,
                          u8"Pointer lock cannot be moved to an element in a different document."});
        return;
      }
      _element = ShareRefPtr(target);
      _options = krys::move(options);
      if (_lockPending)
      {
        // _lockPending means an answer from the ChromeClient for a previous RequestPointerLock on the page is
        // pending. It's currently unknown which way that will go, whether the request will be approved or
        // rejected. Therefore queue the promise for later when that's known and don't send out any
        // pointerlockchange event yet.
        if (promise)
        {
          _promises.push_back(krys::move(promise));
        }
      }
      else
      {
        // !_lockPending means the pointer lock is currently held on the page, and page is just re-targeting
        // it or changing the options.
        EnqueueEvent(event_names::PointerLockChange, target);
        Settle(krys::move(promise), {});
        if (auto *frame = _frame.get())
        {
          frame->GetPointerCaptureController().PointerLockWasApplied();
        }
      }
      return;
    }

    _lockPending = true;
    _element = ShareRefPtr(target);
    _options = krys::move(options);
    if (promise)
    {
      _promises.push_back(krys::move(promise));
    }
    auto *frame = _frame.get();
    if (!frame)
    {
      return;
    }
    frame->GetChromeClient().RequestPointerLock(
      [weakFrame = WeakPtr<Frame>(_frame), target = ShareRefPtr(target)](PointerLockRequestResult result)
      {
        auto *frame = weakFrame.get();
        if (!frame)
        {
          return;
        }
        auto &controller = frame->GetPointerLockController();
        switch (result)
        {
          case PointerLockRequestResult::Success: controller.DidAcquirePointerLock(); break;
          case PointerLockRequestResult::Failure: controller.DidNotAcquirePointerLock(); break;
          case PointerLockRequestResult::Unsupported:
            controller.EnqueueEvent(event_names::PointerLockError, target.get());
            controller.RejectPromises(ExceptionCode::NotSupportedError, u8"Pointer lock is unavailable.");
            controller.ClearElement();
            break;
        }
      });
  }

  void PointerLockController::RequestPointerUnlock() noexcept
  {
    if (!_element)
    {
      return;
    }

    _unlockPending = true;
    auto *frame = _frame.get();
    if (!frame)
    {
      return;
    }
    frame->GetChromeClient().RequestPointerUnlock(
      [weakFrame = WeakPtr<Frame>(_frame)](bool result)
      {
        auto *frame = weakFrame.get();
        if (!frame)
        {
          return;
        }
        if (result)
        {
          frame->GetPointerLockController().DidLosePointerLock();
        }
        else
        {
          frame->GetPointerLockController().DidNotAcquirePointerLock();
        }
      });
  }

  void PointerLockController::RequestPointerUnlockAndForceCursorVisible() noexcept
  {
    _documentAllowedToRelockWithoutUserGesture = nullptr;
    RequestPointerUnlock();
  }

  void PointerLockController::ElementWasRemovedInternal() noexcept
  {
    _documentOfRemovedElementWhileWaitingForUnlock = CreateWeakPtr(&_element->NodeDocument());
    RequestPointerUnlock();
    // It is possible that the element is removed while pointer lock is still pending. This is essentially the
    // same situation as the !target->IsConnected() test in PointerLockController::RequestPointerLock, but it
    // has arisen in between then and now.
    RejectPromises(ExceptionCode::WrongDocumentError, u8"Pointer lock target element was removed.");
    // Set element null immediately to block any future interaction with it including mouse events received
    // before the unlock completes.
    ClearElement();
  }

  void PointerLockController::DocumentDetached(Document &document) noexcept
  {
    if (_documentAllowedToRelockWithoutUserGesture.get() == &document)
    {
      _documentAllowedToRelockWithoutUserGesture = nullptr;
    }

    if (_element && &_element->NodeDocument() == &document)
    {
      _documentOfRemovedElementWhileWaitingForUnlock = CreateWeakPtr(&_element->NodeDocument());
      RequestPointerUnlock();
      // It is possible that the document is detached while pointer lock is still pending.
      RejectPromises(ExceptionCode::WrongDocumentError, u8"Pointer lock target document was detached.");
      ClearElement();
    }
  }

  bool PointerLockController::IsLocked() const noexcept
  {
    return _element && !_lockPending;
  }

  bool PointerLockController::LockPending() const noexcept
  {
    return _lockPending;
  }

  Element *PointerLockController::GetElement() const noexcept
  {
    return _element.get();
  }

  void PointerLockController::DidAcquirePointerLock() noexcept
  {
    if (!_lockPending)
    {
      return;
    }

    krys_debug_assert(_element);
    RefPtr element = _element;
    EnqueueEvent(event_names::PointerLockChange, element.get());
    ResolvePromises();
    _lockPending = false;
    _documentAllowedToRelockWithoutUserGesture = CreateWeakPtr(&element->NodeDocument());

    // When a pointer lock is successfully applied on an element, the pointers' captures are released.
    if (auto *frame = _frame.get())
    {
      frame->GetPointerCaptureController().PointerLockWasApplied();
    }
  }

  void PointerLockController::DidNotAcquirePointerLock() noexcept
  {
    // DidNotAcquirePointerLock is sent in response to ChromeClient::RequestPointerLock if the window does not
    // have focus or the host is not allowing pointer lock. From the draft spec: If the this's
    // shadow-including root is the active document of a browsing context which is not (or has an ancestor
    // browsing context which is not) in focus by a window which is in focus by the operating system's window
    // manager:
    // 1. Fire an event named pointerlockerror at this's node document.
    // 2. Reject promise with a "WrongDocumentError" DOMException.
    RefPtr element = _element;
    EnqueueEvent(event_names::PointerLockError, element.get());
    RejectPromises(ExceptionCode::WrongDocumentError, u8"Pointer lock requires the window to have focus.");
    ClearElement();
    _unlockPending = false;
    _documentOfRemovedElementWhileWaitingForUnlock = nullptr;
  }

  void PointerLockController::DidLosePointerLock() noexcept
  {
    if (!_unlockPending)
    {
      _documentAllowedToRelockWithoutUserGesture = nullptr;
    }

    RefPtr<Document> document = _element ? ShareRefPtr(&_element->NodeDocument())
                                         : ShareRefPtr(_documentOfRemovedElementWhileWaitingForUnlock.get());
    EnqueueEvent(event_names::PointerLockChange, document.get());
    ClearElement();
    _unlockPending = false;
    _documentOfRemovedElementWhileWaitingForUnlock = nullptr;
  }

  void PointerLockController::DispatchLockedMouseEvent(const PlatformMouseEvent &event,
                                                       const DOMStringAtom &eventType) noexcept
  {
    if (!_element || !_element->NodeDocument().Frame())
    {
      return;
    }

    Ref protectedElement = ShareRef(*_element);
    if (_options && _options->UnadjustedMovement)
    {
      (void)protectedElement->DispatchMouseEvent(UnadjustedMovementPlatformMouseEvent(event), eventType,
                                                 event.ClickCount());
    }
    else
    {
      (void)protectedElement->DispatchMouseEvent(event, eventType, event.ClickCount());
    }

    // Create click events
    if (eventType == event_names::MouseUp)
    {
      (void)protectedElement->DispatchMouseEvent(event, event_names::Click, event.ClickCount());
    }
  }

  void PointerLockController::DispatchLockedWheelEvent(const PlatformWheelEvent &event) noexcept
  {
    if (!_element || !_element->NodeDocument().Frame())
    {
      return;
    }

    EnumSet<EventHandling> defaultHandling;
    Ref protectedElement = ShareRef(*_element);
    (void)protectedElement->DispatchWheelEvent(event, defaultHandling);
  }

  void PointerLockController::ClearElement() noexcept
  {
    _lockPending = false;
    _element = nullptr;
    _options = null;
    krys_debug_assert(_promises.empty());
  }

  void PointerLockController::EnqueueEvent(DOMStringAtom type, Element *element) noexcept
  {
    if (element)
    {
      EnqueueEvent(krys::move(type), &element->NodeDocument());
    }
  }

  void PointerLockController::EnqueueEvent(DOMStringAtom type, Document *document) noexcept
  {
    // FIXME: Spec doesn't specify which task source use.
    if (document)
    {
      document->QueueTaskToDispatchEvent(html::TaskSource::UserInteraction,
                                         Event::Create(krys::move(type), true, false));
    }
  }

  void PointerLockController::ResolvePromises() noexcept
  {
    for (auto &promise : std::exchange(_promises, {}))
    {
      Settle(krys::move(promise), {});
    }
  }

  void PointerLockController::RejectPromises(ExceptionCode code, utf8_stringview reason) noexcept
  {
    for (auto &promise : std::exchange(_promises, {}))
    {
      Settle(krys::move(promise), Exception {code, utf8_string(reason)});
    }
  }

  bool PointerLockController::SupportsUnadjustedMovement() const noexcept
  {
    auto *frame = _frame.get();
    return frame && frame->GetChromeClient().HasAccessoryMousePointingDevice();
  }
}
