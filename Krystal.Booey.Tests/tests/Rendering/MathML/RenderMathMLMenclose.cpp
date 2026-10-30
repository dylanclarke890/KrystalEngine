#include "Krystal.Booey/Rendering/MathML/RenderMathMLMenclose.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/MathML/MathMLTestUtils.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    /// @brief `<menclose id=e>` with @p notation around a 2em by 1em space (`id=c`), in 10px Ahem, so the
    /// rule is half a pixel thick.
    KRYS_NODISCARD ResolverFixture Enclosed(utf8_stringview notation) noexcept
    {
      utf8_string body = u8"<math><menclose id=e ";
      body += notation;
      body += u8"><mspace id=c width=2em height=1em></mspace></menclose></math>";
      auto fixture = AhemMathDocument(body);
      (void)fixture.Document->UpdateLayout();
      return fixture;
    }

    KRYS_NODISCARD RenderBox &EnclosureOf(ResolverFixture &fixture) noexcept
    {
      (void)fixture.Document->UpdateLayout();
      auto *renderer = fixture.ById(u8"e").Renderer();
      REQUIRE(renderer);
      REQUIRE(Is<RenderMathMLMenclose>(*renderer));
      return Downcast<RenderBox>(*renderer);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderMathMLMenclose: space around the content",
                   "[Rendering][MathML][RenderMathMLMenclose]")
  {
    SECTION("box leaves five rules on each side")
    {
      auto fixture = Enclosed(u8"notation=box");
      auto &enclosure = EnclosureOf(fixture);
      CHECK(enclosure.Size() == LayoutSize(LayoutUnit(20 + 2 * 2.5f), LayoutUnit(10 + 2 * 2.5f)));
      CHECK(enclosure.MaxPreferredLogicalWidth() == LayoutUnit(20 + 2 * 2.5f));
      CHECK(Downcast<RenderBox>(*fixture.ById(u8"c").Renderer()).Location()
            == LayoutPoint(LayoutUnit(2.5f), LayoutUnit(2.5f)));
      CHECK(Downcast<RenderMathMLMenclose>(enclosure).RenderName() == u8"RenderMathMLMenclose");
    }

    SECTION("The default notation is longdiv, ten rules on the left")
    {
      auto fixture = Enclosed(u8"");
      auto &enclosure = EnclosureOf(fixture);
      CHECK(Downcast<RenderBox>(*fixture.ById(u8"c").Renderer()).Location()
            == LayoutPoint(LayoutUnit(5), LayoutUnit(2.5f)));
      CHECK(enclosure.LogicalWidth() == LayoutUnit(5 + 20 + 2));
    }

    SECTION("Strikes add no space, but diagonal ones half a rule")
    {
      auto fixture = Enclosed(u8"notation='horizontalstrike verticalstrike'");
      CHECK(EnclosureOf(fixture).Size() == LayoutSize(20, 10));

      fixture.ById(u8"e").SetAttribute(u8"notation", u8"updiagonalstrike");
      CHECK(EnclosureOf(fixture).Size() == LayoutSize(LayoutUnit(20.5f), LayoutUnit(10.5f)));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderMathMLMenclose: the notations are stroked",
                   "[Rendering][MathML][RenderMathMLMenclose]")
  {
    SECTION("box strokes four lines")
    {
      auto fixture = Enclosed(u8"notation=box");
      CHECK(PaintedItems<display_list::StrokePath>(fixture).size() == 4);
    }

    SECTION("circle, roundedbox and longdiv stroke one path each")
    {
      auto fixture = Enclosed(u8"notation='circle roundedbox longdiv'");
      CHECK(PaintedItems<display_list::StrokePath>(fixture).size() == 3);
    }

    SECTION("Hidden enclosures aren't stroked")
    {
      auto fixture = Enclosed(u8"notation=box style='visibility: hidden'");
      CHECK(PaintedItems<display_list::StrokePath>(fixture).empty());
    }
  }
}
