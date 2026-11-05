#pragma once

#include "Krystal.Booey/SVG/SVGFilterPrimitiveStandardAttributes.hpp"

namespace krys::boo::svg
{
  /// @brief The `<feMerge>` element: its `<feMergeNode>`s' inputs, each over the one before.
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGFEMergeElement
  class SVGFEMergeElement final : public SVGFilterPrimitiveStandardAttributes
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGFEMergeElement);

  public:
    explicit SVGFEMergeElement(dom::Document &document) noexcept;

    using PropertyRegistry =
      SVGPropertyOwnerRegistry<SVGFEMergeElement, SVGFilterPrimitiveStandardAttributes>;

  private:
    void OnChildrenChanged() noexcept final;

    KRYS_NODISCARD List<dom::DOMStringAtom> FilterEffectInputsNames() const noexcept final;
    KRYS_NODISCARD RefPtr<FilterEffect>
      CreateFilterEffect(const FilterEffectVector &inputs,
                         const GraphicsContext &destinationContext) const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGFEMergeElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGFEMergeElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
