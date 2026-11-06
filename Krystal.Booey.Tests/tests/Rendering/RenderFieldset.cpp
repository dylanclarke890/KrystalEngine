#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"
#include "Krystal.Booey.Tests/Animation/AnimationTestUtils.hpp"
#include "Krystal.Booey/DOM/Geometry/DOMRect.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/Rendering/HitTesting/HitTestResult.hpp"

namespace krys::boo::tests
{
  namespace
  {
    /// @brief A box as Chrome reports it: x, y, width, height.
    struct Rect
    {
      double X;
      double Y;
      double Width;
      double Height;

      bool operator==(const Rect &) const noexcept = default;
    };

    constexpr utf8_stringview BaseStyle =
      u8"body{margin:0}fieldset{margin:0;padding:0;border:10px solid #000;width:300px;box-sizing:content-box}"
      u8"legend{padding:0;width:50px;height:20px}#c{height:30px}";

    KRYS_NODISCARD Maybe<Rect> RectOf(AnimationPageFixture &fixture, dom::DOMStringView id) noexcept
    {
      auto element = fixture.Document->GetElementById(id);
      if (!element)
      {
        return null;
      }
      auto rect = element->GetBoundingClientRect();
      return Rect {rect->X(), rect->Y(), rect->Width(), rect->Height()};
    }

    std::ostream &operator<<(std::ostream &stream, const Rect &rect)
    {
      return stream << rect.X << "," << rect.Y << " " << rect.Width << "x" << rect.Height;
    }
  }

