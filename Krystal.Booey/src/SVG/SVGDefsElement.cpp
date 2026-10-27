#include "Krystal.Booey/SVG/SVGDefsElement.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGHiddenContainer.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"

namespace krys::boo::svg
{
  SVGDefsElement::SVGDefsElement(dom::Document &document) noexcept
      : SVGGraphicsElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::Defs)
  {
  }

  bool SVGDefsElement::IsValid() const noexcept
  {
    return SVGTests::IsValid();
  }

  RenderPtr<RenderElement> SVGDefsElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<LegacyRenderSVGHiddenContainer>(RenderObject::Type::LegacySVGHiddenContainer, *this,
                                                          krys::move(style));
  }
}
