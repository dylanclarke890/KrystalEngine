#include "Krystal.Booey/SVG/SVGFEColorMatrixElement.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  SVGFEColorMatrixElement::SVGFEColorMatrixElement(dom::Document &document) noexcept
      : SVGFilterPrimitiveStandardAttributes(document, CreateUnique<PropertyRegistry>(*this),
                                             SVGElementInterface::FEColorMatrix),
        _in1(SVGAnimatedString::Create(this)),
        _type(SVGAnimatedEnumeration::Create(this, ColorMatrixType::FECOLORMATRIX_TYPE_MATRIX)),
        _values(SVGAnimatedNumberList::Create(this))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGFEColorMatrixElement::_in1>(SVGNames::InAttr());
      PropertyRegistry::RegisterProperty<ColorMatrixType, &SVGFEColorMatrixElement::_type>(
        SVGNames::TypeAttr());
      PropertyRegistry::RegisterProperty<&SVGFEColorMatrixElement::_values>(SVGNames::ValuesAttr());
    }
  }

  bool SVGFEColorMatrixElement::IsInvalidValuesLength() const noexcept
  {
    auto filterType = GetType();
    auto size = _values->CurrentValue().Length();

    return (filterType == ColorMatrixType::FECOLORMATRIX_TYPE_MATRIX && size != 20)
           || (filterType == ColorMatrixType::FECOLORMATRIX_TYPE_HUEROTATE && size != 1)
           || (filterType == ColorMatrixType::FECOLORMATRIX_TYPE_SATURATE && size != 1);
  }

  void SVGFEColorMatrixElement::AttributeChanged(const dom::QualifiedName &name,
                                                 Maybe<dom::DOMStringView> oldValue,
                                                 Maybe<dom::DOMStringView> value) noexcept
  {
    auto string = value.value_or(dom::DOMStringView {});

    if (name == SVGNames::TypeAttr())
    {
      auto propertyValue = SVGPropertyTraits<ColorMatrixType>::FromString(*this, string);
      if (propertyValue != ColorMatrixType::FECOLORMATRIX_TYPE_UNKNOWN)
      {
        _type->SetBaseValInternal<ColorMatrixType>(propertyValue);
      }
    }
    else if (name == SVGNames::InAttr())
    {
      _in1->SetBaseValInternal(dom::DOMString(string));
    }
    else if (name == SVGNames::ValuesAttr())
    {
      _values->BaseVal()->Parse(string);
    }

    SVGFilterPrimitiveStandardAttributes::AttributeChanged(name, oldValue, value);
  }

  bool SVGFEColorMatrixElement::SetFilterEffectAttribute(FilterEffect &effect,
                                                         const dom::QualifiedName &attributeName) noexcept
  {
    auto &feColorMatrix = Downcast<FEColorMatrix>(effect);

    if (attributeName == SVGNames::TypeAttr())
    {
      return feColorMatrix.SetType(GetType());
    }
    if (attributeName == SVGNames::ValuesAttr())
    {
      return feColorMatrix.SetValues(Values());
    }

    krys_debug_assert_msg(false, "Not reached");
    return false;
  }

  void SVGFEColorMatrixElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (attributeName == SVGNames::InAttr())
    {
      InstanceInvalidationGuard guard(*this);
      UpdateSVGRendererForElementChange();
      return;
    }

    if (attributeName == SVGNames::TypeAttr() || attributeName == SVGNames::ValuesAttr())
    {
      InstanceInvalidationGuard guard(*this);
      if (IsInvalidValuesLength())
      {
        MarkFilterEffectForRebuild();
      }
      else
      {
        PrimitiveAttributeChanged(attributeName);
      }
      return;
    }

    SVGFilterPrimitiveStandardAttributes::SVGAttributeChanged(attributeName);
  }

  RefPtr<FilterEffect> SVGFEColorMatrixElement::CreateFilterEffect(const FilterEffectVector &,
                                                                   const GraphicsContext &) const noexcept
  {
    List<float> filterValues;
    ColorMatrixType filterType = GetType();

    // Use defaults if values is empty (SVG 1.1 15.10).
    if (!HasAttribute(SVGNames::ValuesAttr().LocalName()))
    {
      switch (filterType)
      {
        case ColorMatrixType::FECOLORMATRIX_TYPE_MATRIX:
        {
          constexpr uint32 matrixValueCount = 20;
          filterValues.reserve(matrixValueCount);
          for (uint32 i = 0; i < matrixValueCount; ++i)
          {
            filterValues.push_back((i % 6) ? 0.0f : 1.0f);
          }
          break;
        }
        case ColorMatrixType::FECOLORMATRIX_TYPE_HUEROTATE: filterValues = {0}; break;
        case ColorMatrixType::FECOLORMATRIX_TYPE_SATURATE:  filterValues = {1}; break;
        default:                                            break;
      }
    }
    else
    {
      if (IsInvalidValuesLength())
      {
        return nullptr;
      }

      filterValues = Values();
      filterValues.shrink_to_fit();
    }

    return FEColorMatrix::Create(filterType, krys::move(filterValues));
  }
}
