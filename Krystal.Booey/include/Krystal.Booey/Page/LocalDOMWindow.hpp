#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/DOM/Events/Event.hpp"
#include "Krystal.Booey/DOM/Events/EventTarget.hpp"
#include "Krystal.Booey/DOM/Scheduling/ScriptedAnimationController.hpp"
#include "Krystal.Booey/DOM/Types/DOMHighResTimeStamp.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/HTML/Attributes/EventHandlerAttributes.hpp"
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
  class CloseWatcherManager;
  class CustomElementRegistry;
}

namespace krys::boo
{
  class DOMSelection;
  class LocalFrame;
  class Page;
  class VisualViewport;

  /// @brief A document's window: the global object of the document a frame shows, which the window events
  /// (`load`, `resize` and the others) fire at, and which the document's events bubble to.
  /// @note The document makes its window as it becomes a frame's document, and keeps it.
  /// @see https://html.spec.whatwg.org/#the-window-object
  class LocalDOMWindow final : public dom::EventTarget
  {
    KRYS_TYPE_CAST_TRAITS_ACCESS();
    KRYS_NON_COPY_MOVABLE(LocalDOMWindow);

  private:
    WeakPtr<dom::Document> _document;
    RefPtr<html::CustomElementRegistry> _customElementRegistry;
    mutable RefPtr<html::CloseWatcherManager> _closeWatcherManager;
    mutable RefPtr<VisualViewport> _visualViewport;
    /// @brief The host's time as the window was made, which timestamps count from.
    /// @see https://w3c.github.io/hr-time/#dfn-get-time-origin-timestamp
    MonotonicTime::duration _timeOrigin {};
    /// @brief The timestamp of the rendering update in progress, if any.
    Maybe<dom::DOMHighResTimeStamp> _frozenNowTimestamp;
    /// @brief When the window was last activated by the user, as a timestamp: positive infinity if it never
    /// was, negative infinity once an API consumed the activation.
    /// @see https://html.spec.whatwg.org/#last-activation-timestamp
    dom::DOMHighResTimeStamp _lastActivationTimestamp;
    /// @brief Consumed by close watchers, so a page can't block closing without the user's activation.
    /// @see https://html.spec.whatwg.org/#last-history-action-activation-timestamp
    dom::DOMHighResTimeStamp _lastHistoryActionActivationTimestamp;

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

    /// @brief The window's close watchers.
    /// @see https://html.spec.whatwg.org/#close-watcher-manager
    KRYS_NODISCARD html::CloseWatcherManager &GetCloseWatcherManager() const noexcept;

    /// @brief The window's close watcher manager, if it was made.
    KRYS_NODISCARD html::CloseWatcherManager *CloseWatcherManagerIfExists() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-window-customelements
    KRYS_NODISCARD html::CustomElementRegistry &CustomElements() noexcept
    {
      return EnsureCustomElementRegistry();
    }

    /// @brief Asks the host to bring the window to the front.
    /// @see https://html.spec.whatwg.org/#dom-window-focus
    void Focus() noexcept;

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

#pragma region GlobalEventHandlers, WindowEventHandlers - https://html.spec.whatwg.org/#event-handler-attributes

    KRYS_GLOBAL_EVENT_HANDLERS(KRYS_DECLARE_EVENT_HANDLER)
    KRYS_WINDOW_EVENT_HANDLERS(KRYS_DECLARE_EVENT_HANDLER)

#pragma endregion

#pragma region CSSOM View - https://drafts.csswg.org/cssom-view/#extensions-to-the-window-interface

    /// @brief A new `MediaQueryList` for @p query, evaluated against the window's document; null if the
    /// document has been destroyed.
    /// @see https://drafts.csswg.org/cssom-view/#dom-window-matchmedia
    KRYS_NODISCARD RefPtr<css::MediaQueryList> MatchMedia(dom::DOMStringView query) noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-window-visualviewport
    KRYS_NODISCARD VisualViewport &GetVisualViewport() const noexcept;

    /// @brief The window's visual viewport, if something asked for it.
    KRYS_NODISCARD VisualViewport *OptionalVisualViewport() const noexcept;

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

    /// @brief Device pixels per CSS pixel, with the page zoom.
    /// @see https://drafts.csswg.org/cssom-view/#dom-window-devicepixelratio
    KRYS_NODISCARD double DevicePixelRatio() const noexcept;

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

    /// @brief Calls @p function with each window that exists.
    static void ForEachWindow(const Func<void(LocalDOMWindow &)> &function) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-queuemicrotask
    void QueueMicrotask(Func<void()> &&callback) noexcept;

#pragma region Animation frames - https://html.spec.whatwg.org/#animation-frames

    /// @brief Calls @p callback in the next rendering update, with its timestamp.
    /// @return The callback's handle, positive; 0 if the window has no document.
    /// @see https://html.spec.whatwg.org/#dom-animationframeprovider-requestanimationframe
    int32 RequestAnimationFrame(dom::FrameRequestCallback &&callback) noexcept;

    /// @see https://html.spec.whatwg.org/#animationframeprovider-cancelanimationframe
    void CancelAnimationFrame(int32 handle) noexcept;

#pragma endregion

    /// @brief Milliseconds since the window's time origin, in the host's time.
    /// @see https://w3c.github.io/hr-time/#dfn-current-high-resolution-time
    KRYS_NODISCARD dom::DOMHighResTimeStamp NowTimestamp() const noexcept;

    /// @brief The host's time as the window was made, which timestamps count from.
    KRYS_NODISCARD MonotonicTime::duration TimeOrigin() const noexcept
    {
      return _timeOrigin;
    }

    /// @brief Freezes the timestamp for the rendering update in progress: its steps all see the same time.
    void FreezeNowTimestamp() noexcept;
    void UnfreezeNowTimestamp() noexcept;

    /// @brief The frozen timestamp during a rendering update; the current one otherwise.
    KRYS_NODISCARD dom::DOMHighResTimeStamp FrozenNowTimestamp() const noexcept;

    /// @brief The resolved values of @p element's properties, or of its pseudo-element @p pseudoElement (such
    /// as `::before`).
    /// @see https://drafts.csswg.org/cssom/#dom-window-getcomputedstyle
    KRYS_NODISCARD Ref<css::CSSStyleDeclaration>
      GetComputedStyle(dom::Element &element, const css::CSSOMString &pseudoElement = {}) const noexcept;

    /// @brief The document's selection, while the window's document is its frame's.
    /// @see https://w3c.github.io/selection-api/#dom-window-getselection
    KRYS_NODISCARD DOMSelection *GetSelection() const noexcept;

  private:
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
