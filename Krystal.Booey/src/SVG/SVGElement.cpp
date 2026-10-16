#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/SVG/SVGForeignObjectElement.hpp"
#include "Krystal.Booey/SVG/SVGSvgElement.hpp"
#include "Krystal.Booey/SVG/SVGUseElement.hpp"
#include "Krystal.Core/Types/HashMap.hpp"

namespace krys::boo::svg
{
  SVGElement::SVGElement(dom::Document &document, SVGElementInterface interface, SVGElementFlags flags,
                         dom::NodeFlags nodeFlags) noexcept
      : StyledElement(document, dom::NullQualifiedName(), dom::NodeFlags::IsSVGElement | nodeFlags),
        _flags(flags), _interface(interface)
  {
  }

  SVGElement::SVGElement(dom::Document &document) noexcept : SVGElement(document, SVGElementInterface::None)
  {
  }

  bool SVGElement::IsInnerSVGSVGElement() const noexcept
  {
    if (!IsSVGSvgElement())
    {
      return false;
    }

    // Element may not be in the document, pretend we're outermost for viewport(), getCTM(), etc.
    if (!ParentNode())
    {
      return false;
    }

    return Is<SVGElement>(*ParentNode());
  }

  bool SVGElement::IsOutermostSVGSVGElement() const noexcept
  {
    if (!IsSVGSvgElement())
    {
      return false;
    }

    // Element may not be in the document, pretend we're outermost for viewport(), getCTM(), etc.
    auto *parent = ParentNode();
    if (!parent)
    {
      return true;
    }

    // We act like an outermost SVG element, if we're a direct child of a <foreignObject> element.
    if (auto *svgParent = DynamicDowncast<SVGElement>(*parent);
        svgParent && Is<SVGForeignObjectElement>(*svgParent))
    {
      return true;
    }

    // If we're inside the shadow tree of a <use> element, we're always an inner <svg> element.
    if (IsInShadowTree())
    {
      auto *host = dom::TreeQueries::ShadowHost(*this);
      if (auto *svgHost = DynamicDowncast<SVGElement>(host); svgHost && Is<SVGUseElement>(*svgHost))
      {
        return false;
      }
    }

    // This is true whenever this is the outermost SVG, even if there are HTML elements outside it
    return !Is<SVGElement>(*parent);
  }

#pragma region Presentational hints

  css::PropertyId SVGElement::CssPropertyIdForSVGAttributeName(dom::DOMStringAtom localName,
                                                               dom::DOMStringAtom namespaceURI) noexcept
  {
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      return css::PropertyId::Invalid;
    }

