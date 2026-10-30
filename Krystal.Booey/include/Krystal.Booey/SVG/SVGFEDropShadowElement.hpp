#pragma once

#include "Krystal.Booey/SVG/SVGFilterPrimitiveStandardAttributes.hpp"

namespace krys::boo::svg
{
  /// @brief The `<feDropShadow>` element: its input over its blurred, offset and coloured shadow.
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGFEDropShadowElement
  class SVGFEDropShadowElement final : public SVGFilterPrimitiveStandardAttributes
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGFEDropShadowElement);

  private:
    Ref<SVGAnimatedString> _in1;
    Ref<SVGAnimatedNumber> _dx;
    Ref<SVGAnimatedNumber> _dy;
    Ref<SVGAnimatedNumber> _stdDeviationX;
    Ref<SVGAnimatedNumber> _stdDeviationY;

  public:
    explicit SVGFEDropShadowElement(dom::Document &document) noexcept;

    void SetStdDeviation(float stdDeviationX, float stdDeviationY) noexcept;

    KRYS_NODISCARD dom::DOMString In1() const noexcept
    {
      return _in1->CurrentValue();
    }
    KRYS_NODISCARD float Dx() const noexcept
    {
      return _dx->CurrentValue();
    }
    KRYS_NODISCARD float Dy() const noexcept
    {
      return _dy->CurrentValue();
    }
    KRYS_NODISCARD float StdDeviationX() const noexcept
    {
      return _stdDeviationX->CurrentValue();
    }
    KRYS_NODISCARD float StdDeviationY() const noexcept
    {
      return _stdDeviationY->CurrentValue();
    }

    KRYS_NODISCARD SVGAnimatedString &In1Animated() noexcept
    {
      return *_in1;
    }
    KRYS_NODISCARD SVGAnimatedNumber &DxAnimated() noexcept
    {
      return *_dx;
    }
    KRYS_NODISCARD SVGAnimatedNumber &DyAnimated() noexcept
    {
      return *_dy;
    }
    KRYS_NODISCARD SVGAnimatedNumber &StdDeviationXAnimated() noexcept
    {
      return *_stdDeviationX;
    }
    KRYS_NODISCARD SVGAnimatedNumber &StdDeviationYAnimated() noexcept
    {
      return *_stdDeviationY;
    }

    using PropertyRegistry =
      SVGPropertyOwnerRegistry<SVGFEDropShadowElement, SVGFilterPrimitiveStandardAttributes>;

  private:
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept final;

    bool SetFilterEffectAttribute(FilterEffect &effect,
                                  const dom::QualifiedName &attributeName) noexcept final;
    KRYS_NODISCARD List<dom::DOMStringAtom> FilterEffectInputsNames() const noexcept final
    {
      return {dom::DOMStringAtom(In1())};
    }
    KRYS_NODISCARD bool IsIdentity() const noexcept final;
    KRYS_NODISCARD IntOutsets Outsets(const FloatRect &targetBoundingBox,
                                      SVGUnitTypes::SVGUnitType primitiveUnits) const noexcept final;
    KRYS_NODISCARD RefPtr<FilterEffect>
      CreateFilterEffect(const FilterEffectVector &inputs,
                         const GraphicsContext &destinationContext) const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGFEDropShadowElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGFEDropShadowElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
