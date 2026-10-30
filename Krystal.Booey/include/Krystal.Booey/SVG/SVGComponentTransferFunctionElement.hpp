#pragma once

#include "Krystal.Booey/Graphics/Filters/FEComponentTransfer.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimatedPropertyImpl.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"

namespace krys::boo::svg
{
  template <>
  struct SVGPropertyTraits<ComponentTransferType>
  {
    KRYS_NODISCARD static uint32 HighestEnumValue() noexcept
    {
      return static_cast<uint32>(ComponentTransferType::FECOMPONENTTRANSFER_TYPE_GAMMA);
    }

    KRYS_NODISCARD static dom::DOMString ToString(ComponentTransferType type) noexcept
    {
      switch (type)
      {
        case ComponentTransferType::FECOMPONENTTRANSFER_TYPE_UNKNOWN:  return {};
        case ComponentTransferType::FECOMPONENTTRANSFER_TYPE_IDENTITY: return u8"identity";
        case ComponentTransferType::FECOMPONENTTRANSFER_TYPE_TABLE:    return u8"table";
        case ComponentTransferType::FECOMPONENTTRANSFER_TYPE_DISCRETE: return u8"discrete";
        case ComponentTransferType::FECOMPONENTTRANSFER_TYPE_LINEAR:   return u8"linear";
        case ComponentTransferType::FECOMPONENTTRANSFER_TYPE_GAMMA:    return u8"gamma";
      }

      krys_assert_msg(false, "Not reached");
      return {};
    }

    KRYS_NODISCARD static ComponentTransferType FromString(SVGElement &, dom::DOMStringView value) noexcept
    {
      if (value == u8"discrete")
      {
        return ComponentTransferType::FECOMPONENTTRANSFER_TYPE_DISCRETE;
      }
      if (value == u8"gamma")
      {
        return ComponentTransferType::FECOMPONENTTRANSFER_TYPE_GAMMA;
      }
      if (value == u8"identity")
      {
        return ComponentTransferType::FECOMPONENTTRANSFER_TYPE_IDENTITY;
      }
      if (value == u8"linear")
      {
        return ComponentTransferType::FECOMPONENTTRANSFER_TYPE_LINEAR;
      }
      if (value == u8"table")
      {
        return ComponentTransferType::FECOMPONENTTRANSFER_TYPE_TABLE;
      }
      return ComponentTransferType::FECOMPONENTTRANSFER_TYPE_UNKNOWN;
    }
  };

  /// @brief The base of the transfer function elements: how their `<feComponentTransfer>` transfers one
  /// channel.
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGComponentTransferFunctionElement
  class SVGComponentTransferFunctionElement : public SVGElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGComponentTransferFunctionElement);

  private:
    Ref<SVGAnimatedEnumeration> _type;
    Ref<SVGAnimatedNumberList> _tableValues;
    Ref<SVGAnimatedNumber> _slope;
    Ref<SVGAnimatedNumber> _intercept;
    Ref<SVGAnimatedNumber> _amplitude;
    Ref<SVGAnimatedNumber> _exponent;
    Ref<SVGAnimatedNumber> _offset;

  public:
    KRYS_NODISCARD virtual ComponentTransferChannel Channel() const noexcept = 0;
    KRYS_NODISCARD ComponentTransferFunction TransferFunction() const noexcept;

    KRYS_NODISCARD ComponentTransferType GetType() const noexcept
    {
      return _type->CurrentValue<ComponentTransferType>();
    }
    KRYS_NODISCARD const SVGNumberList &TableValues() const noexcept
    {
      return _tableValues->CurrentValue();
    }
    KRYS_NODISCARD float Slope() const noexcept
    {
      return _slope->CurrentValue();
    }
    KRYS_NODISCARD float Intercept() const noexcept
    {
      return _intercept->CurrentValue();
    }
    KRYS_NODISCARD float Amplitude() const noexcept
    {
      return _amplitude->CurrentValue();
    }
    KRYS_NODISCARD float Exponent() const noexcept
    {
      return _exponent->CurrentValue();
    }
    KRYS_NODISCARD float Offset() const noexcept
    {
      return _offset->CurrentValue();
    }

    KRYS_NODISCARD SVGAnimatedEnumeration &TypeAnimated() noexcept
    {
      return *_type;
    }
    KRYS_NODISCARD SVGAnimatedNumberList &TableValuesAnimated() noexcept
    {
      return *_tableValues;
    }
    KRYS_NODISCARD SVGAnimatedNumber &SlopeAnimated() noexcept
    {
      return *_slope;
    }
    KRYS_NODISCARD SVGAnimatedNumber &InterceptAnimated() noexcept
    {
      return *_intercept;
    }
    KRYS_NODISCARD SVGAnimatedNumber &AmplitudeAnimated() noexcept
    {
      return *_amplitude;
    }
    KRYS_NODISCARD SVGAnimatedNumber &ExponentAnimated() noexcept
    {
      return *_exponent;
    }
    KRYS_NODISCARD SVGAnimatedNumber &OffsetAnimated() noexcept
    {
      return *_offset;
    }

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGComponentTransferFunctionElement, SVGElement>;

  protected:
    SVGComponentTransferFunctionElement(dom::Document &document, SVGElementInterface interface) noexcept;

    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept override;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept override;

    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &) noexcept override
    {
      return false;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGComponentTransferFunctionElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGFEFuncAElement() || target.IsSVGFEFuncBElement() || target.IsSVGFEFuncGElement()
           || target.IsSVGFEFuncRElement();
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    auto *svgElement = krys::DynamicDowncast<krys::boo::svg::SVGElement>(node);
    return svgElement && IsType(*svgElement);
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
