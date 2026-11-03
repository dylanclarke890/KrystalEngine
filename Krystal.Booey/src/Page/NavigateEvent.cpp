#include "Krystal.Booey/Page/NavigateEvent.hpp"
#include "Krystal.Booey/DOM/Abort/AbortController.hpp"
#include "Krystal.Booey/DOM/Abort/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/AbortAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Booey/HTML/Forms/DOMFormData.hpp"
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

  // https://html.spec.whatwg.org/#navigateevent-perform-shared-checks
  dom::ExceptionOr<void> NavigateEvent::SharedChecks(dom::Document &document) const noexcept
  {
    // 1. If event's relevant global object's associated Document is not fully active, then throw an
    //    "InvalidStateError" DOMException.
    if (!document.IsFullyActive())
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError, u8"Document is not fully active"};
    }
    // 2. If event's isTrusted attribute was initialized to false, then throw a "SecurityError" DOMException.
    if (!IsTrusted())
    {
      return dom::Exception {dom::ExceptionCode::SecurityError, u8"Event is not trusted"};
    }
    // 3. If event's canceled flag is set, then throw an "InvalidStateError" DOMException.
    if (DefaultPrevented())
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError, u8"Event was already canceled"};
    }
    return {};
  }

  // https://html.spec.whatwg.org/#dom-navigateevent-intercept
  dom::ExceptionOr<void> NavigateEvent::Intercept(dom::Document &document,
                                                  NavigationInterceptOptions &&options) noexcept
  {
    // 1. Perform shared checks given this.
    if (auto checkResult = SharedChecks(document); checkResult.HasException())
    {
      return checkResult;
    }

    // 2. If this's canIntercept attribute was initialized to false, then throw a "SecurityError"
    // DOMException.
    if (!CanIntercept())
    {
      return dom::Exception {dom::ExceptionCode::SecurityError, u8"Event is not interceptable"};
    }

    // 3. If this's dispatch flag is unset, then throw an "InvalidStateError" DOMException.
    if (!IsBeingDispatched())
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError, u8"Event is not being dispatched"};
    }

    // 4. If options["precommitHandler"] exists:
    // 4a. If this's cancelable attribute is initialized to false, then throw an "InvalidStateError"
    //     DOMException.
    // 4b. Append options["precommitHandler"] to this's navigation precommit handler list.
    // FIXME: Precommit handlers.

    // 5. Assert: this's interception state is either "none" or "intercepted".
    krys_debug_assert(!_interceptionState || _interceptionState == InterceptionState::Intercepted);

    // 6. Set this's interception state to "intercepted".
    _interceptionState = InterceptionState::Intercepted;

    // 7. If options["handler"] exists, then append it to this's navigation handler list.
    if (options.Handler)
    {
      _handlers.push_back(krys::move(options.Handler));
    }

    // 8. If options["focusReset"] exists:
    // 8a. If this's focus reset behavior is not null, and it is not equal to options["focusReset"], then the
    //     user agent may report a warning to the console indicating that the focusReset option for a previous
    //     call to intercept() was overridden by this new value, and the previous value will be ignored.
    // 8b. Set this's focus reset behavior to options["focusReset"].
    if (options.FocusReset)
    {
      _focusReset = options.FocusReset;
    }

    // 9. If options["scroll"] exists:
    // 9a. If this's scroll behavior is not null, and it is not equal to options["scroll"], then the user
    // agent
    //     may report a warning to the console indicating that the scroll option for a previous call to
    //     intercept() was overridden by this new value, and the previous value will be ignored.
    // 9b. Set this's scroll behavior to options["scroll"].
    // (There's no console for the warnings.)
    if (options.Scroll)
    {
      _scrollBehavior = options.Scroll;
    }
    return {};
  }

  // https://html.spec.whatwg.org/#process-scroll-behavior
  void NavigateEvent::ProcessScrollBehavior(dom::Document &document) noexcept
  {
    // 1. Assert: event's interception state is "committed".
    krys_debug_assert(_interceptionState == InterceptionState::Committed);
    // 2. Set event's interception state to "scrolled".
    _interceptionState = InterceptionState::Scrolled;

    auto *frame = document.Frame();
    if (!frame)
    {
      return;
    }

    // 3. If event's navigationType was initialized to "traverse", then restore scroll position data given
    //    event's relevant global object's navigable's active session history entry.
    if (_navigationType == NavigationNavigationType::Traverse)
    {
      frame->Loader().History().RestoreScrollPositionAndViewState();
      return;
    }

    // 4. Otherwise:
    // 4a. Let document be event's relevant global object's associated Document.
    // 4b. If document's indicated part is null, then scroll to the beginning of the document given document.
    // (A reload restores the scroll position instead.)
    auto scrollToBeginning = [&]
    {
      if (_navigationType == NavigationNavigationType::Reload)
      {
        frame->Loader().History().RestoreScrollPositionAndViewState();
      }
      else if (auto *view = frame->View())
      {
        view->SetScrollPosition(IntPoint(0, 0));
      }
    };
    if (!document.GetURL().HasFragmentIdentifier()
        || !document.FindAnchor(document.GetURL().FragmentIdentifier()))
    {
      scrollToBeginning();
      return;
    }

    // 4c. Otherwise, scroll to the fragment given document.

    if (!document.HaveStylesheetsLoaded())
    {
      document.SetGotoAnchorNeededAfterStylesheetsLoad(true);
    }
    else if (auto *view = frame->View())
    {
      (void)view->ScrollToFragment(document.GetURL());
    }
  }

  // https://html.spec.whatwg.org/#dom-navigateevent-scroll
  dom::ExceptionOr<void> NavigateEvent::Scroll(dom::Document &document) noexcept
  {
    // 1. Perform shared checks given this.
    if (auto checkResult = SharedChecks(document); checkResult.HasException())
    {
      return checkResult;
    }

    // 2. If this's interception state is not "committed", then throw an "InvalidStateError" DOMException.
    if (_interceptionState != InterceptionState::Committed)
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError, u8"Interception has not been committed"};
    }

    // 3. Process scroll behavior given this.
    ProcessScrollBehavior(document);
    return {};
  }

  // https://html.spec.whatwg.org/#potentially-process-scroll-behavior
  void NavigateEvent::PotentiallyProcessScrollBehavior(dom::Document &document) noexcept
  {
    // 1. Assert: event's interception state is "committed" or "scrolled".
    krys_debug_assert(_interceptionState == InterceptionState::Committed
                      || _interceptionState == InterceptionState::Scrolled);
    // 2. If event's interception state is "scrolled", then return.
    // 3. If event's scroll behavior is "manual", then return.
    if (_interceptionState == InterceptionState::Scrolled
        || _scrollBehavior == NavigationScrollBehavior::Manual)
    {
      return;
    }
    // 4. Process scroll behavior given event.
    ProcessScrollBehavior(document);
  }

  // https://html.spec.whatwg.org/#navigateevent-finish
  void NavigateEvent::Finish(dom::Document &document, InterceptionHandlersDidFulfill didFulfill,
                             FocusDidChange focusChanged) noexcept
  {
    // 1. Assert: event's interception state is not "finished".
    // 2. If event's interception state is "intercepted": (Only precommit handlers, which aren't supported,
    //    finish an event before it's committed.)
    krys_debug_assert(_interceptionState != InterceptionState::Intercepted
                      && _interceptionState != InterceptionState::Finished);
    // 3. If event's interception state is "none", then return.
    if (!_interceptionState)
    {
      return;
    }

    // 4. Potentially reset the focus given event.
    // https://html.spec.whatwg.org/#potentially-reset-the-focus
    // 1. Assert: event's interception state is "committed" or "scrolled".
    krys_debug_assert(_interceptionState == InterceptionState::Committed
                      || _interceptionState == InterceptionState::Scrolled);
    // 2. Let navigation be event's relevant global object's navigation API.
    // 3. Let focusChanged be navigation's focus changed during ongoing navigation.
    // 4. Set navigation's focus changed during ongoing navigation to false.
    // (The navigation passes focusChanged, having reset it.)
    // 5. If focusChanged is true, then return.
    // 6. If event's focus reset behavior is "manual", then return.
    if (focusChanged == FocusDidChange::No && _focusReset != NavigationFocusReset::Manual)
    {
      // 7. Let document be event's relevant global object's associated Document.
      // 8. Let focusTarget be the autofocus delegate for document.
      RefPtr<dom::Node> focusTarget =
        ShareRefPtr(html::FocusAlgorithms::AutofocusDelegate(document, dom::FocusTrigger::Other));
      // 9. If focusTarget is null, then set focusTarget to document's body element.
      if (!focusTarget)
      {
        focusTarget = ShareRefPtr<dom::Node>(document.BodyOrFrameset());
      }
      // 10. If focusTarget is null, then set focusTarget to document's document element.
      if (!focusTarget)
      {
        focusTarget = ShareRefPtr<dom::Node>(dom::TreeQueries::DocumentElement(document));
      }
      if (focusTarget)
      {
        // 11. Run the focusing steps for focusTarget, with document's viewport as the fallback target.
        html::FocusAlgorithms::FocusingSteps(*focusTarget, &document);
        // 12. Move the sequential focus navigation starting point to focusTarget.
        document.SetSequentialFocusNavigationStartingPoint(focusTarget.get());
      }
    }

    // 5. If didFulfill is true, then potentially process scroll behavior given event.
    if (didFulfill == InterceptionHandlersDidFulfill::Yes)
    {
      PotentiallyProcessScrollBehavior(document);
    }
    // 6. Set event's interception state to "finished".
    _interceptionState = InterceptionState::Finished;
  }
}
