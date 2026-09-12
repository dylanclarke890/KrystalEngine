#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/DOM/Types/USVString.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlembedelement
  class HTMLEmbedElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLEmbedElement);

  public:
    HTMLEmbedElement(Document &document) noexcept;

#pragma region HTMLEmbedElement - https://html.spec.whatwg.org/#htmlembedelement

    /// @see https://html.spec.whatwg.org/#dom-embed-src
    KRYS_NODISCARD dom::ExceptionOr<USVString> Src() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-embed-src
    dom::ExceptionOr<void> Src(USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-embed-type
    KRYS_NODISCARD dom::DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-embed-type
    void Type(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-embed-width
    KRYS_NODISCARD dom::DOMString Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-embed-width
    void Width(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-embed-height
    KRYS_NODISCARD dom::DOMString Height() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-embed-height
    void Height(dom::DOMString value) noexcept;

    // TODO(HTMLEmbedElement, HTML): Document? getSVGDocument();

#pragma endregion

#pragma region HTMLEmbedElement Obsolete members - https://html.spec.whatwg.org/#HTMLEmbedElement-partial

    /// @see https://html.spec.whatwg.org/#dom-embed-align
    KRYS_NODISCARD dom::DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-embed-align
    void Align(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-embed-name
    KRYS_NODISCARD dom::DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-embed-name
    void Name(dom::DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLEmbedElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLEmbedElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();