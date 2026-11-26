#pragma once

#include "Krystal.Web/Svg/Elements/SVGElement.hpp"

namespace krys::web
{
  class SVGScriptElement : public SVGElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGScriptElement);

  public:
    SVGScriptElement(Document &document) noexcept;

  private:
    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &) noexcept final
    {
      return false;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::SVGScriptElement)
  KRYS_NODISCARD static bool IsType(const krys::web::Node &target) noexcept
  {
    return target.IsSVGElement() && Downcast<krys::web::SVGElement>(target).IsSVGScriptElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::web::SVGElement &target) noexcept
  {
    return target.IsSVGScriptElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
