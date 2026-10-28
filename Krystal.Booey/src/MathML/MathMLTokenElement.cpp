#include "Krystal.Booey/MathML/MathMLTokenElement.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/MathML/MathMLNames.hpp"
#include "Krystal.Booey/Rendering/MathML/RenderMathMLToken.hpp"

namespace krys::boo::mathml
{
  void MathMLTokenElement::DidAttachRenderers() noexcept
  {
    MathMLPresentationElement::DidAttachRenderers();
    if (auto *mathmlRenderer = DynamicDowncast<RenderMathMLToken>(Renderer()))
    {
      mathmlRenderer->UpdateTokenContent();
    }
  }

  void MathMLTokenElement::OnChildrenChanged() noexcept
  {
    MathMLPresentationElement::OnChildrenChanged();
    if (auto *mathmlRenderer = DynamicDowncast<RenderMathMLToken>(Renderer()))
    {
      mathmlRenderer->UpdateTokenContent();
    }
  }

  RenderPtr<RenderElement> MathMLTokenElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    // FIXME: <mo> is a token until it's made a RenderMathMLOperator (B12d).
    krys_debug_assert(HasTagName(html::TagName::mi) || HasTagName(html::TagName::mn)
                      || HasTagName(html::TagName::ms) || HasTagName(html::TagName::mtext)
                      || HasTagName(html::TagName::mo));

    return CreateRenderer<RenderMathMLToken>(RenderObject::Type::MathMLToken, *this, krys::move(style));
  }

  bool MathMLTokenElement::ChildShouldCreateRenderer(const dom::Node &child) const noexcept
  {
    // The HTML specification defines <mi>, <mo>, <mn>, <ms> and <mtext> as insertion points.
    return StyledElement::ChildShouldCreateRenderer(child);
  }

  bool MathMLTokenElement::HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                              dom::DOMStringAtom namespaceURI) const noexcept
  {
    if (namespaceURI == dom::DOMStringAtom::Null() && localName == MathMLNames::MathvariantAttr().LocalName()
        && HasTagName(html::TagName::mi))
    {
      return true;
    }
    return MathMLPresentationElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void MathMLTokenElement::CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                                  dom::DOMStringAtom namespaceURI,
                                                                  dom::DOMStringView value,
                                                                  css::MutableStyleProperties &style) noexcept
  {
    // https://w3c.github.io/mathml-core/#the-mathvariant-attribute
    if (namespaceURI == dom::DOMStringAtom::Null() && localName == MathMLNames::MathvariantAttr().LocalName()
        && HasTagName(html::TagName::mi))
    {
      if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"normal"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::TextTransform, css::ValueId::None);
      }
      return;
    }
    MathMLPresentationElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }
}
