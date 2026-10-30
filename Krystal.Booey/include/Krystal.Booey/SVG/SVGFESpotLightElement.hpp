#pragma once

#include "Krystal.Booey/SVG/SVGFELightElement.hpp"

namespace krys::boo::svg
{
  /// @brief The `<feSpotLight>` element: a light source at a point, pointing at another.
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGFESpotLightElement
  class SVGFESpotLightElement final : public SVGFELightElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGFESpotLightElement);

  public:
    explicit SVGFESpotLightElement(dom::Document &document) noexcept;

  private:
    KRYS_NODISCARD Ref<LightSource> GetLightSource() const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGFESpotLightElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGFESpotLightElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
