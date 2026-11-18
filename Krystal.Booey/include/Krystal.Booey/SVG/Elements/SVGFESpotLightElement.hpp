#pragma once

#include "Krystal.Booey/SVG/Elements/SVGFELightElement.hpp"

namespace krys::boo
{
  /// @brief The `<feSpotLight>` element: a light source at a point, pointing at another.
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGFESpotLightElement
  class SVGFESpotLightElement final : public SVGFELightElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGFESpotLightElement);

  public:
    explicit SVGFESpotLightElement(Document &document) noexcept;

  private:
    KRYS_NODISCARD Ref<LightSource> GetLightSource() const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::SVGFESpotLightElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::SVGElement &target) noexcept
  {
    return target.IsSVGFESpotLightElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
