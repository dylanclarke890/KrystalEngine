#include "Krystal.Booey/SVG/SVGGElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGHiddenContainer.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGTransformableContainer.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"

namespace krys::boo::svg
{
  SVGGElement::SVGGElement(dom::Document &document) noexcept
      : SVGGraphicsElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::G)
  {
  }

  RenderPtr<RenderElement> SVGGElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    // SVG 1.1 testsuite explicitly uses constructs like <g display="none"><linearGradient>
    // We still have to create renderers for the <g> & <linearGradient> element, though the
    // subtree may be hidden - we only want the resource renderers to exist so they can be
    // referenced from somewhere else.
    if (style.Display() == DisplayType::None)
    {
      return CreateRenderer<LegacyRenderSVGHiddenContainer>(RenderObject::Type::LegacySVGHiddenContainer,
                                                            *this, krys::move(style));
    }

    return CreateRenderer<LegacyRenderSVGTransformableContainer>(*this, krys::move(style));
  }

  bool SVGGElement::RendererIsNeeded(const RenderStyle &) noexcept
  {
    // Unlike SVGElement::rendererIsNeeded(), we still create renderers, even if
    // display is set to 'none' - which is special to SVG <g> container elements.
    auto *parent = dom::TreeQueries::ParentOrShadowHostElement(*this);
    return parent && Is<SVGElement>(*parent);
  }
}
