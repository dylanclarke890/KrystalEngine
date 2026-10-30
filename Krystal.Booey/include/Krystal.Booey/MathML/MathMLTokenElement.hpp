#pragma once

#include "Krystal.Booey/MathML/MathMLPresentationElement.hpp"

namespace krys::boo::mathml
{
  /// @brief A token element: `<mi>`, `<mn>`, `<ms>` and `<mtext>`, whose text is drawn, and the base of
  /// `<mo>`.
  /// @see https://w3c.github.io/mathml-core/#token-elements
  class MathMLTokenElement : public MathMLPresentationElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(MathMLTokenElement);

  public:
    explicit MathMLTokenElement(dom::Document &document,
                                MathMLElementInterface interface = MathMLElementInterface::Token) noexcept
        : MathMLPresentationElement(document, interface)
    {
    }

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&style) noexcept override;
    void DidAttachRenderers() noexcept override;

  protected:
    void OnChildrenChanged() noexcept override;
    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                         dom::DOMStringAtom namespaceURI) const noexcept override;
    void CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI,
                                                dom::DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;

  private:
    KRYS_NODISCARD bool ChildShouldCreateRenderer(const dom::Node &child) const noexcept final;

    KRYS_NODISCARD bool IsMathMLToken() const noexcept final
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::mathml::MathMLTokenElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::mathml::MathMLElement &element) noexcept
  {
    return element.IsMathMLToken();
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    auto *element = krys::DynamicDowncast<krys::boo::mathml::MathMLElement>(node);
    return element && IsType(*element);
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
