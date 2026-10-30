#include "Krystal.Booey/SVG/SVGForeignObjectElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/PageInFrameFixture.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderTreeAsText.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGForeignObject.hpp"
#include "Krystal.Booey/SVG/SVGSvgElement.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::svg::tests
{
  using boo::tests::PageInFrameFixture;

  TEST_CASE("SVGForeignObjectElement: HTML laid out in SVG", "[SVG][SVGForeignObjectElement]")
  {
    PageInFrameFixture fixture(
      u8R"html(<body style="margin: 0"><svg id=outer style="display: block" width=200 height=100>)html"
      u8R"html(<foreignObject id=fo x=10 y=20 width=100 height=50 transform="translate(5 0)">)html"
      u8R"html(<div id=d style="width: 20px; height: 10px; background: green"></div>)html"
      u8R"html(<svg id=inner width=4 height=4></svg></foreignObject>)html"
      u8R"html(<defs><foreignObject id=hidden width=10 height=10><div id=hd></div></foreignObject></defs>)html"
      u8R"html(</svg></body>)html");
    // The parser makes HTML elements in a foreign object, unless they're in an <svg>.
    auto rect = fixture.Document->CreateElementNS(infra::Namespaces::SVG, u8"rect");
    REQUIRE_FALSE(rect.HasException());
    (void)fixture.ById(u8"fo").AppendChild(*rect.ReleaseValue());
    (void)fixture.Document->UpdateLayout();

    // The foreign object is a block the size of its rectangle, placed at its x and y, with its HTML content
    // laid out inside; SVG content other than an <svg> (which starts a fragment of its own, empty here) isn't
    // rendered. The dump writes its untransformed location.
    auto &foreignObject = Downcast<SVGForeignObjectElement>(Downcast<SVGElement>(fixture.ById(u8"fo")));
    REQUIRE(Is<LegacyRenderSVGForeignObject>(foreignObject.Renderer()));
    CHECK_FALSE(foreignObject.LastElementChild()->Renderer());
    auto &inner = Downcast<SVGSvgElement>(Downcast<SVGElement>(fixture.ById(u8"inner")));
    CHECK(inner.IsOutermostSVGSVGElement());
    CHECK(inner.Renderer());
    CHECK(ExternalRepresentation(fixture.Frame.get()) == u8R"(layer at (0,0) size 200x100
  RenderView at (0,0) size 200x100
layer at (0,0) size 200x100
  RenderBlock {HTML} at (0,0) size 200x100
    RenderBody {BODY} at (0,0) size 200x100
      RenderSVGRoot {svg} at (15,20) size 100x50
        RenderSVGForeignObject {foreignObject} at (10,20) size 100x50
          RenderBlock {DIV} at (0,0) size 20x10 [bgcolor=#008000]
          RenderBlock (anonymous) at (0,10) size 100x18
            RenderSVGRoot {svg} at (0,0) size 0x0
        RenderSVGHiddenContainer {defs} at (0,0) size 0x0
)");

    // Foreign objects in hidden containers have no renderer.
    CHECK_FALSE(fixture.ById(u8"hidden").Renderer());
    CHECK_FALSE(fixture.ById(u8"hd").Renderer());

    // Its content is hit in SVG coordinates; the foreign object's own box isn't, as it's only hit tested in
    // the foreground, float and child block background phases.
    CHECK(fixture.Document->ElementFromPoint(16, 21).get() == &fixture.ById(u8"d"));
    CHECK(fixture.Document->ElementFromPoint(40, 21).get() == &fixture.ById(u8"outer"));

    // Changing its rectangle lays it out again.
    (void)foreignObject.SetAttribute(u8"width", u8"60");
    (void)foreignObject.SetAttribute(u8"y", u8"0");
    (void)fixture.Document->UpdateLayout();
    auto &renderer = Downcast<LegacyRenderSVGForeignObject>(*foreignObject.Renderer());
    CHECK(renderer.Width() == 60);
    CHECK(renderer.Location() == LayoutPoint(10, 0));
  }

  TEST_CASE("SVGForeignObjectElement: opacity away from the origin", "[SVG][SVGForeignObjectElement]")
  {
    PageInFrameFixture fixture(
      u8R"html(<body style="margin: 0"><svg style="display: block" width=200 height=100>)html"
      u8R"html(<foreignObject x=50 y=20 width=40 height=30 opacity=0.5>)html"
      u8R"html(<div style="width: 20px; height: 10px; background: #00ff00"></div></foreignObject></svg></body>)html");

    // Opacity clips to the foreign object's repaint rect, which starts at its location, where its content is
    // painted.
    auto painted = fixture.Painted();
    CHECK(painted.find(u8"(clip\n  (rect at (0,0) size 40x30))") != utf8_string::npos);
    CHECK(painted.find(u8"(begin-transparency-layer") != utf8_string::npos);
    CHECK(painted.find(u8"(rect at (0,0) size 20x10)\n  (color #00FF00)") != utf8_string::npos);
  }

  TEST_CASE("SVGForeignObjectElement: positioned content", "[SVG][SVGForeignObjectElement]")
  {
    PageInFrameFixture fixture(
      u8R"html(<body style="margin: 0"><svg style="display: block" width=200 height=100>)html"
      u8R"html(<foreignObject id=fo x=50 y=20 width=100 height=50><div style="height: 10px"></div>)html"
      u8R"html(<div id=abs style="position: absolute; left: 5px; top: 7px; width: 10px; height: 10px"></div>)html"
      u8R"html(</foreignObject></svg></body>)html");
    (void)fixture.Document->UpdateLayout();

    // A foreign object is the containing block of the positioned boxes in it.
    auto &box = Downcast<RenderBox>(*fixture.ById(u8"abs").Renderer());
    CHECK(static_cast<RenderObject *>(box.ContainingBlock()) == fixture.ById(u8"fo").Renderer());
    CHECK(box.Location() == LayoutPoint(5, 7));
    CHECK(box.LocalToAbsolute() == FloatPoint(55, 27));
  }

  // FIXME: The layers of positioned boxes in a foreign object aren't painted, and are hit tested at their
  // location in the page rather than in the foreign object.
  TEST_CASE("SVGForeignObjectElement: positioned content paints and is hit",
            "[SVG][SVGForeignObjectElement][!shouldfail]")
  {
    PageInFrameFixture fixture(
      u8R"html(<body style="margin: 0"><svg style="display: block" width=200 height=100>)html"
      u8R"html(<foreignObject x=50 y=20 width=100 height=50>)html"
      u8R"html(<div id=abs style="position: absolute; left: 5px; top: 7px; width: 10px; height: 10px; background: #00ff00"></div>)html"
      u8R"html(</foreignObject></svg></body>)html");

    CHECK(fixture.Painted().find(u8"#00FF00") != utf8_string::npos);
    CHECK(fixture.Document->ElementFromPoint(57, 29).get() == &fixture.ById(u8"abs"));
  }
}
