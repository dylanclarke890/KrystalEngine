#include "Krystal.Booey/SVG/SVGTextPathElement.hpp"
#include "Krystal.Booey/DOM/DocumentOrShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResource.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGTextPath.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGAElement.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Booey/SVG/SVGPathElement.hpp"
#include "Krystal.Booey/SVG/SVGTextElement.hpp"
#include "Krystal.Booey/SVG/SVGTSpanElement.hpp"

namespace krys::boo::svg
{
  SVGTextPathElement::SVGTextPathElement(dom::Document &document) noexcept
      : SVGTextContentElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::TextPath),
        SVGURIReference(this), _startOffset(SVGAnimatedLength::Create(this, SVGLengthMode::Other)),
        _method(SVGAnimatedEnumeration::Create(this, SVGTextPathMethodAlign)),
        _spacing(SVGAnimatedEnumeration::Create(this, SVGTextPathSpacingExact))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGTextPathElement::_startOffset>(SVGNames::StartOffsetAttr());
      PropertyRegistry::RegisterProperty<SVGTextPathMethodType, &SVGTextPathElement::_method>(
        SVGNames::MethodAttr());
      PropertyRegistry::RegisterProperty<SVGTextPathSpacingType, &SVGTextPathElement::_spacing>(
        SVGNames::SpacingAttr());
    }
  }

  SVGTextPathElement::~SVGTextPathElement() noexcept
  {
    ClearResourceReferences();
  }

  void SVGTextPathElement::ClearResourceReferences() noexcept
  {
    RemoveElementReference();
  }

  void SVGTextPathElement::AttributeChanged(const dom::QualifiedName &name,
                                            Maybe<dom::DOMStringView> oldValue,
                                            Maybe<dom::DOMStringView> value) noexcept
  {
    auto parseError = SVGParsingError::None;
    auto string = value.value_or(dom::DOMStringView {});

    if (name == SVGNames::StartOffsetAttr())
    {
      _startOffset->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Other, string, parseError));
    }
    else if (name == SVGNames::MethodAttr())
    {
      SVGTextPathMethodType propertyValue =
        SVGPropertyTraits<SVGTextPathMethodType>::FromString(*this, string);
      if (propertyValue > 0)
      {
        _method->SetBaseValInternal<SVGTextPathMethodType>(propertyValue);
      }
    }
    else if (name == SVGNames::SpacingAttr())
    {
      SVGTextPathSpacingType propertyValue =
        SVGPropertyTraits<SVGTextPathSpacingType>::FromString(*this, string);
      if (propertyValue > 0)
      {
        _spacing->SetBaseValInternal<SVGTextPathSpacingType>(propertyValue);
      }
    }

    ReportAttributeParsingError(parseError, name, string);

    SVGURIReference::ParseAttribute(name, value);
    SVGTextContentElement::AttributeChanged(name, oldValue, value);
  }

  void SVGTextPathElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      InstanceInvalidationGuard guard(*this);
      if (attributeName == SVGNames::StartOffsetAttr())
      {
        UpdateRelativeLengthsInformation();
      }

      UpdateSVGRendererForElementChange();
      return;
    }

    if (SVGURIReference::IsKnownAttribute(attributeName))
    {
      BuildPendingResource();
      UpdateSVGRendererForElementChange();
      return;
    }

    SVGTextContentElement::SVGAttributeChanged(attributeName);
  }

  RenderPtr<RenderElement> SVGTextPathElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<RenderSVGTextPath>(*this, krys::move(style));
  }

  bool SVGTextPathElement::ChildShouldCreateRenderer(const dom::Node &child) const noexcept
  {
    if (Is<dom::Text>(child))
    {
      return true;
    }
    auto *element = DynamicDowncast<SVGElement>(child);
    return element && (Is<SVGAElement>(*element) || Is<SVGTSpanElement>(*element));
  }

  bool SVGTextPathElement::RendererIsNeeded(const RenderStyle &style) noexcept
  {
    if (auto *parent = DynamicDowncast<SVGElement>(ParentNode()))
    {
      if (Is<SVGAElement>(*parent) || Is<SVGTextElement>(*parent))
      {
        return dom::StyledElement::RendererIsNeeded(style);
      }
    }
    return false;
  }

  void SVGTextPathElement::BuildPendingResource() noexcept
  {
    ClearResourceReferences();
    if (!IsConnected())
    {
      return;
    }

    auto target = SVGURIReference::TargetElementFromIRIString(Href(), TreeScopeForSVGReferences());
    if (!target.Element)
    {
      // Do not register as pending if we are already pending this resource.
      auto &treeScope = TreeScopeForSVGReferences();
      if (treeScope.IsPendingSVGResource(*this, target.Identifier))
      {
        return;
      }

      if (!target.Identifier.View().empty())
      {
        treeScope.AddPendingSVGResource(target.Identifier, *this);
        krys_debug_assert(HasPendingResources());
      }
    }
    else if (auto *svgElement = DynamicDowncast<SVGElement>(target.Element.get());
             svgElement && Is<SVGPathElement>(*svgElement))
    {
      Downcast<SVGPathElement>(*svgElement).AddReferencingElement(*this);
    }

    auto *renderer = Renderer();
    if (!renderer)
    {
      return;
    }
    LegacyRenderSVGResource::MarkForLayoutAndParentResourceInvalidation(*renderer);
  }

  void SVGTextPathElement::OnPostConnection() noexcept
  {
    SVGTextContentElement::OnPostConnection();
    BuildPendingResource();
  }

  void SVGTextPathElement::OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    bool disconnectedFromDocument = oldAncestor.IsConnected() && !IsConnected();
    SVGTextContentElement::OnRemove(isSubtreeRoot, oldAncestor);
    if (disconnectedFromDocument)
    {
      ClearResourceReferences();
    }
  }

  bool SVGTextPathElement::SelfHasRelativeLengths() const noexcept
  {
    return StartOffset().IsRelative() || SVGTextContentElement::SelfHasRelativeLengths();
  }
}
