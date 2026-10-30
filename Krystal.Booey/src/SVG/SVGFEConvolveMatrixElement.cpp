#include "Krystal.Booey/SVG/SVGFEConvolveMatrixElement.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/SVG/Internals/SVGParserUtilities.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Core/Text/StringToIntegerConversion.hpp"
#include <cmath>

namespace krys::boo::svg
{
  SVGFEConvolveMatrixElement::SVGFEConvolveMatrixElement(dom::Document &document) noexcept
      : SVGFilterPrimitiveStandardAttributes(document, CreateUnique<PropertyRegistry>(*this),
                                             SVGElementInterface::FEConvolveMatrix),
        _in1(SVGAnimatedString::Create(this)), _orderX(SVGAnimatedInteger::Create(this, InitialOrderValue)),
        _orderY(SVGAnimatedInteger::Create(this, InitialOrderValue)),
        _kernelMatrix(SVGAnimatedNumberList::Create(this)),
        _divisor(SVGAnimatedNumber::Create(this, InitialDivisorValue)),
        _bias(SVGAnimatedNumber::Create(this)), _targetX(SVGAnimatedInteger::Create(this)),
        _targetY(SVGAnimatedInteger::Create(this)),
        _edgeMode(SVGAnimatedEnumeration::Create(this, EdgeModeType::Duplicate)),
        _kernelUnitLengthX(SVGAnimatedNumber::Create(this)),
        _kernelUnitLengthY(SVGAnimatedNumber::Create(this)), _preserveAlpha(SVGAnimatedBoolean::Create(this))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGFEConvolveMatrixElement::_in1>(SVGNames::InAttr());
      PropertyRegistry::RegisterProperty<&SVGFEConvolveMatrixElement::_orderX,
                                         &SVGFEConvolveMatrixElement::_orderY>(SVGNames::OrderAttr());
      PropertyRegistry::RegisterProperty<&SVGFEConvolveMatrixElement::_kernelMatrix>(
        SVGNames::KernelMatrixAttr());
      PropertyRegistry::RegisterProperty<&SVGFEConvolveMatrixElement::_divisor>(SVGNames::DivisorAttr());
      PropertyRegistry::RegisterProperty<&SVGFEConvolveMatrixElement::_bias>(SVGNames::BiasAttr());
      PropertyRegistry::RegisterProperty<&SVGFEConvolveMatrixElement::_targetX>(SVGNames::TargetXAttr());
      PropertyRegistry::RegisterProperty<&SVGFEConvolveMatrixElement::_targetY>(SVGNames::TargetYAttr());
      PropertyRegistry::RegisterProperty<EdgeModeType, &SVGFEConvolveMatrixElement::_edgeMode>(
        SVGNames::EdgeModeAttr());
      PropertyRegistry::RegisterProperty<&SVGFEConvolveMatrixElement::_kernelUnitLengthX,
                                         &SVGFEConvolveMatrixElement::_kernelUnitLengthY>(
        SVGNames::KernelUnitLengthAttr());
      PropertyRegistry::RegisterProperty<&SVGFEConvolveMatrixElement::_preserveAlpha>(
        SVGNames::PreserveAlphaAttr());
    }
  }

  void SVGFEConvolveMatrixElement::AttributeChanged(const dom::QualifiedName &name,
                                                    Maybe<dom::DOMStringView> oldValue,
                                                    Maybe<dom::DOMStringView> value) noexcept
  {
    auto string = value.value_or(dom::DOMStringView {});

    // FIXME: Report the values that fail to parse ("Filtered element will not be displayed.") as warnings,
    // once there's a console.
    if (name == SVGNames::InAttr())
    {
      _in1->SetBaseValInternal(dom::DOMString(string));
    }
    else if (name == SVGNames::OrderAttr())
    {
      if (auto result = ParseNumberOptionalNumber(string))
      {
        _orderX->SetBaseValInternal(static_cast<int32>(result->first));
        _orderY->SetBaseValInternal(static_cast<int32>(result->second));
      }
      else
      {
        _orderX->SetBaseValInternal(InitialOrderValue);
        _orderY->SetBaseValInternal(InitialOrderValue);
      }
    }
    else if (name == SVGNames::EdgeModeAttr())
    {
      EdgeModeType propertyValue = SVGPropertyTraits<EdgeModeType>::FromString(*this, string);
      if (propertyValue != EdgeModeType::Unknown)
      {
        _edgeMode->SetBaseValInternal<EdgeModeType>(propertyValue);
      }
    }
    else if (name == SVGNames::KernelMatrixAttr())
    {
      _kernelMatrix->BaseVal()->Parse(string);
    }
    else if (name == SVGNames::DivisorAttr())
    {
      if (auto result = ParseNumberFromString(string))
      {
        _divisor->SetBaseValInternal(*result);
      }
      else
      {
        _divisor->SetBaseValInternal(InitialDivisorValue);
      }
    }
    else if (name == SVGNames::BiasAttr())
    {
      _bias->SetBaseValInternal(ToFloatOrZero(string));
    }
    else if (name == SVGNames::TargetXAttr())
    {
      _targetX->SetBaseValInternal(static_cast<int32>(text::ParseInteger<uint32>(string).value_or(0)));
    }
    else if (name == SVGNames::TargetYAttr())
    {
      _targetY->SetBaseValInternal(static_cast<int32>(text::ParseInteger<uint32>(string).value_or(0)));
    }
    else if (name == SVGNames::KernelUnitLengthAttr())
    {
      if (auto result = ParseNumberOptionalNumber(string))
      {
        _kernelUnitLengthX->SetBaseValInternal(result->first);
        _kernelUnitLengthY->SetBaseValInternal(result->second);
      }
      else
      {
        _kernelUnitLengthX->SetBaseValInternal(InitialKernelUnitLengthValue);
        _kernelUnitLengthY->SetBaseValInternal(InitialKernelUnitLengthValue);
      }
    }
    else if (name == SVGNames::PreserveAlphaAttr())
    {
      if (string == u8"true")
      {
        _preserveAlpha->SetBaseValInternal(true);
      }
      else if (string == u8"false")
      {
        _preserveAlpha->SetBaseValInternal(false);
      }
    }

    SVGFilterPrimitiveStandardAttributes::AttributeChanged(name, oldValue, value);
  }

  bool SVGFEConvolveMatrixElement::SetFilterEffectAttribute(FilterEffect &filterEffect,
                                                            const dom::QualifiedName &attributeName) noexcept
  {
    auto &effect = Downcast<FEConvolveMatrix>(filterEffect);
    if (attributeName == SVGNames::EdgeModeAttr())
    {
      return effect.SetEdgeMode(EdgeMode());
    }
    if (attributeName == SVGNames::DivisorAttr())
    {
      return effect.SetDivisor(Divisor());
    }
    if (attributeName == SVGNames::BiasAttr())
    {
      return effect.SetBias(Bias());
    }
    if (attributeName == SVGNames::TargetXAttr() || attributeName == SVGNames::TargetYAttr())
    {
      return effect.SetTargetOffset(IntPoint(TargetX(), TargetY()));
    }
    if (attributeName == SVGNames::KernelUnitLengthAttr())
    {
      return effect.SetKernelUnitLength(FloatPoint(KernelUnitLengthX(), KernelUnitLengthY()));
    }
    if (attributeName == SVGNames::PreserveAlphaAttr())
    {
      return effect.SetPreserveAlpha(PreserveAlpha());
    }

    krys_debug_assert_msg(false, "Not reached");
    return false;
  }

  void SVGFEConvolveMatrixElement::SetOrder(float x, float y) noexcept
  {
    _orderX->SetBaseValInternal(static_cast<int32>(x));
    _orderY->SetBaseValInternal(static_cast<int32>(y));
    UpdateSVGRendererForElementChange();
  }

  void SVGFEConvolveMatrixElement::SetKernelUnitLength(float x, float y) noexcept
  {
    _kernelUnitLengthX->SetBaseValInternal(x);
    _kernelUnitLengthY->SetBaseValInternal(y);
    UpdateSVGRendererForElementChange();
  }

  bool SVGFEConvolveMatrixElement::IsValidTargetXOffset() const noexcept
  {
    auto orderXValue = OrderX();
    auto targetXValue = HasAttribute(SVGNames::TargetXAttr().LocalName())
                          ? TargetX()
                          : static_cast<int32>(std::floor(static_cast<float>(orderXValue) / 2));
    return targetXValue >= 0 && targetXValue < orderXValue;
  }

  bool SVGFEConvolveMatrixElement::IsValidTargetYOffset() const noexcept
  {
    auto orderYValue = OrderY();
    auto targetYValue = HasAttribute(SVGNames::TargetYAttr().LocalName())
                          ? TargetY()
                          : static_cast<int32>(std::floor(static_cast<float>(orderYValue) / 2));
    return targetYValue >= 0 && targetYValue < orderYValue;
  }

  void SVGFEConvolveMatrixElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if ((attributeName == SVGNames::TargetXAttr() || attributeName == SVGNames::OrderAttr())
        && !IsValidTargetXOffset())
    {
      InstanceInvalidationGuard guard(*this);
      MarkFilterEffectForRebuild();
      return;
    }

    if ((attributeName == SVGNames::TargetYAttr() || attributeName == SVGNames::OrderAttr())
        && !IsValidTargetYOffset())
    {
      InstanceInvalidationGuard guard(*this);
      MarkFilterEffectForRebuild();
      return;
    }

    if (attributeName == SVGNames::InAttr() || attributeName == SVGNames::OrderAttr()
        || attributeName == SVGNames::KernelMatrixAttr())
    {
      InstanceInvalidationGuard guard(*this);
      UpdateSVGRendererForElementChange();
      return;
    }

    if (attributeName == SVGNames::EdgeModeAttr() || attributeName == SVGNames::DivisorAttr()
        || attributeName == SVGNames::BiasAttr() || attributeName == SVGNames::TargetXAttr()
        || attributeName == SVGNames::TargetYAttr() || attributeName == SVGNames::KernelUnitLengthAttr()
        || attributeName == SVGNames::PreserveAlphaAttr())
    {
      InstanceInvalidationGuard guard(*this);
      PrimitiveAttributeChanged(attributeName);
      return;
    }

    SVGFilterPrimitiveStandardAttributes::SVGAttributeChanged(attributeName);
  }

  RefPtr<FilterEffect> SVGFEConvolveMatrixElement::CreateFilterEffect(const FilterEffectVector &,
                                                                      const GraphicsContext &) const noexcept
  {
    auto filterOrder = [&]()
    {
      return IntSize(OrderX(), OrderY());
    };

    auto filterDivisor = [&](const SVGNumberList &kernelMatrix)
    {
      if (HasAttribute(SVGNames::DivisorAttr().LocalName()))
      {
        return Divisor();
      }

      float filterDivisor = 0;

      // The spec says the default value is the sum of all values in kernelMatrix.
      for (const auto &item : kernelMatrix.Items())
      {
        filterDivisor += item->Value();
      }

      // The spec says if the sum is zero, then the divisor is set to the initial value.
      return filterDivisor ? filterDivisor : InitialDivisorValue;
    };

    auto filterTarget = [&](const IntSize &order)
    {
      // The spec says the default value is: targetX = floor ( orderX / 2 ))
      return IntPoint(HasAttribute(SVGNames::TargetXAttr().LocalName()) ? TargetX() : order.Width() / 2,
                      HasAttribute(SVGNames::TargetYAttr().LocalName()) ? TargetY() : order.Height() / 2);
    };

    auto filterKernelUnitLength = [&]()
    {
      // Spec says default kernelUnitLength is 1.0.
      if (!HasAttribute(SVGNames::KernelUnitLengthAttr().LocalName()))
      {
        return FloatSize(1, 1);
      }
      return FloatSize(KernelUnitLengthX(), KernelUnitLengthY());
    };

    // Spec says order must be > 0. Bail if it is not.
    auto order = filterOrder();
    if (order.IsEmpty())
    {
      return nullptr;
    }

    const auto &kernelMatrix = KernelMatrix();

    // The spec says this is a requirement, and should bail out if fails
    if (order.UnclampedArea() != kernelMatrix.Length())
    {
      return nullptr;
    }

    // Spec says the specified divisor cannot be 0.
    auto divisor = filterDivisor(kernelMatrix);
    if (!divisor)
    {
      return nullptr;
    }

    auto target = filterTarget(order);
    if (!IntRect({}, order).Contains(target))
    {
      return nullptr;
    }

    // Spec says the specified kernelUnitLength cannot be negative or zero.
    auto kernelUnitLength = filterKernelUnitLength();
    if (kernelUnitLength.IsEmpty())
    {
      return nullptr;
    }

    return FEConvolveMatrix::Create(order, divisor, Bias(), target, EdgeMode(), FloatPoint(kernelUnitLength),
                                    PreserveAlpha(), kernelMatrix);
  }
}
