#pragma once

#include "Krystal.Web/Svg/Elements/SVGElement.hpp"

namespace krys::web
{
  /// @brief The `<metadata>` element: metadata about the document, never rendered.
  /// @see https://svgwg.org/svg2-draft/struct.html#InterfaceSVGMetadataElement
  class SVGMetadataElement final : public SVGElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGMetadataElement);

  public:
    explicit SVGMetadataElement(Document &document) noexcept;

  private:
    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &) noexcept final
    {
      return false;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::SVGMetadataElement)
  KRYS_NODISCARD static bool IsType(const krys::web::SVGElement &target) noexcept
  {
    return target.IsSVGMetadataElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
