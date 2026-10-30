#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  class HTMLTableElement;

  /// @brief The base of the table parts that share presentational hints: cells, columns, rows and row
  /// groups.
  class HTMLTablePartElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTablePartElement);

  protected:
    HTMLTablePartElement(dom::Document &document, HTMLElementInterface interface) noexcept;

  public:
    /// @brief The nearest ancestor table.
    KRYS_NODISCARD const HTMLTableElement *FindParentTable() const noexcept;

  protected:
    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                         dom::DOMStringAtom namespaceURI) const noexcept override;
    void CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI,
                                                dom::DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLTablePartElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLTablePartElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
