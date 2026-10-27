#pragma once

#include "Krystal.Booey/Graphics/Filters/FETurbulence.hpp"
#include "Krystal.Booey/SVG/SVGFilterPrimitiveStandardAttributes.hpp"

namespace krys::boo::svg
{
  enum SVGStitchOptions : uint8
  {
    SVG_STITCHTYPE_UNKNOWN = 0,
    SVG_STITCHTYPE_STITCH = 1,
    SVG_STITCHTYPE_NOSTITCH = 2
  };

  template <>
  struct SVGPropertyTraits<SVGStitchOptions>
  {
    KRYS_NODISCARD static uint32 HighestEnumValue() noexcept
    {
      return SVG_STITCHTYPE_NOSTITCH;
    }

    KRYS_NODISCARD static dom::DOMString ToString(SVGStitchOptions type) noexcept
    {
      switch (type)
      {
        case SVG_STITCHTYPE_UNKNOWN:  return {};
        case SVG_STITCHTYPE_STITCH:   return u8"stitch";
        case SVG_STITCHTYPE_NOSTITCH: return u8"noStitch";
      }

      krys_assert_msg(false, "Not reached");
      return {};
    }

    KRYS_NODISCARD static SVGStitchOptions FromString(SVGElement &, dom::DOMStringView value) noexcept
    {
      if (value == u8"stitch")
      {
        return SVG_STITCHTYPE_STITCH;
      }
      if (value == u8"noStitch")
      {
        return SVG_STITCHTYPE_NOSTITCH;
      }
      return SVG_STITCHTYPE_UNKNOWN;
    }
  };

  template <>
  struct SVGPropertyTraits<TurbulenceType>
  {
    KRYS_NODISCARD static uint32 HighestEnumValue() noexcept
    {
      return static_cast<uint32>(TurbulenceType::Turbulence);
    }

    KRYS_NODISCARD static dom::DOMString ToString(TurbulenceType type) noexcept
    {
      switch (type)
      {
        case TurbulenceType::Unknown:      return {};
        case TurbulenceType::FractalNoise: return u8"fractalNoise";
        case TurbulenceType::Turbulence:   return u8"turbulence";
      }

      krys_assert_msg(false, "Not reached");
      return {};
    }

    KRYS_NODISCARD static TurbulenceType FromString(SVGElement &, dom::DOMStringView value) noexcept
    {
      if (value == u8"fractalNoise")
      {
        return TurbulenceType::FractalNoise;
      }
      if (value == u8"turbulence")
      {
        return TurbulenceType::Turbulence;
      }
      return TurbulenceType::Unknown;
    }
  };

  /// @brief The `<feTurbulence>` element: Perlin noise.
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGFETurbulenceElement
  class SVGFETurbulenceElement final : public SVGFilterPrimitiveStandardAttributes
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGFETurbulenceElement);

    static constexpr int32 InitialOctavesValue = 1;

  private:
    Ref<SVGAnimatedNumber> _baseFrequencyX;
    Ref<SVGAnimatedNumber> _baseFrequencyY;
    Ref<SVGAnimatedInteger> _numOctaves;
    Ref<SVGAnimatedNumber> _seed;
    Ref<SVGAnimatedEnumeration> _stitchTiles;
    Ref<SVGAnimatedEnumeration> _type;

  public:
    explicit SVGFETurbulenceElement(dom::Document &document) noexcept;

    KRYS_NODISCARD float BaseFrequencyX() const noexcept
    {
      return _baseFrequencyX->CurrentValue();
    }
    KRYS_NODISCARD float BaseFrequencyY() const noexcept
    {
      return _baseFrequencyY->CurrentValue();
    }
    KRYS_NODISCARD int32 NumOctaves() const noexcept
    {
      return _numOctaves->CurrentValue();
    }
    KRYS_NODISCARD float Seed() const noexcept
    {
      return _seed->CurrentValue();
    }
    KRYS_NODISCARD SVGStitchOptions StitchTiles() const noexcept
    {
      return _stitchTiles->CurrentValue<SVGStitchOptions>();
    }
    KRYS_NODISCARD TurbulenceType GetType() const noexcept
    {
      return _type->CurrentValue<TurbulenceType>();
    }

    KRYS_NODISCARD SVGAnimatedNumber &BaseFrequencyXAnimated() noexcept
    {
      return *_baseFrequencyX;
    }
    KRYS_NODISCARD SVGAnimatedNumber &BaseFrequencyYAnimated() noexcept
    {
      return *_baseFrequencyY;
    }
    KRYS_NODISCARD SVGAnimatedInteger &NumOctavesAnimated() noexcept
    {
      return *_numOctaves;
    }
    KRYS_NODISCARD SVGAnimatedNumber &SeedAnimated() noexcept
    {
      return *_seed;
    }
    KRYS_NODISCARD SVGAnimatedEnumeration &StitchTilesAnimated() noexcept
    {
      return *_stitchTiles;
    }
    KRYS_NODISCARD SVGAnimatedEnumeration &TypeAnimated() noexcept
    {
      return *_type;
    }

    using PropertyRegistry =
      SVGPropertyOwnerRegistry<SVGFETurbulenceElement, SVGFilterPrimitiveStandardAttributes>;

  private:
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept final;

    bool SetFilterEffectAttribute(FilterEffect &effect,
                                  const dom::QualifiedName &attributeName) noexcept final;
    KRYS_NODISCARD RefPtr<FilterEffect>
      CreateFilterEffect(const FilterEffectVector &inputs,
                         const GraphicsContext &destinationContext) const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGFETurbulenceElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGFETurbulenceElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
