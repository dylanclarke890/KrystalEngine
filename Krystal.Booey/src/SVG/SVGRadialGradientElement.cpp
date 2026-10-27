#include "Krystal.Booey/SVG/SVGRadialGradientElement.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceRadialGradient.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/RadialGradientAttributes.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Core/Types/HashSet.hpp"

namespace krys::boo::svg
{
  SVGRadialGradientElement::SVGRadialGradientElement(dom::Document &document) noexcept
      : SVGGradientElement(document, CreateUnique<PropertyRegistry>(*this),
                           SVGElementInterface::RadialGradient),
        _cx(SVGAnimatedLength::Create(this, SVGLengthMode::Width, u8"50%")),
        _cy(SVGAnimatedLength::Create(this, SVGLengthMode::Height, u8"50%")),
        _r(SVGAnimatedLength::Create(this, SVGLengthMode::Other, u8"50%")),
        _fx(SVGAnimatedLength::Create(this, SVGLengthMode::Width)),
        _fy(SVGAnimatedLength::Create(this, SVGLengthMode::Height)),
        _fr(SVGAnimatedLength::Create(this, SVGLengthMode::Other, u8"0%"))
  {
    // Spec: If the cx/cy/r/fr attribute is not specified, the effect is as if a value of "50%" were
    // specified.
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGRadialGradientElement::_cx>(SVGNames::CxAttr());
      PropertyRegistry::RegisterProperty<&SVGRadialGradientElement::_cy>(SVGNames::CyAttr());
      PropertyRegistry::RegisterProperty<&SVGRadialGradientElement::_r>(SVGNames::RAttr());
      PropertyRegistry::RegisterProperty<&SVGRadialGradientElement::_fx>(SVGNames::FxAttr());
      PropertyRegistry::RegisterProperty<&SVGRadialGradientElement::_fy>(SVGNames::FyAttr());
      PropertyRegistry::RegisterProperty<&SVGRadialGradientElement::_fr>(SVGNames::FrAttr());
    }
  }

  void SVGRadialGradientElement::AttributeChanged(const dom::QualifiedName &name,
                                                  Maybe<dom::DOMStringView> oldValue,
                                                  Maybe<dom::DOMStringView> value) noexcept
  {
    auto parseError = SVGParsingError::None;
    auto string = value.value_or(dom::DOMStringView {});

    if (name == SVGNames::CxAttr())
    {
      _cx->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Width, string, parseError,
                                                        SVGLengthNegativeValuesMode::Allow, u8"50%"));
    }
    else if (name == SVGNames::CyAttr())
    {
      _cy->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Height, string, parseError,
                                                        SVGLengthNegativeValuesMode::Allow, u8"50%"));
    }
    else if (name == SVGNames::RAttr())
    {
      _r->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Other, string, parseError,
                                                       SVGLengthNegativeValuesMode::Allow, u8"50%"));
    }
    else if (name == SVGNames::FxAttr())
    {
      _fx->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Width, string, parseError,
                                                        SVGLengthNegativeValuesMode::Allow, u8"50%"));
    }
    else if (name == SVGNames::FyAttr())
    {
      _fy->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Height, string, parseError,
                                                        SVGLengthNegativeValuesMode::Allow, u8"50%"));
    }
    else if (name == SVGNames::FrAttr())
    {
      _fr->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Other, string, parseError,
                                                        SVGLengthNegativeValuesMode::Forbid, u8"0%"));
    }
    ReportAttributeParsingError(parseError, name, string);
    SVGGradientElement::AttributeChanged(name, oldValue, value);
  }

  void SVGRadialGradientElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      InstanceInvalidationGuard guard(*this);
      UpdateRelativeLengthsInformation();
      InvalidateGradientResource();
      return;
    }

    SVGGradientElement::SVGAttributeChanged(attributeName);
  }

  RenderPtr<RenderElement> SVGRadialGradientElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<LegacyRenderSVGResourceRadialGradient>(*this, krys::move(style));
  }

  namespace
  {
    void SetGradientAttributes(SVGGradientElement &element, RadialGradientAttributes &attributes,
                               bool isRadial = true) noexcept
    {
      if (!attributes.HasSpreadMethod() && element.HasAttribute(SVGNames::SpreadMethodAttr().LocalName()))
      {
        attributes.SetSpreadMethod(element.SpreadMethod());
      }

      if (!attributes.HasGradientUnits() && element.HasAttribute(SVGNames::GradientUnitsAttr().LocalName()))
      {
        attributes.SetGradientUnits(element.GradientUnits());
      }

      if (!attributes.HasGradientTransform()
          && element.HasAttribute(SVGNames::GradientTransformAttr().LocalName()))
      {
        attributes.SetGradientTransform(element.GradientTransform().Concatenate());
      }

      if (!attributes.HasStops())
      {
        attributes.SetStops(element.BuildStops());
      }

      if (isRadial)
      {
        auto &radial = Downcast<SVGRadialGradientElement>(element);

        if (!attributes.HasCx() && element.HasAttribute(SVGNames::CxAttr().LocalName()))
        {
          attributes.SetCx(radial.Cx());
        }

        if (!attributes.HasCy() && element.HasAttribute(SVGNames::CyAttr().LocalName()))
        {
          attributes.SetCy(radial.Cy());
        }

        if (!attributes.HasR() && element.HasAttribute(SVGNames::RAttr().LocalName()))
        {
          attributes.SetR(radial.R());
        }

        if (!attributes.HasFx() && element.HasAttribute(SVGNames::FxAttr().LocalName()))
        {
          attributes.SetFx(radial.Fx());
        }

        if (!attributes.HasFy() && element.HasAttribute(SVGNames::FyAttr().LocalName()))
        {
          attributes.SetFy(radial.Fy());
        }

        if (!attributes.HasFr() && element.HasAttribute(SVGNames::FrAttr().LocalName()))
        {
          attributes.SetFr(radial.Fr());
        }
      }
    }
  }

  bool SVGRadialGradientElement::CollectGradientAttributes(RadialGradientAttributes &attributes) noexcept
  {
    if (!Renderer())
    {
      return false;
    }

    HashSet<const SVGGradientElement *> processedGradients;
    Ref<SVGGradientElement> current = ShareRef<SVGGradientElement>(*this);

    SetGradientAttributes(*current, attributes);
    processedGradients.insert(current.get());

    while (true)
    {
      // Respect xlink:href, take attributes from referenced element
      auto target = SVGURIReference::TargetElementFromIRIString(current->Href(), TreeScopeForSVGReferences());
      if (auto *gradientElement = DynamicDowncast<SVGGradientElement>(target.Element.get()))
      {
        current = ShareRef(*gradientElement);

        // Cycle detection
        if (processedGradients.contains(current.get()))
        {
          break;
        }

        if (!current->Renderer())
        {
          return false;
        }

        SetGradientAttributes(*current, attributes, Is<SVGRadialGradientElement>(*current));
        processedGradients.insert(current.get());
      }
      else
      {
        break;
      }
    }

    // Handle default values for fx/fy
    if (!attributes.HasFx())
    {
      attributes.SetFx(attributes.Cx());
    }

    if (!attributes.HasFy())
    {
      attributes.SetFy(attributes.Cy());
    }

    return true;
  }

  bool SVGRadialGradientElement::SelfHasRelativeLengths() const noexcept
  {
    return Cx().IsRelative() || Cy().IsRelative() || R().IsRelative() || Fx().IsRelative()
           || Fy().IsRelative() || Fr().IsRelative();
  }
}
