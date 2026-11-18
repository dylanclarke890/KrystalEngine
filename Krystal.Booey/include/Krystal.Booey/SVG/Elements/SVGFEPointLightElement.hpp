#pragma once

#include "Krystal.Booey/SVG/Elements/SVGFELightElement.hpp"

namespace krys::boo
{
  /// @brief The `<fePointLight>` element: a light source at a point.
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGFEPointLightElement
  class SVGFEPointLightElement final : public SVGFELightElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGFEPointLightElement);

  public:
    explicit SVGFEPointLightElement(Document &document) noexcept;

  private:
    KRYS_NODISCARD Ref<LightSource> GetLightSource() const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::SVGFEPointLightElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::SVGElement &target) noexcept
  {
    return target.IsSVGFEPointLightElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
