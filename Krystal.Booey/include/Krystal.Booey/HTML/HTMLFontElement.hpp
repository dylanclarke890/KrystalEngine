#pragma once

#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlfontelement
  class HTMLFontElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLFontElement);

  public:
    HTMLFontElement(dom::Document &document) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-font-color
    KRYS_NODISCARD dom::DOMString Color() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-font-color
    void Color(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-font-face
    KRYS_NODISCARD dom::DOMString Face() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-font-face
    void Face(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-font-size
    KRYS_NODISCARD dom::DOMString Size() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-font-size
    void Size(dom::DOMString &&value) noexcept;

    /// @brief The `font-size` keyword for a legacy font size (`1` to `7`, or relative with `+` or `-`).
    /// @see https://html.spec.whatwg.org/#rules-for-parsing-a-legacy-font-size
    KRYS_NODISCARD static Maybe<css::ValueId> CSSValueFromFontSizeNumber(dom::DOMStringView string) noexcept;

  protected:
    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                         dom::DOMStringAtom namespaceURI) const noexcept override;
    void CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI,
                                                dom::DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLFontElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLFontElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
