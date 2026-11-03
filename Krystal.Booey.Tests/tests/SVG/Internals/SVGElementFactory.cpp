#include "Krystal.Booey/SVG/Internals/SVGElementFactory.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/CSS/StyleSheetList.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLUnknownElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/SVG/Elements/SVGAElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGAnimateElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGAnimateMotionElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGAnimateTransformElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGCircleElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGClipPathElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGDefsElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGDescElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGEllipseElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEBlendElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEColorMatrixElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEComponentTransferElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFECompositeElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEConvolveMatrixElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEDiffuseLightingElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEDisplacementMapElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEDistantLightElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEDropShadowElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEFloodElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEFuncAElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEFuncBElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEFuncGElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEFuncRElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEGaussianBlurElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEImageElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEMergeElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEMergeNodeElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEMorphologyElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEOffsetElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEPointLightElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFESpecularLightingElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFESpotLightElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFETileElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFETurbulenceElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFilterElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGForeignObjectElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGGElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGImageElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGLinearGradientElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGLineElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGMarkerElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGMaskElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGMetadataElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGMPathElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGPathElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGPatternElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGPolygonElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGPolylineElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGRadialGradientElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGRectElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGScriptElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGSetElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGStopElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGStyleElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGSvgElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGSwitchElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGSymbolElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGTextElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGTextPathElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGTitleElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGTSpanElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGUnknownElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGUseElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGViewElement.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::svg::tests
{
  using html::TagName;
  using style::tests::ResolverFixture;

  namespace
  {
    template <typename T>
    KRYS_NODISCARD bool IsCreated(dom::Document &document, dom::DOMStringView localName) noexcept
    {
      auto element =
        document.CreateElementNS(dom::DOMStringAtom(infra::Namespaces::SVG), dom::DOMStringAtom(localName));
      krys_assert(!element.HasException());
      auto *svgElement = DynamicDowncast<SVGElement>(*element.Value());
      return svgElement && Is<T>(*svgElement) && element.Value()->LocalName() == localName;
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGElementFactory: the elements", "[SVG][SVGElementFactory]")
  {
    auto document = ResolverFixture::Parse(u8"<!DOCTYPE html>");

    CHECK(IsCreated<SVGAElement>(*document, u8"a"));
    CHECK(IsCreated<SVGCircleElement>(*document, u8"circle"));
    CHECK(IsCreated<SVGClipPathElement>(*document, u8"clipPath"));
    CHECK(IsCreated<SVGDefsElement>(*document, u8"defs"));
    CHECK(IsCreated<SVGDescElement>(*document, u8"desc"));
    CHECK(IsCreated<SVGEllipseElement>(*document, u8"ellipse"));
    CHECK(IsCreated<SVGForeignObjectElement>(*document, u8"foreignObject"));
    CHECK(IsCreated<SVGGElement>(*document, u8"g"));
    CHECK(IsCreated<SVGImageElement>(*document, u8"image"));
    CHECK(IsCreated<SVGLineElement>(*document, u8"line"));
    CHECK(IsCreated<SVGLinearGradientElement>(*document, u8"linearGradient"));
    CHECK(IsCreated<SVGMarkerElement>(*document, u8"marker"));
    CHECK(IsCreated<SVGMaskElement>(*document, u8"mask"));
    CHECK(IsCreated<SVGMetadataElement>(*document, u8"metadata"));
    CHECK(IsCreated<SVGPathElement>(*document, u8"path"));
    CHECK(IsCreated<SVGPatternElement>(*document, u8"pattern"));
    CHECK(IsCreated<SVGPolygonElement>(*document, u8"polygon"));
    CHECK(IsCreated<SVGPolylineElement>(*document, u8"polyline"));
    CHECK(IsCreated<SVGRadialGradientElement>(*document, u8"radialGradient"));
    CHECK(IsCreated<SVGRectElement>(*document, u8"rect"));
    CHECK(IsCreated<SVGScriptElement>(*document, u8"script"));
    CHECK(IsCreated<SVGStopElement>(*document, u8"stop"));
    CHECK(IsCreated<SVGStyleElement>(*document, u8"style"));
    CHECK(IsCreated<SVGSvgElement>(*document, u8"svg"));
    CHECK(IsCreated<SVGSwitchElement>(*document, u8"switch"));
    CHECK(IsCreated<SVGSymbolElement>(*document, u8"symbol"));
    CHECK(IsCreated<SVGTextElement>(*document, u8"text"));
    CHECK(IsCreated<SVGTextPathElement>(*document, u8"textPath"));
    CHECK(IsCreated<SVGTitleElement>(*document, u8"title"));
    CHECK(IsCreated<SVGTSpanElement>(*document, u8"tspan"));
    CHECK(IsCreated<SVGUseElement>(*document, u8"use"));
    CHECK(IsCreated<SVGViewElement>(*document, u8"view"));

    // Filters.
    CHECK(IsCreated<SVGFilterElement>(*document, u8"filter"));
    CHECK(IsCreated<SVGFEBlendElement>(*document, u8"feBlend"));
    CHECK(IsCreated<SVGFEColorMatrixElement>(*document, u8"feColorMatrix"));
    CHECK(IsCreated<SVGFEComponentTransferElement>(*document, u8"feComponentTransfer"));
    CHECK(IsCreated<SVGFECompositeElement>(*document, u8"feComposite"));
    CHECK(IsCreated<SVGFEConvolveMatrixElement>(*document, u8"feConvolveMatrix"));
    CHECK(IsCreated<SVGFEDiffuseLightingElement>(*document, u8"feDiffuseLighting"));
    CHECK(IsCreated<SVGFEDisplacementMapElement>(*document, u8"feDisplacementMap"));
    CHECK(IsCreated<SVGFEDistantLightElement>(*document, u8"feDistantLight"));
    CHECK(IsCreated<SVGFEDropShadowElement>(*document, u8"feDropShadow"));
    CHECK(IsCreated<SVGFEFloodElement>(*document, u8"feFlood"));
    CHECK(IsCreated<SVGFEFuncAElement>(*document, u8"feFuncA"));
    CHECK(IsCreated<SVGFEFuncBElement>(*document, u8"feFuncB"));
    CHECK(IsCreated<SVGFEFuncGElement>(*document, u8"feFuncG"));
    CHECK(IsCreated<SVGFEFuncRElement>(*document, u8"feFuncR"));
    CHECK(IsCreated<SVGFEGaussianBlurElement>(*document, u8"feGaussianBlur"));
    CHECK(IsCreated<SVGFEImageElement>(*document, u8"feImage"));
    CHECK(IsCreated<SVGFEMergeElement>(*document, u8"feMerge"));
    CHECK(IsCreated<SVGFEMergeNodeElement>(*document, u8"feMergeNode"));
    CHECK(IsCreated<SVGFEMorphologyElement>(*document, u8"feMorphology"));
    CHECK(IsCreated<SVGFEOffsetElement>(*document, u8"feOffset"));
    CHECK(IsCreated<SVGFEPointLightElement>(*document, u8"fePointLight"));
    CHECK(IsCreated<SVGFESpecularLightingElement>(*document, u8"feSpecularLighting"));
    CHECK(IsCreated<SVGFESpotLightElement>(*document, u8"feSpotLight"));
    CHECK(IsCreated<SVGFETileElement>(*document, u8"feTile"));
    CHECK(IsCreated<SVGFETurbulenceElement>(*document, u8"feTurbulence"));

    // Animation.
    CHECK(IsCreated<SVGAnimateElement>(*document, u8"animate"));
    CHECK(IsCreated<SVGAnimateMotionElement>(*document, u8"animateMotion"));
    CHECK(IsCreated<SVGAnimateTransformElement>(*document, u8"animateTransform"));
    CHECK(IsCreated<SVGMPathElement>(*document, u8"mpath"));
    CHECK(IsCreated<SVGSetElement>(*document, u8"set"));

    // The SVG 1.1 elements SVG 2 removed, and names SVG doesn't have.
    for (auto localName : {u8"font", u8"font-face", u8"missing-glyph", u8"altGlyph", u8"tref", u8"cursor",
                           u8"animateColor", u8"div", u8"x"})
    {
      CAPTURE(css::tests::ToNarrow(localName));
      CHECK(IsCreated<SVGUnknownElement>(*document, localName));
    }

    // Tag names are case-sensitive.
    CHECK(IsCreated<SVGUnknownElement>(*document, u8"CIRCLE"));
    CHECK(IsCreated<SVGUnknownElement>(*document, u8"Svg"));

    CHECK(SVGElementFactory::TryCreate(*document, TagName::div) == nullptr);
    CHECK(SVGElementFactory::TryCreate(*document, TagName::Unknown) == nullptr);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGElementFactory: parsed elements", "[SVG][SVGElementFactory]")
  {
    // The parser adjusts the case of SVG's tag names.
    auto document = ResolverFixture::Parse(
      u8"<!DOCTYPE html><svg><foreignobject id=f></foreignobject><clippath id=c></clippath>"
      u8"<style>p { color: green }</style></svg><text id=t></text>");

    auto &foreignObject = *document->GetElementById(u8"f");
    CHECK(foreignObject.LocalName() == u8"foreignObject");
    CHECK(Is<SVGForeignObjectElement>(Downcast<SVGElement>(foreignObject)));

    auto &clipPath = *document->GetElementById(u8"c");
    CHECK(clipPath.LocalName() == u8"clipPath");
    CHECK(Is<SVGClipPathElement>(Downcast<SVGElement>(clipPath)));

    // <style> in SVG makes a style sheet.
    CHECK(document->StyleSheets().Length() == 1u);

    // SVG's tag names in HTML are still unknown HTML elements.
    CHECK(Is<html::HTMLUnknownElement>(Downcast<html::HTMLElement>(*document->GetElementById(u8"t"))));
  }

  TEST_CASE("ParseTagName: SVG's tag names", "[HTML][TagName]")
  {
    CHECK(html::ParseTagName(u8"clipPath") == TagName::clipPath);
    CHECK(html::ParseTagName(u8"clippath") == TagName::Unknown);
    CHECK(html::ParseTagName(u8"feSpecularLighting") == TagName::feSpecularLighting);
    CHECK(html::ParseTagName(u8"switch") == TagName::switch_);
    CHECK(html::ParseTagName(u8"tspan") == TagName::tspan);
    CHECK(html::ParseTagName(u8"set") == TagName::set);
  }
}
