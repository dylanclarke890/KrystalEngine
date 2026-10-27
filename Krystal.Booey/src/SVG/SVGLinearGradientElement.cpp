#include "Krystal.Booey/SVG/SVGLinearGradientElement.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceLinearGradient.hpp"
#include "Krystal.Booey/SVG/LinearGradientAttributes.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Core/Types/HashSet.hpp"

namespace krys::boo::svg
{
  SVGLinearGradientElement::SVGLinearGradientElement(dom::Document &document) noexcept
      : SVGGradientElement(document, CreateUnique<PropertyRegistry>(*this),
                           SVGElementInterface::LinearGradient),
        _x1(SVGAnimatedLength::Create(this, SVGLengthMode::Width, u8"0%")),
        _y1(SVGAnimatedLength::Create(this, SVGLengthMode::Height, u8"0%")),
        _x2(SVGAnimatedLength::Create(this, SVGLengthMode::Width, u8"100%")),
        _y2(SVGAnimatedLength::Create(this, SVGLengthMode::Height, u8"0%"))
  {
    // Spec: If the x2 attribute is not specified, the effect is as if a value of "100%" were specified.
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGLinearGradientElement::_x1>(SVGNames::X1Attr());
      PropertyRegistry::RegisterProperty<&SVGLinearGradientElement::_y1>(SVGNames::Y1Attr());
      PropertyRegistry::RegisterProperty<&SVGLinearGradientElement::_x2>(SVGNames::X2Attr());
      PropertyRegistry::RegisterProperty<&SVGLinearGradientElement::_y2>(SVGNames::Y2Attr());
    }
  }

  void SVGLinearGradientElement::AttributeChanged(const dom::QualifiedName &name,
                                                  Maybe<dom::DOMStringView> oldValue,
                                                  Maybe<dom::DOMStringView> value) noexcept
  {
    auto parseError = SVGParsingError::None;
    auto string = value.value_or(dom::DOMStringView {});

    if (name == SVGNames::X1Attr())
    {
      _x1->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Width, string, parseError,
                                                        SVGLengthNegativeValuesMode::Allow, u8"0%"));
    }
    else if (name == SVGNames::Y1Attr())
    {
      _y1->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Height, string, parseError,
                                                        SVGLengthNegativeValuesMode::Allow, u8"0%"));
    }
    else if (name == SVGNames::X2Attr())
    {
      _x2->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Width, string, parseError,
                                                        SVGLengthNegativeValuesMode::Allow, u8"100%"));
    }
    else if (name == SVGNames::Y2Attr())
    {
      _y2->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Height, string, parseError,
                                                        SVGLengthNegativeValuesMode::Allow, u8"0%"));
    }
    ReportAttributeParsingError(parseError, name, string);

    SVGGradientElement::AttributeChanged(name, oldValue, value);
  }

  void SVGLinearGradientElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
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

  RenderPtr<RenderElement> SVGLinearGradientElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<LegacyRenderSVGResourceLinearGradient>(*this, krys::move(style));
  }

  namespace
  {
    void SetGradientAttributes(SVGGradientElement &element, LinearGradientAttributes &attributes,
                               bool isLinear = true) noexcept
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

      if (isLinear)
      {
        auto &linear = Downcast<SVGLinearGradientElement>(element);

        if (!attributes.HasX1() && element.HasAttribute(SVGNames::X1Attr().LocalName()))
        {
          attributes.SetX1(linear.X1());
        }

        if (!attributes.HasY1() && element.HasAttribute(SVGNames::Y1Attr().LocalName()))
        {
          attributes.SetY1(linear.Y1());
        }

        if (!attributes.HasX2() && element.HasAttribute(SVGNames::X2Attr().LocalName()))
        {
          attributes.SetX2(linear.X2());
        }

        if (!attributes.HasY2() && element.HasAttribute(SVGNames::Y2Attr().LocalName()))
        {
          attributes.SetY2(linear.Y2());
        }
      }
    }
  }

  bool SVGLinearGradientElement::CollectGradientAttributes(LinearGradientAttributes &attributes) noexcept
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
          return true;
        }

        if (!current->Renderer())
        {
          return false;
        }

        SetGradientAttributes(*current, attributes, Is<SVGLinearGradientElement>(*current));
        processedGradients.insert(current.get());
      }
      else
      {
        return true;
      }
    }

    krys_debug_assert(false);
    return false;
  }

  bool SVGLinearGradientElement::SelfHasRelativeLengths() const noexcept
  {
    return X1().IsRelative() || Y1().IsRelative() || X2().IsRelative() || Y2().IsRelative();
  }
}
