#include "Krystal.Booey/SVG/SVGForeignObjectElement.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGForeignObject.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Booey/SVG/SVGSvgElement.hpp"

namespace krys::boo::svg
{
  SVGForeignObjectElement::SVGForeignObjectElement(dom::Document &document) noexcept
      : SVGGraphicsElement(document, CreateUnique<PropertyRegistry>(*this),
                           SVGElementInterface::ForeignObject),
        _x(SVGAnimatedLength::Create(this, SVGLengthMode::Width)),
        _y(SVGAnimatedLength::Create(this, SVGLengthMode::Height)),
        _width(SVGAnimatedLength::Create(this, SVGLengthMode::Width)),
        _height(SVGAnimatedLength::Create(this, SVGLengthMode::Height))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGForeignObjectElement::_x>(SVGNames::XAttr());
      PropertyRegistry::RegisterProperty<&SVGForeignObjectElement::_y>(SVGNames::YAttr());
      PropertyRegistry::RegisterProperty<&SVGForeignObjectElement::_width>(SVGNames::WidthAttr());
      PropertyRegistry::RegisterProperty<&SVGForeignObjectElement::_height>(SVGNames::HeightAttr());
    }
  }

  void SVGForeignObjectElement::AttributeChanged(const dom::QualifiedName &name,
                                                 Maybe<dom::DOMStringView> oldValue,
                                                 Maybe<dom::DOMStringView> value) noexcept
  {
    auto parseError = SVGParsingError::None;
    auto string = value.value_or(dom::DOMStringView {});

    if (name == SVGNames::XAttr())
    {
      _x->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Width, string, parseError));
    }
    else if (name == SVGNames::YAttr())
    {
      _y->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Height, string, parseError));
    }
    else if (name == SVGNames::WidthAttr())
    {
      _width->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Width, string, parseError));
    }
    else if (name == SVGNames::HeightAttr())
    {
      _height->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Height, string, parseError));
    }
    ReportAttributeParsingError(parseError, name, string);

    SVGGraphicsElement::AttributeChanged(name, oldValue, value);
  }

  void SVGForeignObjectElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      InstanceInvalidationGuard guard(*this);
      if (attributeName == SVGNames::WidthAttr() || attributeName == SVGNames::HeightAttr())
      {
        SetPresentationalHintStyleIsDirty();
      }
      else
      {
        krys_debug_assert(attributeName == SVGNames::XAttr() || attributeName == SVGNames::YAttr());
        UpdateRelativeLengthsInformation();
        UpdateSVGRendererForElementChange();
      }
      return;
    }

    SVGGraphicsElement::SVGAttributeChanged(attributeName);
  }

  RenderPtr<RenderElement> SVGForeignObjectElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    // FIXME: Tell the document it may have rendered foreign objects (setMayHaveRenderedSVGForeignObjects),
    // once there's composited scrolling, which asks.
    return CreateRenderer<LegacyRenderSVGForeignObject>(*this, krys::move(style));
  }

  bool SVGForeignObjectElement::ChildShouldCreateRenderer(const dom::Node &child) const noexcept
  {
    // Disallow arbitary SVG content. Only allow proper <svg xmlns="svgNS"> subdocuments.
    if (auto *svgChild = DynamicDowncast<SVGElement>(child))
    {
      return Is<SVGSvgElement>(*svgChild);
    }

    // Skip over SVG rules which disallow non-SVG kids
    return StyledElement::ChildShouldCreateRenderer(child);
  }

  bool SVGForeignObjectElement::RendererIsNeeded(const RenderStyle &style) noexcept
  {
    // Suppress foreignObject renderers in SVG hidden containers.
    // (https://bugs.webkit.org/show_bug.cgi?id=87297)
    // Note that we currently do not support foreignObject instantiation via <use>, hence it is safe
    // to use parentElement() here. If that changes, this method should be updated to use
    // parentOrShadowHostElement() instead.
    auto *ancestor = ParentElement();
    if (!ancestor)
    {
      return false;
    }

    while (ancestor && Is<SVGElement>(*ancestor))
    {
      if (ancestor->Renderer() && ancestor->Renderer()->IsLegacyRenderSVGHiddenContainer())
      {
        return false;
      }

      ancestor = ancestor->ParentElement();
    }

    return SVGGraphicsElement::RendererIsNeeded(style);
  }
}
