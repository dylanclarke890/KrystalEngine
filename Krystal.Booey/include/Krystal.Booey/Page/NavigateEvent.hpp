#pragma once

#include "Krystal.Booey/DOM/Dicts/EventInit.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/HTML/SerializedScriptValue.hpp"
#include "Krystal.Booey/Page/NavigationDestination.hpp"
#include "Krystal.Booey/Page/NavigationNavigationType.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::dom
{
  class AbortController;
  class AbortSignal;
  class Document;
  class Element;
}

namespace krys::boo::html
{
  class DOMFormData;
}

namespace krys::boo
{
  /// @brief A handler `intercept()` is given: it does the navigation's work (shows the destination's view),
  /// and calls its completion once it's done, at once or later, with an exception if it failed.
  /// @see https://html.spec.whatwg.org/#dom-navigationinterceptoptions-handler
  using NavigationInterceptHandler = Func<void(Func<void(dom::ExceptionOr<void>)> &&completion)>;

  enum class InterceptionState : uint8
  {
    Intercepted,
    Committed,
    Scrolled,
    Finished,
  };

  enum class InterceptionHandlersDidFulfill : bool
  {
    No,
    Yes
  };

  enum class FocusDidChange : bool
  {
    No,
    Yes
  };

  /// @see https://html.spec.whatwg.org/#navigateeventinit
  struct NavigateEventInit : dom::EventInit
  {
    NavigationNavigationType NavigationType {NavigationNavigationType::Push};
    RefPtr<NavigationDestination> Destination;
    RefPtr<dom::AbortSignal> Signal;
    RefPtr<html::DOMFormData> FormData;
    Maybe<dom::DOMString> DownloadRequest;
    RefPtr<html::SerializedScriptValue> Info;
    RefPtr<dom::Element> SourceElement;
    bool CanIntercept {false};
    bool UserInitiated {false};
    bool HashChange {false};
    bool HasUAVisualTransition {false};
  };

  /// @brief `navigate`: the window's frame is about to navigate. A listener can cancel it (unless it's a
  /// traversal to another document), or intercept a navigation within the document's origin, so that it
  /// commits without loading a document and its handlers do the work instead.
  /// @see https://html.spec.whatwg.org/#navigateevent
  class NavigateEvent final : public dom::Event
  {
  public:
    /// @brief The dictionary its constructor takes.
    using dictionary_type = NavigateEventInit;

    /// @see https://html.spec.whatwg.org/#navigationfocusreset
    enum class NavigationFocusReset : bool
    {
      AfterTransition,
      Manual,
    };

    /// @see https://html.spec.whatwg.org/#navigationscrollbehavior
    enum class NavigationScrollBehavior : bool
    {
      AfterTransition,
      Manual,
    };

    /// @see https://html.spec.whatwg.org/#navigationinterceptoptions
    struct NavigationInterceptOptions
    {
      NavigationInterceptHandler Handler;
      Maybe<NavigationFocusReset> FocusReset;
      Maybe<NavigationScrollBehavior> Scroll;
    };

  private:
    NavigationNavigationType _navigationType;
    RefPtr<NavigationDestination> _destination;
    RefPtr<dom::AbortSignal> _signal;
    RefPtr<html::DOMFormData> _formData;
    Maybe<dom::DOMString> _downloadRequest;
    List<NavigationInterceptHandler> _handlers;
    RefPtr<html::SerializedScriptValue> _info;
    RefPtr<dom::Element> _sourceElement;
    bool _canIntercept {false};
    bool _userInitiated {false};
    bool _hashChange {false};
    bool _hasUAVisualTransition {false};
    Maybe<InterceptionState> _interceptionState;
    Maybe<NavigationFocusReset> _focusReset;
    Maybe<NavigationScrollBehavior> _scrollBehavior;
    UniquePtr<dom::AbortController> _abortController;

  public:
    /// @see https://html.spec.whatwg.org/#dom-navigateevent-navigateevent
    NavigateEvent(dom::DOMStringAtom type, const NavigateEventInit &eventInitDict) noexcept;
    ~NavigateEvent() noexcept override;

    /// @brief A trusted `navigate`, whose signal is @p abortController's.
    KRYS_NODISCARD static Ref<NavigateEvent>
      Create(const NavigateEventInit &init, UniquePtr<dom::AbortController> &&abortController) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-navigateevent-navigationtype
    KRYS_NODISCARD NavigationNavigationType NavigationType() const noexcept
    {
      return _navigationType;
    }

