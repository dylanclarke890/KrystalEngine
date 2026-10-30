#pragma once

#include "Krystal.Booey/SVG/SVGFELightElement.hpp"

namespace krys::boo::svg
{
  /// @brief The `<feDistantLight>` element: an infinitely distant light source, in a direction.
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGFEDistantLightElement
  class SVGFEDistantLightElement final : public SVGFELightElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGFEDistantLightElement);

  public:
    explicit SVGFEDistantLightElement(dom::Document &document) noexcept;

  private:
    KRYS_NODISCARD Ref<LightSource> GetLightSource() const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGFEDistantLightElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGFEDistantLightElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
