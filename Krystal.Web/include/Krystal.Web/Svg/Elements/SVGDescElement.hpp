#pragma once

#include "Krystal.Web/Svg/Elements/SVGElement.hpp"

namespace krys::web
{
  /// @brief The `<desc>` element: a description of its parent, never rendered.
  /// @see https://svgwg.org/svg2-draft/struct.html#InterfaceSVGDescElement
  class SVGDescElement final : public SVGElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGDescElement);

  public:
    explicit SVGDescElement(Document &document) noexcept;

  private:
    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &) noexcept final
    {
      return false;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::SVGDescElement)
  KRYS_NODISCARD static bool IsType(const krys::web::SVGElement &target) noexcept
  {
    return target.IsSVGDescElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
