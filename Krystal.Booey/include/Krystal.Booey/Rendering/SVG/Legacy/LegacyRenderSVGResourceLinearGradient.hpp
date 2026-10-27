#pragma once

#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceGradient.hpp"
#include "Krystal.Booey/SVG/LinearGradientAttributes.hpp"

namespace krys::boo::svg
{
  class SVGLinearGradientElement;
}

namespace krys::boo
{
  /// @brief The renderer of a `<linearGradient>`.
  class LegacyRenderSVGResourceLinearGradient final : public LegacyRenderSVGResourceGradient
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRenderSVGResourceLinearGradient);

  private:
    svg::LinearGradientAttributes _attributes;

  public:
    LegacyRenderSVGResourceLinearGradient(svg::SVGLinearGradientElement &, RenderStyle &&) noexcept;
    ~LegacyRenderSVGResourceLinearGradient() noexcept override;

    KRYS_NODISCARD svg::SVGLinearGradientElement &LinearGradientElement() const noexcept;

    KRYS_NODISCARD FloatPoint StartPoint(const svg::LinearGradientAttributes &) const noexcept;
    KRYS_NODISCARD FloatPoint EndPoint(const svg::LinearGradientAttributes &) const noexcept;

  private:
    KRYS_NODISCARD RenderSVGResourceType ResourceType() const noexcept final
    {
      return RenderSVGResourceType::LinearGradientResourceType;
    }

    KRYS_NODISCARD svg::SVGUnitTypes::SVGUnitType GradientUnits() const noexcept final
    {
      return _attributes.GradientUnits();
    }
    KRYS_NODISCARD AffineTransform GradientTransform() const noexcept final
    {
      return _attributes.GradientTransform();
    }
    bool CollectGradientAttributes() noexcept final;
    KRYS_NODISCARD Ref<Gradient> BuildGradient(const RenderStyle &) const noexcept final;

    void GradientElement() const = delete;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept final
    {
      return u8"RenderSVGResourceLinearGradient";
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LegacyRenderSVGResourceLinearGradient)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.GetType() == krys::boo::RenderObject::Type::LegacySVGResourceLinearGradient;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
