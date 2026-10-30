#pragma once

#include "Krystal.Booey/SVG/SVGElement.hpp"

namespace krys::boo::svg
{
  class SVGUnknownElement : public SVGElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGUnknownElement);

  public:
    SVGUnknownElement(dom::Document &document) noexcept;

  private:
    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &) noexcept final
    {
      return false;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGUnknownElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGUnknownElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();