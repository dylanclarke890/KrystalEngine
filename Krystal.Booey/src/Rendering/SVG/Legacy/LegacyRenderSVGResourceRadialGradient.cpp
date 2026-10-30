#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceRadialGradient.hpp"
#include "Krystal.Booey/SVG/SVGLengthContext.hpp"
#include "Krystal.Booey/SVG/SVGRadialGradientElement.hpp"

namespace krys::boo
{
  LegacyRenderSVGResourceRadialGradient::LegacyRenderSVGResourceRadialGradient(
    svg::SVGRadialGradientElement &element, RenderStyle &&style) noexcept
      : LegacyRenderSVGResourceGradient(Type::LegacySVGResourceRadialGradient, element, krys::move(style))
  {
  }

  LegacyRenderSVGResourceRadialGradient::~LegacyRenderSVGResourceRadialGradient() noexcept = default;

  svg::SVGRadialGradientElement &LegacyRenderSVGResourceRadialGradient::RadialGradientElement() const noexcept
  {
    return Downcast<svg::SVGRadialGradientElement>(LegacyRenderSVGResourceGradient::GradientElement());
  }

  bool LegacyRenderSVGResourceRadialGradient::CollectGradientAttributes() noexcept
  {
    _attributes = svg::RadialGradientAttributes();
    return RadialGradientElement().CollectGradientAttributes(_attributes);
  }

  FloatPoint LegacyRenderSVGResourceRadialGradient::CenterPoint(
    const svg::RadialGradientAttributes &attributes) const noexcept
  {
    return svg::SVGLengthContext::ResolvePoint(&RadialGradientElement(), attributes.GradientUnits(),
                                               attributes.Cx(), attributes.Cy());
  }

  FloatPoint LegacyRenderSVGResourceRadialGradient::FocalPoint(
    const svg::RadialGradientAttributes &attributes) const noexcept
  {
    return svg::SVGLengthContext::ResolvePoint(&RadialGradientElement(), attributes.GradientUnits(),
                                               attributes.Fx(), attributes.Fy());
  }

  float LegacyRenderSVGResourceRadialGradient::Radius(
    const svg::RadialGradientAttributes &attributes) const noexcept
  {
    return svg::SVGLengthContext::ResolveLength(&RadialGradientElement(), attributes.GradientUnits(),
                                                attributes.R());
  }

  float LegacyRenderSVGResourceRadialGradient::FocalRadius(
    const svg::RadialGradientAttributes &attributes) const noexcept
  {
    return svg::SVGLengthContext::ResolveLength(&RadialGradientElement(), attributes.GradientUnits(),
                                                attributes.Fr());
  }

  Ref<Gradient> LegacyRenderSVGResourceRadialGradient::BuildGradient(const RenderStyle &style) const noexcept
  {
    return Gradient::Create(Gradient::RadialData {FocalPoint(_attributes), CenterPoint(_attributes),
                                                  FocalRadius(_attributes), Radius(_attributes), 1},
                            {ColourInterpolationMethod::SRGB {}, AlphaPremultiplication::Unpremultiplied},
                            PlatformSpreadMethodFromSVGType(_attributes.SpreadMethod()),
                            StopsByApplyingColorFilter(_attributes.Stops(), style), false);
  }
}
