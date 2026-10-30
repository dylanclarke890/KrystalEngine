#include "Krystal.Booey/SVG/SVGFEBlendElement.hpp"
#include "Krystal.Booey/Graphics/Filters/FEBlend.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  SVGFEBlendElement::SVGFEBlendElement(dom::Document &document) noexcept
      : SVGFilterPrimitiveStandardAttributes(document, CreateUnique<PropertyRegistry>(*this),
                                             SVGElementInterface::FEBlend),
        _in1(SVGAnimatedString::Create(this)), _in2(SVGAnimatedString::Create(this)),
        _mode(SVGAnimatedEnumeration::Create(this, BlendMode::Normal))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<BlendMode, &SVGFEBlendElement::_mode>(SVGNames::ModeAttr());
      PropertyRegistry::RegisterProperty<&SVGFEBlendElement::_in1>(SVGNames::InAttr());
      PropertyRegistry::RegisterProperty<&SVGFEBlendElement::_in2>(SVGNames::In2Attr());
    }
  }

  void SVGFEBlendElement::AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                           Maybe<dom::DOMStringView> value) noexcept
  {
    auto string = value.value_or(dom::DOMStringView {});

    if (name == SVGNames::ModeAttr())
    {
      BlendMode mode = BlendMode::Normal;
      if (ParseBlendMode(string, mode))
      {
        _mode->SetBaseValInternal<BlendMode>(mode);
      }
    }
    else if (name == SVGNames::InAttr())
    {
      _in1->SetBaseValInternal(dom::DOMString(string));
    }
    else if (name == SVGNames::In2Attr())
    {
      _in2->SetBaseValInternal(dom::DOMString(string));
    }

    SVGFilterPrimitiveStandardAttributes::AttributeChanged(name, oldValue, value);
  }

  bool SVGFEBlendElement::SetFilterEffectAttribute(FilterEffect &effect,
                                                   const dom::QualifiedName &attributeName) noexcept
  {
    auto &feBlend = Downcast<FEBlend>(effect);
    if (attributeName == SVGNames::ModeAttr())
    {
      return feBlend.SetBlendMode(Mode());
    }

    krys_debug_assert_msg(false, "Not reached");
    return false;
  }

  void SVGFEBlendElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      InstanceInvalidationGuard guard(*this);
      if (attributeName == SVGNames::ModeAttr())
      {
        PrimitiveAttributeChanged(attributeName);
      }
      else
      {
        krys_debug_assert(attributeName == SVGNames::InAttr() || attributeName == SVGNames::In2Attr());
        UpdateSVGRendererForElementChange();
      }
      return;
    }

    SVGFilterPrimitiveStandardAttributes::SVGAttributeChanged(attributeName);
  }

  RefPtr<FilterEffect> SVGFEBlendElement::CreateFilterEffect(const FilterEffectVector &,
                                                             const GraphicsContext &) const noexcept
  {
    return FEBlend::Create(Mode());
  }
}
