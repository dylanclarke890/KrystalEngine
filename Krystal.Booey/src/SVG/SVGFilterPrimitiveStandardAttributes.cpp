#include "Krystal.Booey/SVG/SVGFilterPrimitiveStandardAttributes.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterEffect.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceFilterPrimitive.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGFilterElement.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Booey/SVG/SVGParsingError.hpp"

namespace krys::boo::svg
{
  SVGFilterPrimitiveStandardAttributes::SVGFilterPrimitiveStandardAttributes(
    dom::Document &document, UniquePtr<SVGPropertyRegistry> &&propertyRegistry,
    SVGElementInterface interface) noexcept
      : SVGElement(document, krys::move(propertyRegistry), interface),
        _x(SVGAnimatedLength::Create(this, SVGLengthMode::Width, u8"0%")),
        _y(SVGAnimatedLength::Create(this, SVGLengthMode::Height, u8"0%")),
        _width(SVGAnimatedLength::Create(this, SVGLengthMode::Width, u8"100%")),
        _height(SVGAnimatedLength::Create(this, SVGLengthMode::Height, u8"100%")),
        _result(SVGAnimatedString::Create(this))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGFilterPrimitiveStandardAttributes::_x>(SVGNames::XAttr());
      PropertyRegistry::RegisterProperty<&SVGFilterPrimitiveStandardAttributes::_y>(SVGNames::YAttr());
      PropertyRegistry::RegisterProperty<&SVGFilterPrimitiveStandardAttributes::_width>(
        SVGNames::WidthAttr());
      PropertyRegistry::RegisterProperty<&SVGFilterPrimitiveStandardAttributes::_height>(
        SVGNames::HeightAttr());
      PropertyRegistry::RegisterProperty<&SVGFilterPrimitiveStandardAttributes::_result>(
        SVGNames::ResultAttr());
    }
  }

  void SVGFilterPrimitiveStandardAttributes::AttributeChanged(const dom::QualifiedName &name,
                                                              Maybe<dom::DOMStringView> oldValue,
                                                              Maybe<dom::DOMStringView> value) noexcept
  {
    auto parseError = SVGParsingError::None;
    auto string = value.value_or(dom::DOMStringView {});

    if (name == SVGNames::XAttr())
    {
      _x->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Width, string, parseError,
                                                       SVGLengthNegativeValuesMode::Allow, u8"0%"));
    }
    else if (name == SVGNames::YAttr())
    {
      _y->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Height, string, parseError,
                                                       SVGLengthNegativeValuesMode::Allow, u8"0%"));
    }
    else if (name == SVGNames::WidthAttr())
    {
      _width->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Width, string, parseError,
                                                           SVGLengthNegativeValuesMode::Allow, u8"100%"));
    }
    else if (name == SVGNames::HeightAttr())
    {
      _height->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Height, string, parseError,
                                                            SVGLengthNegativeValuesMode::Allow, u8"100%"));
    }
    else if (name == SVGNames::ResultAttr())
    {
      _result->SetBaseValInternal(dom::DOMString(string));
    }
    ReportAttributeParsingError(parseError, name, string);

    SVGElement::AttributeChanged(name, oldValue, value);
  }

  EnumSet<FilterEffectGeometry::Flags>
    SVGFilterPrimitiveStandardAttributes::EffectGeometryFlags() const noexcept
  {
    EnumSet<FilterEffectGeometry::Flags> flags;

    if (HasAttribute(SVGNames::XAttr().LocalName()))
    {
      flags.Add(FilterEffectGeometry::Flags::HasX);
    }
    if (HasAttribute(SVGNames::YAttr().LocalName()))
    {
      flags.Add(FilterEffectGeometry::Flags::HasY);
    }
    if (HasAttribute(SVGNames::WidthAttr().LocalName()))
    {
      flags.Add(FilterEffectGeometry::Flags::HasWidth);
    }
    if (HasAttribute(SVGNames::HeightAttr().LocalName()))
    {
      flags.Add(FilterEffectGeometry::Flags::HasHeight);
    }

    return flags;
  }

  RefPtr<FilterEffect>
    SVGFilterPrimitiveStandardAttributes::GetFilterEffect(const FilterEffectVector &inputs,
                                                          const GraphicsContext &destinationContext) noexcept
  {
    if (!_effect)
    {
      _effect = CreateFilterEffect(inputs, destinationContext);
    }
    return _effect;
  }

  void SVGFilterPrimitiveStandardAttributes::PrimitiveAttributeChanged(
    const dom::QualifiedName &attribute) noexcept
  {
    RefPtr effect = _effect;
    if (effect && !SetFilterEffectAttribute(*effect, attribute))
    {
      return;
    }
    MarkFilterEffectForRepaint();
  }

  void SVGFilterPrimitiveStandardAttributes::PrimitiveAttributeOnChildChanged(
    const dom::Element &child, const dom::QualifiedName &attribute) noexcept
  {
    krys_debug_assert(child.ParentNode() == this);

    RefPtr effect = _effect;
    if (effect && !SetFilterEffectAttributeFromChild(*effect, child, attribute))
    {
      return;
    }
    MarkFilterEffectForRepaint();
  }

  void SVGFilterPrimitiveStandardAttributes::MarkFilterEffectForRepaint() noexcept
  {
    if (auto *filterPrimitiveRenderer = DynamicDowncast<LegacyRenderSVGResourceFilterPrimitive>(Renderer()))
    {
      filterPrimitiveRenderer->MarkFilterEffectForRepaint(_effect.get());
    }
  }

  void SVGFilterPrimitiveStandardAttributes::MarkFilterEffectForRebuild() noexcept
  {
    _effect = nullptr;

    if (auto *filterPrimitiveRenderer = DynamicDowncast<LegacyRenderSVGResourceFilterPrimitive>(Renderer()))
    {
      filterPrimitiveRenderer->MarkFilterEffectForRebuild();
    }
  }

  void SVGFilterPrimitiveStandardAttributes::SVGAttributeChanged(
    const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      InstanceInvalidationGuard guard(*this);
      UpdateSVGRendererForElementChange();
      return;
    }

    SVGElement::SVGAttributeChanged(attributeName);
  }

  void SVGFilterPrimitiveStandardAttributes::OnChildrenChanged() noexcept
  {
    SVGElement::OnChildrenChanged();

    if (!IsFinishedParsingChildren())
    {
      return;
    }

    UpdateSVGRendererForElementChange();
  }

  RenderPtr<RenderElement>
    SVGFilterPrimitiveStandardAttributes::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<LegacyRenderSVGResourceFilterPrimitive>(*this, krys::move(style));
  }

  bool SVGFilterPrimitiveStandardAttributes::RendererIsNeeded(const RenderStyle &style) noexcept
  {
    auto *parent = DynamicDowncast<SVGElement>(ParentNode());
    if (parent && Is<SVGFilterElement>(*parent))
    {
      return SVGElement::RendererIsNeeded(style);
    }

    return false;
  }

  void SVGFilterPrimitiveStandardAttributes::InvalidateFilterPrimitiveParent(SVGElement *element) noexcept
  {
    if (!element)
    {
      return;
    }

    auto *parent = DynamicDowncast<SVGElement>(element->ParentNode());
    if (!parent)
    {
      return;
    }

    auto *renderer = parent->Renderer();
    if (!renderer || !renderer->IsLegacyRenderSVGResourceFilterPrimitive())
    {
      return;
    }

    parent->UpdateSVGRendererForElementChange();
  }
}
