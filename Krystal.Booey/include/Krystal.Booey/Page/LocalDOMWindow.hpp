#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/EventTarget.hpp"
#include "Krystal.Booey/DOM/IdleCallbackController.hpp"
#include "Krystal.Booey/DOM/ScriptedAnimationController.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/HTML/EventHandlerAttributes.hpp"
#include "Krystal.Booey/HTML/SerializedScriptValue.hpp"
#include "Krystal.Booey/Page/ScrollOptions.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::css
{
  class CSSStyleDeclaration;
  class MediaQueryList;
}

namespace krys::boo::dom
{
  class Document;
  class Element;
  class EventDispatcher;
  class HTMLCollection;
}

namespace krys::boo::html
{
  class CustomElementRegistry;
}

namespace krys::boo
{
  class BarProp;
  class DOMSelection;
  class History;
  class Location;
  class Navigation;
  class Navigator;
  class External;
  class LocalFrame;
  class Page;
  class Screen;
  class Storage;
  class VisualViewport;

  /// @note There are no transfers: there are no message ports.
  /// @see https://html.spec.whatwg.org/#windowpostmessageoptions
  struct WindowPostMessageOptions
  {
    dom::USVString TargetOrigin {u8"/"};
  };

  /// @brief A document's window: the global object of the document a frame shows, which the window events
  /// (`load`, `resize`, `hashchange` and the others) fire at, and which the document's events bubble to.
  /// @note The document makes its window as it becomes a frame's document, and keeps it.
  /// @see https://html.spec.whatwg.org/#the-window-object
  class LocalDOMWindow final : public dom::EventTarget
  {
    KRYS_TYPE_CAST_TRAITS_ACCESS();
    KRYS_NON_COPY_MOVABLE(LocalDOMWindow);

    friend class dom::EventDispatcher;

  private:
    WeakPtr<dom::Document> _document;
    RefPtr<html::CustomElementRegistry> _customElementRegistry;
    mutable RefPtr<BarProp> _locationbar;
    mutable RefPtr<BarProp> _menubar;
    mutable RefPtr<BarProp> _personalbar;
    mutable RefPtr<BarProp> _scrollbars;
    mutable RefPtr<BarProp> _statusbar;
    mutable RefPtr<BarProp> _toolbar;
    mutable RefPtr<External> _external;
    mutable RefPtr<Navigator> _navigator;
    mutable RefPtr<boo::Location> _location;
    mutable RefPtr<boo::History> _history;
    mutable RefPtr<boo::Navigation> _navigation;
    RefPtr<Storage> _sessionStorage;
    RefPtr<Storage> _localStorage;
    mutable RefPtr<Screen> _screen;
    mutable RefPtr<VisualViewport> _visualViewport;
    dom::DOMString _status;
    /// @see https://html.spec.whatwg.org/#window-current-event
    RefPtr<dom::Event> _currentEvent;
    /// @brief The host's time as the window was made, which timestamps count from.
    /// @see https://w3c.github.io/hr-time/#dfn-get-time-origin-timestamp
    MonotonicTime::duration _timeOrigin {};
    /// @brief The timestamp of the rendering update in progress, if any.
    Maybe<double> _frozenNowTimestamp;
    /// @brief When the window was last activated by the user, as a timestamp: positive infinity if it never
    /// was, negative infinity once an API consumed the activation.
    /// @see https://html.spec.whatwg.org/#last-activation-timestamp
    double _lastActivationTimestamp;
    /// @see https://html.spec.whatwg.org/#last-history-action-activation-timestamp
    double _lastHistoryActionActivationTimestamp;
    /// @see https://html.spec.whatwg.org/#concept-error-reporting-mode
    bool _errorReportingMode {false};

    explicit LocalDOMWindow(dom::Document &document) noexcept;

  public:
    ~LocalDOMWindow() noexcept override;

    /// @brief A window for @p document, which must be in a frame.
    KRYS_NODISCARD static Ref<LocalDOMWindow> Create(dom::Document &document) noexcept;

