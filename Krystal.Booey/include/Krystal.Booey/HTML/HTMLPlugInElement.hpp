#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @brief The base of the elements that embed external content: `<embed>` and `<object>`.
  class HTMLPlugInElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLPlugInElement);

  protected:
    HTMLPlugInElement(dom::Document &document, HTMLElementInterface interface) noexcept;

    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                         dom::DOMStringAtom namespaceURI) const noexcept override;
    void CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI,
                                                dom::DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLPlugInElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLPlugInElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
