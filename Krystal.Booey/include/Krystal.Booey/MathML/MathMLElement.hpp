#pragma once

#include "Krystal.Booey/DOM/StyledElement.hpp"

namespace krys::boo::mathml
{
  /// @see https://w3c.github.io/mathml-core/#dom-and-javascript
  class MathMLElement : public dom::StyledElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(MathMLElement);

  public:
    explicit MathMLElement(dom::Document &document) noexcept;

  protected:
    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                         dom::DOMStringAtom namespaceURI) const noexcept override;
    void CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI,
                                                dom::DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::mathml::MathMLElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    return node.IsMathMLElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
