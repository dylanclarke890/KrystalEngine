#include "Krystal.Booey/SVG/SVGFETurbulenceElement.hpp"
#include "Krystal.Booey/SVG/Internals/SVGParserUtilities.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Core/Text/StringToIntegerConversion.hpp"

namespace krys::boo::svg
{
  SVGFETurbulenceElement::SVGFETurbulenceElement(dom::Document &document) noexcept
      : SVGFilterPrimitiveStandardAttributes(document, CreateUnique<PropertyRegistry>(*this),
                                             SVGElementInterface::FETurbulence),
        _baseFrequencyX(SVGAnimatedNumber::Create(this)), _baseFrequencyY(SVGAnimatedNumber::Create(this)),
        _numOctaves(SVGAnimatedInteger::Create(this, InitialOctavesValue)),
        _seed(SVGAnimatedNumber::Create(this)),
        _stitchTiles(SVGAnimatedEnumeration::Create(this, SVG_STITCHTYPE_NOSTITCH)),
        _type(SVGAnimatedEnumeration::Create(this, TurbulenceType::Turbulence))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGFETurbulenceElement::_baseFrequencyX,
                                         &SVGFETurbulenceElement::_baseFrequencyY>(
        SVGNames::BaseFrequencyAttr());
      PropertyRegistry::RegisterProperty<&SVGFETurbulenceElement::_numOctaves>(SVGNames::NumOctavesAttr());
      PropertyRegistry::RegisterProperty<&SVGFETurbulenceElement::_seed>(SVGNames::SeedAttr());
      PropertyRegistry::RegisterProperty<SVGStitchOptions, &SVGFETurbulenceElement::_stitchTiles>(
        SVGNames::StitchTilesAttr());
      PropertyRegistry::RegisterProperty<TurbulenceType, &SVGFETurbulenceElement::_type>(
        SVGNames::TypeAttr());
    }
  }

  void SVGFETurbulenceElement::AttributeChanged(const dom::QualifiedName &name,
                                                Maybe<dom::DOMStringView> oldValue,
                                                Maybe<dom::DOMStringView> value) noexcept
  {
    auto string = value.value_or(dom::DOMStringView {});

    if (name == SVGNames::TypeAttr())
    {
      auto propertyValue = SVGPropertyTraits<TurbulenceType>::FromString(*this, string);
      if (propertyValue != TurbulenceType::Unknown)
      {
        _type->SetBaseValInternal<TurbulenceType>(propertyValue);
      }
    }
    else if (name == SVGNames::StitchTilesAttr())
    {
      auto propertyValue = SVGPropertyTraits<SVGStitchOptions>::FromString(*this, string);
      if (propertyValue > 0)
      {
        _stitchTiles->SetBaseValInternal<SVGStitchOptions>(propertyValue);
      }
    }
    else if (name == SVGNames::BaseFrequencyAttr())
    {
      if (auto result = ParseNumberOptionalNumber(string))
      {
        _baseFrequencyX->SetBaseValInternal(result->first);
        _baseFrequencyY->SetBaseValInternal(result->second);
      }
    }
    else if (name == SVGNames::SeedAttr())
    {
      _seed->SetBaseValInternal(ToFloatOrZero(string));
    }
    else if (name == SVGNames::NumOctavesAttr())
    {
      // FIXME: Report a value that isn't > 0 ("feTurbulence: problem parsing numOctaves") as a warning, once
      // there's a console.
      _numOctaves->SetBaseValInternal(text::ParseInteger<int32>(string).value_or(InitialOctavesValue));
    }

    SVGFilterPrimitiveStandardAttributes::AttributeChanged(name, oldValue, value);
  }

  bool SVGFETurbulenceElement::SetFilterEffectAttribute(FilterEffect &filterEffect,
                                                        const dom::QualifiedName &attributeName) noexcept
  {
    auto &effect = Downcast<FETurbulence>(filterEffect);
    if (attributeName == SVGNames::TypeAttr())
    {
      return effect.SetType(GetType());
    }
    if (attributeName == SVGNames::StitchTilesAttr())
    {
      return effect.SetStitchTiles(StitchTiles() == SVG_STITCHTYPE_STITCH);
    }
    if (attributeName == SVGNames::BaseFrequencyAttr())
    {
      bool baseFrequencyXChanged = effect.SetBaseFrequencyX(BaseFrequencyX());
      bool baseFrequencyYChanged = effect.SetBaseFrequencyY(BaseFrequencyY());
      return baseFrequencyXChanged || baseFrequencyYChanged;
    }
    if (attributeName == SVGNames::SeedAttr())
    {
      return effect.SetSeed(Seed());
    }
    if (attributeName == SVGNames::NumOctavesAttr())
    {
      return effect.SetNumOctaves(NumOctaves());
    }

    krys_debug_assert_msg(false, "Not reached");
    return false;
  }

  void SVGFETurbulenceElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      InstanceInvalidationGuard guard(*this);
      PrimitiveAttributeChanged(attributeName);
      return;
    }

    SVGFilterPrimitiveStandardAttributes::SVGAttributeChanged(attributeName);
  }

  RefPtr<FilterEffect> SVGFETurbulenceElement::CreateFilterEffect(const FilterEffectVector &,
                                                                  const GraphicsContext &) const noexcept
  {
    if (BaseFrequencyX() < 0 || BaseFrequencyY() < 0)
    {
      return nullptr;
    }

    return FETurbulence::Create(GetType(), BaseFrequencyX(), BaseFrequencyY(), NumOctaves(), Seed(),
                                StitchTiles() == SVG_STITCHTYPE_STITCH);
  }
}