    /// @brief The window's document (its associated `Document`), unless it has been destroyed.
    /// @see https://html.spec.whatwg.org/#concept-document-window
    KRYS_NODISCARD dom::Document *Document() const noexcept;

    /// @brief The frame showing the window's document (its navigable), if any.
    KRYS_NODISCARD LocalFrame *Frame() const noexcept;

    /// @brief The page of the window's frame, if any.
    KRYS_NODISCARD Page *GetPage() const noexcept;

    /// @brief Whether the window's document is the one its frame shows.
    KRYS_NODISCARD bool IsCurrentlyDisplayedInFrame() const noexcept;

    /// @brief The window's custom element registry, made with the window and given to its document.
    KRYS_NODISCARD html::CustomElementRegistry &EnsureCustomElementRegistry() noexcept;

#pragma region The current browsing context - https://html.spec.whatwg.org/#the-window-object

    /// @note There's no `WindowProxy`: `window`, `self` and `frames` are the window itself.
    /// @see https://html.spec.whatwg.org/#dom-window
    KRYS_NODISCARD LocalDOMWindow &Window() noexcept
    {
      return *this;
    }

    /// @see https://html.spec.whatwg.org/#dom-self
    KRYS_NODISCARD LocalDOMWindow &Self() noexcept
    {
      return *this;
    }

    /// @see https://html.spec.whatwg.org/#dom-frames
    KRYS_NODISCARD LocalDOMWindow &Frames() noexcept
    {
      return *this;
    }

    /// @brief The window's `Location`: its document's URL, which navigates the window when it's set.
    /// @see https://html.spec.whatwg.org/#dom-location
    KRYS_NODISCARD boo::Location &GetLocation() const noexcept;

    /// @brief The window's `History`: its page's session history.
    /// @see https://html.spec.whatwg.org/#dom-history
    KRYS_NODISCARD boo::History &GetHistory() const noexcept;

    /// @brief The window's `Navigation`: its document's origin's session history entries, and the navigations
    /// it starts and intercepts.
    /// @see https://html.spec.whatwg.org/#dom-navigation
    KRYS_NODISCARD boo::Navigation &GetNavigation() const noexcept;

    /// @brief The window's `Navigation`, if it was made.
    KRYS_NODISCARD boo::Navigation *NavigationIfExists() const noexcept;

    /// @brief The frame's name (its navigable's target name), which links and forms can target.
    /// @see https://html.spec.whatwg.org/#dom-name
    KRYS_NODISCARD dom::DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-name
    void SetName(dom::DOMString &&name) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-window-customelements
    KRYS_NODISCARD html::CustomElementRegistry &CustomElements() noexcept
    {
      return EnsureCustomElementRegistry();
    }

    /// @see https://html.spec.whatwg.org/#dom-window-locationbar
    KRYS_NODISCARD BarProp &Locationbar() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-window-menubar
    KRYS_NODISCARD BarProp &Menubar() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-window-personalbar
    KRYS_NODISCARD BarProp &Personalbar() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-window-scrollbars
    KRYS_NODISCARD BarProp &Scrollbars() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-window-statusbar
    KRYS_NODISCARD BarProp &Statusbar() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-window-toolbar
    KRYS_NODISCARD BarProp &Toolbar() const noexcept;

    /// @note Obsolete: the value is kept, and shown nowhere.
    /// @see https://html.spec.whatwg.org/#dom-window-status
    KRYS_NODISCARD const dom::DOMString &Status() const noexcept
    {
      return _status;
    }

    /// @see https://html.spec.whatwg.org/#dom-window-status
    void SetStatus(dom::DOMString &&status) noexcept
    {
      _status = krys::move(status);
    }

    /// @brief Closes the page, through its chrome client, on a later task.
    /// @see https://html.spec.whatwg.org/#dom-window-close
    void Close() noexcept;

    /// @brief Whether the window has no frame, or its page is closing.
    /// @see https://html.spec.whatwg.org/#dom-window-closed
    KRYS_NODISCARD bool Closed() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-window-stop
    void Stop() noexcept;

