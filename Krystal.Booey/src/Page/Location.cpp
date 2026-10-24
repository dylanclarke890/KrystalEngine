#include "Krystal.Booey/Page/Location.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Loader/NavigationScheduler.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/URL/URL.hpp"

namespace krys::boo
{
  Location::Location(LocalDOMWindow &window) noexcept : _window(CreateWeakPtr(&window))
  {
  }

  Location::~Location() noexcept = default;

  url::URL Location::FullURL() const noexcept
  {
    auto *window = _window.get();
    auto *document = window ? window->Document() : nullptr;
    return document ? document->GetURL() : url::URL(u8"about:blank");
  }

  void Location::SetFullURL(const url::URL &url) noexcept
  {
    Navigate(url);
  }

  dom::USVString Location::Href() const noexcept
  {
    return dom::USVString(FullURL().String());
  }

  dom::ExceptionOr<void> Location::SetHref(utf8_stringview url) noexcept
  {
    return Assign(url);
  }

  dom::ExceptionOr<void> Location::Assign(utf8_stringview url) noexcept
  {
    auto *window = _window.get();
    auto *document = window ? window->Document() : nullptr;
    if (!document || !window->Frame())
    {
      return {};
    }

    // Encoding-parse a URL given url, relative to the entry settings object; if failure, throw a
    // "SyntaxError" DOMException.
    auto urlRecord = document->CompleteURL(url);
    if (!urlRecord.IsValid())
    {
      return dom::ExceptionCode::SyntaxError;
    }
    Navigate(urlRecord);
    return {};
  }

  dom::ExceptionOr<void> Location::Replace(utf8_stringview url) noexcept
  {
    auto *window = _window.get();
    auto *document = window ? window->Document() : nullptr;
    if (!document || !window->Frame())
    {
      return {};
    }

    auto urlRecord = document->CompleteURL(url);
    if (!urlRecord.IsValid())
    {
      return dom::ExceptionCode::SyntaxError;
    }
    Navigate(urlRecord, NavigationHistoryBehavior::Replace);
    return {};
  }

  void Location::Reload() noexcept
  {
    auto *window = _window.get();
    auto *document = window ? window->Document() : nullptr;
    auto *frame = window ? window->Frame() : nullptr;
    if (!document || !frame)
    {
      return;
    }
    frame->GetNavigationScheduler().ScheduleRefresh(*document);
  }

  void Location::Navigate(const url::URL &url, NavigationHistoryBehavior historyHandling) noexcept
  {
    RefPtr<LocalDOMWindow> window = ShareRefPtr(_window.get());
    auto *document = window ? window->Document() : nullptr;
    auto *frame = window ? window->Frame() : nullptr;
    if (!document || !frame)
    {
      return;
    }

    // If this's relevant Document is not yet completely loaded, and the incumbent global object does not have
    // transient activation, then set historyHandling to "replace".
    if (!document->LoadEventFinished() && !window->HasTransientActivation())
    {
      historyHandling = NavigationHistoryBehavior::Replace;
    }

    // Navigate the navigable to url using sourceDocument, with historyHandling.
    frame->GetNavigationScheduler().ScheduleLocationChange(*document, url, historyHandling);
  }
}
