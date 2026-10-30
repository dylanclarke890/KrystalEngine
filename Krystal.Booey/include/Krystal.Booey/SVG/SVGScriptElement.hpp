#pragma once

#include "Krystal.Booey/SVG/SVGElement.hpp"

namespace krys::boo::svg
{
  class SVGScriptElement : public SVGElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGScriptElement);

  public:
    SVGScriptElement(dom::Document &document) noexcept;

  private:
    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &) noexcept final
    {
      return false;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGScriptElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return target.IsSVGElement() && Downcast<krys::boo::svg::SVGElement>(target).IsSVGScriptElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGScriptElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();