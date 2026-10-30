#include "Krystal.Booey/SVG/SVGFEFloodElement.hpp"
#include "Krystal.Booey/Graphics/Filters/FEFlood.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  SVGFEFloodElement::SVGFEFloodElement(dom::Document &document) noexcept
      : SVGFilterPrimitiveStandardAttributes(document, CreateUnique<PropertyRegistry>(*this),
                                             SVGElementInterface::FEFlood)
  {
  }

  bool SVGFEFloodElement::SetFilterEffectAttribute(FilterEffect &effect,
                                                   const dom::QualifiedName &attributeName) noexcept
  {
    auto *renderer = Renderer();
    krys_debug_assert(renderer);
    const auto &style = renderer->Style();

    auto &feFlood = Downcast<FEFlood>(effect);
    if (attributeName == SVGNames::FloodColorAttr())
    {
      return feFlood.SetFloodColor(style.ColorResolvingCurrentColor(style.FloodColor()));
    }
    if (attributeName == SVGNames::FloodOpacityAttr())
    {
      return feFlood.SetFloodOpacity(style.FloodOpacity().Value.Value);
    }

    krys_debug_assert_msg(false, "Not reached");
    return false;
  }

  RefPtr<FilterEffect> SVGFEFloodElement::CreateFilterEffect(const FilterEffectVector &,
                                                             const GraphicsContext &) const noexcept
  {
    auto *renderer = Renderer();
    if (!renderer)
    {
      return nullptr;
    }

    const auto &style = renderer->Style();

    auto color = style.ColorWithColorFilter(style.FloodColor());
    float opacity = style.FloodOpacity().Value.Value;

    return FEFlood::Create(color, opacity);
  }
}
