#pragma once

#include "Krystal.Booey/Rendering/RenderElement.hpp"

namespace krys::boo::svg
{
  class SVGGradientElement;
  class SVGStopElement;
}

namespace krys::boo
{
  // This class exists mostly so we can hear about gradient stop style changes
  class RenderSVGGradientStop final : public RenderElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderSVGGradientStop);

  public:
    RenderSVGGradientStop(svg::SVGStopElement &, RenderStyle &&) noexcept;
    ~RenderSVGGradientStop() noexcept override;

    KRYS_NODISCARD svg::SVGStopElement &GetElement() const noexcept;

  private:
    void StyleDidChange(style::Difference, const RenderStyle *oldStyle) noexcept override;

    void Layout() noexcept override;

    // These overrides are needed to prevent ASSERTs on <svg><stop /></svg>
    // RenderObject's default implementations ASSERT_NOT_REACHED()
    // https://bugs.webkit.org/show_bug.cgi?id=20400
    KRYS_NODISCARD RepaintRects LocalRectsForRepaint(RepaintOutlineBounds) const noexcept override
    {
      return {};
    }
    KRYS_NODISCARD FloatRect ObjectBoundingBox() const noexcept override
    {
      return {};
    }
    KRYS_NODISCARD FloatRect StrokeBoundingBox() const noexcept override
    {
      return {};
    }
    KRYS_NODISCARD FloatRect RepaintRectInLocalCoordinates(RepaintRectCalculation) const noexcept override
    {
      return {};
    }
    bool NodeAtFloatPoint(const HitTestRequest &, HitTestResult &, const FloatPoint &,
                          HitTestAction) noexcept override
    {
      return false;
    }

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderSVGGradientStop";
    }

    KRYS_NODISCARD bool CanHaveChildren() const noexcept override
    {
      return false;
    }
    void Paint(PaintInfo &, const LayoutPoint &) noexcept override
    {
    }

    KRYS_NODISCARD svg::SVGGradientElement *GradientElement() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderSVGGradientStop)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderSVGGradientStop();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
