#include "Krystal.Booey/Page/Navigator.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/URLDecomposition.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Loader/LoaderStrategy.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Page/UserActivation.hpp"
#include "Krystal.Booey/Platform/Language.hpp"
#include "Krystal.Booey/Platform/PlatformStrategies.hpp"
#include "Krystal.Booey/Plugins/DOMMimeTypeArray.hpp"
#include "Krystal.Booey/Plugins/DOMPluginArray.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <algorithm>
#include <array>
#include <thread>

namespace krys::boo
{
  namespace
  {
    /// @see https://html.spec.whatwg.org/#safelisted-scheme
    constexpr std::array<utf8_stringview, 24> SafelistedSchemes {
      u8"bitcoin", u8"ftp",    u8"ftps", u8"geo",  u8"im",   u8"irc",         u8"ircs", u8"magnet",
      u8"mailto",  u8"matrix", u8"mms",  u8"news", u8"nntp", u8"openpgp4fpr", u8"sftp", u8"sip",
      u8"sms",     u8"smsto",  u8"ssh",  u8"tel",  u8"urn",  u8"webcal",      u8"wtai", u8"xmpp"};
  }

  Navigator::Navigator(LocalDOMWindow &window) noexcept : _window(CreateWeakPtr(&window))
  {
  }

  Navigator::~Navigator() noexcept = default;

  LocalFrame *Navigator::Frame() const noexcept
  {
    auto *window = _window.get();
    return window ? window->Frame() : nullptr;
  }

  dom::DOMString Navigator::AppVersion() const noexcept
  {
    // Version is everything in the user agent string past the "Mozilla/" prefix.
    dom::DOMStringView agent = UserAgent();
    auto slash = agent.find(u8'/');
    return dom::DOMString(slash == dom::DOMStringView::npos ? agent : agent.substr(slash + 1));
  }

  const dom::DOMString &Navigator::UserAgent() const noexcept
  {
    auto *frame = Frame();
    if (!frame || !frame->GetPage())
    {
      return _userAgent;
    }
    if (_userAgent.empty())
    {
      _userAgent = frame->Loader().Client().UserAgent(frame->Document()->GetURL());
    }
    return _userAgent;
  }

  dom::DOMString Navigator::Language() const noexcept
  {
    auto languages = UserPreferredLanguages();
    return languages.empty() ? dom::DOMString(u8"en-US") : dom::DOMString(languages.front());
  }

  List<dom::DOMString> Navigator::Languages() const noexcept
  {
    // We intentionally expose only the primary language for privacy reasons.
    return {Language()};
  }

  bool Navigator::OnLine() const noexcept
  {
    auto *loaderStrategy = PlatformStrategies::Singleton().GetLoaderStrategy();
    return !loaderStrategy || loaderStrategy->IsOnLine();
  }

