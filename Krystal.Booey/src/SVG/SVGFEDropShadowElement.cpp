#include "Krystal.Booey/SVG/SVGFEDropShadowElement.hpp"
#include "Krystal.Booey/Graphics/Filters/FEDropShadow.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/SVG/Graphics/Filters/SVGFilterRenderer.hpp"
#include "Krystal.Booey/SVG/Internals/SVGParserUtilities.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  SVGFEDropShadowElement::SVGFEDropShadowElement(dom::Document &document) noexcept
      : SVGFilterPrimitiveStandardAttributes(document, CreateUnique<PropertyRegistry>(*this),
                                             SVGElementInterface::FEDropShadow),
        _in1(SVGAnimatedString::Create(this)), _dx(SVGAnimatedNumber::Create(this, 2)),
        _dy(SVGAnimatedNumber::Create(this, 2)), _stdDeviationX(SVGAnimatedNumber::Create(this, 2)),
        _stdDeviationY(SVGAnimatedNumber::Create(this, 2))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGFEDropShadowElement::_in1>(SVGNames::InAttr());
      PropertyRegistry::RegisterProperty<&SVGFEDropShadowElement::_dx>(SVGNames::DxAttr());
      PropertyRegistry::RegisterProperty<&SVGFEDropShadowElement::_dy>(SVGNames::DyAttr());
      PropertyRegistry::RegisterProperty<&SVGFEDropShadowElement::_stdDeviationX,
                                         &SVGFEDropShadowElement::_stdDeviationY>(
        SVGNames::StdDeviationAttr());
    }
  }

  void SVGFEDropShadowElement::SetStdDeviation(float x, float y) noexcept
  {
    _stdDeviationX->SetBaseValInternal(x);
    _stdDeviationY->SetBaseValInternal(y);
    UpdateSVGRendererForElementChange();
  }

  void SVGFEDropShadowElement::AttributeChanged(const dom::QualifiedName &name,
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
    else if (name == SVGNames::DxAttr())
    {
      _dx->SetBaseValInternal(ToFloatOrZero(string));
    }
    else if (name == SVGNames::DyAttr())
    {
      _dy->SetBaseValInternal(ToFloatOrZero(string));
    }

    SVGFilterPrimitiveStandardAttributes::AttributeChanged(name, oldValue, value);
  }

  void SVGFEDropShadowElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
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

    if (attributeName == SVGNames::StdDeviationAttr() || attributeName == SVGNames::DxAttr()
        || attributeName == SVGNames::DyAttr())
    {
      InstanceInvalidationGuard guard(*this);
      PrimitiveAttributeChanged(attributeName);
      return;
    }

    SVGFilterPrimitiveStandardAttributes::SVGAttributeChanged(attributeName);
  }

  bool SVGFEDropShadowElement::SetFilterEffectAttribute(FilterEffect &filterEffect,
                                                        const dom::QualifiedName &attributeName) noexcept
  {
    auto &effect = Downcast<FEDropShadow>(filterEffect);
    if (attributeName == SVGNames::StdDeviationAttr())
    {
      return effect.SetStdDeviationX(StdDeviationX()) || effect.SetStdDeviationY(StdDeviationY());
    }
    if (attributeName == SVGNames::DxAttr())
    {
      return effect.SetDx(Dx());
    }
    if (attributeName == SVGNames::DyAttr())
    {
      return effect.SetDy(Dy());
    }
    if (attributeName == SVGNames::FloodColorAttr())
    {
      const auto &style = Renderer()->Style();
      return effect.SetShadowColor(style.ColorResolvingCurrentColor(style.FloodColor()));
    }
    if (attributeName == SVGNames::FloodOpacityAttr())
    {
      return effect.SetShadowOpacity(Renderer()->Style().FloodOpacity().Value.Value);
    }

    krys_debug_assert_msg(false, "Not reached");
    return false;
  }

  bool SVGFEDropShadowElement::IsIdentity() const noexcept
  {
    return !StdDeviationX() && !StdDeviationY() && !Dx() && !Dy();
  }

  IntOutsets SVGFEDropShadowElement::Outsets(const FloatRect &targetBoundingBox,
                                             SVGUnitTypes::SVGUnitType primitiveUnits) const noexcept
  {
    auto offset = SVGFilterRenderer::CalculateResolvedSize({Dx(), Dy()}, targetBoundingBox, primitiveUnits);
    auto stdDeviation = SVGFilterRenderer::CalculateResolvedSize({StdDeviationX(), StdDeviationY()},
                                                                 targetBoundingBox, primitiveUnits);
    return FEDropShadow::CalculateOutsets(offset, stdDeviation);
  }

  RefPtr<FilterEffect> SVGFEDropShadowElement::CreateFilterEffect(const FilterEffectVector &,
                                                                  const GraphicsContext &) const noexcept
  {
    auto *renderer = Renderer();
    if (!renderer)
    {
      return nullptr;
    }

    if (StdDeviationX() < 0 || StdDeviationY() < 0)
    {
      return nullptr;
    }

    const auto &style = renderer->Style();

    return FEDropShadow::Create(StdDeviationX(), StdDeviationY(), Dx(), Dy(),
                                style.ColorWithColorFilter(style.FloodColor()),
                                style.FloodOpacity().Value.Value);
  }
}