    // FIXME: `d` maps to the d property (as a `path()` value), once there's SVGPathElement.
    static const HashMap<dom::DOMStringView, css::PropertyId> map {
      {u8"alignment-baseline", css::PropertyId::AlignmentBaseline},
      {u8"baseline-shift", css::PropertyId::BaselineShift},
      {u8"buffered-rendering", css::PropertyId::BufferedRendering},
      {u8"clip", css::PropertyId::Clip},
      {u8"clip-path", css::PropertyId::ClipPath},
      {u8"clip-rule", css::PropertyId::ClipRule},
      {u8"color", css::PropertyId::Color},
      {u8"color-interpolation", css::PropertyId::ColorInterpolation},
      {u8"color-interpolation-filters", css::PropertyId::ColorInterpolationFilters},
      {u8"cursor", css::PropertyId::Cursor},
      {u8"cx", css::PropertyId::Cx},
      {u8"cy", css::PropertyId::Cy},
      {u8"direction", css::PropertyId::Direction},
      {u8"display", css::PropertyId::Display},
      {u8"dominant-baseline", css::PropertyId::DominantBaseline},
      {u8"fill", css::PropertyId::Fill},
      {u8"fill-opacity", css::PropertyId::FillOpacity},
      {u8"fill-rule", css::PropertyId::FillRule},
      {u8"filter", css::PropertyId::Filter},
      {u8"flood-color", css::PropertyId::FloodColor},
      {u8"flood-opacity", css::PropertyId::FloodOpacity},
      {u8"font-family", css::PropertyId::FontFamily},
      {u8"font-size", css::PropertyId::FontSize},
      {u8"font-size-adjust", css::PropertyId::FontSizeAdjust},
      {u8"font-stretch", css::PropertyId::FontWidth},
      {u8"font-width", css::PropertyId::FontWidth},
      {u8"font-style", css::PropertyId::FontStyle},
      {u8"font-variant", css::PropertyId::FontVariant},
      {u8"font-weight", css::PropertyId::FontWeight},
      {u8"glyph-orientation-horizontal", css::PropertyId::GlyphOrientationHorizontal},
      {u8"glyph-orientation-vertical", css::PropertyId::GlyphOrientationVertical},
      {u8"image-rendering", css::PropertyId::ImageRendering},
      {u8"height", css::PropertyId::Height},
      {u8"letter-spacing", css::PropertyId::LetterSpacing},
      {u8"lighting-color", css::PropertyId::LightingColor},
      {u8"marker-end", css::PropertyId::MarkerEnd},
      {u8"marker-mid", css::PropertyId::MarkerMid},
      {u8"marker-start", css::PropertyId::MarkerStart},
      {u8"mask", css::PropertyId::Mask},
      {u8"mask-type", css::PropertyId::MaskType},
      {u8"opacity", css::PropertyId::Opacity},
      {u8"overflow", css::PropertyId::Overflow},
      {u8"paint-order", css::PropertyId::PaintOrder},
      {u8"pointer-events", css::PropertyId::PointerEvents},
      {u8"r", css::PropertyId::R},
      {u8"rx", css::PropertyId::Rx},
      {u8"ry", css::PropertyId::Ry},
      {u8"shape-rendering", css::PropertyId::ShapeRendering},
      {u8"stop-color", css::PropertyId::StopColor},
      {u8"stop-opacity", css::PropertyId::StopOpacity},
      {u8"stroke", css::PropertyId::Stroke},
      {u8"stroke-dasharray", css::PropertyId::StrokeDasharray},
      {u8"stroke-dashoffset", css::PropertyId::StrokeDashoffset},
      {u8"stroke-linecap", css::PropertyId::StrokeLinecap},
      {u8"stroke-linejoin", css::PropertyId::StrokeLinejoin},
      {u8"stroke-miterlimit", css::PropertyId::StrokeMiterlimit},
      {u8"stroke-opacity", css::PropertyId::StrokeOpacity},
      {u8"stroke-width", css::PropertyId::StrokeWidth},
      {u8"text-anchor", css::PropertyId::TextAnchor},
      {u8"text-decoration", css::PropertyId::TextDecoration},
      {u8"text-rendering", css::PropertyId::TextRendering},
      {u8"unicode-bidi", css::PropertyId::UnicodeBidi},
      {u8"vector-effect", css::PropertyId::VectorEffect},
      {u8"visibility", css::PropertyId::Visibility},
      {u8"width", css::PropertyId::Width},
      {u8"word-spacing", css::PropertyId::WordSpacing},
      {u8"writing-mode", css::PropertyId::WritingMode},
      {u8"x", css::PropertyId::X},
      {u8"y", css::PropertyId::Y},
      {u8"transform-origin", css::PropertyId::TransformOrigin},
    };

    auto it = map.find(localName.View());
    return it == map.end() ? css::PropertyId::Invalid : it->second;
  }

  bool SVGElement::HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                      dom::DOMStringAtom namespaceURI) const noexcept
  {
    if (CssPropertyIdForSVGAttributeName(localName, namespaceURI) != css::PropertyId::Invalid)
    {
      return true;
    }
    return StyledElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void SVGElement::CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                          dom::DOMStringAtom namespaceURI,
                                                          dom::DOMStringView value,
                                                          css::MutableStyleProperties &style) noexcept
  {
    auto property = CssPropertyIdForSVGAttributeName(localName, namespaceURI);
    if (property != css::PropertyId::Invalid)
    {
      AddPropertyToPresentationalHintStyle(style, property, css::CSSOMString(value));
    }
  }

#pragma endregion
}