  dom::ExceptionOr<Pair<dom::DOMString, dom::DOMString>>
    Navigator::NormalizeProtocolHandlerParameters(dom::DOMStringView scheme,
                                                  utf8_stringview url) const noexcept
  {
    auto *window = _window.get();
    auto *document = window ? window->Document() : nullptr;
    if (!document)
    {
      return dom::ExceptionCode::InvalidStateError;
    }

    // Set scheme to scheme, converted to ASCII lowercase.
    utf8_string normalizedScheme = text::ToASCIILower(utf8_stringview(scheme));

    // If scheme is neither a safelisted scheme nor a string starting with "web+" followed by one or more
    // ASCII lower alphas, then throw a "SecurityError" DOMException.
    bool isSafelisted =
      std::ranges::find(SafelistedSchemes, utf8_stringview(normalizedScheme)) != SafelistedSchemes.end();
    bool isWebScheme = normalizedScheme.starts_with(u8"web+") && normalizedScheme.size() > 4
                       && std::all_of(normalizedScheme.begin() + 4, normalizedScheme.end(),
                                      [](char8 c) { return c >= u8'a' && c <= u8'z'; });
    if (!isSafelisted && !isWebScheme)
    {
      return dom::ExceptionCode::SecurityError;
    }

    // If url does not contain "%s", then throw a "SyntaxError" DOMException.
    if (url.find(u8"%s") == utf8_stringview::npos)
    {
      return dom::ExceptionCode::SyntaxError;
    }

    // Let urlRecord be the result of encoding-parsing a URL given url, relative to environment; if failure,
    // throw a "SyntaxError" DOMException.
    auto urlRecord = document->CompleteURL(url);
    if (!urlRecord.IsValid())
    {
      return dom::ExceptionCode::SyntaxError;
    }

    // If urlRecord's scheme is not an HTTP(S) scheme or urlRecord's origin is not same origin with
    // environment's origin, then throw a "SecurityError" DOMException.
    if (!(urlRecord.ProtocolIs(u8"http") || urlRecord.ProtocolIs(u8"https"))
        || html::URLDecomposition::SerializedOrigin(urlRecord) != window->Origin())
    {
      return dom::ExceptionCode::SecurityError;
    }

    return Pair<dom::DOMString, dom::DOMString>(dom::DOMString(normalizedScheme),
                                                dom::DOMString(urlRecord.String()));
  }

  dom::ExceptionOr<void> Navigator::RegisterProtocolHandler(dom::DOMStringView scheme,
                                                            utf8_stringview url) noexcept
  {
    auto normalized = NormalizeProtocolHandlerParameters(scheme, url);
    if (normalized.HasException())
    {
      return normalized.ReleaseException();
    }

    // In parallel: register a protocol handler (the user agent may ignore it).
    if (auto *frame = Frame(); frame && frame->GetPage())
    {
      auto &[normalizedScheme, normalizedURL] = normalized.Value();
      frame->GetPage()->GetChromeClient().RegisterProtocolHandler(normalizedScheme, normalizedURL);
    }
    return {};
  }

  dom::ExceptionOr<void> Navigator::UnregisterProtocolHandler(dom::DOMStringView scheme,
                                                              utf8_stringview url) noexcept
  {
    auto normalized = NormalizeProtocolHandlerParameters(scheme, url);
    if (normalized.HasException())
    {
      return normalized.ReleaseException();
    }

    if (auto *frame = Frame(); frame && frame->GetPage())
    {
      auto &[normalizedScheme, normalizedURL] = normalized.Value();
      frame->GetPage()->GetChromeClient().UnregisterProtocolHandler(normalizedScheme, normalizedURL);
    }
    return {};
  }

  DOMPluginArray &Navigator::Plugins() const noexcept
  {
    if (!_plugins)
    {
      _plugins = DOMPluginArray::Create();
    }
    return *_plugins;
  }

  DOMMimeTypeArray &Navigator::MimeTypes() const noexcept
  {
    if (!_mimeTypes)
    {
      _mimeTypes = DOMMimeTypeArray::Create();
    }
    return *_mimeTypes;
  }

  uint64 Navigator::HardwareConcurrency() const noexcept
  {
    // Enforce a maximum for the number of cores reported to mitigate fingerprinting for the minority of
    // machines with large numbers of cores.
    static uint64 numberOfCores = std::thread::hardware_concurrency() < 8 ? 4 : 8;
    return numberOfCores;
  }

  UserActivation &Navigator::GetUserActivation() const noexcept
  {
    if (!_userActivation)
    {
      if (auto *window = _window.get())
      {
        _userActivation = UserActivation::Create(*window);
      }
    }
    return *_userActivation;
  }

  UserActivation::UserActivation(LocalDOMWindow &window) noexcept : _window(CreateWeakPtr(&window))
  {
  }

  bool UserActivation::HasBeenActive() const noexcept
  {
    auto *window = _window.get();
    return window && window->HasStickyActivation();
  }

  bool UserActivation::IsActive() const noexcept
  {
    auto *window = _window.get();
    return window && window->HasTransientActivation();
  }
}
