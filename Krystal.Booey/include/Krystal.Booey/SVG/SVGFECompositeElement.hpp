#pragma once

#include "Krystal.Booey/Graphics/Filters/FEComposite.hpp"
#include "Krystal.Booey/SVG/SVGFilterPrimitiveStandardAttributes.hpp"

namespace krys::boo::svg
{
  template <>
  struct SVGPropertyTraits<CompositeOperationType>
  {
    KRYS_NODISCARD static uint32 HighestEnumValue() noexcept
    {
      return static_cast<uint32>(CompositeOperationType::FECOMPOSITE_OPERATOR_LIGHTER);
    }

    KRYS_NODISCARD static dom::DOMString ToString(CompositeOperationType type) noexcept
    {
      switch (type)
      {
        case CompositeOperationType::FECOMPOSITE_OPERATOR_UNKNOWN:    return {};
        case CompositeOperationType::FECOMPOSITE_OPERATOR_OVER:       return u8"over";
        case CompositeOperationType::FECOMPOSITE_OPERATOR_IN:         return u8"in";
        case CompositeOperationType::FECOMPOSITE_OPERATOR_OUT:        return u8"out";
        case CompositeOperationType::FECOMPOSITE_OPERATOR_ATOP:       return u8"atop";
        case CompositeOperationType::FECOMPOSITE_OPERATOR_XOR:        return u8"xor";
        case CompositeOperationType::FECOMPOSITE_OPERATOR_ARITHMETIC: return u8"arithmetic";
        case CompositeOperationType::FECOMPOSITE_OPERATOR_LIGHTER:    return u8"lighter";
      }

      krys_assert_msg(false, "Not reached");
      return {};
    }

    KRYS_NODISCARD static CompositeOperationType FromString(SVGElement &, dom::DOMStringView value) noexcept
    {
      if (value == u8"arithmetic")
      {
        return CompositeOperationType::FECOMPOSITE_OPERATOR_ARITHMETIC;
      }
      if (value == u8"atop")
      {
        return CompositeOperationType::FECOMPOSITE_OPERATOR_ATOP;
      }
      if (value == u8"in")
      {
        return CompositeOperationType::FECOMPOSITE_OPERATOR_IN;
      }
      if (value == u8"lighter")
      {
        return CompositeOperationType::FECOMPOSITE_OPERATOR_LIGHTER;
      }
      if (value == u8"out")
      {
        return CompositeOperationType::FECOMPOSITE_OPERATOR_OUT;
      }
      if (value == u8"over")
      {
        return CompositeOperationType::FECOMPOSITE_OPERATOR_OVER;
      }
      if (value == u8"xor")
      {
        return CompositeOperationType::FECOMPOSITE_OPERATOR_XOR;
      }
      return CompositeOperationType::FECOMPOSITE_OPERATOR_UNKNOWN;
    }
  };

  /// @brief The `<feComposite>` element: its two inputs, combined by a Porter-Duff or arithmetic operator.
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGFECompositeElement
  class SVGFECompositeElement final : public SVGFilterPrimitiveStandardAttributes
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGFECompositeElement);

  private:
    Ref<SVGAnimatedString> _in1;
    Ref<SVGAnimatedString> _in2;
    Ref<SVGAnimatedEnumeration> _svgOperator;
    Ref<SVGAnimatedNumber> _k1;
    Ref<SVGAnimatedNumber> _k2;
    Ref<SVGAnimatedNumber> _k3;
    Ref<SVGAnimatedNumber> _k4;

  public:
    explicit SVGFECompositeElement(dom::Document &document) noexcept;

    KRYS_NODISCARD dom::DOMString In1() const noexcept
    {
      return _in1->CurrentValue();
    }
    KRYS_NODISCARD dom::DOMString In2() const noexcept
    {
      return _in2->CurrentValue();
    }
    KRYS_NODISCARD CompositeOperationType SVGOperator() const noexcept
    {
      return _svgOperator->CurrentValue<CompositeOperationType>();
    }
    KRYS_NODISCARD float K1() const noexcept
    {
      return _k1->CurrentValue();
    }
    KRYS_NODISCARD float K2() const noexcept
    {
      return _k2->CurrentValue();
    }
    KRYS_NODISCARD float K3() const noexcept
    {
      return _k3->CurrentValue();
    }
    KRYS_NODISCARD float K4() const noexcept
    {
      return _k4->CurrentValue();
    }

    KRYS_NODISCARD SVGAnimatedString &In1Animated() noexcept
    {
      return *_in1;
    }
    KRYS_NODISCARD SVGAnimatedString &In2Animated() noexcept
    {
      return *_in2;
    }
    KRYS_NODISCARD SVGAnimatedEnumeration &SVGOperatorAnimated() noexcept
    {
      return *_svgOperator;
    }
    KRYS_NODISCARD SVGAnimatedNumber &K1Animated() noexcept
    {
      return *_k1;
    }
    KRYS_NODISCARD SVGAnimatedNumber &K2Animated() noexcept
    {
      return *_k2;
    }
    KRYS_NODISCARD SVGAnimatedNumber &K3Animated() noexcept
    {
      return *_k3;
    }
    KRYS_NODISCARD SVGAnimatedNumber &K4Animated() noexcept
    {
      return *_k4;
    }

    using PropertyRegistry =
      SVGPropertyOwnerRegistry<SVGFECompositeElement, SVGFilterPrimitiveStandardAttributes>;

  private:
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept final;

    bool SetFilterEffectAttribute(FilterEffect &effect,
                                  const dom::QualifiedName &attributeName) noexcept final;
    KRYS_NODISCARD List<dom::DOMStringAtom> FilterEffectInputsNames() const noexcept final
    {
      return {dom::DOMStringAtom(In1()), dom::DOMStringAtom(In2())};
    }
    KRYS_NODISCARD RefPtr<FilterEffect>
      CreateFilterEffect(const FilterEffectVector &inputs,
                         const GraphicsContext &destinationContext) const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGFECompositeElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGFECompositeElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
