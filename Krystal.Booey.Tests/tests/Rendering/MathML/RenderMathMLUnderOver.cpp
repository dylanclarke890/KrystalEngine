#include "Krystal.Booey/Rendering/MathML/RenderMathMLUnderOver.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/MathML/MathMLTestUtils.hpp"
#include "Krystal.Booey/Rendering/MathML/RenderMathMLOperator.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  // Ported from web-platform-tests' mathml/presentation-markup/scripts/underover-parameters-1.html and
  // underover-parameters-3.html: each font sets one MATH constant, which the gaps measure.
  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderMathMLUnderOver: the MATH table's parameters",
                   "[Rendering][MathML][RenderMathMLUnderOver]")
  {
    SECTION("LowerLimitGapMin, under a large operator")
    {
      auto fixture = MathFontDocument(
        "limits-lowerlimitgapmin11000",
        u8"<math><mspace id=ref height=1em width=3em></mspace>"
        u8"<munder id=u><mo movablelimits=false>&#x2211;</mo><mspace id=under1 depth=1em width=3em></mspace>"
        u8"</munder><munderover><mo movablelimits=false>&#x2211;</mo>"
        u8"<mspace id=under2 depth=1em width=3em></mspace><mspace height=1em width=3em></mspace>"
        u8"</munderover></math>");
      CHECK(Near(BoxOf(fixture, u8"under1").Y() - BoxOf(fixture, u8"ref").MaxY(), 110));
      CHECK(Near(BoxOf(fixture, u8"under2").Y() - BoxOf(fixture, u8"ref").MaxY(), 110));

      auto *underOver = fixture.ById(u8"u").Renderer();
      REQUIRE(underOver);
      CHECK(Is<RenderMathMLUnderOver>(*underOver));
      CHECK_FALSE(Is<RenderMathMLScripts>(*underOver));
      CHECK(Downcast<RenderMathMLUnderOver>(*underOver).RenderName() == u8"RenderMathMLUnderOver");
    }

    SECTION("UpperLimitGapMin, over a large operator")
    {
      auto fixture = MathFontDocument(
        "limits-upperlimitgapmin7000",
        u8"<math><mspace id=ref height=1em width=3em></mspace>"
        u8"<mover><mo movablelimits=false>&#x2211;</mo><mspace id=over1 depth=1em width=3em></mspace></mover>"
        u8"<munderover><mo movablelimits=false>&#x2211;</mo><mspace height=1em width=3em></mspace>"
        u8"<mspace id=over2 depth=1em width=3em></mspace></munderover></math>");
      CHECK(Near(BoxOf(fixture, u8"ref").Y() - BoxOf(fixture, u8"over1").MaxY(), 70));
      CHECK(Near(BoxOf(fixture, u8"ref").Y() - BoxOf(fixture, u8"over2").MaxY(), 70));
    }

    SECTION("OverbarVerticalGap, and AccentBaseHeight for accents")
    {
      auto fixture =
        MathFontDocument("underover-accentbaseheight4000-overbarverticalgap11000",
                         u8"<math><mover id=el1><mspace id=base1 height=3em width=1em></mspace>"
                         u8"<mspace id=over1 height=1em width=3em></mspace></mover>"
                         u8"<mover id=el2 accent=true><mspace id=base2 height=5em width=1em></mspace>"
                         u8"<mspace id=over2 height=1em width=3em></mspace></mover>"
                         u8"<mover id=el3 accent=true><mspace id=base3 height=3em width=1em></mspace>"
                         u8"<mspace id=over3 height=1em width=3em></mspace></mover></math>");
      // A script that isn't an accent is OverbarVerticalGap above the base.
      CHECK(Near(BoxOf(fixture, u8"base1").Y() - BoxOf(fixture, u8"over1").MaxY(), 110));
      // An accent sits on a base at least AccentBaseHeight tall, and is raised to it over a shorter one.
      CHECK(Near(BoxOf(fixture, u8"base2").Y() - BoxOf(fixture, u8"over2").MaxY(), 0));
      CHECK(Near(BoxOf(fixture, u8"base3").Y() - BoxOf(fixture, u8"over3").MaxY(), 40 - 30));
      // The scripts are centred over the base.
      CHECK(BoxOf(fixture, u8"base1").X() - BoxOf(fixture, u8"el1").X() == 10);
      CHECK(BoxOf(fixture, u8"over1").X() == BoxOf(fixture, u8"el1").X());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderMathMLUnderOver: limits and stretching",
                   "[Rendering][MathML][RenderMathMLUnderOver]")
  {
    SECTION("Movable limits move to the side of the operator inline")
    {
      utf8_string body = u8"<math id=math><munder><mo id=o lspace=0 rspace=0>&#x2211;</mo>"
                         u8"<mspace id=s height=1em width=1em></mspace></munder></math>";
      auto fixture = MathFontDocument("limits-lowerlimitgapmin11000", body);
      CHECK(BoxOf(fixture, u8"s").X() >= BoxOf(fixture, u8"o").MaxX());

      // In display style, they're limits under it.
      fixture.ById(u8"math").SetAttribute(u8"display", u8"block");
      CHECK(BoxOf(fixture, u8"s").Y() >= BoxOf(fixture, u8"o").MaxY());
      CHECK(BoxOf(fixture, u8"s").X() < BoxOf(fixture, u8"o").MaxX());
    }

    SECTION("A horizontal stretchy operator stretches to the other children")
    {
      RegisterMathFont("stretchy");
      ResolverFixture fixture {layout::tests::AhemDocument(
        // Scripts' fonts are scaled down; these operators' stay at 10px, so their variants are 10px to 40px.
        u8"math { font-family: stretchy } mo, mspace { font-size: 10px }",
        u8"<math><mover><mspace id=b width=3em height=1em></mspace>"
        u8"<mo id=o lspace=0 rspace=0>&#x295A;</mo></mover>"
        u8"<munder><mo id=a lspace=0 rspace=0>&#x295A;</mo><mo id=c lspace=0 rspace=0>&#x295A;</mo></munder>"
        u8"</math>")};
      (void)fixture.Document->UpdateLayout();
      CHECK(BoxOf(fixture, u8"o").Width() == 30);
      auto &op = Downcast<RenderMathMLOperator>(*fixture.ById(u8"o").Renderer());
      CHECK(op.StretchSize() == 30_lu);
      CHECK_FALSE(op.IsStretchWidthLocked());

      // When every child stretches, they stretch to the widest unstretched one: the 10px base glyph.
      CHECK(BoxOf(fixture, u8"a").Width() == 10);
      CHECK(BoxOf(fixture, u8"c").Width() == 10);
    }

    SECTION("Without the right number of children it's laid out as a row")
    {
      auto fixture = AhemMathDocument(
        u8"<math><munder><mspace id=a width=1em height=1em></mspace><mspace id=b width=1em height=1em>"
        u8"</mspace><mspace width=1em height=1em></mspace></munder></math>");
      (void)fixture.Document->UpdateLayout();
      CHECK(BoxOf(fixture, u8"b").Y() == BoxOf(fixture, u8"a").Y());
      CHECK(BoxOf(fixture, u8"b").X() == BoxOf(fixture, u8"a").MaxX());
    }
  }
}
