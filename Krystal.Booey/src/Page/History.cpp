#include "Krystal.Booey/Page/History.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/History/BackForwardController.hpp"
#include "Krystal.Booey/History/HistoryItem.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Loader/HistoryController.hpp"
#include "Krystal.Booey/Loader/NavigationScheduler.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/Navigation.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include <chrono>
#include <format>

namespace krys::boo
{
  History::History(LocalDOMWindow &window) noexcept : _window(CreateWeakPtr(&window))
  {
  }

  History::~History() noexcept = default;

  LocalFrame *History::Frame() const noexcept
  {
    auto *window = _window.get();
    return window ? window->Frame() : nullptr;
  }

  namespace
  {
    KRYS_NODISCARD bool IsDocumentFullyActive(LocalFrame *frame) noexcept
    {
      return frame && frame->Document() && frame->Document()->IsFullyActive();
    }

    KRYS_NODISCARD dom::Exception DocumentNotFullyActive() noexcept
    {
      return dom::Exception {dom::ExceptionCode::SecurityError,
                             u8"Attempt to use History API from a document that isn't fully active"};
    }
  }

  // https://html.spec.whatwg.org/#dom-history-length
  // https://html.spec.whatwg.org/#can-have-its-url-rewritten
  bool CanHaveURLRewritten(const dom::Document &document, const url::URL &targetURL) noexcept
  {
    // 1. Let documentURL be document's URL.
    const auto &documentURL = document.GetURL();
    // 2. If targetURL and documentURL differ in their scheme, username, password, host, or port components,
    //    then return false.
    if (!url::ProtocolHostAndPortAreEqual(targetURL, documentURL) || targetURL.User() != documentURL.User()
        || targetURL.Password() != documentURL.Password())
    {
      return false;
    }
    // 3. If targetURL's scheme is an HTTP(S) scheme, then return true.
    if (targetURL.ProtocolIsInHTTPFamily())
    {
      return true;
    }
    // 4. If targetURL's scheme is "file":
    if (targetURL.ProtocolIsFile())
    {
      // 4a. If targetURL and documentURL differ in their path component, then return false.
      // 4b. Return true.
      return targetURL.Path() == documentURL.Path();
    }
    // 5. If targetURL and documentURL differ in their path component or query components, then return false.
    // 6. Return true.
    return targetURL.Path() == documentURL.Path() && targetURL.HasQuery() == documentURL.HasQuery()
           && targetURL.Query() == documentURL.Query();
  }

  dom::ExceptionOr<uint32> History::Length() const noexcept
  {
    // 1. If this's relevant global object's associated Document is not fully active, then throw a
    //    "SecurityError" DOMException.
    auto *frame = Frame();
    if (!IsDocumentFullyActive(frame))
    {
      return DocumentNotFullyActive();
    }
    // 2. Return this's length.
    auto *page = frame->GetPage();
    if (!page)
    {
      return 0u;
    }
    return page->BackForward().Count();
  }

  // https://html.spec.whatwg.org/#dom-history-scroll-restoration
  dom::ExceptionOr<History::ScrollRestoration> History::GetScrollRestoration() const noexcept
  {
    // 1. If this's relevant global object's associated Document is not fully active, then throw a
    //    "SecurityError" DOMException.
    auto *frame = Frame();
    if (!IsDocumentFullyActive(frame))
    {
      return DocumentNotFullyActive();
    }
    // 2. Return this's relevant global object's navigable's active session history entry's scroll
    //    restoration mode.
    auto *historyItem = frame->Loader().History().CurrentItem();
    if (!historyItem)
    {
      return ScrollRestoration::Auto;
    }
    return historyItem->ShouldRestoreScrollPosition() ? ScrollRestoration::Auto : ScrollRestoration::Manual;
  }

  // https://html.spec.whatwg.org/#dom-history-scroll-restoration
  dom::ExceptionOr<void> History::SetScrollRestoration(ScrollRestoration scrollRestoration) noexcept
  {
    // 1. If this's relevant global object's associated Document is not fully active, then throw a
    //    "SecurityError" DOMException.
    auto *frame = Frame();
    if (!IsDocumentFullyActive(frame))
    {
      return DocumentNotFullyActive();
    }
    // 2. Set this's relevant global object's navigable's active session history entry's scroll restoration
    //    mode to the given value.
    if (auto *historyItem = frame->Loader().History().CurrentItem())
    {
      historyItem->SetShouldRestoreScrollPosition(scrollRestoration == ScrollRestoration::Auto);
    }
    return {};
  }

