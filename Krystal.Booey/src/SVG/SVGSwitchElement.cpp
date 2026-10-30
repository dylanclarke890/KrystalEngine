#include "Krystal.Booey/SVG/SVGSwitchElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGTransformableContainer.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"

namespace krys::boo::svg
{
  SVGSwitchElement::SVGSwitchElement(dom::Document &document) noexcept
      : SVGGraphicsElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::Switch)
  {
  }

  bool SVGSwitchElement::ChildShouldCreateRenderer(const dom::Node &child) const noexcept
  {
    // We create a renderer for the first valid SVG element child.
    // FIXME: The renderer must be updated after dynamic change of the requiredFeatures, requiredExtensions
    // and systemLanguage attributes (https://bugs.webkit.org/show_bug.cgi?id=74749).
    for (auto *childElement = dom::TreeQueries::FirstElementChild(*this); childElement;
         childElement = dom::TreeQueries::NextElementSibling(*childElement))
    {
      auto *element = DynamicDowncast<SVGElement>(*childElement);
      if (!element || !element->IsValid())
      {
        continue;
      }
      return element == &child; // Only allow this child if it's the first valid child
    }

    return false;
  }

  RenderPtr<RenderElement> SVGSwitchElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<LegacyRenderSVGTransformableContainer>(*this, krys::move(style));
  }
}
