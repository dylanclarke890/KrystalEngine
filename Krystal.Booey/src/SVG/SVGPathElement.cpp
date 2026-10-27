#include "Krystal.Booey/SVG/SVGPathElement.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Values/PathValue.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGPath.hpp"
#include "Krystal.Booey/Style/Values/Shapes/PathComputation.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathUtilities.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Booey/SVG/SVGPoint.hpp"

namespace krys::boo::svg
{
  namespace
  {
    constexpr EnumSet<dom::LayoutOptions> PathQueryLayoutOptions {
      dom::LayoutOptions::TreatContentVisibilityHiddenAsVisible,
      dom::LayoutOptions::TreatContentVisibilityAutoAsVisible};
  }

  SVGPathElement::SVGPathElement(dom::Document &document) noexcept
      : SVGGeometryElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::Path)
  {
  }

  void SVGPathElement::AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                        Maybe<dom::DOMStringView> value) noexcept
  {
    if (name == SVGNames::DAttr())
    {
      auto string = value.value_or(dom::DOMStringView {});
      if (string.empty())
      {
        _pathByteStream.Clear();
      }
      else if (!BuildSVGPathByteStreamFromString(string, _pathByteStream, PathParsingMode::UnalteredParsing))
      {
        // FIXME: Report "Problem parsing d="value"" to the console, once there's one.
      }
    }

    SVGGeometryElement::AttributeChanged(name, oldValue, value);
  }

  void SVGPathElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (attributeName == SVGNames::DAttr())
    {
      InstanceInvalidationGuard guard(*this);

      if (auto *path = DynamicDowncast<LegacyRenderSVGPath>(Renderer()))
      {
        path->SetNeedsShapeUpdate();
      }

      UpdateSVGRendererForElementChange();
      SetPresentationalHintStyleIsDirty();
      return;
    }

    SVGGeometryElement::SVGAttributeChanged(attributeName);
  }

  float SVGPathElement::GetTotalLength() const noexcept
  {
    (void)NodeDocument().UpdateLayoutIgnorePendingStylesheets(PathQueryLayoutOptions, this);

    return GetTotalLengthOfSVGPathByteStream(PathByteStream());
  }

  dom::ExceptionOr<Ref<SVGPoint>> SVGPathElement::GetPointAtLength(float distance) const noexcept
  {
    (void)NodeDocument().UpdateLayoutIgnorePendingStylesheets(PathQueryLayoutOptions, this);

    // Spec: If it is not able to compute the total length of path, then throw.
    if (PathByteStream().IsEmpty())
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError, u8"The element's path is empty."};
    }

    // Spec: Clamp distance to [0, length].
    distance = std::clamp<float>(distance, 0, GetTotalLength());

    // Spec: Return a newly created, detached SVGPoint object.
    return SVGPoint::Create(GetPointAtLengthOfSVGPathByteStream(PathByteStream(), distance));
  }

  FloatRect SVGPathElement::GetBBox(StyleUpdateStrategy styleUpdateStrategy) noexcept
  {
    if (styleUpdateStrategy == AllowStyleUpdate)
    {
      (void)NodeDocument().UpdateLayoutIgnorePendingStylesheets(PathQueryLayoutOptions, this);
    }

    // FIXME: Eventually we should support getBBox for detached elements.
    // FIXME: If the path is null it means we're calling getBBox() before laying out this element,
    // which is an error.

    if (auto *path = DynamicDowncast<LegacyRenderSVGPath>(Renderer()); path && path->HasPath())
    {
      return path->GetPath().BoundingRect();
    }

    return {};
  }

  RenderPtr<RenderElement> SVGPathElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<LegacyRenderSVGPath>(*this, krys::move(style));
  }

  const SVGPathByteStream &SVGPathElement::PathByteStream() const noexcept
  {
    if (auto *renderer = Renderer())
    {
      if (const auto &pathFunction = renderer->Style().D().TryPath())
      {
        return pathFunction->Parameters.Data.ByteStream;
      }
      return SVGPathByteStream::Empty();
    }

    return _pathByteStream;
  }

  Path SVGPathElement::GetPath() const noexcept
  {
    if (auto *renderer = Renderer())
    {
      if (const auto &pathFunction = renderer->Style().D().TryPath())
      {
        return style::ComputePath(pathFunction->Parameters, FloatRect {});
      }
      return {};
    }

    return BuildPathFromByteStream(_pathByteStream);
  }

  void SVGPathElement::CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                              dom::DOMStringAtom namespaceURI,
                                                              dom::DOMStringView value,
                                                              css::MutableStyleProperties &style) noexcept
  {
    if (localName == SVGNames::DAttr().LocalName() && namespaceURI == dom::DOMStringAtom::Null())
    {
      CollectDPresentationalHint(style);
    }
    else
    {
      SVGGeometryElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
    }
  }

  void SVGPathElement::CollectExtraStyleForPresentationalHints(css::MutableStyleProperties &style) noexcept
  {
    if (!style.FindPropertyIndex(css::PropertyId::D))
    {
      CollectDPresentationalHint(style);
    }
  }

  void SVGPathElement::CollectDPresentationalHint(css::MutableStyleProperties &style) noexcept
  {
    // In the case of the `d` property, we want to avoid providing a string value since it will require
    // the path data to be parsed again and path data can be unwieldy.
    auto property =
      CssPropertyIdForSVGAttributeName(SVGNames::DAttr().LocalName(), dom::DOMStringAtom::Null());
    // The fill rule value passed here is not relevant for the `d` property.
    auto cssPathValue = css::PathValue::Create(css::PathFunction {
      .Parameters = css::Path {.FillRule = css::FillRule {css::keywords::Nonzero {}},
                               .Data = css::Path::PathData {.ByteStream = _pathByteStream}}});
    AddPropertyToPresentationalHintStyle(style, property, krys::move(cssPathValue));
  }

  void SVGPathElement::PathDidChange() noexcept
  {
    // FIXME: Tell the <mpath> elements referencing the path (invalidateMPathDependencies), with SMIL (B11i).
  }
}
