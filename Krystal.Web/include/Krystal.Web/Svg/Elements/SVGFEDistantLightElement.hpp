#pragma once

#include "Krystal.Web/Svg/Elements/SVGFELightElement.hpp"

namespace krys::web
{
  /// @brief The `<feDistantLight>` element: an infinitely distant light source, in a direction.
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGFEDistantLightElement
  class SVGFEDistantLightElement final : public SVGFELightElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGFEDistantLightElement);

  public:
    explicit SVGFEDistantLightElement(Document &document) noexcept;

  private:
    KRYS_NODISCARD Ref<LightSource> GetLightSource() const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::SVGFEDistantLightElement)
  KRYS_NODISCARD static bool IsType(const krys::web::SVGElement &target) noexcept
  {
    return target.IsSVGFEDistantLightElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
