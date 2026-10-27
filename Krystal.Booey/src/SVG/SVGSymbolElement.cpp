#include "Krystal.Booey/SVG/SVGSymbolElement.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGHiddenContainer.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  SVGSymbolElement::SVGSymbolElement(dom::Document &document) noexcept
      : SVGGraphicsElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::Symbol),
        SVGFitToViewBox(this)
  {
  }

  void SVGSymbolElement::AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                          Maybe<dom::DOMStringView> value) noexcept
  {
    SVGFitToViewBox::ParseAttribute(name, value);
    SVGGraphicsElement::AttributeChanged(name, oldValue, value);
  }

  bool SVGSymbolElement::SelfHasRelativeLengths() const noexcept
  {
    return HasAttributeNS(dom::DOMStringAtom::Null(), SVGNames::ViewBoxAttr().LocalName());
  }

  RenderPtr<RenderElement> SVGSymbolElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<LegacyRenderSVGHiddenContainer>(RenderObject::Type::LegacySVGHiddenContainer, *this,
                                                          krys::move(style));
  }
}
