#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGViewportContainer.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderSupport.hpp"
#include "Krystal.Booey/SVG/SVGLengthContext.hpp"
#include "Krystal.Booey/SVG/SVGSvgElement.hpp"

namespace krys::boo
{
  LegacyRenderSVGViewportContainer::LegacyRenderSVGViewportContainer(svg::SVGSvgElement &element,
                                                                     RenderStyle &&style) noexcept
      : LegacyRenderSVGContainer(Type::LegacySVGViewportContainer, element, krys::move(style))
  {
    krys_debug_assert(IsLegacyRenderSVGViewportContainer());
  }

  LegacyRenderSVGViewportContainer::~LegacyRenderSVGViewportContainer() noexcept = default;

  svg::SVGSvgElement &LegacyRenderSVGViewportContainer::SvgSvgElement() const noexcept
  {
    return Downcast<svg::SVGSvgElement>(LegacyRenderSVGContainer::GetElement());
  }

  void LegacyRenderSVGViewportContainer::DetermineIfLayoutSizeChanged() noexcept
  {
    _isLayoutSizeChanged = SvgSvgElement().HasRelativeLengths() && SelfNeedsLayout();
  }

  void LegacyRenderSVGViewportContainer::ApplyViewportClip(PaintInfo &paintInfo) noexcept
  {
    if (SVGRenderSupport::IsOverflowHidden(*this))
    {
      paintInfo.Context().Clip(_viewport);
    }
  }

  void LegacyRenderSVGViewportContainer::CalcViewport() noexcept
  {
    auto &element = SvgSvgElement();
    svg::SVGLengthContext lengthContext(&element);
    FloatRect newViewport(element.X().Value(lengthContext), element.Y().Value(lengthContext),
                          element.Width().Value(lengthContext), element.Height().Value(lengthContext));

    if (_viewport == newViewport)
    {
      return;
    }

    _viewport = newViewport;

    InvalidateCachedBoundaries();
    SetNeedsTransformUpdate();
  }

  bool LegacyRenderSVGViewportContainer::CalculateLocalTransform() noexcept
  {
    _didTransformToRootUpdate = _needsTransformUpdate || SVGRenderSupport::TransformToRootChanged(Parent());
    if (!_needsTransformUpdate)
    {
      return false;
    }

    _localToParentTransform =
      AffineTransform::MakeTranslation(ToFloatSize(_viewport.Location())) * ViewportTransform();
    _needsTransformUpdate = false;
    return true;
  }

  AffineTransform LegacyRenderSVGViewportContainer::ViewportTransform() const noexcept
  {
    return SvgSvgElement().ViewBoxToViewTransform(_viewport.Width(), _viewport.Height());
  }

  bool LegacyRenderSVGViewportContainer::PointIsInsideViewportClip(const FloatPoint &pointInParent) noexcept
  {
    // Respect the viewport clip (which is in parent coords)
    if (!SVGRenderSupport::IsOverflowHidden(*this))
    {
      return true;
    }

    return _viewport.Contains(pointInParent);
  }

  void LegacyRenderSVGViewportContainer::Paint(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept
  {
    // An empty viewBox disables rendering.
    if (SvgSvgElement().HasEmptyViewBox())
    {
      return;
    }

    LegacyRenderSVGContainer::Paint(paintInfo, paintOffset);
  }
}