    /// @brief Asks the host to bring the window to the front.
    /// @see https://html.spec.whatwg.org/#dom-window-focus
    void Focus() noexcept;

    /// @brief Does nothing, as the spec has it.
    /// @see https://html.spec.whatwg.org/#dom-window-blur
    void Blur() const noexcept
    {
    }

#pragma endregion

#pragma region Other browsing contexts - https://html.spec.whatwg.org/#the-window-object

    /// @brief The number of child frames: none, as there's no frame tree.
    /// @see https://html.spec.whatwg.org/#dom-length
    KRYS_NODISCARD uint32 Length() const noexcept
    {
      return 0;
    }

    /// @brief The window itself, or null if it has no frame.
    /// @see https://html.spec.whatwg.org/#dom-top
    KRYS_NODISCARD LocalDOMWindow *Top() noexcept;

    /// @brief The window that opened this one: none, as pages aren't opened by other pages.
    /// @see https://html.spec.whatwg.org/#dom-opener
    KRYS_NODISCARD LocalDOMWindow *Opener() const noexcept
    {
      return nullptr;
    }

    /// @brief The window itself, or null if it has no frame.
    /// @see https://html.spec.whatwg.org/#dom-parent
    KRYS_NODISCARD LocalDOMWindow *Parent() noexcept;

    /// @brief The element the window's frame is in: none, as there are no frames in documents.
    /// @see https://html.spec.whatwg.org/#dom-frameelement
    KRYS_NODISCARD dom::Element *FrameElement() const noexcept
    {
      return nullptr;
    }

#pragma endregion

#pragma region Named access - https://html.spec.whatwg.org/#named-access-on-the-window-object

    /// @brief The document's named objects of name @p name (`embed`, `form`, `img` and `object` elements by
    /// their `name`, and HTML elements by their ID): the element if there's one, an `HTMLCollection` of them
    /// if there are several, nothing if there are none.
    /// @see https://html.spec.whatwg.org/#dom-window-nameditem
    KRYS_NODISCARD Maybe<Variant<RefPtr<dom::Element>, RefPtr<dom::HTMLCollection>>>
      NamedItem(dom::DOMStringView name) const noexcept;

    /// @brief The names `NamedItem` finds something for, in tree order.
    /// @see https://html.spec.whatwg.org/#named-access-on-the-window-object
    KRYS_NODISCARD List<dom::DOMString> SupportedPropertyNames() const noexcept;

#pragma endregion

#pragma region The user agent - https://html.spec.whatwg.org/#the-window-object

    /// @see https://html.spec.whatwg.org/#dom-navigator
    KRYS_NODISCARD Navigator &GetNavigator() const noexcept;

    /// @note Legacy: `navigator`.
    /// @see https://html.spec.whatwg.org/#dom-clientinformation
    KRYS_NODISCARD Navigator &ClientInformation() const noexcept
    {
      return GetNavigator();
    }

    /// @brief The user's preferred languages changed: queues `languagechange` at the window.
    /// @see https://html.spec.whatwg.org/#event-languagechange
    void LanguagesChanged() noexcept;

    /// @brief Whether the window's agent cluster is keyed by origin rather than by site: never, as no
    /// document sends `Origin-Agent-Cluster`.
    /// @see https://html.spec.whatwg.org/#dom-originagentcluster
    KRYS_NODISCARD bool OriginAgentCluster() const noexcept
    {
      return false;
    }

#pragma endregion

#pragma region GlobalEventHandlers, WindowEventHandlers - https://html.spec.whatwg.org/#event-handler-attributes

    KRYS_GLOBAL_EVENT_HANDLERS(KRYS_DECLARE_EVENT_HANDLER)
    KRYS_WINDOW_EVENT_HANDLERS(KRYS_DECLARE_EVENT_HANDLER)

#pragma endregion

#pragma region Obsolete - https://html.spec.whatwg.org/#Window-partial

