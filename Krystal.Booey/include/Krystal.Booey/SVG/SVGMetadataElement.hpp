#pragma once

#include "Krystal.Booey/SVG/SVGElement.hpp"

namespace krys::boo::svg
{
  /// @brief The `<metadata>` element: metadata about the document, never rendered.
  /// @see https://svgwg.org/svg2-draft/struct.html#InterfaceSVGMetadataElement
  class SVGMetadataElement final : public SVGElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGMetadataElement);

  public:
    explicit SVGMetadataElement(dom::Document &document) noexcept;

  private:
    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &) noexcept final
    {
      return false;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGMetadataElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGMetadataElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();