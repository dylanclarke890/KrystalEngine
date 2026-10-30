#include "Krystal.Booey/SVG/SVGTextPositioningElement.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInline.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGText.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  SVGTextPositioningElement::SVGTextPositioningElement(dom::Document &document,
                                                       UniquePtr<SVGPropertyRegistry> &&propertyRegistry,
                                                       SVGElementInterface interface) noexcept
      : SVGTextContentElement(document, krys::move(propertyRegistry), interface,
                              SVGElementFlags::IsSvgTextPositioningElement),
        _x(SVGAnimatedLengthList::Create(this, SVGLengthMode::Width)),
        _y(SVGAnimatedLengthList::Create(this, SVGLengthMode::Height)),
        _dx(SVGAnimatedLengthList::Create(this, SVGLengthMode::Width)),
        _dy(SVGAnimatedLengthList::Create(this, SVGLengthMode::Height)),
        _rotate(SVGAnimatedNumberList::Create(this))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGTextPositioningElement::_x>(SVGNames::XAttr());
      PropertyRegistry::RegisterProperty<&SVGTextPositioningElement::_y>(SVGNames::YAttr());
      PropertyRegistry::RegisterProperty<&SVGTextPositioningElement::_dx>(SVGNames::DxAttr());
      PropertyRegistry::RegisterProperty<&SVGTextPositioningElement::_dy>(SVGNames::DyAttr());
      PropertyRegistry::RegisterProperty<&SVGTextPositioningElement::_rotate>(SVGNames::RotateAttr());
    }
  }

  void SVGTextPositioningElement::AttributeChanged(const dom::QualifiedName &name,
                                                   Maybe<dom::DOMStringView> oldValue,
                                                   Maybe<dom::DOMStringView> value) noexcept
  {
    auto string = value.value_or(dom::DOMStringView {});
    if (name == SVGNames::XAttr())
    {
      _x->BaseVal()->Parse(string);
    }
    else if (name == SVGNames::YAttr())
    {
      _y->BaseVal()->Parse(string);
    }
    else if (name == SVGNames::DxAttr())
    {
      _dx->BaseVal()->Parse(string);
    }
    else if (name == SVGNames::DyAttr())
    {
      _dy->BaseVal()->Parse(string);
    }
    else if (name == SVGNames::RotateAttr())
    {
      _rotate->BaseVal()->Parse(string);
    }

    SVGTextContentElement::AttributeChanged(name, oldValue, value);
  }

  namespace
  {
    KRYS_NODISCARD bool IsXOrYAttribute(dom::DOMStringAtom localName,
                                        dom::DOMStringAtom namespaceURI) noexcept
    {
      return namespaceURI == dom::DOMStringAtom::Null() && (localName == u8"x" || localName == u8"y");
    }
  }

  void SVGTextPositioningElement::CollectPresentationalHintsForAttribute(
    dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI, dom::DOMStringView value,
    css::MutableStyleProperties &style) noexcept
  {
    if (IsXOrYAttribute(localName, namespaceURI))
    {
      return;
    }
    SVGTextContentElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }

  bool SVGTextPositioningElement::HasPresentationalHintsForAttribute(
    dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI) const noexcept
  {
    if (IsXOrYAttribute(localName, namespaceURI))
    {
      return false;
    }
    return SVGTextContentElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void SVGTextPositioningElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      InstanceInvalidationGuard guard(*this);

      if (attributeName != SVGNames::RotateAttr())
      {
        UpdateRelativeLengthsInformation();
      }

      if (auto *renderer = Renderer())
      {
        if (auto *textAncestor = RenderSVGText::LocateRenderSVGTextAncestor(*renderer))
        {
          textAncestor->SetNeedsPositioningValuesUpdate();
        }
      }
      UpdateSVGRendererForElementChange();
      return;
    }

    SVGTextContentElement::SVGAttributeChanged(attributeName);
  }

  RefPtr<SVGTextPositioningElement>
    SVGTextPositioningElement::ElementFromRenderer(RenderBoxModelObject &renderer) noexcept
  {
    if (!Is<RenderSVGText>(renderer) && !Is<RenderSVGInline>(renderer))
    {
      return nullptr;
    }

    krys_debug_assert(renderer.GetElement());
    return ShareRefPtr(
      DynamicDowncast<SVGTextPositioningElement>(Downcast<SVGElement>(renderer.GetElement())));
  }
}
