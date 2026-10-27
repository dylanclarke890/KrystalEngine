#pragma once

#include "Krystal.Booey/SVG/SVGGraphicsElement.hpp"

namespace krys::boo::svg
{
  /// @brief The `<g>` element: a group of graphics.
  /// @see https://svgwg.org/svg2-draft/struct.html#InterfaceSVGGElement
  class SVGGElement final : public SVGGraphicsElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGGElement);

  public:
    explicit SVGGElement(dom::Document &document) noexcept;

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGGElement, SVGGraphicsElement>;

  private:
    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&) noexcept final;
    KRYS_NODISCARD bool IsValid() const noexcept final
    {
      return SVGTests::IsValid();
    }
    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGGElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGGElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
