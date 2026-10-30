#pragma once

#include "Krystal.Booey/Graphics/Filters/FEConvolveMatrix.hpp"
#include "Krystal.Booey/SVG/SVGFilterPrimitiveStandardAttributes.hpp"

namespace krys::boo::svg
{
  template <>
  struct SVGPropertyTraits<EdgeModeType>
  {
    KRYS_NODISCARD static uint32 HighestEnumValue() noexcept
    {
      return static_cast<uint32>(EdgeModeType::None);
    }
    KRYS_NODISCARD static EdgeModeType InitialValue() noexcept
    {
      return EdgeModeType::None;
    }

    KRYS_NODISCARD static dom::DOMString ToString(EdgeModeType type) noexcept
    {
      switch (type)
      {
        case EdgeModeType::Unknown:   return {};
        case EdgeModeType::Duplicate: return u8"duplicate";
        case EdgeModeType::Wrap:      return u8"wrap";
        case EdgeModeType::None:      return u8"none";
      }

      krys_assert_msg(false, "Not reached");
      return {};
    }

    KRYS_NODISCARD static EdgeModeType FromString(SVGElement &, dom::DOMStringView value) noexcept
    {
      if (value == u8"duplicate")
      {
        return EdgeModeType::Duplicate;
      }
      if (value == u8"wrap")
      {
        return EdgeModeType::Wrap;
      }
      if (value == u8"none")
      {
        return EdgeModeType::None;
      }
      return EdgeModeType::Unknown;
    }
  };

  /// @brief The `<feConvolveMatrix>` element: a kernel convolved with its input.
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGFEConvolveMatrixElement
  class SVGFEConvolveMatrixElement final : public SVGFilterPrimitiveStandardAttributes
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGFEConvolveMatrixElement);

    static constexpr int32 InitialOrderValue = 3;
    static constexpr float InitialDivisorValue = 1;
    static constexpr float InitialKernelUnitLengthValue = 0;

  private:
    Ref<SVGAnimatedString> _in1;
    Ref<SVGAnimatedInteger> _orderX;
    Ref<SVGAnimatedInteger> _orderY;
    Ref<SVGAnimatedNumberList> _kernelMatrix;
    Ref<SVGAnimatedNumber> _divisor;
    Ref<SVGAnimatedNumber> _bias;
    Ref<SVGAnimatedInteger> _targetX;
    Ref<SVGAnimatedInteger> _targetY;
    Ref<SVGAnimatedEnumeration> _edgeMode;
    Ref<SVGAnimatedNumber> _kernelUnitLengthX;
    Ref<SVGAnimatedNumber> _kernelUnitLengthY;
    Ref<SVGAnimatedBoolean> _preserveAlpha;

  public:
    explicit SVGFEConvolveMatrixElement(dom::Document &document) noexcept;

    void SetOrder(float orderX, float orderY) noexcept;
    void SetKernelUnitLength(float kernelUnitLengthX, float kernelUnitLengthY) noexcept;

    KRYS_NODISCARD dom::DOMString In1() const noexcept
    {
      return _in1->CurrentValue();
    }
    KRYS_NODISCARD int32 OrderX() const noexcept
    {
      return _orderX->CurrentValue();
    }
    KRYS_NODISCARD int32 OrderY() const noexcept
    {
      return _orderY->CurrentValue();
    }
    KRYS_NODISCARD const SVGNumberList &KernelMatrix() const noexcept
    {
      return _kernelMatrix->CurrentValue();
    }
    KRYS_NODISCARD float Divisor() const noexcept
    {
      return _divisor->CurrentValue();
    }
    KRYS_NODISCARD float Bias() const noexcept
    {
      return _bias->CurrentValue();
    }
    KRYS_NODISCARD int32 TargetX() const noexcept
    {
      return _targetX->CurrentValue();
    }
    KRYS_NODISCARD int32 TargetY() const noexcept
    {
      return _targetY->CurrentValue();
    }
    KRYS_NODISCARD EdgeModeType EdgeMode() const noexcept
    {
      return _edgeMode->CurrentValue<EdgeModeType>();
    }
    KRYS_NODISCARD float KernelUnitLengthX() const noexcept
    {
      return _kernelUnitLengthX->CurrentValue();
    }
    KRYS_NODISCARD float KernelUnitLengthY() const noexcept
    {
      return _kernelUnitLengthY->CurrentValue();
    }
    KRYS_NODISCARD bool PreserveAlpha() const noexcept
    {
      return _preserveAlpha->CurrentValue();
    }

    KRYS_NODISCARD SVGAnimatedString &In1Animated() noexcept
    {
      return *_in1;
    }
    KRYS_NODISCARD SVGAnimatedInteger &OrderXAnimated() noexcept
    {
      return *_orderX;
    }
    KRYS_NODISCARD SVGAnimatedInteger &OrderYAnimated() noexcept
    {
      return *_orderY;
    }
    KRYS_NODISCARD SVGAnimatedNumberList &KernelMatrixAnimated() noexcept
    {
      return *_kernelMatrix;
    }
    KRYS_NODISCARD SVGAnimatedNumber &DivisorAnimated() noexcept
    {
      return *_divisor;
    }
    KRYS_NODISCARD SVGAnimatedNumber &BiasAnimated() noexcept
    {
      return *_bias;
    }
    KRYS_NODISCARD SVGAnimatedInteger &TargetXAnimated() noexcept
    {
      return *_targetX;
    }
    KRYS_NODISCARD SVGAnimatedInteger &TargetYAnimated() noexcept
    {
      return *_targetY;
    }
    KRYS_NODISCARD SVGAnimatedEnumeration &EdgeModeAnimated() noexcept
    {
      return *_edgeMode;
    }
    KRYS_NODISCARD SVGAnimatedNumber &KernelUnitLengthXAnimated() noexcept
    {
      return *_kernelUnitLengthX;
    }
    KRYS_NODISCARD SVGAnimatedNumber &KernelUnitLengthYAnimated() noexcept
    {
      return *_kernelUnitLengthY;
    }
    KRYS_NODISCARD SVGAnimatedBoolean &PreserveAlphaAnimated() noexcept
    {
      return *_preserveAlpha;
    }

    using PropertyRegistry =
      SVGPropertyOwnerRegistry<SVGFEConvolveMatrixElement, SVGFilterPrimitiveStandardAttributes>;

  private:
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept final;

    KRYS_NODISCARD bool IsValidTargetXOffset() const noexcept;
    KRYS_NODISCARD bool IsValidTargetYOffset() const noexcept;

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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGFEConvolveMatrixElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGFEConvolveMatrixElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
