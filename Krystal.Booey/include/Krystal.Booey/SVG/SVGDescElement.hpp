#pragma once

#include "Krystal.Booey/SVG/SVGElement.hpp"

namespace krys::boo::svg
{
  /// @brief The `<desc>` element: a description of its parent, never rendered.
  /// @see https://svgwg.org/svg2-draft/struct.html#InterfaceSVGDescElement
  class SVGDescElement final : public SVGElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGDescElement);

  public:
    explicit SVGDescElement(dom::Document &document) noexcept;

  private:
    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &) noexcept final
    {
      return false;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGDescElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGDescElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();