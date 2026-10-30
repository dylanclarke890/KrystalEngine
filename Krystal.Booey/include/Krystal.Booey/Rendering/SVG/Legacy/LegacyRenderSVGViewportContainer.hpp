#pragma once

#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGContainer.hpp"

namespace krys::boo::svg
{
  class SVGSvgElement;
}

namespace krys::boo
{
  // This is used for non-root <svg> elements and <marker> elements, neither of which are SVGTransformable
  // thus we inherit from LegacyRenderSVGContainer instead of LegacyRenderSVGTransformableContainer
  class LegacyRenderSVGViewportContainer final : public LegacyRenderSVGContainer
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRenderSVGViewportContainer);

  private:
    bool _didTransformToRootUpdate : 1 {false};
    bool _isLayoutSizeChanged : 1 {false};
    bool _needsTransformUpdate : 1 {true};
    FloatRect _viewport {};
    mutable AffineTransform _localToParentTransform {};

  public:
    LegacyRenderSVGViewportContainer(svg::SVGSvgElement &, RenderStyle &&) noexcept;
    ~LegacyRenderSVGViewportContainer() noexcept override;

    KRYS_NODISCARD svg::SVGSvgElement &SvgSvgElement() const noexcept;

    KRYS_NODISCARD FloatRect Viewport() const noexcept
    {
      return _viewport;
    }

    KRYS_NODISCARD bool IsLayoutSizeChanged() const noexcept
    {
      return _isLayoutSizeChanged;
    }
    KRYS_NODISCARD bool DidTransformToRootUpdate() noexcept override
    {
      return _didTransformToRootUpdate;
    }
    void DetermineIfLayoutSizeChanged() noexcept override;
    void SetNeedsTransformUpdate() noexcept override
    {
      _needsTransformUpdate = true;
    }
    void Paint(PaintInfo &, const LayoutPoint &) noexcept override;

  private:
    void GetElement() const = delete;
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderSVGViewportContainer";
    }

    KRYS_NODISCARD AffineTransform ViewportTransform() const noexcept;
    KRYS_NODISCARD const AffineTransform &LocalToParentTransform() const noexcept override
    {
      return _localToParentTransform;
    }

    void CalcViewport() noexcept override;
    bool CalculateLocalTransform() noexcept override;

    void ApplyViewportClip(PaintInfo &) noexcept override;
    KRYS_NODISCARD bool PointIsInsideViewportClip(const FloatPoint &pointInParent) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LegacyRenderSVGViewportContainer)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsLegacyRenderSVGViewportContainer();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
