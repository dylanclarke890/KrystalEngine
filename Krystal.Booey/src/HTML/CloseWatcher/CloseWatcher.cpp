#include "Krystal.Booey/HTML/CloseWatcher/CloseWatcher.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Algorithms/AbortAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Events/Event.hpp"
#include "Krystal.Booey/HTML/CloseWatcher/CloseWatcherManager.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"

namespace krys::boo::html
{
  CloseWatcher::CloseWatcher(dom::Document &document, CancelAction &&cancelAction, CloseAction &&closeAction,
                             GetEnabledState &&getEnabledState) noexcept
      : _document(CreateWeakPtr(&document)), _cancelAction(krys::move(cancelAction)),
        _closeAction(krys::move(closeAction)), _getEnabledState(krys::move(getEnabledState))
  {
  }

  // https://html.spec.whatwg.org/#dom-closewatcher
  dom::ExceptionOr<Ref<CloseWatcher>> CloseWatcher::Create(dom::Document &document,
                                                           const Options &options) noexcept
  {
    // 1. If this's relevant global object's associated Document is not fully active, then throw an
    //    "InvalidStateError" DOMException.
    if (!document.IsFullyActive())
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError, u8"Document is not fully active."};
    }

    // 2. Let closeWatcher be the result of establishing a close watcher given this's relevant global object,
    //    with:
    //    - cancelAction given canPreventClose being to return the result of firing an event named cancel at
    //      this, with the cancelable attribute initialized to canPreventClose.
    //    - closeAction being to fire an event named close at this.
    //    - getEnabledState being to return true.
    // 4. Set this's internal close watcher to closeWatcher.
    // (The CloseWatcher object is its own internal close watcher; its actions fire events at it.)
    Ref<CloseWatcher> watcher = Establish(document, nullptr, nullptr, [] { return true; });
    auto *self = watcher.get();
    watcher->_cancelAction = [self](bool canPreventClose)
    {
      auto cancelEvent = dom::Event::Create(dom::DOMStringAtom(EventNames::Cancel), false, canPreventClose);
      (void)dom::EventDispatcher::DispatchToTarget(*cancelEvent, self);
      return !cancelEvent->DefaultPrevented();
    };
    watcher->_closeAction = [self]
    {
      auto closeEvent = dom::Event::Create(dom::DOMStringAtom(EventNames::Close), false, false);
      (void)dom::EventDispatcher::DispatchToTarget(*closeEvent, self);
    };

    // 3. If options["signal"] exists:
    if (RefPtr<dom::AbortSignal> signal = options.Signal)
    {
      // 3.1. If options["signal"] is aborted, then destroy closeWatcher.
      if (signal->Aborted())
      {
        watcher->_active = false;
        document.Window()->GetCloseWatcherManager().Remove(*watcher);
      }
      // 3.2. Add the following steps to options["signal"]: Destroy closeWatcher.
      else
      {
        watcher->_signal = signal;
        dom::AbortAlgorithmCallback algorithm = [weakWatcher = CreateWeakPtr(watcher.get())](const Any &)
        {
          if (auto strongWatcher = weakWatcher.lock())
          {
            strongWatcher->Destroy();
          }
        };
        dom::AbortAlgorithms::Add(algorithm, *signal);
        watcher->_signalAlgorithm = krys::move(algorithm);
      }
    }

    return watcher;
  }

  // https://html.spec.whatwg.org/#establish-a-close-watcher
  Ref<CloseWatcher> CloseWatcher::Establish(dom::Document &document, CancelAction &&cancelAction,
                                            CloseAction &&closeAction,
                                            GetEnabledState &&getEnabledState) noexcept
  {
    // 1. Assert: window's associated Document is fully active.
    krys_debug_assert(document.IsFullyActive());

    // 2. Let closeWatcher be a new close watcher, with window window, cancel action cancelAction, close
    // action
    //    closeAction, is running cancel action false, get enabled state getEnabledState.
    // 3. Let manager be window's close watcher manager.
    // 4.-6. (CloseWatcherManager::Add.)
    // 7. Return closeWatcher.
    auto watcher = CreateRef<CloseWatcher>(document, krys::move(cancelAction), krys::move(closeAction),
                                           krys::move(getEnabledState));
    document.Window()->GetCloseWatcherManager().Add(watcher);
    return watcher;
  }

  // https://html.spec.whatwg.org/#dom-closewatcher-requestclose
  // The requestClose() method steps are to request to close this's internal close watcher with false.
  void CloseWatcher::RequestClose() noexcept
  {
    (void)RequestToClose(false);
  }

  // https://html.spec.whatwg.org/#close-watcher-request-close
  bool CloseWatcher::RequestToClose(bool requireHistoryActionActivation) noexcept
  {
    // 1. If closeWatcher is not active, then return true.
    // 2. If the result of running closeWatcher's get enabled state is false, then return true.
    // 3. If closeWatcher's is running cancel action is true, then return true.
    if (!IsActive() || !IsEnabled() || _isRunningCancelAction)
    {
      return true;
    }

    // 4. Let window be closeWatcher's window.
    // 5. If window's associated Document is not fully active, then return true.
    RefPtr<LocalDOMWindow> window = _document ? ShareRefPtr(_document->Window()) : nullptr;
    if (!window || !_document->IsFullyActive())
    {
      return true;
    }

    // 6. Let canPreventClose be true if requireHistoryActionActivation is false, or if window's close watcher
    //    manager's groups's size is less than window's close watcher manager's allowed number of groups, and
    //    window has history-action activation; otherwise false.
    bool canPreventClose =
      !requireHistoryActionActivation
      || (window->GetCloseWatcherManager().CanPreventClose() && window->HasHistoryActionActivation());
    Ref<CloseWatcher> protectedThis = ShareRef(*this);
    // 7. Set closeWatcher's is running cancel action to true.
    _isRunningCancelAction = true;
    // 8. Let shouldContinue be the result of running closeWatcher's cancel action given canPreventClose.
    bool shouldContinue = _cancelAction(canPreventClose);
    // 9. Set closeWatcher's is running cancel action to false.
    _isRunningCancelAction = false;
    // 10. If shouldContinue is false:
    if (!shouldContinue)
    {
      // 10.1. Assert: canPreventClose is true.
      krys_debug_assert(canPreventClose);
      // 10.2. Consume history-action user activation given window.
      (void)window->ConsumeHistoryActionUserActivation();
      // 10.3. Return false.
      return false;
    }

    // 11. Close closeWatcher.
    Close();
    // 12. Return true.
    return true;
  }

  // https://html.spec.whatwg.org/#close-watcher-close
  // (The close() method steps are to close this's internal close watcher.)
  void CloseWatcher::Close() noexcept
  {
    // 1. If closeWatcher is not active, then return.
    // 2. If the result of running closeWatcher's get enabled state is false, then return.
    // 3. If closeWatcher's window's associated Document is not fully active, then return.
    if (!IsActive() || !IsEnabled() || !_document || !_document->IsFullyActive())
    {
      return;
    }

    Ref<CloseWatcher> protectedThis = ShareRef(*this);
    // 4. Destroy closeWatcher.
    Destroy();
    // 5. Run closeWatcher's close action.
    _closeAction();
  }

  // https://html.spec.whatwg.org/#close-watcher-destroy
  // (The destroy() method steps are to destroy this's internal close watcher.)
  void CloseWatcher::Destroy() noexcept
  {
    // (A close watcher is active while it's in its manager's groups, which _active tracks.)
    if (!IsActive())
    {
      return;
    }

    // 1. Let manager be closeWatcher's window's close watcher manager.
    // 2.-3. (CloseWatcherManager::Remove.)

    if (_document && _document->Window())
    {
      _document->Window()->GetCloseWatcherManager().Remove(*this);
    }

    _active = false;

    if (_signal && _signalAlgorithm)
    {
      dom::AbortAlgorithms::Remove(*_signalAlgorithm, *_signal);
    }
  }
}
