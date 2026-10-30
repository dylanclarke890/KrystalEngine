#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGHiddenContainer.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderSupport.hpp"

namespace krys::boo
{
  LegacyRenderSVGHiddenContainer::LegacyRenderSVGHiddenContainer(
    Type type, svg::SVGElement &element, RenderStyle &&style, EnumSet<SVGModelObjectFlag> svgFlags) noexcept
      : LegacyRenderSVGContainer(type, element, krys::move(style),
                                 svgFlags | SVGModelObjectFlag::IsHiddenContainer)
  {
  }

  void LegacyRenderSVGHiddenContainer::Layout() noexcept
  {
    krys_debug_assert(NeedsLayout());
    SVGRenderSupport::LayoutChildren(*this, SelfNeedsLayout());
    ClearNeedsLayout();
  }

  void LegacyRenderSVGHiddenContainer::Paint(PaintInfo &, const LayoutPoint &) noexcept
  {
    // This subtree does not paint.
  }

  LayoutRect LegacyRenderSVGHiddenContainer::ClippedOverflowRect(const RenderLayerModelObject *,
                                                                 VisibleRectContext) const noexcept
  {
    return {};
  }

  void LegacyRenderSVGHiddenContainer::AbsoluteQuads(List<FloatQuad> &, bool *) const noexcept
  {
    // This subtree does not take up space or paint
  }

  bool LegacyRenderSVGHiddenContainer::NodeAtFloatPoint(const HitTestRequest &, HitTestResult &,
                                                        const FloatPoint &, HitTestAction) noexcept
  {
    return false;
  }
}
