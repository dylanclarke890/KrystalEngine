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

  // https://html.spec.whatwg.org/#dom-location-href
  dom::USVString Location::Href() const noexcept
  {
    // 1. If this's relevant Document is non-null and its origin is not same origin-domain with the entry
    //    settings object's origin, then throw a "SecurityError" DOMException.
    // (There's one origin; the same is so for the other getters.)
    // 2. Return this's url, serialized.
    return dom::USVString(FullURL().String());
  }

  // https://html.spec.whatwg.org/#dom-location-href
  // 1. If this's relevant Document is null, then return.
  // 2. Let url be the result of encoding-parsing a URL given the given value, relative to the entry settings
  //    object.
  // 3. If url is failure, then throw a "SyntaxError" DOMException.
  // 4. Location-object navigate this to url.
  // (The same steps as assign(), whose origin check doesn't apply.)
  dom::ExceptionOr<void> Location::SetHref(utf8_stringview url) noexcept
  {
    return Assign(url);
  }

  // https://html.spec.whatwg.org/#dom-location-protocol
  dom::ExceptionOr<void> Location::Protocol(dom::DOMStringView value) noexcept
  {
    // 1. If this's relevant Document is null, then return.
    auto *window = _window.get();
    if (!window || !window->Document() || !window->Frame())
    {
      return {};
    }
    // 2. If this's relevant Document's origin is not same origin-domain with the entry settings object's
    //    origin, then throw a "SecurityError" DOMException.
    // 3. Let copyURL be a copy of this's url.
    auto copyURL = FullURL();
    // 4. Let possibleFailure be the result of basic URL parsing the given value, followed by ":", with
    // copyURL
    //    as url and scheme start state as state override.
    // 5. If possibleFailure is failure, then throw a "SyntaxError" DOMException.
    if (!copyURL.SetProtocol(value))
    {
      return dom::ExceptionCode::SyntaxError;
    }
    // 6. If copyURL's scheme is not an HTTP(S) scheme, then terminate these steps.
    if (!copyURL.ProtocolIsInHTTPFamily())
    {
      return {};
    }
    // 7. Location-object navigate this to copyURL.
    Navigate(copyURL);
    return {};
  }

  // https://html.spec.whatwg.org/#dom-location-hash
  void Location::Hash(dom::DOMStringView value) noexcept
  {
    // 1. If this's relevant Document is null, then return.
    auto *window = _window.get();
    if (!window || !window->Document() || !window->Frame())
    {
      return;
    }
    // 2. If this's relevant Document's origin is not same origin-domain with the entry settings object's
    //    origin, then throw a "SecurityError" DOMException.
    // 3. Let copyURL be a copy of this's url.
    auto copyURL = FullURL();
    // 4. Let thisURLFragment be copyURL's fragment if it is non-null; otherwise the empty string.
    utf8_string thisURLFragment(copyURL.FragmentIdentifier());
    // 5. Let input be the given value with a single leading "#" removed, if any.
    // 6. Set copyURL's fragment to the empty string.
    // 7. Basic URL parse input, with copyURL as url and fragment state as state override.
    copyURL.SetFragmentIdentifier(value.starts_with(u8'#') ? value.substr(1) : value);
    // 8. If copyURL's fragment is thisURLFragment, then return.
    if (copyURL.FragmentIdentifier() == thisURLFragment)
    {
      return;
    }
    // 9. Location-object navigate this to copyURL.
    Navigate(copyURL);
  }

  // https://html.spec.whatwg.org/#dom-location-assign
  dom::ExceptionOr<void> Location::Assign(utf8_stringview url) noexcept
  {
    // 1. If this's relevant Document is null, then return.
    auto *window = _window.get();
    auto *document = window ? window->Document() : nullptr;
    if (!document || !window->Frame())
    {
      return {};
    }

    // 2. If this's relevant Document's origin is not same origin-domain with the entry settings object's
    //    origin, then throw a "SecurityError" DOMException.
    // 3. Let urlRecord be the result of encoding-parsing a URL given url, relative to the entry settings
    //    object.
    // 4. If urlRecord is failure, then throw a "SyntaxError" DOMException.
    auto urlRecord = document->CompleteURL(url);
    if (!urlRecord.IsValid())
    {
      return dom::ExceptionCode::SyntaxError;
    }
    // 5. Location-object navigate this to urlRecord.
    Navigate(urlRecord);
    return {};
  }

  // https://html.spec.whatwg.org/#dom-location-replace
  dom::ExceptionOr<void> Location::Replace(utf8_stringview url) noexcept
  {
    // 1. If this's relevant Document is null, then return.
    auto *window = _window.get();
    auto *document = window ? window->Document() : nullptr;
    if (!document || !window->Frame())
    {
      return {};
    }

    // 2. Let urlRecord be the result of encoding-parsing a URL given url, relative to the entry settings
    //    object.
    // 3. If urlRecord is failure, then throw a "SyntaxError" DOMException.
    auto urlRecord = document->CompleteURL(url);
    if (!urlRecord.IsValid())
    {
      return dom::ExceptionCode::SyntaxError;
    }
    // 4. Location-object navigate this to urlRecord given "replace".
    Navigate(urlRecord, NavigationHistoryBehavior::Replace);
    return {};
  }

  // https://html.spec.whatwg.org/#dom-location-reload
  void Location::Reload() noexcept
  {
    // 1. Let document be this's relevant Document.
    // 2. If document is null, then return.
    auto *window = _window.get();
    auto *document = window ? window->Document() : nullptr;
    auto *frame = window ? window->Frame() : nullptr;
    if (!document || !frame)
    {
      return;
    }
    // 3. If document's origin is not same origin-domain with the entry settings object's origin, then throw a
    //    "SecurityError" DOMException.
    // 4. Reload document's node navigable.
    frame->GetNavigationScheduler().ScheduleRefresh(*document);
  }

  // https://html.spec.whatwg.org/#location-object-navigate
  void Location::Navigate(const url::URL &url, NavigationHistoryBehavior historyHandling) noexcept
  {
    // 1. Let navigable be location's relevant global object's navigable.
    // 2. Let sourceDocument be the incumbent global object's associated Document.
    RefPtr<LocalDOMWindow> window = ShareRefPtr(_window.get());
    auto *document = window ? window->Document() : nullptr;
    auto *frame = window ? window->Frame() : nullptr;
    if (!document || !frame)
    {
      return;
    }

    // 3. If location's relevant Document is not yet completely loaded, and the incumbent global object does
    //    not have transient activation, then set historyHandling to "replace".
    if (!document->LoadEventFinished() && !window->HasTransientActivation())
    {
      historyHandling = NavigationHistoryBehavior::Replace;
    }

    // 4. Navigate navigable to url using sourceDocument, with exceptionsEnabled set to true and
    //    historyHandling set to historyHandling.
    frame->GetNavigationScheduler().ScheduleLocationChange(*document, url, historyHandling);
  }
}
