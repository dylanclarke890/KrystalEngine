#pragma once

#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGModelObject.hpp"
#include "Krystal.Core/Types/Markable.hpp"

namespace krys::boo
{
  /// @brief An SVG renderer whose children are SVG renderers, painted in its coordinates.
  class LegacyRenderSVGContainer : public LegacyRenderSVGModelObject
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRenderSVGContainer);

  private:
    Markable<FloatRect> _objectBoundingBox {};
    mutable Markable<FloatRect> _strokeBoundingBox {};
    FloatRect _repaintBoundingBox {};
    mutable Markable<FloatRect> _accurateRepaintBoundingBox {};

    bool _needsBoundariesUpdate {true};
    bool _repaintIsSuspendedForChildrenDuringLayout {false};

  public:
    ~LegacyRenderSVGContainer() noexcept override;

    void Paint(PaintInfo &, const LayoutPoint &) noexcept override;
    void SetNeedsBoundariesUpdate() noexcept final
    {
      _needsBoundariesUpdate = true;
    }
    KRYS_NODISCARD virtual bool DidTransformToRootUpdate() noexcept
    {
      return false;
    }
    KRYS_NODISCARD bool IsObjectBoundingBoxValid() const noexcept
    {
      return static_cast<bool>(_objectBoundingBox);
    }
    KRYS_NODISCARD bool IsRepaintSuspendedForChildren() const noexcept
    {
      return _repaintIsSuspendedForChildrenDuringLayout;
    }

    KRYS_NODISCARD FloatRect ObjectBoundingBox() const noexcept final
    {
      return _objectBoundingBox.value_or(FloatRect());
    }

  protected:
    LegacyRenderSVGContainer(Type, svg::SVGElement &, RenderStyle &&,
                             EnumSet<SVGModelObjectFlag> = {}) noexcept;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderSVGContainer";
    }

    KRYS_NODISCARD bool CanHaveChildren() const noexcept final
    {
      return true;
    }

    void Layout() noexcept override;

    void AddFocusRingRects(List<LayoutRect> &, const LayoutPoint &additionalOffset,
                           const RenderLayerModelObject *paintContainer = nullptr) const noexcept final;

    KRYS_NODISCARD FloatRect StrokeBoundingBox() const noexcept final;
    KRYS_NODISCARD FloatRect RepaintRectInLocalCoordinates(
      RepaintRectCalculation = RepaintRectCalculation::Fast) const noexcept final;
    KRYS_NODISCARD FloatRect DecoratedBoundingBox() const noexcept final;

    bool NodeAtFloatPoint(const HitTestRequest &, HitTestResult &, const FloatPoint &pointInParent,
                          HitTestAction) noexcept override;

    // Allow LegacyRenderSVGTransformableContainer to hook in at the right time in layout()
    virtual bool CalculateLocalTransform() noexcept
    {
      return false;
    }

    // Allow RenderSVGViewportContainer to hook in at the right times in layout(), paint() and
    // nodeAtFloatPoint()
    virtual void CalcViewport() noexcept
    {
    }
    virtual void ApplyViewportClip(PaintInfo &) noexcept
    {
    }
    KRYS_NODISCARD virtual bool PointIsInsideViewportClip(const FloatPoint & /*pointInParent*/) noexcept
    {
      return true;
    }

    virtual void DetermineIfLayoutSizeChanged() noexcept
    {
    }

    KRYS_NODISCARD bool SelfWillPaint() noexcept;
    void UpdateCachedBoundaries() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LegacyRenderSVGContainer)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsLegacyRenderSVGContainer();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
