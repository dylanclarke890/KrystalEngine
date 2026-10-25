#include "Krystal.Booey/Page/PointerLockController.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/HTML/Enums/TaskSource.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Page/PointerCaptureController.hpp"
#include "Krystal.Booey/Platform/PlatformMouseEvent.hpp"
#include "Krystal.Booey/Platform/PlatformWheelEvent.hpp"
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
    KRYS_NODISCARD bool HasTransientActivation(dom::Document &document) noexcept
    {
      auto *window = document.Window();
      return window && window->HasTransientActivation();
    }

    /// @brief Settles @p promise with @p result, as a microtask.
    void Settle(PointerLockCallback &&promise, dom::ExceptionOr<void> &&result) noexcept
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

  PointerLockController::PointerLockController(Page &page) noexcept : _page(WeakPtr<Page>::WithRef(page))
  {
  }

  PointerLockController::~PointerLockController() noexcept = default;

  void PointerLockController::RequestPointerLock(dom::Element *target,
                                                 Maybe<dom::PointerLockOptions> &&options,
                                                 PointerLockCallback &&promise) noexcept
  {
    if (!target || !target->IsConnected() || _documentOfRemovedElementWhileWaitingForUnlock)
    {
      EnqueueEvent(dom::DOMStringAtom(EventNames::PointerLockError), target);
      Settle(krys::move(promise), dom::Exception {dom::ExceptionCode::WrongDocumentError,
                                                  u8"Pointer lock target must be in an active document."});
      return;
    }

    if (_documentAllowedToRelockWithoutUserGesture.get() != &target->NodeDocument()
        && !HasTransientActivation(target->NodeDocument()))
    {
      EnqueueEvent(dom::DOMStringAtom(EventNames::PointerLockError), target);
      // If the request was not started from an engagement gesture and the Document has not previously
      // released a successful Pointer Lock with exitPointerLock():
      Settle(krys::move(promise),
             dom::Exception {dom::ExceptionCode::NotAllowedError, u8"Pointer lock requires a user gesture."});
      return;
    }

    // FIXME: The sandboxed pointer lock browsing context flag (a SecurityError), once there are sandboxed
    // frames.

    if (options && options->UnadjustedMovement && !SupportsUnadjustedMovement())
    {
      // If options["unadjustedMovement"] is true and the platform does not support unadjustedMovement:
      EnqueueEvent(dom::DOMStringAtom(EventNames::PointerLockError), target);
      Settle(krys::move(promise),
             dom::Exception {dom::ExceptionCode::NotSupportedError, u8"Unadjusted movement is unavailable."});
      return;
    }

    if (_element)
    {
      if (&_element->NodeDocument() != &target->NodeDocument())
      {
        EnqueueEvent(dom::DOMStringAtom(EventNames::PointerLockError), target);
        // If the user agent's pointer-lock target is an element whose shadow-including root is not equal to
        // this's shadow-including root, then:
        Settle(krys::move(promise),
               dom::Exception {dom::ExceptionCode::InvalidStateError,
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
        EnqueueEvent(dom::DOMStringAtom(EventNames::PointerLockChange), target);
        Settle(krys::move(promise), {});
        if (auto *page = _page.get())
        {
          page->GetPointerCaptureController().PointerLockWasApplied();
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
    auto *page = _page.get();
    if (!page)
    {
      return;
    }
    page->GetChromeClient().RequestPointerLock(
      [weakPage = WeakPtr<Page>(_page), target = ShareRefPtr(target)](PointerLockRequestResult result)
      {
        auto *page = weakPage.get();
        if (!page)
        {
          return;
        }
        auto &controller = page->GetPointerLockController();
        switch (result)
        {
          case PointerLockRequestResult::Success: controller.DidAcquirePointerLock(); break;
          case PointerLockRequestResult::Failure: controller.DidNotAcquirePointerLock(); break;
          case PointerLockRequestResult::Unsupported:
            controller.EnqueueEvent(dom::DOMStringAtom(EventNames::PointerLockError), target.get());
            controller.RejectPromises(dom::ExceptionCode::NotSupportedError,
                                      u8"Pointer lock is unavailable.");
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
    auto *page = _page.get();
    if (!page)
    {
      return;
    }
    page->GetChromeClient().RequestPointerUnlock(
      [weakPage = WeakPtr<Page>(_page)](bool result)
      {
        auto *page = weakPage.get();
        if (!page)
        {
          return;
        }
        if (result)
        {
          page->GetPointerLockController().DidLosePointerLock();
        }
        else
        {
          page->GetPointerLockController().DidNotAcquirePointerLock();
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
    _documentOfRemovedElementWhileWaitingForUnlock =
      WeakPtr<dom::Document>::WithRef(_element->NodeDocument());
    RequestPointerUnlock();
    // It is possible that the element is removed while pointer lock is still pending. This is essentially the
    // same situation as the !target->IsConnected() test in PointerLockController::RequestPointerLock, but it
    // has arisen in between then and now.
    RejectPromises(dom::ExceptionCode::WrongDocumentError, u8"Pointer lock target element was removed.");
    // Set element null immediately to block any future interaction with it including mouse events received
    // before the unlock completes.
    ClearElement();
  }

  void PointerLockController::DocumentDetached(dom::Document &document) noexcept
  {
    if (_documentAllowedToRelockWithoutUserGesture.get() == &document)
    {
      _documentAllowedToRelockWithoutUserGesture = nullptr;
    }

    if (_element && &_element->NodeDocument() == &document)
    {
      _documentOfRemovedElementWhileWaitingForUnlock =
        WeakPtr<dom::Document>::WithRef(_element->NodeDocument());
      RequestPointerUnlock();
      // It is possible that the document is detached while pointer lock is still pending.
      RejectPromises(dom::ExceptionCode::WrongDocumentError, u8"Pointer lock target document was detached.");
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

  dom::Element *PointerLockController::GetElement() const noexcept
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
    EnqueueEvent(dom::DOMStringAtom(EventNames::PointerLockChange), element.get());
    ResolvePromises();
    _lockPending = false;
    _documentAllowedToRelockWithoutUserGesture = WeakPtr<dom::Document>::WithRef(element->NodeDocument());

    // When a pointer lock is successfully applied on an element, the pointers' captures are released.
    if (auto *page = _page.get())
    {
      page->GetPointerCaptureController().PointerLockWasApplied();
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
    EnqueueEvent(dom::DOMStringAtom(EventNames::PointerLockError), element.get());
    RejectPromises(dom::ExceptionCode::WrongDocumentError,
                   u8"Pointer lock requires the window to have focus.");
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

    RefPtr<dom::Document> document = _element
                                       ? ShareRefPtr(&_element->NodeDocument())
                                       : ShareRefPtr(_documentOfRemovedElementWhileWaitingForUnlock.get());
    EnqueueEvent(dom::DOMStringAtom(EventNames::PointerLockChange), document.get());
    ClearElement();
    _unlockPending = false;
    _documentOfRemovedElementWhileWaitingForUnlock = nullptr;
  }

  void PointerLockController::DispatchLockedMouseEvent(const PlatformMouseEvent &event,
                                                       dom::DOMStringAtom eventType) noexcept
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
    if (eventType == EventNames::MouseUp)
    {
      (void)protectedElement->DispatchMouseEvent(event, dom::DOMStringAtom(EventNames::Click),
                                                 event.ClickCount());
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

  void PointerLockController::EnqueueEvent(dom::DOMStringAtom type, dom::Element *element) noexcept
  {
    if (element)
    {
      EnqueueEvent(type, &element->NodeDocument());
    }
  }

  void PointerLockController::EnqueueEvent(dom::DOMStringAtom type, dom::Document *document) noexcept
  {
    // FIXME: Spec doesn't specify which task source use.
    if (document)
    {
      document->QueueTaskToDispatchEvent(html::TaskSource::UserInteraction,
                                         dom::Event::Create(type, true, false));
    }
  }

  void PointerLockController::ResolvePromises() noexcept
  {
    for (auto &promise : std::exchange(_promises, {}))
    {
      Settle(krys::move(promise), {});
    }
  }

  void PointerLockController::RejectPromises(dom::ExceptionCode code, utf8_stringview reason) noexcept
  {
    for (auto &promise : std::exchange(_promises, {}))
    {
      Settle(krys::move(promise), dom::Exception {code, utf8_string(reason)});
    }
  }

  bool PointerLockController::SupportsUnadjustedMovement() const noexcept
  {
    auto *page = _page.get();
    return page && page->GetChromeClient().HasAccessoryMousePointingDevice();
  }
}
