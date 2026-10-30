#include "Krystal.Booey/Rendering/SVG/RenderSVGGradientStop.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceContainer.hpp"
#include "Krystal.Booey/SVG/SVGGradientElement.hpp"
#include "Krystal.Booey/SVG/SVGStopElement.hpp"

namespace krys::boo
{
  RenderSVGGradientStop::RenderSVGGradientStop(svg::SVGStopElement &element, RenderStyle &&style) noexcept
      : RenderElement(Type::SVGGradientStop, element, krys::move(style), {}, {})
  {
    krys_debug_assert(IsRenderSVGGradientStop());
  }

  RenderSVGGradientStop::~RenderSVGGradientStop() noexcept = default;

  svg::SVGStopElement &RenderSVGGradientStop::GetElement() const noexcept
  {
    return Downcast<svg::SVGStopElement>(Downcast<svg::SVGElement>(NodeForNonAnonymous()));
  }

  void RenderSVGGradientStop::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    RenderElement::StyleDidChange(diff, oldStyle);
    if (diff == style::DifferenceResult::Equal)
    {
      return;
    }

    // <stop> elements should only be allowed to make renderers under gradient elements
    // but I can imagine a few cases we might not be catching, so let's not crash if our parent isn't a
    // gradient.
    RefPtr<svg::SVGGradientElement> gradient = ShareRefPtr(GradientElement());
    if (!gradient)
    {
      return;
    }

    auto *renderer = gradient->Renderer();
    if (!renderer)
    {
      return;
    }

    Downcast<LegacyRenderSVGResourceContainer>(*renderer).RemoveAllClientsFromCacheAndMarkForInvalidation();
  }

  void RenderSVGGradientStop::Layout() noexcept
  {
    ClearNeedsLayout();
  }

  svg::SVGGradientElement *RenderSVGGradientStop::GradientElement() noexcept
  {
    return DynamicDowncast<svg::SVGGradientElement>(GetElement().ParentElement());
  }
}
