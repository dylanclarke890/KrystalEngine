#pragma once

#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Booey/SVG/SVGFilterPrimitiveStandardAttributes.hpp"

namespace krys::boo::svg
{
  template <>
  struct SVGPropertyTraits<BlendMode>
  {
    KRYS_NODISCARD static uint32 HighestEnumValue() noexcept
    {
      return static_cast<uint32>(BlendMode::Luminosity);
    }

    KRYS_NODISCARD static BlendMode FromString(SVGElement &, dom::DOMStringView string) noexcept
    {
      BlendMode mode = BlendMode::Normal;
      ParseBlendMode(string, mode);
      return mode;
    }

    KRYS_NODISCARD static dom::DOMString ToString(BlendMode type) noexcept
    {
      if (type < BlendMode::PlusDarker)
      {
        auto name = BlendModeName(type);
        return dom::DOMString(name.begin(), name.end());
      }

      return {};
    }
  };

  /// @brief The `<feBlend>` element: its first input blended over its second.
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGFEBlendElement
  class SVGFEBlendElement final : public SVGFilterPrimitiveStandardAttributes
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGFEBlendElement);

  private:
    Ref<SVGAnimatedString> _in1;
    Ref<SVGAnimatedString> _in2;
    Ref<SVGAnimatedEnumeration> _mode;

  public:
    explicit SVGFEBlendElement(dom::Document &document) noexcept;

    KRYS_NODISCARD dom::DOMString In1() const noexcept
    {
      return _in1->CurrentValue();
    }
    KRYS_NODISCARD dom::DOMString In2() const noexcept
    {
      return _in2->CurrentValue();
    }
    KRYS_NODISCARD BlendMode Mode() const noexcept
    {
      return _mode->CurrentValue<BlendMode>();
    }

    KRYS_NODISCARD SVGAnimatedString &In1Animated() noexcept
    {
      return *_in1;
    }
    KRYS_NODISCARD SVGAnimatedString &In2Animated() noexcept
    {
      return *_in2;
    }
    KRYS_NODISCARD SVGAnimatedEnumeration &ModeAnimated() noexcept
    {
      return *_mode;
    }

    using PropertyRegistry =
      SVGPropertyOwnerRegistry<SVGFEBlendElement, SVGFilterPrimitiveStandardAttributes>;

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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGFEBlendElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGFEBlendElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
