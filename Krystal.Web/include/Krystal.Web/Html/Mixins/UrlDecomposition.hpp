/*
 * Copyright (C) 2014-2020 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Web/Dom/Types/USVString.hpp"

namespace krys::net
{
  class Url;
}

namespace krys::web::html
{
  /// @brief The parts of a URL, read and written through an element's URL: the members of
  /// `HTMLHyperlinkElementUtils` other than `href`.
  /// @see https://html.spec.whatwg.org/multipage/links.html#htmlhyperlinkelementutils
  class UrlDecomposition
  {
  public:
    /// @see https://html.spec.whatwg.org/multipage/links.html#dom-hyperlink-origin
    KRYS_NODISCARD dom::USVString Origin() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/links.html#dom-hyperlink-protocol
    KRYS_NODISCARD dom::USVString Protocol() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/links.html#dom-hyperlink-protocol
    void Protocol(DOMStringView value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/links.html#dom-hyperlink-username
    KRYS_NODISCARD dom::USVString Username() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/links.html#dom-hyperlink-username
    void Username(DOMStringView user) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/links.html#dom-hyperlink-password
    KRYS_NODISCARD dom::USVString Password() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/links.html#dom-hyperlink-password
    void Password(DOMStringView password) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/links.html#dom-hyperlink-host
    KRYS_NODISCARD dom::USVString Host() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/links.html#dom-hyperlink-host
    void Host(DOMStringView value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/links.html#dom-hyperlink-hostname
    KRYS_NODISCARD dom::USVString Hostname() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/links.html#dom-hyperlink-hostname
    void Hostname(DOMStringView host) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/links.html#dom-hyperlink-port
    KRYS_NODISCARD dom::USVString Port() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/links.html#dom-hyperlink-port
    void Port(DOMStringView value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/links.html#dom-hyperlink-pathname
    KRYS_NODISCARD dom::USVString Pathname() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/links.html#dom-hyperlink-pathname
    void Pathname(DOMStringView value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/links.html#dom-hyperlink-search
    KRYS_NODISCARD dom::USVString Search() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/links.html#dom-hyperlink-search
    void Search(DOMStringView value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/links.html#dom-hyperlink-hash
    KRYS_NODISCARD dom::USVString Hash() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/links.html#dom-hyperlink-hash
    void Hash(DOMStringView value) noexcept;

    /// @brief Parses @p string as the port of a URL with the scheme @p protocol (the URL standard's port
    /// state with a state override).
    /// @return Null if it doesn't parse; a null port for no port (or the scheme's default port).
    KRYS_NODISCARD static Maybe<Maybe<uint16>> ParsePort(DOMStringView string,
                                                         DOMStringView protocol) noexcept;

    /// @brief The serialization of @p url's origin: `null` for an opaque origin.
    /// @see https://html.spec.whatwg.org/multipage/browsers.html#ascii-serialisation-of-an-origin
    KRYS_NODISCARD static dom::USVString SerializedOrigin(const net::Url &url) noexcept;

  protected:
    UrlDecomposition() noexcept = default;

    virtual ~UrlDecomposition() noexcept = default;

  private:
    KRYS_NODISCARD virtual net::Url FullUrl() const noexcept = 0;

    virtual void SetFullUrl(const net::Url &url) noexcept = 0;
  };
}
