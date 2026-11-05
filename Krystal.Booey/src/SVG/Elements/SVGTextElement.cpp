#include "Krystal.Booey/SVG/Elements/SVGTextElement.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGText.hpp"
#include "Krystal.Booey/SVG/Elements/SVGAElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGTextPathElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGTSpanElement.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"

namespace krys::boo::svg
{
  SVGTextElement::SVGTextElement(dom::Document &document) noexcept
      : SVGTextPositioningElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::Text)
  {
  }

  RenderPtr<RenderElement> SVGTextElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<RenderSVGText>(*this, krys::move(style));
  }

  bool SVGTextElement::ChildShouldCreateRenderer(const dom::Node &child) const noexcept
  {
    if (Is<dom::Text>(child))
    {
      return true;
    }
    auto *element = DynamicDowncast<SVGElement>(child);
    return element
           && (Is<SVGAElement>(*element) || Is<SVGTextPathElement>(*element)
               || Is<SVGTSpanElement>(*element));
  }
}
