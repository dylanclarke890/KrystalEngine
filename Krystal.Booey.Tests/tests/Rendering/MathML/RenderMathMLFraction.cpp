#include "Krystal.Booey/Rendering/MathML/RenderMathMLFraction.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/MathML/MathMLTestUtils.hpp"
#include "Krystal.Booey/Rendering/MathML/RenderMathMLOperator.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    KRYS_NODISCARD RenderMathMLFraction &FractionById(ResolverFixture &fixture,
                                                      dom::DOMStringView id) noexcept
    {
      (void)fixture.Document->UpdateLayout();
      auto *renderer = fixture.ById(id).Renderer();
      REQUIRE(renderer);
      return Downcast<RenderMathMLFraction>(*renderer);
    }
  }

  // Ported from web-platform-tests' mathml/presentation-markup/fractions/frac-parameters-1.html and
  // frac-parameters-2.html: each font sets one MATH constant, which the gaps and shifts measure.
  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderMathMLFraction: the MATH table's parameters",
                   "[Rendering][MathML][RenderMathMLFraction]")
  {
    SECTION("AxisHeight")
    {
      auto fixture =
        MathFontDocument("fraction-axisheight7000-rulethickness1000",
                         u8"<math><mspace id=ref depth=1em width=3em></mspace><mfrac id=f>"
                         u8"<mspace width=3em height=1em id=num></mspace><mspace width=3em></mspace>"
                         u8"</mfrac></math>");
      CHECK(Near(BoxOf(fixture, u8"ref").Y() - BoxOf(fixture, u8"num").MaxY(), 70 + 10 / 2.0f));
      CHECK(FractionById(fixture, u8"f").LineThickness() == 10_lu);
      CHECK(FractionById(fixture, u8"f").RelativeLineThickness() == 1.0f);
    }

    SECTION("NumeratorShiftUp")
    {
      auto fixture =
        MathFontDocument("fraction-numeratorshiftup11000-axisheight1000-rulethickness1000",
                         u8"<math><mspace id=ref width=3em depth=1em></mspace><mfrac>"
                         u8"<mspace width=3em height=1em id=num></mspace><mspace width=3em></mspace>"
                         u8"</mfrac></math>");
      CHECK(Near(BoxOf(fixture, u8"ref").Y() - BoxOf(fixture, u8"num").MaxY(), 110));
    }

    SECTION("DenominatorGapMin")
    {
      auto fixture =
        MathFontDocument("fraction-denominatorgapmin4000-rulethickness1000",
                         u8"<math><mspace id=ref width=3em height=.5em depth=.5em></mspace><mfrac>"
                         u8"<mspace width=3em></mspace><mspace width=3em height=1em id=den></mspace>"
                         u8"</mfrac></math>");
      CHECK(Near(BoxOf(fixture, u8"den").Y() - BoxOf(fixture, u8"ref").MaxY(), 40));
    }

    SECTION("NumeratorDisplayStyleGapMin, in display style only")
    {
      utf8_string body = u8"<math id=math display=block><mspace id=ref width=3em height=.5em depth=.5em>"
                         u8"</mspace><mfrac><mspace width=3em depth=1em id=num></mspace>"
                         u8"<mspace width=3em></mspace></mfrac></math>";
      auto fixture = MathFontDocument("fraction-numeratordisplaystylegapmin8000-rulethickness1000", body);
      CHECK(Near(BoxOf(fixture, u8"ref").Y() - BoxOf(fixture, u8"num").MaxY(), 80));

      // Inline, the font's other gaps are zero, so the numerator sits on the bar.
      fixture.ById(u8"math").SetAttribute(u8"display", u8"inline");
      CHECK(Near(BoxOf(fixture, u8"ref").Y() - BoxOf(fixture, u8"num").MaxY(), 0));
    }

    SECTION("StackGapMin, with no bar")
    {
      auto fixture =
        MathFontDocument("stack-gapmin8000", u8"<math><mfrac id=f linethickness=0px>"
                                             u8"<mspace width=3em height=1em id=num></mspace>"
                                             u8"<mspace width=3em depth=1em id=den></mspace></mfrac></math>");
      CHECK(FractionById(fixture, u8"f").LineThickness() == 0_lu);
      CHECK(Near(BoxOf(fixture, u8"den").Y() - BoxOf(fixture, u8"num").MaxY(), 80));
    }

    SECTION("StackTopShiftUp")
    {
      auto fixture =
        MathFontDocument("stack-topshiftup9000-axisheight1000",
                         u8"<math><mspace id=ref width=3em depth=1em></mspace>"
                         u8"<mfrac linethickness=0px><mspace width=3em height=1em id=num></mspace>"
                         u8"<mspace width=3em></mspace></mfrac></math>");
      CHECK(Near(BoxOf(fixture, u8"ref").Y() - BoxOf(fixture, u8"num").MaxY(), 90));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderMathMLFraction: layout",
                   "[Rendering][MathML][RenderMathMLFraction]")
  {
    SECTION("The numerator and denominator are centred, and the fraction is as wide as the wider")
    {
      auto fixture = MathFontDocument("fraction-axisheight7000-rulethickness1000",
                                      u8"<math><mfrac id=f><mspace id=num width=2em height=1em></mspace>"
                                      u8"<mspace id=den width=4em height=1em></mspace></mfrac></math>");
      auto &fraction = FractionById(fixture, u8"f");
      // Fractions have a pixel of padding on either side.
      CHECK(fraction.LogicalWidth() == 42_lu);
      CHECK(fraction.MaxPreferredLogicalWidth() == 42_lu);
      CHECK(BoxOf(fixture, u8"num").X() - BoxOf(fixture, u8"den").X() == 10);
      CHECK(fraction.RenderName() == u8"RenderMathMLFraction");

      // Its baseline is the numerator's shift above the axis: the numerator's bottom is the bar's top.
      auto baseline = static_cast<RenderBox &>(fraction).FirstLineBaseline();
      REQUIRE(baseline);
      CHECK(*baseline == 10_lu + 70_lu + 5_lu);
    }

    SECTION("linethickness, and changing it lays the fraction out again")
    {
      auto fixture = MathFontDocument("fraction-axisheight7000-rulethickness1000",
                                      u8"<math><mfrac id=f linethickness=50%><mspace width=3em height=1em>"
                                      u8"</mspace><mspace width=3em></mspace></mfrac></math>");
      CHECK(FractionById(fixture, u8"f").LineThickness() == 5_lu);
      CHECK(FractionById(fixture, u8"f").RelativeLineThickness() == 0.5f);

      fixture.ById(u8"f").SetAttribute(u8"linethickness", u8"3px");
      CHECK(FractionById(fixture, u8"f").LineThickness() == 3_lu);

      // Negative thicknesses are clamped to zero.
      fixture.ById(u8"f").SetAttribute(u8"linethickness", u8"-3px");
      CHECK(FractionById(fixture, u8"f").LineThickness() == 0_lu);
    }

    SECTION("Without exactly two children it's laid out as a row")
    {
      auto fixture = AhemMathDocument(
        u8"<math><mfrac id=f><mspace id=a width=1em height=1em></mspace><mspace id=b width=2em height=1em>"
        u8"</mspace><mspace width=1em height=1em></mspace></mfrac></math>");
      auto &fraction = FractionById(fixture, u8"f");
      CHECK(fraction.LogicalWidth() == 42_lu);
      CHECK(BoxOf(fixture, u8"b").X() - BoxOf(fixture, u8"a").X() == 10);
      CHECK(BoxOf(fixture, u8"b").Y() == BoxOf(fixture, u8"a").Y());
    }

    SECTION("A fraction with an operator for its numerator embellishes it")
    {
      auto fixture = AhemMathDocument(u8"<math><mfrac id=f><mo id=o>+</mo><mn>2</mn></mfrac></math>");
      auto &fraction = FractionById(fixture, u8"f");
      auto *op = fixture.ById(u8"o").Renderer();
      const RenderObject *unembellished = static_cast<RenderMathMLBlock &>(fraction).UnembellishedOperator();
      CHECK(unembellished == op);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderMathMLFraction: the bar",
                   "[Rendering][MathML][RenderMathMLFraction]")
  {
    auto fixture = MathFontDocument("fraction-axisheight7000-rulethickness1000",
                                    u8"<math><mfrac id=f><mspace id=num width=3em height=1em></mspace>"
                                    u8"<mspace width=2em></mspace></mfrac></math>");

    // The bar is drawn across the fraction on the axis, under the numerator.
    auto lines = PaintedItems<display_list::DrawLine>(fixture);
    REQUIRE(lines.size() == 1);
    auto numerator = BoxOf(fixture, u8"num");
    CHECK(lines[0].Point1().X() == numerator.X());
    CHECK(lines[0].Point2().X() == numerator.MaxX());
    CHECK(lines[0].Point1().Y() == lines[0].Point2().Y());
    CHECK(Near(lines[0].Point1().Y(), numerator.MaxY() + 5));

    // Without a thickness there's no bar.
    fixture.ById(u8"f").SetAttribute(u8"linethickness", u8"0");
    CHECK(PaintedItems<display_list::DrawLine>(fixture).empty());
  }
}
