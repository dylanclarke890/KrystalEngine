#pragma once

#include "Krystal.Booey/SVG/SVGElement.hpp"

namespace krys::boo::svg
{
  /// @brief The `<title>` element: a title for its parent (or the document, as the document element's child),
  /// never rendered.
  /// @see https://svgwg.org/svg2-draft/struct.html#InterfaceSVGTitleElement
  class SVGTitleElement final : public SVGElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGTitleElement);

  public:
    explicit SVGTitleElement(dom::Document &document) noexcept;

  private:
    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &) noexcept final
    {
      return false;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGTitleElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGTitleElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();