    /// @see https://html.spec.whatwg.org/#dom-navigateevent-destination
    KRYS_NODISCARD NavigationDestination *Destination() const noexcept
    {
      return _destination.get();
    }

    /// @brief Aborted if the navigation is (a newer one starts, or it's stopped).
    /// @see https://html.spec.whatwg.org/#dom-navigateevent-signal
    KRYS_NODISCARD dom::AbortSignal *Signal() const noexcept;

    /// @brief The form's data, for a `POST` form submission.
    /// @see https://html.spec.whatwg.org/#dom-navigateevent-formdata
    KRYS_NODISCARD html::DOMFormData *FormData() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-navigateevent-downloadrequest
    KRYS_NODISCARD const Maybe<dom::DOMString> &DownloadRequest() const noexcept
    {
      return _downloadRequest;
    }

    /// @brief The `info` the navigation's method was given.
    /// @see https://html.spec.whatwg.org/#dom-navigateevent-info
    KRYS_NODISCARD html::SerializedScriptValue *Info() const noexcept
    {
      return _info.get();
    }

    /// @see https://html.spec.whatwg.org/#dom-navigateevent-sourceelement
    KRYS_NODISCARD dom::Element *SourceElement() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-navigateevent-canintercept
    KRYS_NODISCARD bool CanIntercept() const noexcept
    {
      return _canIntercept;
    }

    /// @see https://html.spec.whatwg.org/#dom-navigateevent-userinitiated
    KRYS_NODISCARD bool UserInitiated() const noexcept
    {
      return _userInitiated;
    }

    /// @see https://html.spec.whatwg.org/#dom-navigateevent-hashchange
    KRYS_NODISCARD bool HashChange() const noexcept
    {
      return _hashChange;
    }

    /// @see https://html.spec.whatwg.org/#dom-navigateevent-hasuavisualtransition
    KRYS_NODISCARD bool HasUAVisualTransition() const noexcept
    {
      return _hasUAVisualTransition;
    }

    /// @brief Intercepts the navigation, while the event is dispatched: it commits without a load, and
    /// @p options' handler does its work.
    /// @return `SecurityError` if the event isn't trusted or can't be intercepted; `InvalidStateError` if the
    /// window's document isn't fully active, the event was canceled, or it isn't being dispatched.
    /// @see https://html.spec.whatwg.org/#dom-navigateevent-intercept
    dom::ExceptionOr<void> Intercept(dom::Document &document, NavigationInterceptOptions &&options) noexcept;

    /// @brief Scrolls as the navigation would once its handlers are done, now.
    /// @see https://html.spec.whatwg.org/#dom-navigateevent-scroll
    dom::ExceptionOr<void> Scroll(dom::Document &document) noexcept;

    KRYS_NODISCARD bool WasIntercepted() const noexcept
    {
      return _interceptionState.has_value();
    }

    void SetInterceptionState(InterceptionState interceptionState) noexcept
    {
      _interceptionState = interceptionState;
    }

    /// @see https://html.spec.whatwg.org/#navigateevent-finish
    void Finish(dom::Document &document, InterceptionHandlersDidFulfill didFulfill,
                FocusDidChange focusChanged) noexcept;

    KRYS_NODISCARD List<NavigationInterceptHandler> &Handlers() noexcept
    {
      return _handlers;
    }

    /// @brief Signals the event's abort controller with @p exception.
    void SignalAbort(const dom::Exception &exception) noexcept;

    KRYS_NODISCARD bool IsNavigateEvent() const noexcept final
    {
      return true;
    }

  private:
    /// @see https://html.spec.whatwg.org/#navigateevent-perform-shared-checks
    dom::ExceptionOr<void> SharedChecks(dom::Document &document) const noexcept;
    /// @see https://html.spec.whatwg.org/#potentially-process-scroll-behavior
    void PotentiallyProcessScrollBehavior(dom::Document &document) noexcept;
    /// @see https://html.spec.whatwg.org/#process-scroll-behavior
    void ProcessScrollBehavior(dom::Document &document) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::NavigateEvent)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Event &event) noexcept
  {
    return event.IsNavigateEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
