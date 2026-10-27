#include "Krystal.Booey/SVG/SVGTSpanElement.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGTSpan.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGAElement.hpp"
#include "Krystal.Booey/SVG/SVGTextElement.hpp"
#include "Krystal.Booey/SVG/SVGTextPathElement.hpp"
#include "Krystal.Booey/SVG/SVGTSpanElement.hpp"

namespace krys::boo::svg
{
  SVGTSpanElement::SVGTSpanElement(dom::Document &document) noexcept
      : SVGTextPositioningElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::TSpan)
  {
  }

  RenderPtr<RenderElement> SVGTSpanElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<RenderSVGTSpan>(*this, krys::move(style));
  }

  bool SVGTSpanElement::ChildShouldCreateRenderer(const dom::Node &child) const noexcept
  {
    if (Is<dom::Text>(child))
    {
      return true;
    }
    auto *element = DynamicDowncast<SVGElement>(child);
    return element && (Is<SVGAElement>(*element) || Is<SVGTSpanElement>(*element));
  }

  bool SVGTSpanElement::RendererIsNeeded(const RenderStyle &style) noexcept
  {
    if (auto *parent = DynamicDowncast<SVGElement>(ParentNode()))
    {
      if (Is<SVGAElement>(*parent) || Is<SVGTextElement>(*parent) || Is<SVGTextPathElement>(*parent)
          || Is<SVGTSpanElement>(*parent))
      {
        return dom::StyledElement::RendererIsNeeded(style);
      }
    }
    return false;
  }
}