  TEST_CASE("RenderBlock: a fieldset's legend, as Chrome lays it out",
            "[Rendering][RenderBlock][html:the-fieldset-and-legend-elements]")
  {
    struct Case
    {
      const char8_t *Name;
      const char8_t *Style;
      const char8_t *Body;
      Maybe<Rect> Fieldset;
      Maybe<Rect> Legend;
      Maybe<Rect> SecondLegend;
      Maybe<Rect> Content;
    };
    auto testCase = GENERATE(values<Case>({
      {u8"no legend", u8"", u8"<div id=c></div>", Rect {0, 0, 320, 50}, null, null, Rect {10, 10, 300, 30}},
      {u8"basic", u8"", u8"<legend id=l></legend><div id=c></div>", Rect {0, 0, 320, 60},
       Rect {10, 0, 50, 20}, null, Rect {10, 20, 300, 30}},
      {u8"short legend", u8"legend{height:4px}", u8"<legend id=l></legend><div id=c></div>",
       Rect {0, 0, 320, 50}, Rect {10, 3, 50, 4}, null, Rect {10, 10, 300, 30}},
      {u8"thin border", u8"fieldset{border-width:2px}", u8"<legend id=l></legend><div id=c></div>",
       Rect {0, 0, 304, 52}, Rect {2, 0, 50, 20}, null, Rect {2, 20, 300, 30}},
      {u8"padding", u8"fieldset{padding:7px}", u8"<legend id=l></legend><div id=c></div>",
       Rect {0, 0, 334, 74}, Rect {17, 0, 50, 20}, null, Rect {17, 27, 300, 30}},
      {u8"text-align center", u8"legend{text-align:center}", u8"<legend id=l></legend><div id=c></div>",
       Rect {0, 0, 320, 60}, Rect {135, 0, 50, 20}, null, Rect {10, 20, 300, 30}},
      {u8"text-align right", u8"legend{text-align:right}", u8"<legend id=l></legend><div id=c></div>",
       Rect {0, 0, 320, 60}, Rect {260, 0, 50, 20}, null, Rect {10, 20, 300, 30}},
      {u8"margin-left auto", u8"legend{margin-left:auto}", u8"<legend id=l></legend><div id=c></div>",
       Rect {0, 0, 320, 60}, Rect {260, 0, 50, 20}, null, Rect {10, 20, 300, 30}},
      {u8"margin auto", u8"legend{margin-inline:auto}", u8"<legend id=l></legend><div id=c></div>",
       Rect {0, 0, 320, 60}, Rect {135, 0, 50, 20}, null, Rect {10, 20, 300, 30}},
      {u8"margin-left 15", u8"legend{margin-left:15px}", u8"<legend id=l></legend><div id=c></div>",
       Rect {0, 0, 320, 60}, Rect {25, 0, 50, 20}, null, Rect {10, 20, 300, 30}},
      {u8"align center", u8"", u8"<legend id=l align=center></legend><div id=c></div>", Rect {0, 0, 320, 60},
       Rect {135, 0, 50, 20}, null, Rect {10, 20, 300, 30}},
      {u8"align right", u8"", u8"<legend id=l align=right></legend><div id=c></div>", Rect {0, 0, 320, 60},
       Rect {260, 0, 50, 20}, null, Rect {10, 20, 300, 30}},
      {u8"justify-self center", u8"legend{justify-self:center}", u8"<legend id=l></legend><div id=c></div>",
       Rect {0, 0, 320, 60}, Rect {10, 0, 50, 20}, null, Rect {10, 20, 300, 30}},
      {u8"justify-self end", u8"legend{justify-self:end}", u8"<legend id=l></legend><div id=c></div>",
       Rect {0, 0, 320, 60}, Rect {10, 0, 50, 20}, null, Rect {10, 20, 300, 30}},
      {u8"rtl", u8"fieldset{direction:rtl}", u8"<legend id=l></legend><div id=c></div>", Rect {0, 0, 320, 60},
       Rect {260, 0, 50, 20}, null, Rect {10, 20, 300, 30}},
      {u8"rtl margin-right 15", u8"fieldset{direction:rtl}legend{margin-right:15px}",
       u8"<legend id=l></legend><div id=c></div>", Rect {0, 0, 320, 60}, Rect {245, 0, 50, 20}, null,
       Rect {10, 20, 300, 30}},
      {u8"margin-bottom", u8"legend{margin-bottom:9px}", u8"<legend id=l></legend><div id=c></div>",
       Rect {0, 0, 320, 69}, Rect {10, 0, 50, 20}, null, Rect {10, 29, 300, 30}},
      {u8"margin-top", u8"legend{margin-top:9px}", u8"<legend id=l></legend><div id=c></div>",
       Rect {0, 0, 320, 60}, Rect {10, 0, 50, 20}, null, Rect {10, 20, 300, 30}},
      {u8"margin-top short", u8"legend{margin-top:3px;height:4px}",
       u8"<legend id=l></legend><div id=c></div>", Rect {0, 0, 320, 50}, Rect {10, 3, 50, 4}, null,
       Rect {10, 10, 300, 30}},
      {u8"height content-box", u8"fieldset{height:100px}", u8"<legend id=l></legend><div id=c></div>",
       Rect {0, 0, 320, 120}, Rect {10, 0, 50, 20}, null, Rect {10, 20, 300, 30}},
      {u8"height border-box", u8"fieldset{height:100px;box-sizing:border-box}",
       u8"<legend id=l></legend><div id=c></div>", Rect {0, 0, 300, 100}, Rect {10, 0, 50, 20}, null,
       Rect {10, 20, 280, 30}},
      {u8"min-height", u8"fieldset{min-height:100px}", u8"<legend id=l></legend><div id=c></div>",
       Rect {0, 0, 320, 120}, Rect {10, 0, 50, 20}, null, Rect {10, 20, 300, 30}},
      {u8"floated legend", u8"legend{float:left}", u8"<legend id=l></legend><div id=c></div>",
       Rect {0, 0, 320, 50}, Rect {10, 10, 50, 20}, null, Rect {10, 10, 300, 30}},
      {u8"abspos legend", u8"legend{position:absolute}", u8"<legend id=l></legend><div id=c></div>",
       Rect {0, 0, 320, 50}, Rect {10, 10, 50, 20}, null, Rect {10, 10, 300, 30}},
      {u8"relative legend", u8"legend{position:relative;left:5px;top:5px}",
       u8"<legend id=l></legend><div id=c></div>", Rect {0, 0, 320, 60}, Rect {15, 5, 50, 20}, null,
       Rect {10, 20, 300, 30}},
      {u8"second legend", u8"", u8"<legend id=l></legend><legend id=m></legend><div id=c></div>",
       Rect {0, 0, 320, 80}, Rect {10, 0, 50, 20}, Rect {10, 20, 50, 20}, Rect {10, 40, 300, 30}},
      {u8"legend not first", u8"", u8"<div id=c></div><legend id=l></legend>", Rect {0, 0, 320, 60},
       Rect {10, 0, 50, 20}, null, Rect {10, 20, 300, 30}},
      {u8"wide legend", u8"legend{width:400px}", u8"<legend id=l></legend><div id=c></div>",
       Rect {0, 0, 420, 60}, Rect {10, 0, 400, 20}, null, Rect {10, 20, 400, 30}},
      {u8"shrink to legend", u8"fieldset{width:auto;display:inline-block}legend{width:400px}",
       u8"<legend id=l></legend><div id=c></div>", Rect {0, 0, 420, 60}, Rect {10, 0, 400, 20}, null,
       Rect {10, 20, 400, 30}},
      {u8"shrink to content", u8"fieldset{width:auto;display:inline-block}#c{width:80px}",
       u8"<legend id=l></legend><div id=c></div>", Rect {0, 0, 100, 60}, Rect {10, 0, 50, 20}, null,
       Rect {10, 20, 80, 30}},
      {u8"auto width legend", u8"legend{width:auto}",
       u8"<legend id=l><div style='width:70px;height:20px'></div></legend><div id=c></div>",
       Rect {0, 0, 320, 60}, Rect {10, 0, 70, 20}, null, Rect {10, 20, 300, 30}},
      {u8"min-content", u8"fieldset{width:10px}legend{width:auto}",
       u8"<legend id=l><div style='width:70px;height:20px'></div></legend><div id=c></div>",
       Rect {0, 0, 90, 60}, Rect {10, 0, 70, 20}, null, Rect {10, 20, 70, 30}},
      {u8"vertical-rl",
       u8"fieldset{writing-mode:vertical-rl;height:200px;width:auto}#c{height:auto;width:30px}",
       u8"<legend id=l></legend><div id=c></div>", Rect {0, 0, 90, 220}, Rect {40, 10, 50, 20}, null,
       Rect {10, 10, 30, 200}},
      {u8"vertical-lr",
       u8"fieldset{writing-mode:vertical-lr;height:200px;width:auto}#c{height:auto;width:30px}",
       u8"<legend id=l></legend><div id=c></div>", Rect {0, 0, 90, 220}, Rect {0, 10, 50, 20}, null,
       Rect {50, 10, 30, 200}},
      {u8"display flex", u8"fieldset{display:flex}#c{width:40px}", u8"<legend id=l></legend><div id=c></div>",
       Rect {0, 0, 320, 60}, Rect {10, 0, 50, 20}, null, Rect {10, 20, 40, 30}},
      {u8"display grid", u8"fieldset{display:grid}", u8"<legend id=l></legend><div id=c></div>",
       Rect {0, 0, 320, 60}, Rect {10, 0, 50, 20}, null, Rect {10, 20, 300, 30}},
      {u8"overflow auto", u8"fieldset{overflow:auto;height:60px;scrollbar-width:none}#c{height:300px}",
       u8"<legend id=l></legend><div id=c></div>", Rect {0, 0, 320, 80}, Rect {10, 0, 50, 20}, null,
       Rect {10, 20, 300, 300}},
      {u8"display inline", u8"fieldset{display:inline}", u8"<legend id=l></legend><div id=c></div>",
       Rect {0, 0, 320, 60}, Rect {10, 0, 50, 20}, null, Rect {10, 20, 300, 30}},
      {u8"hidden legend", u8"legend{display:none}", u8"<legend id=l></legend><div id=c></div>",
       Rect {0, 0, 320, 50}, Rect {0, 0, 0, 0}, null, Rect {10, 10, 300, 30}},
      {u8"nested in div", u8"", u8"<div><legend id=l></legend></div><div id=c></div>", Rect {0, 0, 320, 70},
       Rect {10, 10, 50, 20}, null, Rect {10, 30, 300, 30}},
    }));

    AnimationPageFixture fixture {u8"<!DOCTYPE html><style>" + utf8_string(BaseStyle) + testCase.Style
                                  + u8"</style><fieldset id=f>" + testCase.Body + u8"</fieldset>"};
    (void)fixture.Document->UpdateLayout();

    INFO(reinterpret_cast<const char *>(testCase.Name));
    if (auto rect = RectOf(fixture, u8"f"); testCase.Fieldset)
    {
      REQUIRE(rect);
      CHECK(*rect == *testCase.Fieldset);
    }
    if (auto rect = RectOf(fixture, u8"l"); testCase.Legend)
    {
      REQUIRE(rect);
      CHECK(*rect == *testCase.Legend);
    }
    if (auto rect = RectOf(fixture, u8"m"); testCase.SecondLegend)
    {
      REQUIRE(rect);
      CHECK(*rect == *testCase.SecondLegend);
    }
    if (auto rect = RectOf(fixture, u8"c"); testCase.Content)
    {
      REQUIRE(rect);
      CHECK(*rect == *testCase.Content);
    }
  }

