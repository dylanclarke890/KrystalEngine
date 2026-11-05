#pragma once

#include "Krystal.Booey/SVG/SVGFilterPrimitiveStandardAttributes.hpp"

namespace krys::boo::svg
{
  class SVGFELightElement;

  /// @brief The `<feDiffuseLighting>` element: its input's alpha as a bump map, lit by its light source
  /// child.
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGFEDiffuseLightingElement
  class SVGFEDiffuseLightingElement final : public SVGFilterPrimitiveStandardAttributes
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGFEDiffuseLightingElement);

  private:
    Ref<SVGAnimatedString> _in1;
    Ref<SVGAnimatedNumber> _diffuseConstant;
    Ref<SVGAnimatedNumber> _surfaceScale;
    Ref<SVGAnimatedNumber> _kernelUnitLengthX;
    Ref<SVGAnimatedNumber> _kernelUnitLengthY;

  public:
    explicit SVGFEDiffuseLightingElement(dom::Document &document) noexcept;

    void LightElementAttributeChanged(const SVGFELightElement *lightElement,
                                      const dom::QualifiedName &attributeName) noexcept;

    KRYS_NODISCARD dom::DOMString In1() const noexcept
    {
      return _in1->CurrentValue();
    }
    KRYS_NODISCARD float DiffuseConstant() const noexcept
    {
      return _diffuseConstant->CurrentValue();
    }
    KRYS_NODISCARD float SurfaceScale() const noexcept
    {
      return _surfaceScale->CurrentValue();
    }
    KRYS_NODISCARD float KernelUnitLengthX() const noexcept
    {
      return _kernelUnitLengthX->CurrentValue();
    }
    KRYS_NODISCARD float KernelUnitLengthY() const noexcept
    {
      return _kernelUnitLengthY->CurrentValue();
    }

    KRYS_NODISCARD SVGAnimatedString &In1Animated() noexcept
    {
      return *_in1;
    }
    KRYS_NODISCARD SVGAnimatedNumber &DiffuseConstantAnimated() noexcept
    {
      return *_diffuseConstant;
    }
    KRYS_NODISCARD SVGAnimatedNumber &SurfaceScaleAnimated() noexcept
    {
      return *_surfaceScale;
    }
    KRYS_NODISCARD SVGAnimatedNumber &KernelUnitLengthXAnimated() noexcept
    {
      return *_kernelUnitLengthX;
    }
    KRYS_NODISCARD SVGAnimatedNumber &KernelUnitLengthYAnimated() noexcept
    {
      return *_kernelUnitLengthY;
    }

    using PropertyRegistry =
      SVGPropertyOwnerRegistry<SVGFEDiffuseLightingElement, SVGFilterPrimitiveStandardAttributes>;

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
    KRYS_NODISCARD RefPtr<FilterEffect>
      CreateFilterEffect(const FilterEffectVector &inputs,
                         const GraphicsContext &destinationContext) const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGFEDiffuseLightingElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGFEDiffuseLightingElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