    /// @see https://html.spec.whatwg.org/#dom-window-captureevents
    void CaptureEvents() const noexcept
    {
    }

    /// @see https://html.spec.whatwg.org/#dom-window-releaseevents
    void ReleaseEvents() const noexcept
    {
    }

    /// @see https://html.spec.whatwg.org/#dom-window-external
    KRYS_NODISCARD External &GetExternal() const noexcept;

#pragma endregion

    /// @brief The event being dispatched to a listener in the window's document, outside shadow trees, if
    /// any.
    /// @note Legacy.
    /// @see https://dom.spec.whatwg.org/#dom-window-event
    KRYS_NODISCARD dom::Event *CurrentEvent() const noexcept
    {
      return _currentEvent.get();
    }

#pragma region CSSOM View - https://drafts.csswg.org/cssom-view/#extensions-to-the-window-interface

    /// @brief A new `MediaQueryList` for @p query, evaluated against the window's document; null if the
    /// document has been destroyed.
    /// @see https://drafts.csswg.org/cssom-view/#dom-window-matchmedia
    KRYS_NODISCARD RefPtr<css::MediaQueryList> MatchMedia(dom::DOMStringView query) noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-window-screen
    KRYS_NODISCARD Screen &GetScreen() const noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-window-visualviewport
    KRYS_NODISCARD VisualViewport &GetVisualViewport() const noexcept;

    /// @brief The window's visual viewport, if something asked for it.
    KRYS_NODISCARD VisualViewport *OptionalVisualViewport() const noexcept;

    /// @brief Moves the host's window to (@p x, @p y) on the screen, kept on the screen.
    /// @see https://drafts.csswg.org/cssom-view/#dom-window-moveto
    void MoveTo(int32 x, int32 y) const noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-window-moveby
    void MoveBy(int32 x, int32 y) const noexcept;

    /// @brief Resizes the host's window, kept on the screen and at least its minimum size.
    /// @see https://drafts.csswg.org/cssom-view/#dom-window-resizeto
    void ResizeTo(int32 width, int32 height) const noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-window-resizeby
    void ResizeBy(int32 x, int32 y) const noexcept;

    /// @brief The viewport's width, with its scrollbar, in CSS pixels.
    /// @see https://drafts.csswg.org/cssom-view/#dom-window-innerwidth
    KRYS_NODISCARD int32 InnerWidth() const noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-window-innerheight
    KRYS_NODISCARD int32 InnerHeight() const noexcept;

    /// @brief How far the viewport is scrolled across, in CSS pixels.
    /// @see https://drafts.csswg.org/cssom-view/#dom-window-scrollx
    KRYS_NODISCARD int32 ScrollX() const noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-window-pagexoffset
    KRYS_NODISCARD int32 PageXOffset() const noexcept
    {
      return ScrollX();
    }

    /// @see https://drafts.csswg.org/cssom-view/#dom-window-scrolly
    KRYS_NODISCARD int32 ScrollY() const noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-window-pageyoffset
    KRYS_NODISCARD int32 PageYOffset() const noexcept
    {
      return ScrollY();
    }

    /// @brief Scrolls the viewport to @p options' position, in CSS pixels; a missing coordinate stays.
    /// @see https://drafts.csswg.org/cssom-view/#dom-window-scroll
    void ScrollTo(const ScrollToOptions &options = {}) const noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-window-scroll
    void ScrollTo(double x, double y) const noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-window-scroll
    void Scroll(const ScrollToOptions &options = {}) const noexcept
    {
      ScrollTo(options);
    }

    /// @see https://drafts.csswg.org/cssom-view/#dom-window-scroll
    void Scroll(double x, double y) const noexcept
    {
      ScrollTo(x, y);
    }

    /// @brief Scrolls the viewport by @p options' offsets, in CSS pixels.
    /// @see https://drafts.csswg.org/cssom-view/#dom-window-scrollby
    void ScrollBy(const ScrollToOptions &options = {}) const noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-window-scrollby
    void ScrollBy(double x, double y) const noexcept;

