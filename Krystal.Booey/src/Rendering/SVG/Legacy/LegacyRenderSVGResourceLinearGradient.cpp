#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceLinearGradient.hpp"
#include "Krystal.Booey/SVG/SVGLengthContext.hpp"
#include "Krystal.Booey/SVG/SVGLinearGradientElement.hpp"

namespace krys::boo
{
  LegacyRenderSVGResourceLinearGradient::LegacyRenderSVGResourceLinearGradient(
    svg::SVGLinearGradientElement &element, RenderStyle &&style) noexcept
      : LegacyRenderSVGResourceGradient(Type::LegacySVGResourceLinearGradient, element, krys::move(style))
  {
  }

  LegacyRenderSVGResourceLinearGradient::~LegacyRenderSVGResourceLinearGradient() noexcept = default;

  svg::SVGLinearGradientElement &LegacyRenderSVGResourceLinearGradient::LinearGradientElement() const noexcept
  {
    return Downcast<svg::SVGLinearGradientElement>(LegacyRenderSVGResourceGradient::GradientElement());
  }

  bool LegacyRenderSVGResourceLinearGradient::CollectGradientAttributes() noexcept
  {
    _attributes = svg::LinearGradientAttributes();
    return LinearGradientElement().CollectGradientAttributes(_attributes);
  }

  FloatPoint LegacyRenderSVGResourceLinearGradient::StartPoint(
    const svg::LinearGradientAttributes &attributes) const noexcept
  {
    return svg::SVGLengthContext::ResolvePoint(&LinearGradientElement(), attributes.GradientUnits(),
                                               attributes.X1(), attributes.Y1());
  }

  FloatPoint LegacyRenderSVGResourceLinearGradient::EndPoint(
    const svg::LinearGradientAttributes &attributes) const noexcept
  {
    return svg::SVGLengthContext::ResolvePoint(&LinearGradientElement(), attributes.GradientUnits(),
                                               attributes.X2(), attributes.Y2());
  }

  Ref<Gradient> LegacyRenderSVGResourceLinearGradient::BuildGradient(const RenderStyle &style) const noexcept
  {
    return Gradient::Create(Gradient::LinearData {StartPoint(_attributes), EndPoint(_attributes)},
                            {ColourInterpolationMethod::SRGB {}, AlphaPremultiplication::Unpremultiplied},
                            PlatformSpreadMethodFromSVGType(_attributes.SpreadMethod()),
                            StopsByApplyingColorFilter(_attributes.Stops(), style), false);
  }
}
