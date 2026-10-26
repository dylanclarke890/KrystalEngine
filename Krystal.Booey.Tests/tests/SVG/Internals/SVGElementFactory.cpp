#include "Krystal.Booey/SVG/Internals/SVGElementFactory.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/CSS/StyleSheetList.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/HTMLUnknownElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/SVG/SVGCircleElement.hpp"
#include "Krystal.Booey/SVG/SVGDefsElement.hpp"
#include "Krystal.Booey/SVG/SVGDescElement.hpp"
#include "Krystal.Booey/SVG/SVGEllipseElement.hpp"
#include "Krystal.Booey/SVG/SVGForeignObjectElement.hpp"
#include "Krystal.Booey/SVG/SVGGElement.hpp"
#include "Krystal.Booey/SVG/SVGImageElement.hpp"
#include "Krystal.Booey/SVG/SVGLineElement.hpp"
#include "Krystal.Booey/SVG/SVGMetadataElement.hpp"
#include "Krystal.Booey/SVG/SVGPolygonElement.hpp"
#include "Krystal.Booey/SVG/SVGPolylineElement.hpp"
#include "Krystal.Booey/SVG/SVGRectElement.hpp"
#include "Krystal.Booey/SVG/SVGScriptElement.hpp"
#include "Krystal.Booey/SVG/SVGStyleElement.hpp"
#include "Krystal.Booey/SVG/SVGSvgElement.hpp"
#include "Krystal.Booey/SVG/SVGSwitchElement.hpp"
#include "Krystal.Booey/SVG/SVGSymbolElement.hpp"
#include "Krystal.Booey/SVG/SVGTitleElement.hpp"
#include "Krystal.Booey/SVG/SVGUnknownElement.hpp"
#include "Krystal.Booey/SVG/SVGUseElement.hpp"
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

    CHECK(IsCreated<SVGCircleElement>(*document, u8"circle"));
    CHECK(IsCreated<SVGDefsElement>(*document, u8"defs"));
    CHECK(IsCreated<SVGDescElement>(*document, u8"desc"));
    CHECK(IsCreated<SVGEllipseElement>(*document, u8"ellipse"));
    CHECK(IsCreated<SVGForeignObjectElement>(*document, u8"foreignObject"));
    CHECK(IsCreated<SVGGElement>(*document, u8"g"));
    CHECK(IsCreated<SVGImageElement>(*document, u8"image"));
    CHECK(IsCreated<SVGLineElement>(*document, u8"line"));
    CHECK(IsCreated<SVGMetadataElement>(*document, u8"metadata"));
    CHECK(IsCreated<SVGPolygonElement>(*document, u8"polygon"));
    CHECK(IsCreated<SVGPolylineElement>(*document, u8"polyline"));
    CHECK(IsCreated<SVGRectElement>(*document, u8"rect"));
    CHECK(IsCreated<SVGScriptElement>(*document, u8"script"));
    CHECK(IsCreated<SVGStyleElement>(*document, u8"style"));
    CHECK(IsCreated<SVGSvgElement>(*document, u8"svg"));
    CHECK(IsCreated<SVGSwitchElement>(*document, u8"switch"));
    CHECK(IsCreated<SVGSymbolElement>(*document, u8"symbol"));
    CHECK(IsCreated<SVGTitleElement>(*document, u8"title"));
    CHECK(IsCreated<SVGUseElement>(*document, u8"use"));

    // The elements that aren't ported yet, the SVG 1.1 elements SVG 2 removed, and names SVG doesn't have.
    for (auto localName : {u8"a",
                           u8"path",
                           u8"view",
                           u8"linearGradient",
                           u8"stop",
                           u8"clipPath",
                           u8"text",
                           u8"filter",
                           u8"feGaussianBlur",
                           u8"animate",
                           u8"set",
                           u8"font",
                           u8"font-face",
                           u8"missing-glyph",
                           u8"altGlyph",
                           u8"tref",
                           u8"cursor",
                           u8"animateColor",
                           u8"div",
                           u8"x"})
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
    CHECK(Is<SVGUnknownElement>(Downcast<SVGElement>(clipPath)));

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