    /// @brief The host's window's position on the screen.
    /// @see https://drafts.csswg.org/cssom-view/#dom-window-screenx
    KRYS_NODISCARD int32 ScreenX() const noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-window-screenleft
    KRYS_NODISCARD int32 ScreenLeft() const noexcept
    {
      return ScreenX();
    }

    /// @see https://drafts.csswg.org/cssom-view/#dom-window-screeny
    KRYS_NODISCARD int32 ScreenY() const noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-window-screentop
    KRYS_NODISCARD int32 ScreenTop() const noexcept
    {
      return ScreenY();
    }

    /// @brief The host's window's size.
    /// @see https://drafts.csswg.org/cssom-view/#dom-window-outerwidth
    KRYS_NODISCARD int32 OuterWidth() const noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-window-outerheight
    KRYS_NODISCARD int32 OuterHeight() const noexcept;

    /// @brief Device pixels per CSS pixel, with the page zoom.
    /// @see https://drafts.csswg.org/cssom-view/#dom-window-devicepixelratio
    KRYS_NODISCARD double DevicePixelRatio() const noexcept;

#pragma endregion

#pragma region User prompts - https://html.spec.whatwg.org/#user-prompts

    /// @brief Shows @p message to the user, through the host.
    /// @see https://html.spec.whatwg.org/#dom-alert
    void Alert(const dom::DOMString &message = {}) noexcept;

    /// @brief Asks the user to confirm @p message, through the host.
    /// @see https://html.spec.whatwg.org/#dom-confirm
    KRYS_NODISCARD bool Confirm(const dom::DOMString &message = {}) noexcept;

    /// @brief Asks the user for text, through the host.
    /// @return What they entered; null if they cancelled.
    /// @see https://html.spec.whatwg.org/#dom-prompt
    KRYS_NODISCARD Maybe<dom::DOMString> Prompt(const dom::DOMString &message = {},
                                                const dom::DOMString &defaultValue = {}) noexcept;

    /// @brief Fires `beforeprint`, asks the host to print the page, then fires `afterprint`.
    /// @see https://html.spec.whatwg.org/#dom-print
    void Print() noexcept;

#pragma endregion

#pragma region Opening windows - https://html.spec.whatwg.org/#apis-for-creating-and-navigating-browsing-contexts-by-name

    /// @brief Navigates the browsing context @p target names to @p url, or asks the host for a new window
    /// (through the frame loader client's new window policy, with @p features), which needs transient
    /// activation unless @p target names this window.
    /// @return This window if it navigates, null otherwise (a new window is the host's), or `SyntaxError` if
    /// @p url can't be parsed.
    /// @see https://html.spec.whatwg.org/#dom-open
    dom::ExceptionOr<LocalDOMWindow *> Open(utf8_stringview url = {},
                                            dom::DOMStringAtom target = dom::DOMStringAtom(u8"_blank"),
                                            dom::DOMStringView features = {}) noexcept;

#pragma endregion

#pragma region User activation - https://html.spec.whatwg.org/#tracking-user-activation

    /// @brief How long an activation is transient: 5s.
    KRYS_NODISCARD static double TransientActivationDuration() noexcept
    {
      return 5'000;
    }

    /// @brief The user activated the window just now (a press, a key): its activation notification.
    /// @see https://html.spec.whatwg.org/#activation-notification
    void NotifyActivated() noexcept;

    /// @see https://html.spec.whatwg.org/#sticky-activation
    KRYS_NODISCARD bool HasStickyActivation() const noexcept;

    /// @see https://html.spec.whatwg.org/#transient-activation
    KRYS_NODISCARD bool HasTransientActivation() const noexcept;

    /// @see https://html.spec.whatwg.org/#history-action-activation
    KRYS_NODISCARD bool HasHistoryActionActivation() const noexcept;

    /// @return Whether there was a transient activation to consume.
    /// @see https://html.spec.whatwg.org/#consume-user-activation
    bool ConsumeTransientActivation() noexcept;

