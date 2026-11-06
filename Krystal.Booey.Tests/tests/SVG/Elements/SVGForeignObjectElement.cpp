#include "Krystal.Booey/SVG/Elements/SVGForeignObjectElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/PageInFrameFixture.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderTreeAsText.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGForeignObject.hpp"
#include "Krystal.Booey/SVG/Elements/SVGSvgElement.hpp"
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

  TEST_CASE("SVGForeignObjectElement: positioned content paints and is hit in it",
            "[SVG][SVGForeignObjectElement]")
  {
    auto markup = [](utf8_stringview foreignObjectAttributes, utf8_stringview content)
    {
      return u8R"html(<body style="margin: 0"><svg style="display: block" width=200 height=100>)html"
             u8"<foreignObject x=50 y=20 width=100 height=50 "
             + utf8_string(foreignObjectAttributes) + u8">" + utf8_string(content)
             + u8"</foreignObject></svg></body>";
    };
    constexpr auto box =
      u8"position: absolute; left: 5px; top: 7px; width: 10px; height: 10px; background: #0f0";

    SECTION("absolutely positioned, inside the foreign object's coordinates")
    {
      PageInFrameFixture fixture(markup({}, u8"<div id=abs style=\"" + utf8_string(box) + u8"\"></div>"));
      auto painted = fixture.Painted();
      auto fill = painted.find(u8"#00FF00");
      REQUIRE(fill != utf8_string::npos);
      // Within the save that moves to the foreign object.
      auto move = painted.find(u8"t=(50.00,20.00)");
      REQUIRE(move != utf8_string::npos);
      CHECK(move < fill);
      CHECK(painted.find(u8"(restore)", move) > fill);

      CHECK(fixture.Document->ElementFromPoint(57, 29).get() == &fixture.ById(u8"abs"));
      CHECK(fixture.Document->ElementFromPoint(7, 9).get() != &fixture.ById(u8"abs"));
    }

    SECTION("with the foreign object's transform")
    {
      PageInFrameFixture fixture(markup(u8"transform='translate(20 10)'",
                                        u8"<div id=abs style=\"" + utf8_string(box) + u8"\"></div>"));
      CHECK(fixture.Document->ElementFromPoint(77, 39).get() == &fixture.ById(u8"abs"));
      CHECK(fixture.Document->ElementFromPoint(57, 29).get() != &fixture.ById(u8"abs"));
    }

    SECTION("relatively positioned and in flow")
    {
      PageInFrameFixture fixture(markup(
        {},
        u8R"html(<div style="height: 5px"></div>)html"
        u8R"html(<div id=rel style="position: relative; left: 3px; height: 10px; width: 10px"></div>)html"));
      CHECK(fixture.Document->ElementFromPoint(55, 29).get() == &fixture.ById(u8"rel"));
      CHECK(fixture.Document->ElementFromPoint(51, 29).get() != &fixture.ById(u8"rel"));
    }

    SECTION("with z-index ordering it among the content")
    {
      PageInFrameFixture fixture(markup(
        {},
        u8R"html(<div id=flow style="height: 30px"></div>)html"
        u8R"html(<div id=under style="position: absolute; z-index: -1; top: 0; width: 50px; height: 50px"></div>)html"
        u8R"html(<div id=over style="position: absolute; z-index: 1; top: 0; left: 60px; width: 10px; height: 10px"></div>)html"));
      // Under the block in flow, and hit where the block isn't.
      CHECK(fixture.Document->ElementFromPoint(60, 30).get() == &fixture.ById(u8"flow"));
      CHECK(fixture.Document->ElementFromPoint(60, 60).get() == &fixture.ById(u8"under"));
      // Over the block in flow.
      CHECK(fixture.Document->ElementFromPoint(115, 25).get() == &fixture.ById(u8"over"));
    }

    SECTION("clipped by a foreign object that clips")
    {
      PageInFrameFixture fixture(markup(
        {},
        u8R"html(<div id=abs style="position: absolute; left: 95px; top: 0; width: 40px; height: 10px"></div>)html"));
      CHECK(fixture.Document->ElementFromPoint(148, 25).get() == &fixture.ById(u8"abs"));
      CHECK(fixture.Document->ElementFromPoint(160, 25).get() != &fixture.ById(u8"abs"));
    }

    SECTION("restyled and removed")
    {
      PageInFrameFixture fixture(markup({}, u8"<div id=abs style=\"" + utf8_string(box) + u8"\"></div>"));
      CHECK(fixture.Document->ElementFromPoint(57, 29).get() == &fixture.ById(u8"abs"));
      Ref<dom::Element> element = ShareRef(fixture.ById(u8"abs"));
      REQUIRE_FALSE(
        element
          ->SetAttribute(u8"style", u8"position: absolute; left: 20px; top: 7px; width: 10px; height: 10px")
          .HasException());
      (void)fixture.Document->UpdateLayout();
      CHECK(fixture.Document->ElementFromPoint(72, 29).get() == element.get());
      CHECK(fixture.Document->ElementFromPoint(57, 29).get() != element.get());

      REQUIRE_FALSE(element->ParentNode()->RemoveChild(*element).HasException());
      (void)fixture.Document->UpdateLayout();
      CHECK(fixture.Document->ElementFromPoint(72, 29).get() != element.get());
      CHECK_FALSE(fixture.Painted().empty());
    }
  }
}
