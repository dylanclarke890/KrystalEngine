#pragma once

#include "Krystal.Booey/SVG/SVGFilterPrimitiveStandardAttributes.hpp"

namespace krys::boo::svg
{
  /// @brief The `<feFlood>` element: its subregion, filled with its `flood-color` and `flood-opacity`.
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGFEFloodElement
  class SVGFEFloodElement final : public SVGFilterPrimitiveStandardAttributes
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGFEFloodElement);

  public:
    explicit SVGFEFloodElement(dom::Document &document) noexcept;

    using PropertyRegistry =
      SVGPropertyOwnerRegistry<SVGFEFloodElement, SVGFilterPrimitiveStandardAttributes>;

  private:
    bool SetFilterEffectAttribute(FilterEffect &effect,
                                  const dom::QualifiedName &attributeName) noexcept final;
    KRYS_NODISCARD RefPtr<FilterEffect>
      CreateFilterEffect(const FilterEffectVector &inputs,
                         const GraphicsContext &destinationContext) const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGFEFloodElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGFEFloodElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