    /// @see https://html.spec.whatwg.org/#consume-history-action-user-activation
    bool ConsumeHistoryActionUserActivation() noexcept;

#pragma endregion

#pragma region WindowOrWorkerGlobalScope - https://html.spec.whatwg.org/#windoworworkerglobalscope

    /// @brief The serialisation of the document's origin.
    /// @see https://html.spec.whatwg.org/#dom-origin
    KRYS_NODISCARD dom::USVString Origin() const noexcept;

    /// @brief Whether the document's URL is potentially trustworthy (`https:`, `file:`, the local host).
    /// @see https://html.spec.whatwg.org/#dom-issecurecontext
    KRYS_NODISCARD bool IsSecureContext() const noexcept;

    /// @brief Never: documents aren't cross-origin isolated.
    /// @see https://html.spec.whatwg.org/#dom-crossoriginisolated
    KRYS_NODISCARD bool CrossOriginIsolated() const noexcept
    {
      return false;
    }

    /// @brief Fires a cancelable `error` (an `ErrorEvent`) at the window, then reports @p message to the
    /// host's console unless a listener cancelled it.
    /// @see https://html.spec.whatwg.org/#dom-reporterror
    void ReportError(const dom::DOMString &message,
                     RefPtr<html::SerializedScriptValue> &&error = nullptr) noexcept;

    /// @brief @p data in base64; `InvalidCharacterError` if it has a code point above U+00FF.
    /// @see https://html.spec.whatwg.org/#dom-btoa
    KRYS_NODISCARD dom::ExceptionOr<dom::DOMString> Btoa(dom::DOMStringView data) const noexcept;

    /// @brief The bytes @p data encodes in base64, as code points; `InvalidCharacterError` if it isn't
    /// base64.
    /// @see https://html.spec.whatwg.org/#dom-atob
    KRYS_NODISCARD dom::ExceptionOr<dom::DOMString> Atob(dom::DOMStringView data) const noexcept;

    /// @brief A clone of @p value (its `Clone()`).
    /// @see https://html.spec.whatwg.org/#dom-structuredclone
    KRYS_NODISCARD RefPtr<html::SerializedScriptValue>
      StructuredClone(html::SerializedScriptValue *value) const noexcept;

#pragma endregion

#pragma region Posting messages - https://html.spec.whatwg.org/#posting-messages

    /// @brief Queues a `message` (a `MessageEvent`) with @p message at the window, from the window itself,
    /// if the window's document's origin is @p targetOrigin (`*` for any, `/` for its own).
    /// @return `SyntaxError` if @p targetOrigin isn't `*`, `/` or a URL.
    /// @see https://html.spec.whatwg.org/#dom-window-postmessage
    dom::ExceptionOr<void> PostMessage(RefPtr<html::SerializedScriptValue> &&message,
                                       utf8_stringview targetOrigin) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-window-postmessage-options
    dom::ExceptionOr<void> PostMessage(RefPtr<html::SerializedScriptValue> &&message,
                                       const WindowPostMessageOptions &options = {}) noexcept;

#pragma endregion

#pragma region Storage - https://html.spec.whatwg.org/#webstorage

    /// @brief The page's session storage for the document's origin.
    /// @return `SecurityError` if the origin is opaque.
    /// @see https://html.spec.whatwg.org/#dom-sessionstorage
    dom::ExceptionOr<Storage *> SessionStorage() noexcept;

    /// @brief The origin's local storage, through the host's storage provider.
    /// @return `SecurityError` if the origin is opaque.
    /// @see https://html.spec.whatwg.org/#dom-localstorage
    dom::ExceptionOr<Storage *> LocalStorage() noexcept;

    KRYS_NODISCARD Storage *OptionalSessionStorage() const noexcept;
    KRYS_NODISCARD Storage *OptionalLocalStorage() const noexcept;

#pragma endregion

    /// @brief Calls @p function with each window that exists.
    static void ForEachWindow(const Func<void(LocalDOMWindow &)> &function) noexcept;

#pragma region Timers - https://html.spec.whatwg.org/#timers

