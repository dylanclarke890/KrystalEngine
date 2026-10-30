#pragma once

#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::url
{
  class URL;
}

namespace krys::boo::html
{
  /// @brief The parts of a URL, read and written through an element's URL: the members of
  /// `HTMLHyperlinkElementUtils` other than `href`.
  /// @see https://html.spec.whatwg.org/#htmlhyperlinkelementutils
  class URLDecomposition
  {
  public:
    /// @see https://html.spec.whatwg.org/#dom-hyperlink-origin
    KRYS_NODISCARD dom::USVString Origin() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hyperlink-protocol
    KRYS_NODISCARD dom::USVString Protocol() const noexcept;
    /// @see https://html.spec.whatwg.org/#dom-hyperlink-protocol
    void Protocol(dom::DOMStringView value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hyperlink-username
    KRYS_NODISCARD dom::USVString Username() const noexcept;
    /// @see https://html.spec.whatwg.org/#dom-hyperlink-username
    void Username(dom::DOMStringView user) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hyperlink-password
    KRYS_NODISCARD dom::USVString Password() const noexcept;
    /// @see https://html.spec.whatwg.org/#dom-hyperlink-password
    void Password(dom::DOMStringView password) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hyperlink-host
    KRYS_NODISCARD dom::USVString Host() const noexcept;
    /// @see https://html.spec.whatwg.org/#dom-hyperlink-host
    void Host(dom::DOMStringView value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hyperlink-hostname
    KRYS_NODISCARD dom::USVString Hostname() const noexcept;
    /// @see https://html.spec.whatwg.org/#dom-hyperlink-hostname
    void Hostname(dom::DOMStringView host) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hyperlink-port
    KRYS_NODISCARD dom::USVString Port() const noexcept;
    /// @see https://html.spec.whatwg.org/#dom-hyperlink-port
    void Port(dom::DOMStringView value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hyperlink-pathname
    KRYS_NODISCARD dom::USVString Pathname() const noexcept;
    /// @see https://html.spec.whatwg.org/#dom-hyperlink-pathname
    void Pathname(dom::DOMStringView value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hyperlink-search
    KRYS_NODISCARD dom::USVString Search() const noexcept;
    /// @see https://html.spec.whatwg.org/#dom-hyperlink-search
    void Search(dom::DOMStringView value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hyperlink-hash
    KRYS_NODISCARD dom::USVString Hash() const noexcept;
    /// @see https://html.spec.whatwg.org/#dom-hyperlink-hash
    void Hash(dom::DOMStringView value) noexcept;

    /// @brief Parses @p string as the port of a URL with the scheme @p protocol (the URL standard's port
    /// state with a state override).
    /// @return Null if it doesn't parse; a null port for no port (or the scheme's default port).
    KRYS_NODISCARD static Maybe<Maybe<uint16>> ParsePort(dom::DOMStringView string,
                                                         dom::DOMStringView protocol) noexcept;

    /// @brief The serialization of @p url's origin: `null` for an opaque origin.
    /// @see https://html.spec.whatwg.org/#ascii-serialisation-of-an-origin
    KRYS_NODISCARD static dom::USVString SerializedOrigin(const url::URL &url) noexcept;

  protected:
    URLDecomposition() noexcept = default;
    virtual ~URLDecomposition() noexcept = default;

  private:
    KRYS_NODISCARD virtual url::URL FullURL() const noexcept = 0;
    virtual void SetFullURL(const url::URL &url) noexcept = 0;
  };
}
