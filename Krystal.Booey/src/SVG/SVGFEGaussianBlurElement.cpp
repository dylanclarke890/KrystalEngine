#include "Krystal.Booey/SVG/SVGFEGaussianBlurElement.hpp"
#include "Krystal.Booey/Graphics/Filters/FEGaussianBlur.hpp"
#include "Krystal.Booey/SVG/Graphics/Filters/SVGFilterRenderer.hpp"
#include "Krystal.Booey/SVG/Internals/SVGParserUtilities.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  SVGFEGaussianBlurElement::SVGFEGaussianBlurElement(dom::Document &document) noexcept
      : SVGFilterPrimitiveStandardAttributes(document, CreateUnique<PropertyRegistry>(*this),
                                             SVGElementInterface::FEGaussianBlur),
        _in1(SVGAnimatedString::Create(this)), _stdDeviationX(SVGAnimatedNumber::Create(this)),
        _stdDeviationY(SVGAnimatedNumber::Create(this)),
        _edgeMode(SVGAnimatedEnumeration::Create(this, EdgeModeType::None))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGFEGaussianBlurElement::_in1>(SVGNames::InAttr());
      PropertyRegistry::RegisterProperty<&SVGFEGaussianBlurElement::_stdDeviationX,
                                         &SVGFEGaussianBlurElement::_stdDeviationY>(
        SVGNames::StdDeviationAttr());
      PropertyRegistry::RegisterProperty<EdgeModeType, &SVGFEGaussianBlurElement::_edgeMode>(
        SVGNames::EdgeModeAttr());
    }
  }

  void SVGFEGaussianBlurElement::SetStdDeviation(float x, float y) noexcept
  {
    _stdDeviationX->SetBaseValInternal(x);
    _stdDeviationY->SetBaseValInternal(y);
    UpdateSVGRendererForElementChange();
  }

  void SVGFEGaussianBlurElement::AttributeChanged(const dom::QualifiedName &name,
                                                  Maybe<dom::DOMStringView> oldValue,
                                                  Maybe<dom::DOMStringView> value) noexcept
  {
    auto string = value.value_or(dom::DOMStringView {});

    if (name == SVGNames::StdDeviationAttr())
    {
      if (auto result = ParseNumberOptionalNumber(string))
      {
        _stdDeviationX->SetBaseValInternal(result->first);
        _stdDeviationY->SetBaseValInternal(result->second);
      }
    }
    else if (name == SVGNames::InAttr())
    {
      _in1->SetBaseValInternal(dom::DOMString(string));
    }
    else if (name == SVGNames::EdgeModeAttr())
    {
      auto propertyValue = SVGPropertyTraits<EdgeModeType>::FromString(*this, string);
      if (propertyValue != EdgeModeType::Unknown)
      {
        _edgeMode->SetBaseValInternal<EdgeModeType>(propertyValue);
      }
      // FIXME: Otherwise, report "feGaussianBlur: problem parsing edgeMode" as a warning, once there's a
      // console.
    }

    SVGFilterPrimitiveStandardAttributes::AttributeChanged(name, oldValue, value);
  }

  void SVGFEGaussianBlurElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (attributeName == SVGNames::InAttr())
    {
      InstanceInvalidationGuard guard(*this);
      UpdateSVGRendererForElementChange();
      return;
    }

    if (attributeName == SVGNames::StdDeviationAttr() && (StdDeviationX() < 0 || StdDeviationY() < 0))
    {
      InstanceInvalidationGuard guard(*this);
      MarkFilterEffectForRebuild();
      return;
    }

    if (attributeName == SVGNames::StdDeviationAttr() || attributeName == SVGNames::EdgeModeAttr())
    {
      InstanceInvalidationGuard guard(*this);
      PrimitiveAttributeChanged(attributeName);
      return;
    }

    SVGFilterPrimitiveStandardAttributes::SVGAttributeChanged(attributeName);
  }

  bool SVGFEGaussianBlurElement::SetFilterEffectAttribute(FilterEffect &effect,
                                                          const dom::QualifiedName &attributeName) noexcept
  {
    auto &feGaussianBlur = Downcast<FEGaussianBlur>(effect);

    if (attributeName == SVGNames::StdDeviationAttr())
    {
      bool stdDeviationXChanged = feGaussianBlur.SetStdDeviationX(StdDeviationX());
      bool stdDeviationYChanged = feGaussianBlur.SetStdDeviationY(StdDeviationY());
      return stdDeviationXChanged || stdDeviationYChanged;
    }

    if (attributeName == SVGNames::EdgeModeAttr())
    {
      return feGaussianBlur.SetEdgeMode(EdgeMode());
    }

    krys_debug_assert_msg(false, "Not reached");
    return false;
  }

  bool SVGFEGaussianBlurElement::IsIdentity() const noexcept
  {
    return !StdDeviationX() && !StdDeviationY();
  }

  IntOutsets SVGFEGaussianBlurElement::Outsets(const FloatRect &targetBoundingBox,
                                               SVGUnitTypes::SVGUnitType primitiveUnits) const noexcept
  {
    auto stdDeviation = SVGFilterRenderer::CalculateResolvedSize({StdDeviationX(), StdDeviationY()},
                                                                 targetBoundingBox, primitiveUnits);
    return FEGaussianBlur::CalculateOutsets(stdDeviation);
  }

  RefPtr<FilterEffect> SVGFEGaussianBlurElement::CreateFilterEffect(const FilterEffectVector &,
                                                                    const GraphicsContext &) const noexcept
  {
    if (StdDeviationX() < 0 || StdDeviationY() < 0)
    {
      return nullptr;
    }

    return FEGaussianBlur::Create(StdDeviationX(), StdDeviationY(), EdgeMode());
  }
}
