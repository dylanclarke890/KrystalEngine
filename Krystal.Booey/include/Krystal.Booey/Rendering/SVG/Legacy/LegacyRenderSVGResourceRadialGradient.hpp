#pragma once

#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceGradient.hpp"
#include "Krystal.Booey/SVG/RadialGradientAttributes.hpp"

namespace krys::boo::svg
{
  class SVGRadialGradientElement;
}

namespace krys::boo
{
  /// @brief The renderer of a `<radialGradient>`.
  class LegacyRenderSVGResourceRadialGradient final : public LegacyRenderSVGResourceGradient
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRenderSVGResourceRadialGradient);

  private:
    svg::RadialGradientAttributes _attributes;

  public:
    LegacyRenderSVGResourceRadialGradient(svg::SVGRadialGradientElement &, RenderStyle &&) noexcept;
    ~LegacyRenderSVGResourceRadialGradient() noexcept override;

    KRYS_NODISCARD svg::SVGRadialGradientElement &RadialGradientElement() const noexcept;

    KRYS_NODISCARD FloatPoint CenterPoint(const svg::RadialGradientAttributes &) const noexcept;
    KRYS_NODISCARD FloatPoint FocalPoint(const svg::RadialGradientAttributes &) const noexcept;
    KRYS_NODISCARD float Radius(const svg::RadialGradientAttributes &) const noexcept;
    KRYS_NODISCARD float FocalRadius(const svg::RadialGradientAttributes &) const noexcept;

  private:
    KRYS_NODISCARD RenderSVGResourceType ResourceType() const noexcept final
    {
      return RenderSVGResourceType::RadialGradientResourceType;
    }

    KRYS_NODISCARD svg::SVGUnitTypes::SVGUnitType GradientUnits() const noexcept final
    {
      return _attributes.GradientUnits();
    }
    KRYS_NODISCARD AffineTransform GradientTransform() const noexcept final
    {
      return _attributes.GradientTransform();
    }
    KRYS_NODISCARD Ref<Gradient> BuildGradient(const RenderStyle &) const noexcept final;

    void GradientElement() const = delete;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept final
    {
      return u8"RenderSVGResourceRadialGradient";
    }
    bool CollectGradientAttributes() noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LegacyRenderSVGResourceRadialGradient)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.GetType() == krys::boo::RenderObject::Type::LegacySVGResourceRadialGradient;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
