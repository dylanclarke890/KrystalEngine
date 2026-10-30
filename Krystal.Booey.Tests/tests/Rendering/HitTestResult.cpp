#include "Krystal.Booey/Rendering/HitTestResult.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Text.hpp"

namespace krys::boo::tests
{
  namespace
  {
    /// @brief A 200x100 view of @p markup, after a doctype and a style sheet with no body margin.
    struct HitFixture : PaintFixture
    {
      explicit HitFixture(utf8_stringview markup) noexcept
          : PaintFixture(u8"<!DOCTYPE html><style>body{margin:0;font:16px sans-serif}</style>"
                         + utf8_string(markup))
      {
        (void)Document->UpdateLayout();
      }

      /// @brief The element at the point, or null.
      KRYS_NODISCARD dom::Element *Hit(double x, double y) noexcept
      {
        return Document->ElementFromPoint(x, y).get();
      }

      KRYS_NODISCARD dom::Element *Body() noexcept
      {
        return Document->Body().get();
      }
    };
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Hit testing: the topmost box", "[Rendering][HitTestResult]")
  {
    HitFixture fixture {u8"<style>div{position:absolute;width:50px;height:50px}</style>"
                        u8"<div id=a style=left:0;top:0></div><div id=b style=left:25px;top:25px></div>"};
    CHECK(fixture.Hit(10, 10) == &fixture.ById(u8"a"));
    // Later boxes are in front.
    CHECK(fixture.Hit(30, 30) == &fixture.ById(u8"b"));
    CHECK(fixture.Hit(70, 70) == &fixture.ById(u8"b"));
    // Nothing but the root elements.
    CHECK(fixture.Hit(150, 50) == fixture.Document->DocumentElement().get());
    // Outside the viewport.
    CHECK(fixture.Hit(250, 50) == nullptr);
    CHECK(fixture.Hit(-1, 50) == nullptr);

    SECTION("the local point is in the box's coordinates")
    {
      HitTestResult result(LayoutPoint(30, 40));
      fixture.Document->HitTest(HitTestRequest(), result);
      CHECK(result.InnerNode() == &fixture.ById(u8"b"));
      CHECK(result.LocalPoint() == LayoutPoint(5, 15));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Hit testing: z-index", "[Rendering][HitTestResult]")
  {
    HitFixture fixture {u8"<style>div{position:absolute;width:50px;height:50px;left:0;top:0}</style>"
                        u8"<div id=a style=z-index:2></div><div id=b style=z-index:1></div>"
                        u8"<div id=c style=z-index:-1;width:100px></div>"};
    CHECK(fixture.Hit(10, 10) == &fixture.ById(u8"a"));
    // Negative z-index layers are behind the body's content but in front of nothing else.
    CHECK(fixture.Hit(75, 10) == &fixture.ById(u8"c"));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Hit testing: transforms", "[Rendering][HitTestResult]")
  {
    HitFixture fixture {u8"<style>div{width:40px;height:40px}</style>"
                        u8"<div id=t style=transform:translateX(100px)></div>"
                        u8"<div id=r style=margin-left:20px;transform:rotate(45deg)></div>"};
    CHECK(fixture.Hit(110, 10) == &fixture.ById(u8"t"));
    CHECK(fixture.Hit(10, 10) != &fixture.ById(u8"t"));

    // The rotated square's centre is at (40, 60): its corners now point up, down, left and right.
    CHECK(fixture.Hit(40, 60) == &fixture.ById(u8"r"));
    CHECK(fixture.Hit(40, 36) == &fixture.ById(u8"r"));
    CHECK(fixture.Hit(22, 42) != &fixture.ById(u8"r"));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Hit testing: rounded corners and clip paths",
                   "[Rendering][HitTestResult]")
  {
    HitFixture fixture {u8"<style>div{position:absolute;top:0;width:60px;height:60px}</style>"
                        u8"<div id=r style=left:0;border-radius:30px></div>"
                        u8"<div id=c style=left:100px;clip-path:circle(50%)></div>"};
    CHECK(fixture.Hit(30, 30) == &fixture.ById(u8"r"));
    CHECK(fixture.Hit(3, 3) == fixture.Document->DocumentElement().get());
    CHECK(fixture.Hit(130, 30) == &fixture.ById(u8"c"));
    CHECK(fixture.Hit(103, 3) == fixture.Document->DocumentElement().get());
    CHECK(fixture.Hit(157, 57) == fixture.Document->DocumentElement().get());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Hit testing: pointer-events and visibility",
                   "[Rendering][HitTestResult]")
  {
    HitFixture fixture {u8"<style>div{position:absolute;left:0;top:0;width:50px;height:50px}</style>"
                        u8"<div id=under></div><div id=none style=pointer-events:none></div>"
                        u8"<div id=hidden style=visibility:hidden;left:100px></div>"};
    CHECK(fixture.Hit(10, 10) == &fixture.ById(u8"under"));
    CHECK(fixture.Hit(110, 10) == fixture.Document->DocumentElement().get());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Hit testing: overflow clips", "[Rendering][HitTestResult]")
  {
    HitFixture fixture {u8"<div id=clip style=width:50px;height:50px;overflow:hidden>"
                        u8"<div id=child style=width:150px;height:20px></div></div>"};
    CHECK(fixture.Hit(10, 10) == &fixture.ById(u8"child"));
    CHECK(fixture.Hit(10, 30) == &fixture.ById(u8"clip"));
    // The child's overflow is clipped.
    CHECK(fixture.Hit(100, 10) != &fixture.ById(u8"child"));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Hit testing: text and inline boxes",
                   "[Rendering][HitTestResult]")
  {
    HitFixture fixture {u8"<div style=width:40px;line-height:20px><span id=s>aaaa bbbb</span></div>"
                        u8"<p id=p style=margin:0;line-height:20px>cc<em id=e>dd</em></p>"};
    // The span's first and second lines.
    CHECK(fixture.Hit(2, 10) == &fixture.ById(u8"s"));
    CHECK(fixture.Hit(2, 30) == &fixture.ById(u8"s"));
    // Past the end of the line, the block.
    CHECK(fixture.Hit(150, 10) != &fixture.ById(u8"s"));

    // Text is reported as its parent element; the inner node is the text itself.
    CHECK(fixture.Hit(2, 50) == &fixture.ById(u8"p"));
    HitTestResult result(LayoutPoint(2, 50));
    fixture.Document->HitTest(HitTestRequest(), result);
    CHECK(Is<dom::Text>(result.InnerNode()));
    CHECK(fixture.Hit(22, 50) == &fixture.ById(u8"e"));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Hit testing: floats", "[Rendering][HitTestResult]")
  {
    HitFixture fixture {
      u8"<div id=block style=height:60px><div id=f style=float:right;width:30px;height:30px>"
      u8"</div></div>"};
    CHECK(fixture.Hit(185, 10) == &fixture.ById(u8"f"));
    CHECK(fixture.Hit(185, 45) == &fixture.ById(u8"block"));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Hit testing: tables", "[Rendering][HitTestResult]")
  {
    HitFixture fixture {u8"<style>td{width:40px;height:20px;padding:0}</style>"
                        u8"<table id=t style=border-spacing:10px><tr><td id=a></td><td id=b></td></tr>"
                        u8"<tr><td id=c></td><td id=d></td></tr></table>"};
    CHECK(fixture.Hit(20, 20) == &fixture.ById(u8"a"));
    CHECK(fixture.Hit(70, 20) == &fixture.ById(u8"b"));
    CHECK(fixture.Hit(20, 50) == &fixture.ById(u8"c"));
    CHECK(fixture.Hit(70, 50) == &fixture.ById(u8"d"));
    // The spacing between cells is the table's.
    CHECK(fixture.Hit(55, 20) == &fixture.ById(u8"t"));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Hit testing: flex and grid items", "[Rendering][HitTestResult]")
  {
    HitFixture fixture {
      u8"<style>span{width:40px;height:20px}#g{display:grid;grid-template-columns:40px 40px}</style>"
      u8"<div style=display:flex><span id=a></span><span id=b style=order:-1></span></div>"
      u8"<div id=g>"
      u8"<span id=c></span><span id=d></span></div>"};
    // Order moves b before a.
    CHECK(fixture.Hit(10, 10) == &fixture.ById(u8"b"));
    CHECK(fixture.Hit(50, 10) == &fixture.ById(u8"a"));
    CHECK(fixture.Hit(10, 30) == &fixture.ById(u8"c"));
    CHECK(fixture.Hit(50, 30) == &fixture.ById(u8"d"));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Hit testing: elementsFromPoint", "[Rendering][HitTestResult]")
  {
    HitFixture fixture {u8"<div id=outer style=padding:10px><div id=inner style=height:20px></div></div>"};
    auto elements = fixture.Document->ElementsFromPoint(15, 15);
    REQUIRE(elements.size() == 4);
    CHECK(elements[0] == &fixture.ById(u8"inner"));
    CHECK(elements[1] == &fixture.ById(u8"outer"));
    CHECK(elements[2] == fixture.Body());
    CHECK(elements[3] == fixture.Document->DocumentElement().get());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Hit testing: hover", "[Rendering][HitTestResult]")
  {
    HitFixture fixture {u8"<style>#a{height:20px}#a:hover{height:40px}</style><div id=a></div>"};
    auto &document = *fixture.Document;
    auto &a = fixture.ById(u8"a");
    auto HeightOf = [](dom::Element &element)
    {
      return style::tests::ResolverFixture::Serialised(*element.GetRenderStyle(),
                                                       element.GetRenderStyle()->Height());
    };
    auto hoverAt = [&](int32 x, int32 y)
    {
      HitTestRequest request(EnumSet<HitTestRequest::Type> {HitTestRequest::Type::Move});
      HitTestResult result(LayoutPoint(x, y));
      document.HitTest(request, result);
      document.UpdateHoverActiveState(request, result.TargetElement());
      (void)document.UpdateLayout();
    };

    hoverAt(10, 10);
    CHECK(a.Hovered());
    CHECK(fixture.Body()->Hovered());
    CHECK(HeightOf(a) == "40px");
    // The box grew, so it's still hovered further down.
    hoverAt(10, 30);
    CHECK(a.Hovered());
    hoverAt(10, 60);
    CHECK_FALSE(a.Hovered());
    CHECK(HeightOf(a) == "20px");
  }
}
