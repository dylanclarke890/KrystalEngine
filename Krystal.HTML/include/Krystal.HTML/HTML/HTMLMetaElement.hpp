#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmlmetaelement
  class HTMLMetaElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLMetaElement);

  public:
    HTMLMetaElement(Document &document) noexcept;

#pragma region HTMLMetaElement - https://html.spec.whatwg.org/#htmlmetaelement

    /// @see https://html.spec.whatwg.org/#dom-meta-name
    KRYS_NODISCARD DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meta-name
    void Name(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meta-httpequiv
    KRYS_NODISCARD DOMString HttpEquiv() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meta-httpequiv
    void HttpEquiv(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meta-content
    KRYS_NODISCARD DOMString Content() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meta-content
    void Content(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meta-media
    KRYS_NODISCARD DOMString Media() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meta-media
    void Media(DOMString &&value) noexcept;

#pragma endregion

#pragma region HTMLMetaElement Obsolete members - https://html.spec.whatwg.org/#HTMLMetaElement-partial

    /// @see https://html.spec.whatwg.org/#dom-meta-scheme
    KRYS_NODISCARD DOMString Scheme() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meta-scheme
    void Scheme(DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLMetaElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLMetaElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();