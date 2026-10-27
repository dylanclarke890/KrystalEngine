#pragma once

#include "Krystal.Booey/SVG/SVGGraphicsElement.hpp"

namespace krys::boo::svg
{
  /// @brief The `<switch>` element: draws only its first child whose conditional processing attributes pass.
  /// @see https://svgwg.org/svg2-draft/struct.html#InterfaceSVGSwitchElement
  class SVGSwitchElement final : public SVGGraphicsElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGSwitchElement);

  public:
    explicit SVGSwitchElement(dom::Document &document) noexcept;

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGSwitchElement, SVGGraphicsElement>;

  private:
    KRYS_NODISCARD bool IsValid() const noexcept final
    {
      return SVGTests::IsValid();
    }
    KRYS_NODISCARD bool ChildShouldCreateRenderer(const dom::Node &) const noexcept final;
    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGSwitchElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGSwitchElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
