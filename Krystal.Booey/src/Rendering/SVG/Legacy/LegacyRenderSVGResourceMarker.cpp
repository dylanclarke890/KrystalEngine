#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceMarker.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGRoot.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderSupport.hpp"
#include "Krystal.Booey/SVG/SVGLengthContext.hpp"
#include "Krystal.Booey/SVG/SVGMarkerElement.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo
{
  LegacyRenderSVGResourceMarker::LegacyRenderSVGResourceMarker(svg::SVGMarkerElement &element,
                                                               RenderStyle &&style) noexcept
      : LegacyRenderSVGResourceContainer(Type::LegacySVGResourceMarker, element, krys::move(style))
  {
  }

  LegacyRenderSVGResourceMarker::~LegacyRenderSVGResourceMarker() noexcept = default;

  svg::SVGMarkerElement &LegacyRenderSVGResourceMarker::MarkerElement() const noexcept
  {
    return Downcast<svg::SVGMarkerElement>(LegacyRenderSVGResourceContainer::GetElement());
  }

  svg::SVGMarkerUnitsType LegacyRenderSVGResourceMarker::MarkerUnits() const noexcept
  {
    return MarkerElement().MarkerUnits();
  }

  void LegacyRenderSVGResourceMarker::Layout() noexcept
  {
    // Invalidate all resources if our layout changed.
    if (EverHadLayout() && SelfNeedsLayout())
    {
      LegacyRenderSVGRoot::AddResourceForClientInvalidation(this);
    }

    // RenderSVGHiddenContainer overwrites layout(). We need the
    // layouting of LegacyRenderSVGContainer for calculating  local
    // transformations and repaint.
    LegacyRenderSVGContainer::Layout();
  }

  void LegacyRenderSVGResourceMarker::ApplyViewportClip(PaintInfo &paintInfo) noexcept
  {
    if (SVGRenderSupport::IsOverflowHidden(*this))
    {
      paintInfo.Context().Clip(_viewport);
    }
  }

  FloatRect LegacyRenderSVGResourceMarker::MarkerBoundaries(
    RepaintRectCalculation repaintRectCalculation, const AffineTransform &markerTransformation) const noexcept
  {
    FloatRect coordinates = LegacyRenderSVGContainer::RepaintRectInLocalCoordinates(repaintRectCalculation);

    // Map repaint rect into parent coordinate space, in which the marker boundaries have to be evaluated
    coordinates = LocalToParentTransform().MapRect(coordinates);

    return markerTransformation.MapRect(coordinates);
  }

  const AffineTransform &LegacyRenderSVGResourceMarker::LocalToParentTransform() const noexcept
  {
    _localToParentTransform =
      AffineTransform::MakeTranslation(ToFloatSize(_viewport.Location())) * ViewportTransform();
    return _localToParentTransform;
    // If this class were ever given a localTransform(), then the above would read:
    // return viewportTranslation * localTransform() * viewportTransform();
  }

  FloatPoint LegacyRenderSVGResourceMarker::ReferencePoint() const noexcept
  {
    auto &markerElement = MarkerElement();
    svg::SVGLengthContext lengthContext(&markerElement);
    return FloatPoint(markerElement.RefX().Value(lengthContext), markerElement.RefY().Value(lengthContext));
  }

  Maybe<float> LegacyRenderSVGResourceMarker::Angle() const noexcept
  {
    auto &markerElement = MarkerElement();
    if (markerElement.OrientType() == svg::SVGMarkerOrientAngle)
    {
      return markerElement.OrientAngle().Value();
    }
    return {};
  }

  AffineTransform LegacyRenderSVGResourceMarker::MarkerTransformation(const FloatPoint &origin,
                                                                      float autoAngle,
                                                                      float strokeWidth) const noexcept
  {
    bool useStrokeWidth = MarkerElement().MarkerUnits() == svg::SVGMarkerUnitsType::StrokeWidth;

    AffineTransform transform;
    transform.Translate(origin);
    transform.Rotate(Angle().value_or(autoAngle));
    transform = MarkerContentTransformation(transform, ReferencePoint(), useStrokeWidth ? strokeWidth : -1);
    return transform;
  }

  void LegacyRenderSVGResourceMarker::Draw(PaintInfo &paintInfo, const AffineTransform &transform) noexcept
  {
    auto &markerElement = MarkerElement();
    // An empty viewBox disables rendering.
    if (markerElement.HasAttribute(svg::SVGNames::ViewBoxAttr().LocalName())
        && markerElement.HasEmptyViewBox())
    {
      return;
    }

    PaintInfo info(paintInfo);
    GraphicsContextStateSaver stateSaver(info.Context());
    info.ApplyTransform(transform);
    LegacyRenderSVGContainer::Paint(info, IntPoint());
  }

  AffineTransform LegacyRenderSVGResourceMarker::MarkerContentTransformation(
    const AffineTransform &contentTransformation, const FloatPoint &origin, float strokeWidth) const noexcept
  {
    // The 'origin' coordinate maps to SVGs refX/refY, given in coordinates relative to the viewport
    // established by the marker
    FloatPoint mappedOrigin = ViewportTransform().MapPoint(origin);

    AffineTransform transformation = contentTransformation;
    if (strokeWidth != -1)
    {
      transformation.ScaleNonUniform(strokeWidth, strokeWidth);
    }

    transformation.Translate(-mappedOrigin.X(), -mappedOrigin.Y());
    return transformation;
  }

  AffineTransform LegacyRenderSVGResourceMarker::ViewportTransform() const noexcept
  {
    return MarkerElement().ViewBoxToViewTransform(_viewport.Width(), _viewport.Height());
  }

  void LegacyRenderSVGResourceMarker::CalcViewport() noexcept
  {
    if (!SelfNeedsLayout())
    {
      return;
    }

    auto &markerElement = MarkerElement();
    svg::SVGLengthContext lengthContext(&markerElement);
    float w = markerElement.MarkerWidth().Value(lengthContext);
    float h = markerElement.MarkerHeight().Value(lengthContext);
    _viewport = FloatRect(0, 0, w, h);
  }
}
