#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class DOMMimeTypeArray;
  class DOMPluginArray;
  class LocalDOMWindow;
  class LocalFrame;
  class UserActivation;

  /// @brief The user agent's identity and state: what `window.navigator` reports.
  /// @see https://html.spec.whatwg.org/#the-navigator-object
  class Navigator final : public RefCounted<Navigator>
  {
    KRYS_NON_COPY_MOVABLE(Navigator);

  private:
    WeakPtr<LocalDOMWindow> _window;
    mutable dom::DOMString _userAgent;
    mutable RefPtr<DOMPluginArray> _plugins;
    mutable RefPtr<DOMMimeTypeArray> _mimeTypes;
    mutable RefPtr<UserActivation> _userActivation;

  public:
    explicit Navigator(LocalDOMWindow &window) noexcept;
    ~Navigator() noexcept;

    KRYS_NODISCARD static Ref<Navigator> Create(LocalDOMWindow &window) noexcept
    {
      return CreateRef<Navigator>(window);
    }

#pragma region NavigatorID - https://html.spec.whatwg.org/#navigatorid

    /// @see https://html.spec.whatwg.org/#dom-navigator-appcodename
    KRYS_NODISCARD dom::DOMString AppCodeName() const noexcept
    {
      return dom::DOMString(u8"Mozilla");
    }

    /// @see https://html.spec.whatwg.org/#dom-navigator-appname
    KRYS_NODISCARD dom::DOMString AppName() const noexcept
    {
      return dom::DOMString(u8"Netscape");
    }

    /// @brief The user agent string after `Mozilla/`.
    /// @see https://html.spec.whatwg.org/#dom-navigator-appversion
    KRYS_NODISCARD dom::DOMString AppVersion() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-navigator-platform
    KRYS_NODISCARD dom::DOMString Platform() const noexcept
    {
      return dom::DOMString(u8"Win32");
    }

    /// @see https://html.spec.whatwg.org/#dom-navigator-product
    KRYS_NODISCARD dom::DOMString Product() const noexcept
    {
      return dom::DOMString(u8"Gecko");
    }

    /// @see https://html.spec.whatwg.org/#dom-navigator-productsub
    KRYS_NODISCARD dom::DOMString ProductSub() const noexcept
    {
      return dom::DOMString(u8"20030107");
    }

    /// @brief The frame loader client's user agent string for the document's URL.
    /// @see https://html.spec.whatwg.org/#dom-navigator-useragent
    KRYS_NODISCARD const dom::DOMString &UserAgent() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-navigator-vendor
    KRYS_NODISCARD dom::DOMString Vendor() const noexcept
    {
      return {};
    }

    /// @see https://html.spec.whatwg.org/#dom-navigator-vendorsub
    KRYS_NODISCARD dom::DOMString VendorSub() const noexcept
    {
      return {};
    }

    /// @note Obsolete: always false.
    /// @see https://html.spec.whatwg.org/#dom-navigator-taintenabled
    KRYS_NODISCARD bool TaintEnabled() const noexcept
    {
      return false;
    }

    /// @brief The user agent string changed: it's read again from the client.
    void UserAgentChanged() noexcept
    {
      _userAgent = {};
    }

#pragma endregion

#pragma region NavigatorLanguage - https://html.spec.whatwg.org/#navigatorlanguage

    /// @brief The user's most preferred language.
    /// @see https://html.spec.whatwg.org/#dom-navigator-language
    KRYS_NODISCARD dom::DOMString Language() const noexcept;

    /// @brief The user's preferred languages: only the most preferred, as upstream exposes for privacy.
    /// @see https://html.spec.whatwg.org/#dom-navigator-languages
    KRYS_NODISCARD List<dom::DOMString> Languages() const noexcept;

#pragma endregion

#pragma region NavigatorOnLine - https://html.spec.whatwg.org/#navigator.online

    /// @brief Whether the host says the network is reachable (`LoaderStrategy::IsOnLine`).
    /// @see https://html.spec.whatwg.org/#dom-navigator-online
    KRYS_NODISCARD bool OnLine() const noexcept;

#pragma endregion

#pragma region NavigatorContentUtils - https://html.spec.whatwg.org/#custom-handlers

    /// @brief Asks the host to register @p url (with `%s` for the URL handled) as the handler of @p scheme.
    /// @return `SecurityError` if @p scheme isn't safelisted or `web+` and letters, or @p url isn't an
    /// HTTP(S) URL of the document's origin; `SyntaxError` if @p url has no `%s` or can't be parsed.
    /// @see https://html.spec.whatwg.org/#dom-navigator-registerprotocolhandler
    dom::ExceptionOr<void> RegisterProtocolHandler(dom::DOMStringView scheme, utf8_stringview url) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-navigator-unregisterprotocolhandler
    dom::ExceptionOr<void> UnregisterProtocolHandler(dom::DOMStringView scheme, utf8_stringview url) noexcept;

#pragma endregion

#pragma region NavigatorCookies - https://html.spec.whatwg.org/#cookies

    /// @brief Whether cookies can be set: never, as there are no cookies without a network.
    /// @see https://html.spec.whatwg.org/#dom-navigator-cookieenabled
    KRYS_NODISCARD bool CookieEnabled() const noexcept
    {
      return false;
    }

#pragma endregion

#pragma region NavigatorPlugins - https://html.spec.whatwg.org/#navigatorplugins

    /// @brief No plugins, as there's no PDF viewer.
    /// @see https://html.spec.whatwg.org/#dom-navigator-plugins
    KRYS_NODISCARD DOMPluginArray &Plugins() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-navigator-mimetypes
    KRYS_NODISCARD DOMMimeTypeArray &MimeTypes() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-navigator-javaenabled
    KRYS_NODISCARD bool JavaEnabled() const noexcept
    {
      return false;
    }

    /// @see https://html.spec.whatwg.org/#dom-navigator-pdfviewerenabled
    KRYS_NODISCARD bool PdfViewerEnabled() const noexcept
    {
      return false;
    }

#pragma endregion

#pragma region NavigatorConcurrentHardware - https://html.spec.whatwg.org/#navigatorconcurrenthardware

    /// @brief The number of logical processors: 4 below 8, 8 from 8 (capped, as upstream does).
    /// @see https://html.spec.whatwg.org/#dom-navigator-hardwareconcurrency
    KRYS_NODISCARD uint64 HardwareConcurrency() const noexcept;

#pragma endregion

    /// @see https://html.spec.whatwg.org/#dom-navigator-useractivation
    KRYS_NODISCARD UserActivation &GetUserActivation() const noexcept;

  private:
    KRYS_NODISCARD LocalFrame *Frame() const noexcept;

    /// @brief The steps both registering and unregistering run on their arguments.
    /// @return The scheme, lowercased, and the URL, completed.
    /// @see https://html.spec.whatwg.org/#normalize-protocol-handler-parameters
    KRYS_NODISCARD dom::ExceptionOr<Pair<dom::DOMString, dom::DOMString>>
      NormalizeProtocolHandlerParameters(dom::DOMStringView scheme, utf8_stringview url) const noexcept;
  };
}
