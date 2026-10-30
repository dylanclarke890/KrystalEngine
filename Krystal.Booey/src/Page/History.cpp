#include "Krystal.Booey/Page/History.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
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

  dom::ExceptionOr<uint32> History::Length() const noexcept
  {
    auto *frame = Frame();
    if (!IsDocumentFullyActive(frame))
    {
      return DocumentNotFullyActive();
    }
    auto *page = frame->GetPage();
    if (!page)
    {
      return 0u;
    }
    return page->BackForward().Count();
  }

  dom::ExceptionOr<History::ScrollRestoration> History::GetScrollRestoration() const noexcept
  {
    auto *frame = Frame();
    if (!IsDocumentFullyActive(frame))
    {
      return DocumentNotFullyActive();
    }
    auto *historyItem = frame->Loader().History().CurrentItem();
    if (!historyItem)
    {
      return ScrollRestoration::Auto;
    }
    return historyItem->ShouldRestoreScrollPosition() ? ScrollRestoration::Auto : ScrollRestoration::Manual;
  }

  dom::ExceptionOr<void> History::SetScrollRestoration(ScrollRestoration scrollRestoration) noexcept
  {
    auto *frame = Frame();
    if (!IsDocumentFullyActive(frame))
    {
      return DocumentNotFullyActive();
    }
    if (auto *historyItem = frame->Loader().History().CurrentItem())
    {
      historyItem->SetShouldRestoreScrollPosition(scrollRestoration == ScrollRestoration::Auto);
    }
    return {};
  }

  dom::ExceptionOr<html::SerializedScriptValue *> History::State() noexcept
  {
    auto *frame = Frame();
    if (!IsDocumentFullyActive(frame))
    {
      return DocumentNotFullyActive();
    }
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

  dom::ExceptionOr<void> History::Go(int32 distance) noexcept
  {
    RefPtr<LocalFrame> frame = ShareRefPtr(Frame());
    if (!IsDocumentFullyActive(frame.get()))
    {
      return DocumentNotFullyActive();
    }
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
    RefPtr<LocalFrame> frame = ShareRefPtr(Frame());
    if (!IsDocumentFullyActive(frame.get()))
    {
      return DocumentNotFullyActive();
    }

    if (!frame->GetPage())
    {
      return {};
    }

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

    if (!urlString.empty())
    {
      fullURL = document->CompleteURL(urlString);
      if (!fullURL.IsValid())
      {
        return blocked(u8"URL is invalid");
      }

      // The checks below are loosely implementing https://html.spec.whatwg.org/#can-have-its-url-rewritten
      if (!url::ProtocolHostAndPortAreEqual(fullURL, documentURL) || fullURL.User() != documentURL.User()
          || fullURL.Password() != documentURL.Password())
      {
        return blocked(u8"Protocols, domains, ports, usernames, and passwords must match.");
      }

      if (fullURL.ProtocolIsFile() && fullURL.FileSystemPath() != documentURL.FileSystemPath())
      {
        return blocked(u8"Only differences in query and fragment are allowed for file: URLs.");
      }

      // FIXME: The sandboxed documents' and origins' checks, once there are origins.
    }

    if (auto result = UpdateAndCheckStateObjectQuota(fullURL, historyBehavior); result.HasException())
    {
      return result.ReleaseException();
    }

    if (!document->Window()->GetNavigation().DispatchPushReplaceReloadNavigateEvent(
          fullURL,
          historyBehavior == NavigationHistoryBehavior::Push ? NavigationNavigationType::Push
                                                             : NavigationNavigationType::Replace,
          true, nullptr, data.get()))
    {
      return {};
    }

    frame->Loader().UpdateURLAndHistory(fullURL, krys::move(data), historyBehavior);
    return {};
  }
}