  // https://html.spec.whatwg.org/#dom-history-state
  dom::ExceptionOr<html::SerializedScriptValue *> History::State() noexcept
  {
    // 1. If this's relevant global object's associated Document is not fully active, then throw a
    //    "SecurityError" DOMException.
    auto *frame = Frame();
    if (!IsDocumentFullyActive(frame))
    {
      return DocumentNotFullyActive();
    }
    // 2. Return this's state.
    _lastStateObjectRequested = ShareRefPtr(StateInternal());
    return _lastStateObjectRequested.get();
  }

  html::SerializedScriptValue *History::StateInternal() const noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return nullptr;
    }
    auto *historyItem = frame->Loader().History().CurrentItem();
    return historyItem ? historyItem->StateObject() : nullptr;
  }

  bool History::IsSameAsCurrentState(const html::SerializedScriptValue *state) const noexcept
  {
    return state == StateInternal();
  }

  dom::ExceptionOr<void> History::Back() noexcept
  {
    return Go(-1);
  }

  dom::ExceptionOr<void> History::Forward() noexcept
  {
    return Go(1);
  }

  // https://html.spec.whatwg.org/#delta-traverse
  dom::ExceptionOr<void> History::Go(int32 distance) noexcept
  {
    // 1. Let document be history's relevant global object's associated Document.
    // 2. If document is not fully active, then throw a "SecurityError" DOMException.
    RefPtr<LocalFrame> frame = ShareRefPtr(Frame());
    if (!IsDocumentFullyActive(frame.get()))
    {
      return DocumentNotFullyActive();
    }
    // 3. If history's relevant global object's navigable's allowed to perform a navigation or history update
    //    returns blocked, then return.
    // (There's no such limit for traversals.)
    // 4. If delta is 0, then reload document's node navigable, and return.
    // 5. Traverse the history by a delta given document's node navigable's traversable navigable, delta, and
    //    with sourceDocument set to document.
    // (Going to the current entry reloads it.)
    frame->GetNavigationScheduler().ScheduleHistoryNavigation(distance);
    return {};
  }

  dom::ExceptionOr<void> History::PushState(RefPtr<html::SerializedScriptValue> &&data, dom::DOMStringView,
                                            dom::DOMStringView url) noexcept
  {
    return StateObjectAdded(krys::move(data), url, NavigationHistoryBehavior::Push);
  }

  dom::ExceptionOr<void> History::ReplaceState(RefPtr<html::SerializedScriptValue> &&data, dom::DOMStringView,
                                               dom::DOMStringView url) noexcept
  {
    return StateObjectAdded(krys::move(data), url, NavigationHistoryBehavior::Replace);
  }

  dom::ExceptionOr<void>
    History::UpdateAndCheckStateObjectQuota(const url::URL &fullURL,
                                            NavigationHistoryBehavior historyBehavior) noexcept
  {
    constexpr MonotonicTime::duration stateObjectTimeSpan = std::chrono::seconds(10);
    constexpr uint32 perStateObjectTimeSpanLimit = 100;
    // Each document is only allowed to keep 64MB of state object payload.
    constexpr uint64 totalStateObjectPayloadLimit = 64ull * 1'024 * 1'024;

    // FIXME: The state object's size, once a state can tell its size.
    uint64 payloadSize = static_cast<uint64>(fullURL.String().size()) * 2;

    auto functionName = historyBehavior == NavigationHistoryBehavior::Replace ? "history.replaceState()"
                                                                              : "history.pushState()";
    auto currentTimestamp = html::EventLoop::Current().Now();
    if (currentTimestamp - _currentStateObjectTimeSpanStart > stateObjectTimeSpan)
    {
      _currentStateObjectTimeSpanStart = currentTimestamp;
      _currentStateObjectTimeSpanObjectsAdded = 0;
    }

    if (_currentStateObjectTimeSpanObjectsAdded >= perStateObjectTimeSpanLimit)
    {
      auto message = std::format("Attempt to use {} more than {} times per {} seconds", functionName,
                                 perStateObjectTimeSpanLimit, 10);
      return dom::Exception {dom::ExceptionCode::SecurityError, utf8_string(message.begin(), message.end())};
    }

    auto newTotalUsage = _totalStateObjectUsage;
    if (historyBehavior == NavigationHistoryBehavior::Replace)
    {
      newTotalUsage -= std::min(newTotalUsage, _mostRecentStateObjectUsage);
    }
    newTotalUsage += payloadSize;

    if (newTotalUsage > totalStateObjectPayloadLimit)
    {
      auto message = std::format("Attempt to store more data than allowed using {}", functionName);
      return dom::Exception {dom::ExceptionCode::QuotaExceededError,
                             utf8_string(message.begin(), message.end())};
    }

    _totalStateObjectUsage = newTotalUsage;
    ++_currentStateObjectTimeSpanObjectsAdded;
    _mostRecentStateObjectUsage = payloadSize;
    return {};
  }

  dom::ExceptionOr<void> History::StateObjectAdded(RefPtr<html::SerializedScriptValue> &&data,
                                                   dom::DOMStringView urlString,
                                                   NavigationHistoryBehavior historyBehavior) noexcept
  {
    // https://html.spec.whatwg.org/#shared-history-push/replace-state-steps
    // 1. Let document be history's relevant global object's associated Document.
    // 2. If document is not fully active, then throw a "SecurityError" DOMException.
    RefPtr<LocalFrame> frame = ShareRefPtr(Frame());
    if (!IsDocumentFullyActive(frame.get()))
    {
      return DocumentNotFullyActive();
    }

    if (!frame->GetPage())
    {
      return {};
    }

    // 3. Let serializedData be StructuredSerializeForStorage(data). Rethrow any exceptions.
    // (The state is serialized already.)
    // 4. Let newURL be document's URL.
    RefPtr<dom::Document> document = ShareRefPtr(frame->Document());
    const auto &documentURL = document->GetURL();
    auto fullURL = documentURL;

    auto blocked = [&](utf8_stringview suffix)
    {
      auto functionName = historyBehavior == NavigationHistoryBehavior::Replace ? u8"history.replaceState()"
                                                                                : u8"history.pushState()";
      utf8_string message = u8"Blocked attempt to use ";
      message += functionName;
      message += u8" to change session history URL from ";
      message += documentURL.String();
      message += u8" to ";
      message += fullURL.String();
      message += u8". ";
      message += suffix;
      return dom::Exception {dom::ExceptionCode::SecurityError, krys::move(message)};
    };

    // 5. If url is not null or the empty string:
    if (!urlString.empty())
    {
      // 5a. Set newURL to the result of encoding-parsing a URL given url, relative to the relevant settings
      //     object of history.
      fullURL = document->CompleteURL(urlString);
      // 5b. If newURL is failure, then throw a "SecurityError" DOMException.
      if (!fullURL.IsValid())
      {
        return blocked(u8"URL is invalid");
      }

      // 5c. If document cannot have its URL rewritten to newURL, then throw a "SecurityError" DOMException.
      if (!CanHaveURLRewritten(*document, fullURL))
      {
        return blocked(
          u8"Only the path, query and fragment of an http: or https: URL, the query and fragment "
          u8"of a file: URL, and the fragment of any other may change.");
      }
    }

    // 6. If history's relevant global object's navigable's allowed to perform a navigation or history update
    //    returns blocked, then return.
    // (Too many calls in a short time, or too much state, is an exception instead.)
    if (auto result = UpdateAndCheckStateObjectQuota(fullURL, historyBehavior); result.HasException())
    {
      return result.ReleaseException();
    }

    // 7. Let navigation be history's relevant global object's navigation API.
    // 8. Let continue be the result of firing a push/replace/reload navigate event at navigation with
    //    navigationType set to historyHandling, isSameDocument set to true, destinationURL set to newURL, and
    //    classicHistoryAPIState set to serializedData.
    // 9. If continue is false, then return.
    if (!document->Window()->GetNavigation().DispatchPushReplaceReloadNavigateEvent(
          fullURL,
          historyBehavior == NavigationHistoryBehavior::Push ? NavigationNavigationType::Push
                                                             : NavigationNavigationType::Replace,
          true, nullptr, data.get()))
    {
      return {};
    }

    // 10. Run the URL and history update steps given document and newURL, with serializedData set to
    //     serializedData and historyHandling set to historyHandling.
    frame->Loader().UpdateURLAndHistory(fullURL, krys::move(data), historyBehavior);
    return {};
  }
}
