#pragma once

#include "Krystal.Booey/Graphics/Filters/FEDisplacementMap.hpp"
#include "Krystal.Booey/SVG/SVGFilterPrimitiveStandardAttributes.hpp"

namespace krys::boo::svg
{
  template <>
  struct SVGPropertyTraits<ChannelSelectorType>
  {
    KRYS_NODISCARD static uint32 HighestEnumValue() noexcept
    {
      return static_cast<uint32>(ChannelSelectorType::CHANNEL_A);
    }

    KRYS_NODISCARD static dom::DOMString ToString(ChannelSelectorType type) noexcept
    {
      switch (type)
      {
        case ChannelSelectorType::CHANNEL_UNKNOWN: return {};
        case ChannelSelectorType::CHANNEL_R:       return u8"R";
        case ChannelSelectorType::CHANNEL_G:       return u8"G";
        case ChannelSelectorType::CHANNEL_B:       return u8"B";
        case ChannelSelectorType::CHANNEL_A:       return u8"A";
      }

      krys_assert_msg(false, "Not reached");
      return {};
    }

    KRYS_NODISCARD static ChannelSelectorType FromString(SVGElement &, dom::DOMStringView value) noexcept
    {
      if (value == u8"R")
      {
        return ChannelSelectorType::CHANNEL_R;
      }
      if (value == u8"G")
      {
        return ChannelSelectorType::CHANNEL_G;
      }
      if (value == u8"B")
      {
        return ChannelSelectorType::CHANNEL_B;
      }
      if (value == u8"A")
      {
        return ChannelSelectorType::CHANNEL_A;
      }
      return ChannelSelectorType::CHANNEL_UNKNOWN;
    }
  };

  /// @brief The `<feDisplacementMap>` element: its first input, displaced by its second's channels.
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGFEDisplacementMapElement
  class SVGFEDisplacementMapElement final : public SVGFilterPrimitiveStandardAttributes
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGFEDisplacementMapElement);

  private:
    Ref<SVGAnimatedString> _in1;
    Ref<SVGAnimatedString> _in2;
    Ref<SVGAnimatedEnumeration> _xChannelSelector;
    Ref<SVGAnimatedEnumeration> _yChannelSelector;
    Ref<SVGAnimatedNumber> _scale;

  public:
    explicit SVGFEDisplacementMapElement(dom::Document &document) noexcept;

    KRYS_NODISCARD dom::DOMString In1() const noexcept
    {
      return _in1->CurrentValue();
    }
    KRYS_NODISCARD dom::DOMString In2() const noexcept
    {
      return _in2->CurrentValue();
    }
    KRYS_NODISCARD ChannelSelectorType XChannelSelector() const noexcept
    {
      return _xChannelSelector->CurrentValue<ChannelSelectorType>();
    }
    KRYS_NODISCARD ChannelSelectorType YChannelSelector() const noexcept
    {
      return _yChannelSelector->CurrentValue<ChannelSelectorType>();
    }
    KRYS_NODISCARD float Scale() const noexcept
    {
      return _scale->CurrentValue();
    }

    KRYS_NODISCARD SVGAnimatedString &In1Animated() noexcept
    {
      return *_in1;
    }
    KRYS_NODISCARD SVGAnimatedString &In2Animated() noexcept
    {
      return *_in2;
    }
    KRYS_NODISCARD SVGAnimatedEnumeration &XChannelSelectorAnimated() noexcept
    {
      return *_xChannelSelector;
    }
    KRYS_NODISCARD SVGAnimatedEnumeration &YChannelSelectorAnimated() noexcept
    {
      return *_yChannelSelector;
    }
    KRYS_NODISCARD SVGAnimatedNumber &ScaleAnimated() noexcept
    {
      return *_scale;
    }

    using PropertyRegistry =
      SVGPropertyOwnerRegistry<SVGFEDisplacementMapElement, SVGFilterPrimitiveStandardAttributes>;

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
    KRYS_NODISCARD IntOutsets Outsets(const FloatRect &targetBoundingBox,
                                      SVGUnitTypes::SVGUnitType primitiveUnits) const noexcept final;
    KRYS_NODISCARD RefPtr<FilterEffect>
      CreateFilterEffect(const FilterEffectVector &inputs,
                         const GraphicsContext &destinationContext) const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGFEDisplacementMapElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGFEDisplacementMapElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
