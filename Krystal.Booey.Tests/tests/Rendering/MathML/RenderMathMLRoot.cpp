#include "Krystal.Booey/Rendering/MathML/RenderMathMLRoot.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/MathML/MathMLTestUtils.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    /// @brief A square root (`id=r1`) and a root with an index (`id=r2`), each of a 3em by 1em base (`id=b1`
    /// and `id=b2`), in @p font.
    KRYS_NODISCARD ResolverFixture Roots(const char *font) noexcept
    {
      return MathFontDocument(font,
                              u8"<math><msqrt id=r1><mspace id=b1 width=3em height=1em></mspace></msqrt>"
                              u8"<mroot id=r2><mspace id=b2 width=3em height=1em></mspace>"
                              u8"<mspace width=3em height=1em></mspace></mroot></math>");
    }
  }

  // Ported from web-platform-tests' mathml/presentation-markup/radicals/root-parameters-1.html: each font
  // sets one MATH constant, which the gaps and kerns measure.
  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderMathMLRoot: the MATH table's parameters",
                   "[Rendering][MathML][RenderMathMLRoot]")
  {
    SECTION("RadicalVerticalGap")
    {
      auto fixture = Roots("radical-verticalgap6000-rulethickness1000");
      CHECK(Near(BoxOf(fixture, u8"b1").Y() - BoxOf(fixture, u8"r1").Y(), 60 + 10));
      CHECK(Near(BoxOf(fixture, u8"b2").Y() - BoxOf(fixture, u8"r2").Y(), 60 + 10));
      auto *root = fixture.ById(u8"r1").Renderer();
      REQUIRE(root);
      CHECK(Is<RenderMathMLRoot>(*root));
      CHECK(Downcast<RenderMathMLRoot>(*root).RenderName() == u8"RenderMathMLRoot");
    }

    SECTION("RadicalExtraAscender")
    {
      auto fixture = Roots("radical-extraascender3000-rulethickness1000");
      CHECK(Near(BoxOf(fixture, u8"b1").Y() - BoxOf(fixture, u8"r1").Y(), 30 + 10));
      CHECK(Near(BoxOf(fixture, u8"b2").Y() - BoxOf(fixture, u8"r2").Y(), 30 + 10));
    }

    SECTION("RadicalKernBeforeDegree")
    {
      auto fixture = MathFontDocument("radical-kernbeforedegree4000-rulethickness1000",
                                      u8"<math><mroot id=r><mspace width=3em height=1em></mspace>"
                                      u8"<mspace id=i width=3em height=1em></mspace></mroot></math>");
      CHECK(Near(BoxOf(fixture, u8"i").X() - BoxOf(fixture, u8"r").X(), 40));
    }

    SECTION("RadicalDegreeBottomRaisePercent")
    {
      auto fixture = MathFontDocument("radical-degreebottomraisepercent25-rulethickness1000",
                                      u8"<math><mspace id=ref width=3em depth=1em></mspace><mroot>"
                                      u8"<mspace id=b width=3em height=10em></mspace>"
                                      u8"<mspace id=i width=3em height=1em></mspace></mroot></math>");
      float radicalHeight = BoxOf(fixture, u8"b").Height() + 10;
      CHECK(Near(BoxOf(fixture, u8"ref").Y() - BoxOf(fixture, u8"i").MaxY(), 25 * radicalHeight / 100));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderMathMLRoot: layout and painting",
                   "[Rendering][MathML][RenderMathMLRoot]")
  {
    SECTION("The radical is drawn before the base, with the rule over it")
    {
      auto fixture = Roots("radical-verticalgap6000-rulethickness1000");
      auto root = BoxOf(fixture, u8"r1");
      auto base = BoxOf(fixture, u8"b1");
      CHECK(base.MaxX() == root.MaxX());
      CHECK(base.X() > root.X());

      auto lines = PaintedItems<display_list::DrawLine>(fixture);
      REQUIRE(lines.size() == 2);
      CHECK(lines[0].Point1().X() == base.X());
      CHECK(lines[0].Point2().X() == base.MaxX());
      CHECK(lines[0].Point1().Y() == root.Y() + 5);
      CHECK_FALSE(PaintedItems<display_list::DrawGlyphs>(fixture).empty());
    }

    SECTION("Right to left, the radical is mirrored after the base")
    {
      auto fixture =
        MathFontDocument("radical-verticalgap6000-rulethickness1000",
                         u8"<math dir=rtl><msqrt id=r><mspace id=b width=3em height=1em></mspace>"
                         u8"</msqrt></math>");
      CHECK(BoxOf(fixture, u8"b").X() == BoxOf(fixture, u8"r").X());
      CHECK(BoxOf(fixture, u8"b").MaxX() < BoxOf(fixture, u8"r").MaxX());
    }

    SECTION("A root without exactly two children is a row, until its children change")
    {
      auto fixture = MathFontDocument("radical-kernbeforedegree4000-rulethickness1000",
                                      u8"<math><mroot id=r><mspace id=b width=3em height=1em></mspace>"
                                      u8"<mspace id=i width=3em height=1em></mspace>"
                                      u8"<mspace id=x width=3em height=1em></mspace></mroot></math>");
      CHECK(BoxOf(fixture, u8"b").X() == BoxOf(fixture, u8"r").X());
      CHECK(BoxOf(fixture, u8"i").X() == BoxOf(fixture, u8"b").MaxX());

      REQUIRE_FALSE(fixture.ById(u8"x").Remove().HasException());
      CHECK(Near(BoxOf(fixture, u8"i").X() - BoxOf(fixture, u8"r").X(), 40));
      CHECK(BoxOf(fixture, u8"b").X() > BoxOf(fixture, u8"i").MaxX());
    }
  }
}
