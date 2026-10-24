#include "Krystal.Booey/HTML/CloseWatcher/CloseWatcher.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Algorithms/AbortAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
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

  dom::ExceptionOr<Ref<CloseWatcher>> CloseWatcher::Create(dom::Document &document,
                                                           const Options &options) noexcept
  {
    if (!document.IsFullyActive())
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError, u8"Document is not fully active."};
    }

    // Its actions fire events at the watcher, which owns them.
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

    if (RefPtr<dom::AbortSignal> signal = options.Signal)
    {
      if (signal->Aborted())
      {
        watcher->_active = false;
        document.Window()->GetCloseWatcherManager().Remove(*watcher);
      }
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

  Ref<CloseWatcher> CloseWatcher::Establish(dom::Document &document, CancelAction &&cancelAction,
                                            CloseAction &&closeAction,
                                            GetEnabledState &&getEnabledState) noexcept
  {
    krys_debug_assert(document.IsFullyActive());

    auto watcher = CreateRef<CloseWatcher>(document, krys::move(cancelAction), krys::move(closeAction),
                                           krys::move(getEnabledState));
    document.Window()->GetCloseWatcherManager().Add(watcher);
    return watcher;
  }

  void CloseWatcher::RequestClose() noexcept
  {
    (void)RequestToClose(false);
  }

  bool CloseWatcher::RequestToClose(bool requireHistoryActionActivation) noexcept
  {
    if (!IsActive() || !IsEnabled() || _isRunningCancelAction)
    {
      return true;
    }

    RefPtr<LocalDOMWindow> window = _document ? ShareRefPtr(_document->Window()) : nullptr;
    if (!window || !_document->IsFullyActive())
    {
      return true;
    }

    bool canPreventClose =
      !requireHistoryActionActivation
      || (window->GetCloseWatcherManager().CanPreventClose() && window->HasHistoryActionActivation());
    Ref<CloseWatcher> protectedThis = ShareRef(*this);
    _isRunningCancelAction = true;
    bool shouldContinue = _cancelAction(canPreventClose);
    _isRunningCancelAction = false;
    if (!shouldContinue)
    {
      krys_debug_assert(canPreventClose);
      (void)window->ConsumeHistoryActionUserActivation();
      return false;
    }

    Close();
    return true;
  }

  void CloseWatcher::Close() noexcept
  {
    if (!IsActive() || !IsEnabled() || !_document || !_document->IsFullyActive())
    {
      return;
    }

    Ref<CloseWatcher> protectedThis = ShareRef(*this);
    Destroy();
    _closeAction();
  }

  void CloseWatcher::Destroy() noexcept
  {
    if (!IsActive())
    {
      return;
    }

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
