#pragma once

#include "Krystal.Booey/SVG/SVGGraphicsElement.hpp"

namespace krys::boo::svg
{
  /// @brief The `<defs>` element: graphics that aren't drawn where they are, only referenced.
  /// @see https://svgwg.org/svg2-draft/struct.html#InterfaceSVGDefsElement
  class SVGDefsElement final : public SVGGraphicsElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGDefsElement);

  public:
    explicit SVGDefsElement(dom::Document &document) noexcept;

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGDefsElement, SVGGraphicsElement>;

  private:
    KRYS_NODISCARD bool IsValid() const noexcept final;
    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGDefsElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGDefsElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
