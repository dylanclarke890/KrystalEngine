#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceFilterPrimitive.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceFilter.hpp"
#include "Krystal.Booey/SVG/SVGFEDiffuseLightingElement.hpp"
#include "Krystal.Booey/SVG/SVGFEDropShadowElement.hpp"
#include "Krystal.Booey/SVG/SVGFEFloodElement.hpp"
#include "Krystal.Booey/SVG/SVGFESpecularLightingElement.hpp"
#include "Krystal.Booey/SVG/SVGFilterPrimitiveStandardAttributes.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo
{
  LegacyRenderSVGResourceFilterPrimitive::LegacyRenderSVGResourceFilterPrimitive(
    svg::SVGFilterPrimitiveStandardAttributes &filterPrimitiveElement, RenderStyle &&style) noexcept
      : LegacyRenderSVGHiddenContainer(Type::LegacySVGResourceFilterPrimitive, filterPrimitiveElement,
                                       krys::move(style))
  {
  }

  svg::SVGFilterPrimitiveStandardAttributes &
    LegacyRenderSVGResourceFilterPrimitive::FilterPrimitiveElement() const noexcept
  {
    return static_cast<svg::SVGFilterPrimitiveStandardAttributes &>(
      LegacyRenderSVGHiddenContainer::GetElement());
  }

  void LegacyRenderSVGResourceFilterPrimitive::StyleDidChange(style::Difference diff,
                                                              const RenderStyle *oldStyle) noexcept
  {
    LegacyRenderSVGHiddenContainer::StyleDidChange(diff, oldStyle);

    if (diff == style::DifferenceResult::Equal || !oldStyle)
    {
      return;
    }

    const auto &newStyle = Style();
    auto &element = FilterPrimitiveElement();
    if (Is<svg::SVGFEFloodElement>(element) || Is<svg::SVGFEDropShadowElement>(element))
    {
      if (newStyle.FloodColor() != oldStyle->FloodColor())
      {
        element.PrimitiveAttributeChanged(svg::SVGNames::FloodColorAttr());
      }
      if (newStyle.FloodOpacity() != oldStyle->FloodOpacity())
      {
        element.PrimitiveAttributeChanged(svg::SVGNames::FloodOpacityAttr());
      }
    }
    else if (Is<svg::SVGFEDiffuseLightingElement>(element) || Is<svg::SVGFESpecularLightingElement>(element))
    {
      if (newStyle.LightingColor() != oldStyle->LightingColor())
      {
        element.PrimitiveAttributeChanged(svg::SVGNames::LightingColorAttr());
      }
    }
  }

  void LegacyRenderSVGResourceFilterPrimitive::MarkFilterEffectForRepaint(FilterEffect *effect) noexcept
  {
    auto *parent = DynamicDowncast<LegacyRenderSVGResourceFilter>(Parent());
    if (!parent)
    {
      return;
    }

    if (effect)
    {
      parent->MarkFilterForRepaint(*effect);
    }

    parent->MarkAllClientLayersForInvalidation();
  }

  void LegacyRenderSVGResourceFilterPrimitive::MarkFilterEffectForRebuild() noexcept
  {
    auto *parent = DynamicDowncast<LegacyRenderSVGResourceFilter>(Parent());
    if (!parent)
    {
      return;
    }

    parent->MarkFilterForRebuild();
    parent->MarkAllClientLayersForInvalidation();
  }
}
