#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"
#include "Krystal.HTML/Types/USVString.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmlembedelement
  class HTMLEmbedElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLEmbedElement);

  public:
    HTMLEmbedElement(Document &document) noexcept;

#pragma region HTMLEmbedElement - https://html.spec.whatwg.org/#htmlembedelement

    /// @see https://html.spec.whatwg.org/#dom-embed-src
    KRYS_NODISCARD ExceptionOr<USVString> Src() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-embed-src
    ExceptionOr<void> Src(USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-embed-type
    KRYS_NODISCARD DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-embed-type
    void Type(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-embed-width
    KRYS_NODISCARD DOMString Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-embed-width
    void Width(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-embed-height
    KRYS_NODISCARD DOMString Height() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-embed-height
    void Height(DOMString value) noexcept;

    // TODO(HTMLEmbedElement, HTML): Document? getSVGDocument();

#pragma endregion

#pragma region HTMLEmbedElement Obsolete members - https://html.spec.whatwg.org/#HTMLEmbedElement-partial

    /// @see https://html.spec.whatwg.org/#dom-embed-align
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-embed-align
    void Align(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-embed-name
    KRYS_NODISCARD DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-embed-name
    void Name(DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLEmbedElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLEmbedElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();