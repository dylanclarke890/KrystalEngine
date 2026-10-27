#include "Krystal.Booey/SVG/SVGViewElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGRoot.hpp"
#include "Krystal.Booey/SVG/SVGSvgElement.hpp"
#include "Krystal.Booey/SVG/SVGTitleElement.hpp"
#include "Krystal.Booey/SVG/SVGTransformList.hpp"
#include "Krystal.Booey/SVG/SVGViewSpec.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::svg::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    template <typename T>
    KRYS_NODISCARD T &ById(dom::Document &document, dom::DOMStringView id) noexcept
    {
      auto element = document.GetElementById(id);
      krys_assert(element);
      return Downcast<T>(Downcast<SVGElement>(*element));
    }

    // Makes an <svg> parsed from @p markup the document element, as in an SVG document.
    void MakeSVGTheDocumentElement(dom::Document &document, dom::DOMStringView id) noexcept
    {
      Ref<dom::Element> svg = ShareRef(*document.GetElementById(id));
      Ref<dom::Element> html = ShareRef(*document.DocumentElement());
      krys_assert(!document.ReplaceChild(*svg, *html).HasException());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGViewElement: svgView fragments", "[SVG][SVGViewElement]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><body style="margin: 0"><svg id=s width=200 height=100 viewBox="0 0 100 50">)html"
      u8R"html(</svg><svg id=t viewBox="0 0 10 10"><g id=target /></svg></body>)html"};
    (void)fixture.Document->UpdateLayout();
    auto &svg = ById<SVGSvgElement>(*fixture.Document, u8"s");
    auto &root = Downcast<LegacyRenderSVGRoot>(*svg.Renderer());
    CHECK(root.LocalToBorderBoxTransform() == AffineTransform(2, 0, 0, 2, 0, 0));

    // A view specification replaces the view box, aspect ratio and zoom and pan, and adds a transform.
    CHECK(svg.ScrollToFragment(u8"svgView(viewBox(0,0,50,25);preserveAspectRatio(none);zoomAndPan(disable);"
                               u8"transform(translate(5 5)))"));
    CHECK(svg.UseCurrentView());
    CHECK(svg.CurrentViewBoxRect() == FloatRect(0, 0, 50, 25));
    CHECK(svg.CurrentView().ZoomAndPan() == SVGZoomAndPanDisable);
    CHECK(svg.CurrentView().TransformString() == u8"translate(5 5)");
    (void)fixture.Document->UpdateLayout();
    CHECK(root.LocalToBorderBoxTransform() == AffineTransform(4, 0, 0, 4, 20, 20));

    // A view target is kept by name.
    auto &other = ById<SVGSvgElement>(*fixture.Document, u8"t");
    CHECK(other.ScrollToFragment(u8"svgView(viewTarget(target))"));
    CHECK(other.CurrentView().ViewTarget().get() == fixture.Document->GetElementById(u8"target").get());

    // A view specification that doesn't parse resets the view.
    CHECK_FALSE(svg.ScrollToFragment(u8"svgView(scale(2))"));
    CHECK_FALSE(svg.UseCurrentView());
    CHECK(svg.CurrentViewBoxRect() == FloatRect(0, 0, 100, 50));
    (void)fixture.Document->UpdateLayout();
    CHECK(root.LocalToBorderBoxTransform() == AffineTransform(2, 0, 0, 2, 0, 0));

    // Neither does anything other than a view specification or a <view>.
    CHECK_FALSE(svg.ScrollToFragment(u8"other"));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGViewElement: views of the document's <svg>",
                   "[SVG][SVGViewElement]")
  {
    auto document = ResolverFixture::Parse(
      u8R"html(<svg id=s viewBox="0 0 100 50" preserveAspectRatio="xMinYMin"><view id=v viewBox="0 0 20 10" />)html"
      u8R"html(<view id=w zoomAndPan=disable /></svg>)html");
    MakeSVGTheDocumentElement(*document, u8"s");
    auto &svg = ById<SVGSvgElement>(*document, u8"s");
    auto &view = ById<SVGViewElement>(*document, u8"v");

    // A <view> overrides the view attributes it has on the document's <svg>.
    CHECK(svg.ScrollToFragment(u8"v"));
    CHECK(svg.UseCurrentView());
    CHECK(svg.CurrentViewBoxRect() == FloatRect(0, 0, 20, 10));
    CHECK(svg.CurrentView().PreserveAspectRatio().Align()
          == SVGPreserveAspectRatioValue::SVG_PRESERVEASPECTRATIO_XMINYMIN);
    CHECK(view.TargetElement() == &svg);

    // Changes to the view's attributes follow.
    REQUIRE_FALSE(view.SetAttribute(u8"viewBox", u8"0 0 40 20").HasException());
    CHECK(svg.CurrentViewBoxRect() == FloatRect(0, 0, 40, 20));

    // Another view replaces it.
    auto &other = ById<SVGViewElement>(*document, u8"w");
    CHECK(svg.ScrollToFragment(u8"w"));
    CHECK(svg.CurrentViewBoxRect() == FloatRect(0, 0, 100, 50));
    CHECK(svg.CurrentView().ZoomAndPan() == SVGZoomAndPanDisable);
    CHECK(other.TargetElement() == &svg);
    CHECK(view.TargetElement() == nullptr);

    // Resetting the scroll anchor goes back to the <svg>'s own view.
    svg.ResetScrollAnchor();
    CHECK_FALSE(svg.UseCurrentView());
    CHECK(svg.CurrentViewBoxRect() == FloatRect(0, 0, 100, 50));

    // A <view> is never rendered.
    CHECK(view.Renderer() == nullptr);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGTitleElement: the document's title",
                   "[SVG][SVGTitleElement]")
  {
    auto document = ResolverFixture::Parse(u8R"html(<svg id=s><g><title>Not this</title></g></svg>)html");
    MakeSVGTheDocumentElement(*document, u8"s");

    // Only a <title> that's a child of the document's <svg> counts.
    CHECK(document->Title().empty());

    // Setting the title inserts one as its first child.
    REQUIRE_FALSE(document->Title(u8"  A   title ").HasException());
    auto first = document->DocumentElement()->FirstElementChild();
    REQUIRE(first);
    CHECK(Is<SVGTitleElement>(Downcast<SVGElement>(*first)));
    CHECK(document->Title() == u8"A title");

    // And replaces its text after that.
    REQUIRE_FALSE(document->Title(u8"Another").HasException());
    CHECK(dom::TextAlgorithms::ChildTextContent(*first) == u8"Another");
    CHECK(document->DocumentElement()->ChildElementCount() == 2);
  }
}
