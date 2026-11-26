#pragma once

#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Web/Html/Elements/HTMLElement.hpp"

namespace krys::web
{
  /// @see https://html.spec.whatwg.org/multipage/obsolete.html#htmlfontelement
  class HTMLFontElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLFontElement);

  public:
    HTMLFontElement(Document &document) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-font-color
    KRYS_NODISCARD DOMString Color() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-font-color
    void Color(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-font-face
    KRYS_NODISCARD DOMString Face() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-font-face
    void Face(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-font-size
    KRYS_NODISCARD DOMString Size() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-font-size
    void Size(DOMString value) noexcept;

    /// @brief The `font-size` keyword for a legacy font size (`1` to `7`, or relative with `+` or `-`).
    /// @see https://html.spec.whatwg.org/multipage/rendering.html#rules-for-parsing-a-legacy-font-size
    KRYS_NODISCARD static Maybe<css::ValueId> CSSValueFromFontSizeNumber(DOMStringView string) noexcept;

  protected:
    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                         const DOMStringAtom &namespaceURI) const noexcept override;

    void CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                const DOMStringAtom &namespaceURI, DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::HTMLFontElement)
  KRYS_NODISCARD static bool IsType(const krys::web::HTMLElement &target) noexcept
  {
    return target.IsHTMLFontElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