  TEST_CASE("RenderBlock: a fieldset's legend changing", "[Rendering][RenderBlock]")
  {
    AnimationPageFixture fixture {
      u8"<!DOCTYPE html><style>" + utf8_string(BaseStyle)
      + u8"</style><fieldset id=f><legend id=l></legend><div id=c></div></fieldset>"};
    (void)fixture.Document->UpdateLayout();
    REQUIRE(RectOf(fixture, u8"c")->Y == 20);

    SECTION("removing it takes back what it added to the border")
    {
      REQUIRE_FALSE(fixture.ById(u8"f").RemoveChild(fixture.ById(u8"l")).HasException());
      (void)fixture.Document->UpdateLayout();
      CHECK(RectOf(fixture, u8"c")->Y == 10);
      CHECK(RectOf(fixture, u8"f")->Height == 50);
    }

    SECTION("floating it returns it to normal layout")
    {
      fixture.SetStyle(u8"l", u8"float:left");
      (void)fixture.Document->UpdateLayout();
      CHECK(*RectOf(fixture, u8"l") == Rect {10, 10, 50, 20});
      CHECK(RectOf(fixture, u8"f")->Height == 50);
    }

    SECTION("making it absolutely positioned returns it to normal layout")
    {
      fixture.SetStyle(u8"l", u8"position:absolute");
      (void)fixture.Document->UpdateLayout();
      CHECK(*RectOf(fixture, u8"l") == Rect {10, 10, 50, 20});
      CHECK(RectOf(fixture, u8"f")->Height == 50);
      CHECK(RectOf(fixture, u8"c")->Y == 10);
    }

    SECTION("a taller one moves the contents")
    {
      fixture.SetStyle(u8"l", u8"height:40px");
      (void)fixture.Document->UpdateLayout();
      CHECK(RectOf(fixture, u8"c")->Y == 40);
    }
  }

