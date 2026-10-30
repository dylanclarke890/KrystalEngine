#include "Krystal.Booey/Page/NavigateEvent.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/AbortController.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/AbortAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/DOMFormData.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Loader/HistoryController.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"

namespace krys::boo
{
  NavigateEvent::NavigateEvent(dom::DOMStringAtom type, const NavigateEventInit &init) noexcept
      : Event(type, init), _navigationType(init.NavigationType), _destination(init.Destination),
        _signal(init.Signal), _formData(init.FormData), _downloadRequest(init.DownloadRequest),
        _info(init.Info), _sourceElement(init.SourceElement), _canIntercept(init.CanIntercept),
        _userInitiated(init.UserInitiated), _hashChange(init.HashChange),
        _hasUAVisualTransition(init.HasUAVisualTransition)
  {
  }

  NavigateEvent::~NavigateEvent() noexcept = default;

  Ref<NavigateEvent> NavigateEvent::Create(const NavigateEventInit &init,
                                           UniquePtr<dom::AbortController> &&abortController) noexcept
  {
    auto event = CreateRef<NavigateEvent>(dom::DOMStringAtom(EventNames::Navigate), init);
    event->_abortController = krys::move(abortController);
    event->_isTrusted = true;
    return event;
  }

  dom::AbortSignal *NavigateEvent::Signal() const noexcept
  {
    return _signal.get();
  }

  html::DOMFormData *NavigateEvent::FormData() const noexcept
  {
    return _formData.get();
  }

  dom::Element *NavigateEvent::SourceElement() const noexcept
  {
    return _sourceElement.get();
  }

  void NavigateEvent::SignalAbort(const dom::Exception &exception) noexcept
  {
    if (_abortController)
    {
      dom::AbortAlgorithms::SignalAbort(*_abortController, krys::Any(dom::Exception(exception)));
    }
  }

  dom::ExceptionOr<void> NavigateEvent::SharedChecks(dom::Document &document) const noexcept
  {
    if (!document.IsFullyActive())
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError, u8"Document is not fully active"};
    }
    if (!IsTrusted())
    {
      return dom::Exception {dom::ExceptionCode::SecurityError, u8"Event is not trusted"};
    }
    if (DefaultPrevented())
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError, u8"Event was already canceled"};
    }
    return {};
  }

  dom::ExceptionOr<void> NavigateEvent::Intercept(dom::Document &document,
                                                  NavigationInterceptOptions &&options) noexcept
  {
    if (auto checkResult = SharedChecks(document); checkResult.HasException())
    {
      return checkResult;
    }

    if (!CanIntercept())
    {
      return dom::Exception {dom::ExceptionCode::SecurityError, u8"Event is not interceptable"};
    }

    if (!IsBeingDispatched())
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError, u8"Event is not being dispatched"};
    }

    krys_debug_assert(!_interceptionState || _interceptionState == InterceptionState::Intercepted);

    if (options.Handler)
    {
      _handlers.push_back(krys::move(options.Handler));
    }

    // FIXME: Warning in the console if the focus reset or the scroll behavior was already set, once there's a
    // console.
    if (options.FocusReset)
    {
      _focusReset = options.FocusReset;
    }

    if (options.Scroll)
    {
      _scrollBehavior = options.Scroll;
    }

    _interceptionState = InterceptionState::Intercepted;
    return {};
  }

  void NavigateEvent::ProcessScrollBehavior(dom::Document &document) noexcept
  {
    krys_debug_assert(_interceptionState == InterceptionState::Committed);
    _interceptionState = InterceptionState::Scrolled;

    auto *frame = document.Frame();
    if (!frame)
    {
      return;
    }

    if (_navigationType == NavigationNavigationType::Traverse)
    {
      frame->Loader().History().RestoreScrollPositionAndViewState();
      return;
    }

    if (!document.GetURL().HasFragmentIdentifier())
    {
      if (_navigationType == NavigationNavigationType::Reload)
      {
        frame->Loader().History().RestoreScrollPositionAndViewState();
      }
      else if (auto *view = frame->View())
      {
        view->SetScrollPosition(IntPoint(0, 0));
      }
      return;
    }

    // LocalFrameView::scrollToFragment() will fail only when anchor is not found if the url has fragment and
    // the fragment is not text fragment.
    if (!document.FindAnchor(document.GetURL().FragmentIdentifier()))
    {
      if (_navigationType == NavigationNavigationType::Reload)
      {
        frame->Loader().History().RestoreScrollPositionAndViewState();
      }
      return;
    }

    if (!document.HaveStylesheetsLoaded())
    {
      document.SetGotoAnchorNeededAfterStylesheetsLoad(true);
    }
    else if (auto *view = frame->View())
    {
      (void)view->ScrollToFragment(document.GetURL());
    }
  }

  dom::ExceptionOr<void> NavigateEvent::Scroll(dom::Document &document) noexcept
  {
    if (auto checkResult = SharedChecks(document); checkResult.HasException())
    {
      return checkResult;
    }

    if (_interceptionState != InterceptionState::Committed)
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError, u8"Interception has not been committed"};
    }

    ProcessScrollBehavior(document);
    return {};
  }

  void NavigateEvent::PotentiallyProcessScrollBehavior(dom::Document &document) noexcept
  {
    krys_debug_assert(_interceptionState == InterceptionState::Committed
                      || _interceptionState == InterceptionState::Scrolled);
    if (_interceptionState == InterceptionState::Scrolled
        || _scrollBehavior == NavigationScrollBehavior::Manual)
    {
      return;
    }
    ProcessScrollBehavior(document);
  }

  void NavigateEvent::Finish(dom::Document &document, InterceptionHandlersDidFulfill didFulfill,
                             FocusDidChange focusChanged) noexcept
  {
    krys_debug_assert(_interceptionState != InterceptionState::Intercepted
                      && _interceptionState != InterceptionState::Finished);
    if (!_interceptionState)
    {
      return;
    }

    krys_debug_assert(_interceptionState == InterceptionState::Committed
                      || _interceptionState == InterceptionState::Scrolled);
    if (focusChanged == FocusDidChange::No && _focusReset != NavigationFocusReset::Manual)
    {
      if (RefPtr<dom::Element> documentElement = ShareRefPtr(dom::TreeQueries::DocumentElement(document)))
      {
        RefPtr<dom::Node> focusTarget =
          ShareRefPtr(html::FocusAlgorithms::AutofocusDelegate(*documentElement, dom::FocusTrigger::Other));
        if (!focusTarget)
        {
          focusTarget = ShareRefPtr<dom::Node>(document.BodyOrFrameset());
        }
        if (!focusTarget)
        {
          focusTarget = documentElement;
        }
        html::FocusAlgorithms::FocusingSteps(*focusTarget);
      }
    }

    if (didFulfill == InterceptionHandlersDidFulfill::Yes)
    {
      PotentiallyProcessScrollBehavior(document);
    }
    _interceptionState = InterceptionState::Finished;
  }
}
