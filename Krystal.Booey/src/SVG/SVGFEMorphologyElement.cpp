#include "Krystal.Booey/SVG/SVGFEMorphologyElement.hpp"
#include "Krystal.Booey/SVG/Graphics/Filters/SVGFilterRenderer.hpp"
#include "Krystal.Booey/SVG/Internals/SVGParserUtilities.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include <algorithm>

namespace krys::boo::svg
{
  SVGFEMorphologyElement::SVGFEMorphologyElement(dom::Document &document) noexcept
      : SVGFilterPrimitiveStandardAttributes(document, CreateUnique<PropertyRegistry>(*this),
                                             SVGElementInterface::FEMorphology),
        _in1(SVGAnimatedString::Create(this)),
        _svgOperator(SVGAnimatedEnumeration::Create(this, MorphologyOperatorType::Erode)),
        _radiusX(SVGAnimatedNumber::Create(this)), _radiusY(SVGAnimatedNumber::Create(this))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGFEMorphologyElement::_in1>(SVGNames::InAttr());
      PropertyRegistry::RegisterProperty<MorphologyOperatorType, &SVGFEMorphologyElement::_svgOperator>(
        SVGNames::OperatorAttr());
      PropertyRegistry::RegisterProperty<&SVGFEMorphologyElement::_radiusX,
                                         &SVGFEMorphologyElement::_radiusY>(SVGNames::RadiusAttr());
    }
  }

  void SVGFEMorphologyElement::AttributeChanged(const dom::QualifiedName &name,
                                                Maybe<dom::DOMStringView> oldValue,
                                                Maybe<dom::DOMStringView> value) noexcept
  {
    auto string = value.value_or(dom::DOMStringView {});

    if (name == SVGNames::OperatorAttr())
    {
      auto propertyValue = SVGPropertyTraits<MorphologyOperatorType>::FromString(*this, string);
      if (propertyValue != MorphologyOperatorType::Unknown)
      {
        _svgOperator->SetBaseValInternal<MorphologyOperatorType>(propertyValue);
      }
    }
    else if (name == SVGNames::InAttr())
    {
      _in1->SetBaseValInternal(dom::DOMString(string));
    }
    else if (name == SVGNames::RadiusAttr())
    {
      if (auto result = ParseNumberOptionalNumber(string))
      {
        _radiusX->SetBaseValInternal(result->first);
        _radiusY->SetBaseValInternal(result->second);
      }
    }

    SVGFilterPrimitiveStandardAttributes::AttributeChanged(name, oldValue, value);
  }

  bool SVGFEMorphologyElement::SetFilterEffectAttribute(FilterEffect &effect,
                                                        const dom::QualifiedName &attributeName) noexcept
  {
    auto &feMorphology = Downcast<FEMorphology>(effect);
    if (attributeName == SVGNames::OperatorAttr())
    {
      return feMorphology.SetMorphologyOperator(SVGOperator());
    }
    if (attributeName == SVGNames::RadiusAttr())
    {
      // Both setRadius functions should be evaluated separately.
      bool isRadiusXChanged = feMorphology.SetRadiusX(RadiusX());
      bool isRadiusYChanged = feMorphology.SetRadiusY(RadiusY());
      return isRadiusXChanged || isRadiusYChanged;
    }

    krys_debug_assert_msg(false, "Not reached");
    return false;
  }

  void SVGFEMorphologyElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (attributeName == SVGNames::InAttr())
    {
      InstanceInvalidationGuard guard(*this);
      UpdateSVGRendererForElementChange();
      return;
    }

    if (attributeName == SVGNames::OperatorAttr() || attributeName == SVGNames::RadiusAttr())
    {
      InstanceInvalidationGuard guard(*this);
      PrimitiveAttributeChanged(attributeName);
      return;
    }

    SVGFilterPrimitiveStandardAttributes::SVGAttributeChanged(attributeName);
  }

  bool SVGFEMorphologyElement::IsIdentity() const noexcept
  {
    return !std::max(0.0f, RadiusX()) && !std::max(0.0f, RadiusY());
  }

  IntOutsets SVGFEMorphologyElement::Outsets(const FloatRect &targetBoundingBox,
                                             SVGUnitTypes::SVGUnitType primitiveUnits) const noexcept
  {
    auto radius =
      SVGFilterRenderer::CalculateResolvedSize({RadiusX(), RadiusY()}, targetBoundingBox, primitiveUnits);
    return {static_cast<int>(radius.Height()), static_cast<int>(radius.Width()),
            static_cast<int>(radius.Height()), static_cast<int>(radius.Width())};
  }

  RefPtr<FilterEffect> SVGFEMorphologyElement::CreateFilterEffect(const FilterEffectVector &,
                                                                  const GraphicsContext &) const noexcept
  {
    // "A negative or zero value disables the effect of the given filter
    // primitive (i.e., the result is the filter input image)."
    // https://drafts.fxtf.org/filters/#element-attrdef-femorphology-radius
    // This is handled by FEMorphology.
    return FEMorphology::Create(SVGOperator(), RadiusX(), RadiusY());
  }
}