  TEST_CASE("RenderBlock: hit testing a fieldset's legend", "[Rendering][RenderBlock]")
  {
    SECTION("in the border")
    {
      AnimationPageFixture fixture {
        u8"<!DOCTYPE html><style>" + utf8_string(BaseStyle)
        + u8"</style><fieldset id=f><legend id=l></legend><div id=c></div></fieldset>"};
      (void)fixture.Document->UpdateLayout();
      CHECK(fixture.Document->ElementFromPoint(15, 5).get() == &fixture.ById(u8"l"));
      CHECK(fixture.Document->ElementFromPoint(100, 5).get() == &fixture.ById(u8"f"));
      CHECK(fixture.Document->ElementFromPoint(15, 25).get() == &fixture.ById(u8"c"));
    }

    SECTION("when the contents scroll")
    {
      AnimationPageFixture fixture {
        u8"<!DOCTYPE html><style>" + utf8_string(BaseStyle)
        + u8"fieldset{overflow:auto;height:60px;scrollbar-width:none}#c{height:300px}</style>"
          u8"<fieldset id=f><legend id=l></legend><div id=c></div></fieldset>"};
      (void)fixture.Document->UpdateLayout();
      fixture.ById(u8"f").SetScrollTop(40);
      (void)fixture.Document->UpdateLayout();
      // The legend stays where it is: it's outside what scrolls.
      CHECK(*RectOf(fixture, u8"l") == Rect {10, 0, 50, 20});
      CHECK(fixture.Document->ElementFromPoint(15, 5).get() == &fixture.ById(u8"l"));
      CHECK(fixture.Document->ElementFromPoint(15, 25).get() == &fixture.ById(u8"c"));
      CHECK(fixture.Document->ElementFromPoint(15, 75).get() == &fixture.ById(u8"f"));
      CHECK(*RectOf(fixture, u8"c") == Rect {10, -20, 300, 300});
    }

    SECTION("positioned, and so with a layer, in a fieldset that clips")
    {
      AnimationPageFixture fixture {u8"<!DOCTYPE html><style>" + utf8_string(BaseStyle)
                                    + u8"fieldset{overflow:hidden;height:60px;scrollbar-width:none}"
                                      u8"legend{position:relative;left:5px}#c{height:300px}</style>"
                                      u8"<fieldset id=f><legend id=l></legend><div id=c></div></fieldset>"};
      (void)fixture.Document->UpdateLayout();
      fixture.ById(u8"f").SetScrollTop(40);
      (void)fixture.Document->UpdateLayout();
      CHECK(*RectOf(fixture, u8"l") == Rect {15, 0, 50, 20});
      CHECK(fixture.Document->ElementFromPoint(20, 5).get() == &fixture.ById(u8"l"));
    }

    SECTION("overflowing the clip of a fieldset whose legend's taller than the border")
    {
      AnimationPageFixture fixture {
        u8"<!DOCTYPE html><style>" + utf8_string(BaseStyle)
        + u8"fieldset{overflow:hidden;height:60px}legend{height:40px}#c{height:300px}</style>"
          u8"<fieldset id=f><legend id=l></legend><div id=c></div></fieldset>"};
      (void)fixture.Document->UpdateLayout();
      CHECK(fixture.Document->ElementFromPoint(15, 30).get() == &fixture.ById(u8"l"));
      CHECK(fixture.Document->ElementFromPoint(15, 45).get() == &fixture.ById(u8"c"));
    }
  }

  TEST_CASE("RenderBlock: a fieldset's client box leaves its legend in", "[Rendering][RenderBlock]")
  {
    SECTION("not scrolling")
    {
      AnimationPageFixture fixture {
        u8"<!DOCTYPE html><style>" + utf8_string(BaseStyle)
        + u8"</style><fieldset id=f><legend id=l></legend><div id=c></div></fieldset>"};
      auto &fieldset = fixture.ById(u8"f");
      CHECK(fieldset.ClientTop() == 10);
      CHECK(fieldset.ClientHeight() == 40);
      CHECK(fieldset.ScrollHeight() == 30);
    }

    SECTION("scrolling")
    {
      AnimationPageFixture fixture {
        u8"<!DOCTYPE html><style>" + utf8_string(BaseStyle)
        + u8"fieldset{overflow:auto;height:60px;scrollbar-width:none}#c{height:300px}</style>"
          u8"<fieldset id=f><legend id=l></legend><div id=c></div></fieldset>"};
      auto &fieldset = fixture.ById(u8"f");
      CHECK(fieldset.ClientTop() == 10);
      CHECK(fieldset.ClientHeight() == 60);
      CHECK(fieldset.ScrollHeight() == 300);
    }
  }
}