    /// @brief Runs @p handler after @p timeout milliseconds (0 if negative), as a task.
    /// @return The timer's ID, positive; 0 if the window has no document.
    /// @see https://html.spec.whatwg.org/#dom-settimeout
    int32 SetTimeout(Func<void()> &&handler, int32 timeout = 0) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-cleartimeout
    void ClearTimeout(int32 id) noexcept;

    /// @brief Runs @p handler every @p timeout milliseconds, as a task, until cleared.
    /// @see https://html.spec.whatwg.org/#dom-setinterval
    int32 SetInterval(Func<void()> &&handler, int32 timeout = 0) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-clearinterval
    void ClearInterval(int32 id) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-queuemicrotask
    void QueueMicrotask(Func<void()> &&callback) noexcept;

#pragma endregion

#pragma region Animation frames - https://html.spec.whatwg.org/#animation-frames

    /// @brief Calls @p callback in the next rendering update, with its timestamp.
    /// @return The callback's handle, positive; 0 if the window has no document.
    /// @see https://html.spec.whatwg.org/#dom-animationframeprovider-requestanimationframe
    int32 RequestAnimationFrame(dom::FrameRequestCallback &&callback) noexcept;

    /// @see https://html.spec.whatwg.org/#animationframeprovider-cancelanimationframe
    void CancelAnimationFrame(int32 handle) noexcept;

#pragma endregion

#pragma region Idle callbacks - https://w3c.github.io/requestidlecallback/#the-requestidlecallback-method

    /// @brief Calls @p callback in an idle period after a rendering update, or after @p options' timeout.
    /// @see https://w3c.github.io/requestidlecallback/#the-requestidlecallback-method
    int32 RequestIdleCallback(dom::IdleRequestCallback &&callback,
                              const dom::IdleRequestOptions &options = {}) noexcept;

    /// @see https://w3c.github.io/requestidlecallback/#the-cancelidlecallback-method
    void CancelIdleCallback(int32 handle) noexcept;

#pragma endregion

    /// @brief Milliseconds since the window's time origin, in the host's time.
    /// @see https://w3c.github.io/hr-time/#dfn-current-high-resolution-time
    KRYS_NODISCARD double NowTimestamp() const noexcept;

    /// @brief Freezes the timestamp for the rendering update in progress: its steps all see the same time.
    void FreezeNowTimestamp() noexcept;
    void UnfreezeNowTimestamp() noexcept;

    /// @brief The frozen timestamp during a rendering update; the current one otherwise.
    KRYS_NODISCARD double FrozenNowTimestamp() const noexcept;

    /// @brief The resolved values of @p element's properties, or of its pseudo-element @p pseudoElement (such
    /// as `::before`).
    /// @see https://drafts.csswg.org/cssom/#dom-window-getcomputedstyle
    KRYS_NODISCARD Ref<css::CSSStyleDeclaration>
      GetComputedStyle(dom::Element &element, const css::CSSOMString &pseudoElement = {}) const noexcept;

    /// @brief The document's selection, while the window's document is its frame's.
    /// @see https://w3c.github.io/selection-api/#dom-window-getselection
    KRYS_NODISCARD DOMSelection *GetSelection() const noexcept;

  private:
    /// @brief Whether the page may move and resize its host's window: not while a button is held, so a page
    /// can't start a drag the user didn't.
    KRYS_NODISCARD bool AllowedToChangeWindowGeometry() const noexcept;

    /// @brief @p pendingChanges' finite parts applied to the host's window, kept on the screen and at least
    /// its minimum size.
    KRYS_NODISCARD static FloatRect AdjustWindowRect(Page &page, const FloatRect &pendingChanges) noexcept;

    /// @brief Whether @p element is a named object of the window with the name @p name.
    KRYS_NODISCARD static bool IsNamedObject(const dom::Element &element, dom::DOMStringView name) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LocalDOMWindow)
  static bool IsType(const krys::boo::dom::EventTarget &target) noexcept
  {
    return target.IsWindow();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
