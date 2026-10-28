#include "Krystal.Booey/Rendering/MathML/RenderMathMLScripts.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/MathML/MathMLTestUtils.hpp"
#include "Krystal.Booey/Rendering/MathML/RenderMathMLOperator.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  // Ported from web-platform-tests' mathml/presentation-markup/scripts/subsup-parameters-1.html: each font
  // sets one MATH constant, which the gaps and shifts measure.
  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderMathMLScripts: the MATH table's parameters",
                   "[Rendering][MathML][RenderMathMLScripts]")
  {
    SECTION("SpaceAfterScript")
    {
      auto fixture = MathFontDocument(
        "scripts-spaceafterscript3000",
        u8"<math><msub id=s><mspace height=2em width=2em></mspace><mspace id=sub001 height=1em width=1em>"
        u8"</mspace></msub><mspace id=ref001 height=1em width=1em></mspace></math>"
        u8"<math><msup><mspace height=2em width=2em></mspace><mspace id=sup002 height=1em width=1em></mspace>"
        u8"</msup><mspace id=ref002 height=1em width=1em></mspace></math>"
        u8"<math><msubsup><mspace height=2em width=2em></mspace><mspace></mspace>"
        u8"<mspace id=sup003 height=1em width=1em></mspace></msubsup>"
        u8"<mspace id=ref003 height=1em width=1em></mspace></math>"
        u8"<math><mmultiscripts><mspace height=2em width=2em></mspace>"
        u8"<none></none><mspace id=sup0041 height=1em width=1em></mspace>"
        u8"<none></none><mspace id=sup0042 height=1em width=1em></mspace>"
        u8"<none></none><mspace id=sup0043 height=1em width=1em></mspace>"
        u8"</mmultiscripts><mspace id=ref004 height=1em width=1em></mspace></math>");
      CHECK(Near(BoxOf(fixture, u8"ref001").X() - BoxOf(fixture, u8"sub001").MaxX(), 30));
      CHECK(Near(BoxOf(fixture, u8"ref002").X() - BoxOf(fixture, u8"sup002").MaxX(), 30));
      CHECK(Near(BoxOf(fixture, u8"ref003").X() - BoxOf(fixture, u8"sup003").MaxX(), 30));
      CHECK(Near(BoxOf(fixture, u8"sup0042").X() - BoxOf(fixture, u8"sup0041").MaxX(), 30));
      CHECK(Near(BoxOf(fixture, u8"sup0043").X() - BoxOf(fixture, u8"sup0042").MaxX(), 30));
      CHECK(Near(BoxOf(fixture, u8"ref004").X() - BoxOf(fixture, u8"sup0043").MaxX(), 30));

      auto *scripts = fixture.ById(u8"s").Renderer();
      REQUIRE(scripts);
      CHECK(Is<RenderMathMLScripts>(*scripts));
      CHECK(Downcast<RenderMathMLScripts>(*scripts).RenderName() == u8"RenderMathMLScripts");
      CHECK(Downcast<RenderBox>(*scripts).MaxPreferredLogicalWidth() == 20_lu + 10_lu + 30_lu);
    }

    SECTION("SuperscriptShiftUp")
    {
      auto fixture = MathFontDocument(
        "scripts-superscriptshiftup7000",
        u8"<math><mspace id=ref101 height=1em width=1em></mspace>"
        u8"<msup><mspace height=2em width=2em></mspace><mspace id=sup102 height=1em "
        u8"width=1em></mspace></msup>"
        u8"<msubsup><mspace height=2em width=2em></mspace><mspace height=1em width=1em></mspace>"
        u8"<mspace id=sup103 height=1em width=1em></mspace></msubsup>"
        u8"<mmultiscripts><mspace height=2em width=2em></mspace>"
        u8"<none></none><mspace id=sup1041 height=1em width=1em></mspace>"
        u8"<none></none><mspace id=sup1042 height=1em width=1em></mspace></mmultiscripts></math>");
      for (const auto *id : {u8"sup102", u8"sup103", u8"sup1041", u8"sup1042"})
      {
        CHECK(Near(BoxOf(fixture, u8"ref101").MaxY() - BoxOf(fixture, id).MaxY(), 70));
      }
    }

    SECTION("SubscriptShiftDown")
    {
      auto fixture = MathFontDocument(
        "scripts-subscriptshiftdown6000",
        u8"<math><mspace id=ref300 height=1em width=1em></mspace>"
        u8"<msub><mspace height=2em width=2em></mspace><mspace id=sub301 height=1em "
        u8"width=1em></mspace></msub>"
        u8"<msubsup><mspace height=2em width=2em></mspace><mspace id=sub302 height=1em width=1em></mspace>"
        u8"<mspace height=1em width=1em></mspace></msubsup>"
        u8"<mmultiscripts><mspace height=2em width=2em></mspace><mspace id=sub303 height=1em width=1em>"
        u8"</mspace><none></none><mspace id=sub304 height=1em width=1em></mspace><none></none>"
        u8"</mmultiscripts></math>");
      for (const auto *id : {u8"sub301", u8"sub302", u8"sub303", u8"sub304"})
      {
        CHECK(Near(BoxOf(fixture, id).MaxY() - BoxOf(fixture, u8"ref300").MaxY(), 60));
      }
    }

    SECTION("SubSuperscriptGapMin")
    {
      auto fixture = MathFontDocument(
        "scripts-subsuperscriptgapmin11000",
        u8"<math><msubsup><mspace height=2em width=2em></mspace>"
        u8"<mspace id=sub4011 height=1em width=1em></mspace><mspace id=sup4012 height=1em width=1em></mspace>"
        u8"</msubsup><mmultiscripts><mspace height=2em width=2em></mspace><none></none><none></none>"
        u8"<mspace id=sub4021 height=1em width=1em></mspace><mspace id=sup4022 height=1em width=1em></mspace>"
        u8"</mmultiscripts></math>");
      CHECK(Near(BoxOf(fixture, u8"sub4011").Y() - BoxOf(fixture, u8"sup4012").MaxY(), 110));
      CHECK(Near(BoxOf(fixture, u8"sub4021").Y() - BoxOf(fixture, u8"sup4022").MaxY(), 110));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderMathMLScripts: layout",
                   "[Rendering][MathML][RenderMathMLScripts]")
  {
    SECTION("Prescripts go before the base, postscripts after it")
    {
      auto fixture = MathFontDocument(
        "scripts-spaceafterscript3000",
        u8"<math><mmultiscripts id=m><mspace id=base height=2em width=2em></mspace>"
        u8"<mspace id=sub height=1em width=1em></mspace><mspace id=sup height=1em width=1em>"
        u8"</mspace><mprescripts></mprescripts><mspace id=presub height=1em width=1em></mspace>"
        u8"<mspace id=presup height=1em width=1em></mspace></mmultiscripts></math>");
      auto base = BoxOf(fixture, u8"base");
      CHECK(BoxOf(fixture, u8"presub").MaxX() == base.X());
      CHECK(BoxOf(fixture, u8"presup").MaxX() == base.X());
      CHECK(BoxOf(fixture, u8"sub").X() == base.MaxX());
      CHECK(BoxOf(fixture, u8"sup").X() == base.MaxX());
      // The space after each pair of scripts, the prescripts' first.
      CHECK(Near(BoxOf(fixture, u8"m").Width(), 30 + 10 + 20 + 10 + 30));
      CHECK(Near(base.X() - BoxOf(fixture, u8"m").X(), 30 + 10));
    }

    SECTION("The subscript is pulled under a large operator's italic correction")
    {
      // The display variant of U+2AFF has an italic correction of 30px.
      auto fixture = MathFontDocument(
        "largeop-displayoperatorminheight2000-2AFF-italiccorrection3000",
        u8"<math display=block><msubsup><mo id=o lspace=0 rspace=0 largeop=true>&#x2AFF;</mo>"
        u8"<mspace id=sub height=1em width=1em></mspace><mspace id=sup height=1em width=1em></mspace>"
        u8"</msubsup></math>");
      CHECK(Near(BoxOf(fixture, u8"sup").X() - BoxOf(fixture, u8"sub").X(), 30));
      CHECK(Near(BoxOf(fixture, u8"sup").X(), BoxOf(fixture, u8"o").MaxX()));
    }

    SECTION("Scripts embellish an operator base")
    {
      auto fixture = AhemMathDocument(u8"<math><msub id=s><mo id=o>+</mo><mn>2</mn></msub></math>");
      (void)fixture.Document->UpdateLayout();
      auto *scripts = fixture.ById(u8"s").Renderer();
      REQUIRE(scripts);
      const RenderObject *op = Downcast<RenderMathMLBlock>(*scripts).UnembellishedOperator();
      CHECK(op == fixture.ById(u8"o").Renderer());
    }

    SECTION("Invalid scripts are laid out as rows")
    {
      auto fixture = AhemMathDocument(
        u8"<math><msub><mspace id=a width=1em height=1em></mspace></msub>"
        u8"<mmultiscripts><mspace id=b width=1em height=1em></mspace><mspace id=c width=1em height=1em>"
        u8"</mspace></mmultiscripts>"
        u8"<msubsup><mspace id=d width=1em height=1em></mspace><mprescripts></mprescripts>"
        u8"<mspace width=1em height=1em></mspace></msubsup></math>");
      (void)fixture.Document->UpdateLayout();
      CHECK(BoxOf(fixture, u8"b").Y() == BoxOf(fixture, u8"a").Y());
      CHECK(BoxOf(fixture, u8"c").Y() == BoxOf(fixture, u8"b").Y());
      CHECK(BoxOf(fixture, u8"c").X() == BoxOf(fixture, u8"b").MaxX());
      CHECK(BoxOf(fixture, u8"d").Y() == BoxOf(fixture, u8"a").Y());
    }
  }
}
