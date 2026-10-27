#pragma once

#include "Krystal.Booey/SVG/SVGFELightElement.hpp"

namespace krys::boo::svg
{
  /// @brief The `<fePointLight>` element: a light source at a point.
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGFEPointLightElement
  class SVGFEPointLightElement final : public SVGFELightElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGFEPointLightElement);

  public:
    explicit SVGFEPointLightElement(dom::Document &document) noexcept;

  private:
    KRYS_NODISCARD Ref<LightSource> GetLightSource() const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGFEPointLightElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGFEPointLightElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
