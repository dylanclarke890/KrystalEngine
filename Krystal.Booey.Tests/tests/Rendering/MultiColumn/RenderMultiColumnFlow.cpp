#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Geometry/DOMRect.hpp"
#include "Krystal.Booey/DOM/Geometry/DOMRectList.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"

namespace krys::boo::layout::tests
{
  using style::tests::ResolverFixture;

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderMultiColumnFlow: laying out content in columns",
                   "[Rendering][RenderMultiColumnFlow]")
  {
    SECTION("balancing")
    {
      // The content is balanced across the columns, which leave a 1em gap between them.
      ResolverFixture fixture {AhemDocument(u8"#m { columns: 2; width: 200px }",
                                            u8"<div id=m><p>aa bb cc dd</p><p>ee ff</p></div>")};
      CHECK(DumpWithIds(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x30
  RenderBlock {HTML} at (0,0) size 800x30
    RenderBody {BODY} at (0,0) size 800x30
layer at (0,0) size 200x30
  RenderBlock {DIV} at (0,0) size 200x30 id="m"
    RenderMultiColumnSet at (0,0) size 200x30
layer at (0,0) size 95x60
  RenderMultiColumnFlowThread at (0,0) size 95x60
    RenderBlock {P} at (0,10) size 95x20
      RenderText {#text} at (0,0) size 80x20
        text run at (0,0) width 80: "aa bb cc"
        text run at (0,10) width 20: "dd"
    RenderBlock {P} at (0,40) size 95x10
      RenderText {#text} at (0,0) size 50x10
        text run at (0,0) width 50: "ee ff"
)");
    }

    SECTION("column-width")
    {
      // A column width makes as many columns as fit, widened to fill the container.
      ResolverFixture fixture {
        AhemDocument(u8"#m { column-width: 40px; width: 190px }", u8"<div id=m>aa bb cc dd ee</div>")};
      CHECK(DumpWithIds(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x20
  RenderBlock {HTML} at (0,0) size 800x20
    RenderBody {BODY} at (0,0) size 800x20
layer at (0,0) size 190x20
  RenderBlock {DIV} at (0,0) size 190x20 id="m"
    RenderMultiColumnSet at (0,0) size 190x20
layer at (0,0) size 40x50
  RenderMultiColumnFlowThread at (0,0) size 40x50
    RenderText {#text} at (0,0) size 20x50
      text run at (0,0) width 20: "aa"
      text run at (0,10) width 20: "bb"
      text run at (0,20) width 20: "cc"
      text run at (0,30) width 20: "dd"
      text run at (0,40) width 20: "ee"
)");
    }

    SECTION("intrinsic")
    {
      // A shrink-to-fit multi-column container is as wide as its content's columns and gaps.
      ResolverFixture fixture {AhemDocument(u8"#m { columns: 2; float: left }", u8"<div id=m>aa bb</div>")};
      CHECK(DumpWithIds(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x10
  RenderBlock {HTML} at (0,0) size 800x10
    RenderBody {BODY} at (0,0) size 800x0
layer at (0,0) size 110x10
  RenderBlock (floating) {DIV} at (0,0) size 110x10 id="m"
    RenderMultiColumnSet at (0,0) size 110x10
layer at (0,0) size 50x10
  RenderMultiColumnFlowThread at (0,0) size 50x10
    RenderText {#text} at (0,0) size 50x10
      text run at (0,0) width 50: "aa bb"
)");
    }

    SECTION("lines")
    {
      // A line that doesn't fit in the rest of a column moves to the next one.
      ResolverFixture fixture {AhemDocument(
        u8"#m { columns: 2; width: 200px; height: 25px; column-fill: auto; orphans: 1; widows: 1 }",
        u8"<div id=m>aa bb cc dd ee ff gg hh</div>")};
      CHECK(DumpWithIds(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x25
  RenderBlock {HTML} at (0,0) size 800x25
    RenderBody {BODY} at (0,0) size 800x25
layer at (0,0) size 200x25
  RenderBlock {DIV} at (0,0) size 200x25 id="m"
    RenderMultiColumnSet at (0,0) size 200x25
layer at (0,0) size 95x35
  RenderMultiColumnFlowThread at (0,0) size 95x35
    RenderText {#text} at (0,0) size 80x35
      text run at (0,0) width 80: "aa bb cc"
      text run at (0,10) width 80: "dd ee ff"
      text run at (0,25) width 50: "gg hh"
)");
    }

    SECTION("break")
    {
      // A forced break moves the box to the next column.
      ResolverFixture fixture {
        AhemDocument(u8"#m { columns: 2; width: 200px } #b { break-before: column }",
                     u8"<div id=m><div>aa</div><div id=b>bb</div><div>cc</div></div>")};
      CHECK(DumpWithIds(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x20
  RenderBlock {HTML} at (0,0) size 800x20
    RenderBody {BODY} at (0,0) size 800x20
layer at (0,0) size 200x20
  RenderBlock {DIV} at (0,0) size 200x20 id="m"
    RenderMultiColumnSet at (0,0) size 200x20
layer at (0,0) size 95x40
  RenderMultiColumnFlowThread at (0,0) size 95x40
    RenderBlock {DIV} at (0,0) size 95x10
      RenderText {#text} at (0,0) size 20x10
        text run at (0,0) width 20: "aa"
    RenderBlock {DIV} at (0,20) size 95x10 id="b"
      RenderText {#text} at (0,0) size 20x10
        text run at (0,0) width 20: "bb"
    RenderBlock {DIV} at (0,30) size 95x10
      RenderText {#text} at (0,0) size 20x10
        text run at (0,0) width 20: "cc"
)");
    }

    SECTION("avoid")
    {
      // A box that mustn't be split moves to the next column when it doesn't fit.
      ResolverFixture fixture {AhemDocument(u8"#m { columns: 2; width: 200px; height: 30px; column-fill: "
                                            u8"auto } #u { break-inside: avoid; height: 30px }",
                                            u8"<div id=m><div>aa</div><div id=u>uu</div></div>")};
      CHECK(DumpWithIds(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x30
  RenderBlock {HTML} at (0,0) size 800x30
    RenderBody {BODY} at (0,0) size 800x30
layer at (0,0) size 200x30
  RenderBlock {DIV} at (0,0) size 200x30 id="m"
    RenderMultiColumnSet at (0,0) size 200x30
layer at (0,0) size 95x60
  RenderMultiColumnFlowThread at (0,0) size 95x60
    RenderBlock {DIV} at (0,0) size 95x10
      RenderText {#text} at (0,0) size 20x10
        text run at (0,0) width 20: "aa"
    RenderBlock {DIV} at (0,30) size 95x30 id="u"
      RenderText {#text} at (0,0) size 20x10
        text run at (0,0) width 20: "uu"
)");
    }

    SECTION("widows")
    {
      // Lines move to the next column to keep the widows together.
      ResolverFixture fixture {AhemDocument(
        u8"#m { columns: 2; width: 100px; height: 30px; orphans: 1; widows: 2; column-fill: auto }",
        u8"<div id=m>aa bb cc dd</div>")};
      CHECK(DumpWithIds(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x30
  RenderBlock {HTML} at (0,0) size 800x30
    RenderBody {BODY} at (0,0) size 800x30
layer at (0,0) size 100x30
  RenderBlock {DIV} at (0,0) size 100x30 id="m"
    RenderMultiColumnSet at (0,0) size 100x30
layer at (0,0) size 45x50
  RenderMultiColumnFlowThread at (0,0) size 45x50
    RenderText {#text} at (0,0) size 20x50
      text run at (0,0) width 20: "aa"
      text run at (0,10) width 20: "bb"
      text run at (0,30) width 20: "cc"
      text run at (0,40) width 20: "dd"
)");
    }

    SECTION("spanner")
    {
      // A spanner splits the columns into a set before it and one after it.
      ResolverFixture fixture {AhemDocument(u8"#m { columns: 2; width: 200px } #s { column-span: all }",
                                            u8"<div id=m>aa bb<div id=s>ss</div>cc dd</div>")};
      CHECK(DumpWithIds(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x30
  RenderBlock {HTML} at (0,0) size 800x30
    RenderBody {BODY} at (0,0) size 800x30
layer at (0,0) size 200x30
  RenderBlock {DIV} at (0,0) size 200x30 id="m"
    RenderMultiColumnSet at (0,0) size 200x10
    RenderBlock {DIV} at (0,10) size 200x10 id="s"
      RenderText {#text} at (0,0) size 20x10
        text run at (0,0) width 20: "ss"
    RenderMultiColumnSet at (0,20) size 200x10
layer at (0,0) size 95x20
  RenderMultiColumnFlowThread at (0,0) size 95x20
    RenderBlock (anonymous) at (0,0) size 95x10
      RenderText {#text} at (0,0) size 50x10
        text run at (0,0) width 50: "aa bb"
    RenderMultiColumnSpannerPlaceholder at (0,10) size 0x0
    RenderBlock (anonymous) at (0,10) size 95x10
      RenderText {#text} at (0,0) size 50x10
        text run at (0,0) width 50: "cc dd"
)");
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderMultiColumnFlow: geometry in columns",
                   "[Rendering][RenderMultiColumnFlow]")
  {
    // The flow's content is mapped into the columns showing it: b is at the top of the second column, and t
    // is split across both.
    ResolverFixture fixture {
      AhemDocument(u8"#m { columns: 2; width: 200px; position: relative } #b { break-before: column }"
                   u8"#t { columns: 2; width: 200px } #t div { height: 40px }",
                   u8"<div id=m><div>aa</div><div id=b>bb</div></div>"
                   u8"<div id=t><div id=s></div></div>")};
    auto rectIs = [](const dom::DOMRectReadOnly &rect, double x, double y, double width, double height)
    {
      return rect.X() == x && rect.Y() == y && rect.Width() == width && rect.Height() == height;
    };

    auto &b = fixture.ById(u8"b");
    CHECK(rectIs(*b.GetBoundingClientRect(), 105, 0, 95, 10));
    CHECK(b.OffsetLeft() == 105);
    CHECK(b.OffsetTop() == 0);

    auto &s = fixture.ById(u8"s");
    auto rects = s.GetClientRects();
    REQUIRE(rects->Length() == 2u);
    CHECK(rectIs(*rects->Item(0), 0, 10, 95, 20));
    CHECK(rectIs(*rects->Item(1), 105, 10, 95, 20));
    CHECK(rectIs(*s.GetBoundingClientRect(), 0, 10, 200, 20));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderMultiColumnFlow: hit testing in columns",
                   "[Rendering][RenderMultiColumnFlow]")
  {
    // Points in a column hit the content shown there, whether it's in the flow, in a layer of its own or in a
    // transformed layer.
    ResolverFixture fixture {AhemDocument(u8"#m { columns: 2; width: 200px } #m div { height: 10px }"
                                          u8"#r { position: relative } #x { transform: translateX(0) }",
                                          u8"<div id=m><div id=a></div><div id=b></div><div id=r></div>"
                                          u8"<div id=x></div></div>")};
    auto &document = fixture.ById(u8"m").NodeDocument();
    CHECK(document.ElementFromPoint(10, 5).get() == &fixture.ById(u8"a"));
    CHECK(document.ElementFromPoint(10, 15).get() == &fixture.ById(u8"b"));
    CHECK(document.ElementFromPoint(110, 5).get() == &fixture.ById(u8"r"));
    CHECK(document.ElementFromPoint(110, 15).get() == &fixture.ById(u8"x"));
    // The gap between the columns is the container's.
    CHECK(document.ElementFromPoint(100, 5).get() == &fixture.ById(u8"m"));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderMultiColumnFlow: anchors split across columns",
                   "[Rendering][RenderMultiColumnFlow]")
  {
    // An anchor split across the columns is the bounding box of its fragments.
    ResolverFixture fixture {
      AhemDocument(u8"#m { columns: 2; width: 200px; position: relative }"
                   u8"#s { height: 40px; anchor-name: --a }"
                   u8"#p { position: absolute; position-anchor: --a; left: anchor(left);"
                   u8"top: anchor(top); width: anchor-size(width); height: anchor-size(height) }",
                   u8"<div id=m><div id=s></div><div id=p></div></div>")};
    auto rect = fixture.ById(u8"p").GetBoundingClientRect();
    CHECK(rect->X() == 0);
    CHECK(rect->Y() == 0);
    CHECK(rect->Width() == 200);
    CHECK(rect->Height() == 20);
  }
}
