#include "Krystal.Booey/SVG/SVGFEOffsetElement.hpp"
#include "Krystal.Booey/Graphics/Filters/FEOffset.hpp"
#include "Krystal.Booey/SVG/Graphics/Filters/SVGFilterRenderer.hpp"
#include "Krystal.Booey/SVG/Internals/SVGParserUtilities.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  SVGFEOffsetElement::SVGFEOffsetElement(dom::Document &document) noexcept
      : SVGFilterPrimitiveStandardAttributes(document, CreateUnique<PropertyRegistry>(*this),
                                             SVGElementInterface::FEOffset),
        _in1(SVGAnimatedString::Create(this)), _dx(SVGAnimatedNumber::Create(this)),
        _dy(SVGAnimatedNumber::Create(this))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGFEOffsetElement::_in1>(SVGNames::InAttr());
      PropertyRegistry::RegisterProperty<&SVGFEOffsetElement::_dx>(SVGNames::DxAttr());
      PropertyRegistry::RegisterProperty<&SVGFEOffsetElement::_dy>(SVGNames::DyAttr());
    }
  }

  void SVGFEOffsetElement::AttributeChanged(const dom::QualifiedName &name,
                                            Maybe<dom::DOMStringView> oldValue,
                                            Maybe<dom::DOMStringView> value) noexcept
  {
    auto string = value.value_or(dom::DOMStringView {});

    if (name == SVGNames::DxAttr())
    {
      _dx->SetBaseValInternal(ToFloatOrZero(string));
    }
    else if (name == SVGNames::DyAttr())
    {
      _dy->SetBaseValInternal(ToFloatOrZero(string));
    }
    else if (name == SVGNames::InAttr())
    {
      _in1->SetBaseValInternal(dom::DOMString(string));
    }

    SVGFilterPrimitiveStandardAttributes::AttributeChanged(name, oldValue, value);
  }

  void SVGFEOffsetElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (attributeName == SVGNames::InAttr())
    {
      InstanceInvalidationGuard guard(*this);
      UpdateSVGRendererForElementChange();
      return;
    }
    if (attributeName == SVGNames::DxAttr() || attributeName == SVGNames::DyAttr())
    {
      InstanceInvalidationGuard guard(*this);
      PrimitiveAttributeChanged(attributeName);
      return;
    }

    SVGFilterPrimitiveStandardAttributes::SVGAttributeChanged(attributeName);
  }

  bool SVGFEOffsetElement::SetFilterEffectAttribute(FilterEffect &effect,
                                                    const dom::QualifiedName &attributeName) noexcept
  {
    auto &offset = Downcast<FEOffset>(effect);

    if (attributeName == SVGNames::DxAttr())
    {
      return offset.SetDx(Dx());
    }
    if (attributeName == SVGNames::DyAttr())
    {
      return offset.SetDy(Dy());
    }

    krys_debug_assert_msg(false, "Not reached");
    return false;
  }

  bool SVGFEOffsetElement::IsIdentity() const noexcept
  {
    return !Dx() && !Dy();
  }

  IntOutsets SVGFEOffsetElement::Outsets(const FloatRect &targetBoundingBox,
                                         SVGUnitTypes::SVGUnitType primitiveUnits) const noexcept
  {
    auto offset = SVGFilterRenderer::CalculateResolvedSize({Dx(), Dy()}, targetBoundingBox, primitiveUnits);
    return FEOffset::CalculateOutsets(offset);
  }

  RefPtr<FilterEffect> SVGFEOffsetElement::CreateFilterEffect(const FilterEffectVector &,
                                                              const GraphicsContext &) const noexcept
  {
    return FEOffset::Create(Dx(), Dy());
  }
}
