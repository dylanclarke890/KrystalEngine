#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/HTML/SerializedScriptValue.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::url
{
  class URL;
}

namespace krys::boo
{
  class LocalDOMWindow;
  class LocalFrame;
  enum class NavigationHistoryBehavior : uint8;

  /// @brief A window's `history`: its page's session history, which it moves through (`go()`, `back()`,
  /// `forward()`), and its document's entries, which it adds and replaces with a state and a URL
  /// (`pushState()`, `replaceState()`) without leaving the document.
  /// @see https://html.spec.whatwg.org/#the-history-interface
  class History final : public RefCounted<History>
  {
    KRYS_NON_COPY_MOVABLE(History);

  private:
    WeakPtr<LocalDOMWindow> _window;
    RefPtr<html::SerializedScriptValue> _lastStateObjectRequested;
    uint32 _currentStateObjectTimeSpanObjectsAdded {0};
    MonotonicTime::duration _currentStateObjectTimeSpanStart {};
    uint64 _totalStateObjectUsage {0};
    // For each individual History object to keep track of the most recent state object added.
    uint64 _mostRecentStateObjectUsage {0};

  public:
    explicit History(LocalDOMWindow &window) noexcept;
    ~History() noexcept;

    KRYS_NODISCARD static Ref<History> Create(LocalDOMWindow &window) noexcept
    {
      return CreateRef<History>(window);
    }

    /// @brief How many entries the page's session history has.
    /// @return `SecurityError` if the window's document isn't fully active.
    /// @see https://html.spec.whatwg.org/#dom-history-length
    KRYS_NODISCARD dom::ExceptionOr<uint32> Length() const noexcept;

    /// @see https://html.spec.whatwg.org/#scrollrestoration
    enum class ScrollRestoration : uint8
    {
      Auto,
      Manual
    };

    /// @brief Whether going back to the current entry restores its scroll position.
    /// @see https://html.spec.whatwg.org/#dom-history-scroll-restoration
    KRYS_NODISCARD dom::ExceptionOr<ScrollRestoration> GetScrollRestoration() const noexcept;
    dom::ExceptionOr<void> SetScrollRestoration(ScrollRestoration scrollRestoration) noexcept;

    /// @brief The current entry's state; null for the spec's null.
    /// @see https://html.spec.whatwg.org/#dom-history-state
    KRYS_NODISCARD dom::ExceptionOr<html::SerializedScriptValue *> State() noexcept;

    /// @brief Whether @p state is the current entry's.
    KRYS_NODISCARD bool IsSameAsCurrentState(const html::SerializedScriptValue *state) const noexcept;

    /// @brief Goes back an entry, on a later task.
    /// @see https://html.spec.whatwg.org/#dom-history-back
    dom::ExceptionOr<void> Back() noexcept;
    /// @brief Goes forward an entry, on a later task.
    /// @see https://html.spec.whatwg.org/#dom-history-forward
    dom::ExceptionOr<void> Forward() noexcept;
    /// @brief Goes @p delta entries back (if negative) or forward, on a later task; reloads for 0.
    /// @see https://html.spec.whatwg.org/#dom-history-go
    dom::ExceptionOr<void> Go(int32 delta = 0) noexcept;

    /// @brief Adds an entry for the document, at @p url (the document's URL if empty) with @p data, which
    /// becomes the current one.
    /// @return `SecurityError` if @p url can't be the document's (another origin), or too many states are
    /// added; `QuotaExceededError` if the states are too large.
    /// @see https://html.spec.whatwg.org/#dom-history-pushstate
    dom::ExceptionOr<void> PushState(RefPtr<html::SerializedScriptValue> &&data, dom::DOMStringView unused,
                                     dom::DOMStringView url = {}) noexcept;
    /// @brief Gives the current entry @p url (if not empty) and @p data.
    /// @see https://html.spec.whatwg.org/#dom-history-replacestate
    dom::ExceptionOr<void> ReplaceState(RefPtr<html::SerializedScriptValue> &&data, dom::DOMStringView unused,
                                        dom::DOMStringView url = {}) noexcept;

  private:
    KRYS_NODISCARD LocalFrame *Frame() const noexcept;

    /// @see https://html.spec.whatwg.org/#shared-history-push/replace-state-steps
    dom::ExceptionOr<void> StateObjectAdded(RefPtr<html::SerializedScriptValue> &&data,
                                            dom::DOMStringView url,
                                            NavigationHistoryBehavior historyBehavior) noexcept;
    dom::ExceptionOr<void> UpdateAndCheckStateObjectQuota(const url::URL &fullURL,
                                                          NavigationHistoryBehavior historyBehavior) noexcept;
    KRYS_NODISCARD html::SerializedScriptValue *StateInternal() const noexcept;
  };
}
