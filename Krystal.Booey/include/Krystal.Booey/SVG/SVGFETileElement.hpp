#pragma once

#include "Krystal.Booey/SVG/SVGFilterPrimitiveStandardAttributes.hpp"

namespace krys::boo::svg
{
  /// @brief The `<feTile>` element: its subregion, tiled with its input.
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGFETileElement
  class SVGFETileElement final : public SVGFilterPrimitiveStandardAttributes
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGFETileElement);

  private:
    Ref<SVGAnimatedString> _in1;

  public:
    explicit SVGFETileElement(dom::Document &document) noexcept;

    KRYS_NODISCARD dom::DOMString In1() const noexcept
    {
      return _in1->CurrentValue();
    }
    KRYS_NODISCARD SVGAnimatedString &In1Animated() noexcept
    {
      return *_in1;
    }

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGFETileElement, SVGFilterPrimitiveStandardAttributes>;

  private:
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept final;

    KRYS_NODISCARD List<dom::DOMStringAtom> FilterEffectInputsNames() const noexcept final
    {
      return {dom::DOMStringAtom(In1())};
    }
    KRYS_NODISCARD RefPtr<FilterEffect>
      CreateFilterEffect(const FilterEffectVector &inputs,
                         const GraphicsContext &destinationContext) const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGFETileElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGFETileElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
