#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlmetaelement
  class HTMLMetaElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLMetaElement);

  public:
    HTMLMetaElement(dom::Document &document) noexcept;

#pragma region HTMLMetaElement - https://html.spec.whatwg.org/#htmlmetaelement

    /// @see https://html.spec.whatwg.org/#dom-meta-name
    KRYS_NODISCARD dom::DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meta-name
    void Name(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meta-httpequiv
    KRYS_NODISCARD dom::DOMString HttpEquiv() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meta-httpequiv
    void HttpEquiv(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meta-content
    KRYS_NODISCARD dom::DOMString Content() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meta-content
    void Content(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meta-media
    KRYS_NODISCARD dom::DOMString Media() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meta-media
    void Media(dom::DOMString &&value) noexcept;

#pragma endregion

#pragma region HTMLMetaElement Obsolete members - https://html.spec.whatwg.org/#HTMLMetaElement-partial

    /// @see https://html.spec.whatwg.org/#dom-meta-scheme
    KRYS_NODISCARD dom::DOMString Scheme() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meta-scheme
    void Scheme(dom::DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLMetaElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLMetaElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();