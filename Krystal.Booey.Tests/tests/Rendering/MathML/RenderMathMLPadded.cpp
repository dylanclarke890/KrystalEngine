#include "Krystal.Booey/Rendering/MathML/RenderMathMLPadded.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    /// @brief `<mpadded id=p>` with @p attributes, around a 20px wide space 10px high and 5px deep
    /// (`id=c`), laid out.
    KRYS_NODISCARD ResolverFixture PaddedFixture(utf8_stringview attributes) noexcept
    {
      utf8_string body = u8"<math><mpadded id=p ";
      body += attributes;
      body += u8"><mspace id=c width=20px height=10px depth=5px></mspace></mpadded></math>";
      ResolverFixture fixture {layout::tests::AhemDocument(u8"", body)};
      (void)fixture.Document->UpdateLayout();
      return fixture;
    }

    KRYS_NODISCARD RenderBox &PaddedBoxById(ResolverFixture &fixture, dom::DOMStringView id) noexcept
    {
      auto *renderer = fixture.ById(id).Renderer();
      REQUIRE(renderer);
      return Downcast<RenderBox>(*renderer);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderMathMLPadded: the attributes",
                   "[Rendering][MathML][RenderMathMLPadded]")
  {
    SECTION("Without attributes it's the content's size")
    {
      auto fixture = PaddedFixture(u8"");
      auto &padded = PaddedBoxById(fixture, u8"p");
      CHECK(Is<RenderMathMLPadded>(padded));
      CHECK(padded.Size() == LayoutSize(20, 15));
      CHECK(padded.FirstLineBaseline() == 10_lu);
      CHECK(PaddedBoxById(fixture, u8"c").Location() == LayoutPoint(0, 0));
      CHECK(padded.MaxPreferredLogicalWidth() == 20_lu);
    }

    SECTION("Width, height, depth, lspace and voffset")
    {
      auto fixture = PaddedFixture(u8"width=40px height=30px depth=0 lspace=5px voffset=2px");
      auto &padded = PaddedBoxById(fixture, u8"p");
      CHECK(padded.Size() == LayoutSize(40, 30));
      // The content is moved by lspace, and up by voffset, onto the new baseline.
      CHECK(PaddedBoxById(fixture, u8"c").Location() == LayoutPoint(5, 30 - 10 - 2));
      CHECK(padded.FirstLineBaseline() == LayoutUnit(30 - 2 + 2));
      // Only the width changes the preferred width.
      CHECK(padded.MaxPreferredLogicalWidth() == 40_lu);
    }

    SECTION("Percentages are of the content's size")
    {
      auto fixture = PaddedFixture(u8"width=200% height=50% depth=200%");
      auto &padded = PaddedBoxById(fixture, u8"p");
      CHECK(padded.Size() == LayoutSize(40, 5 + 10));
      CHECK(PaddedBoxById(fixture, u8"c").Location() == LayoutPoint(0, -5));
      CHECK(padded.MaxPreferredLogicalWidth() == 40_lu);
    }

    SECTION("Negative sizes and lspace are clamped to zero")
    {
      auto fixture = PaddedFixture(u8"width=-10px height=-10px depth=-10px lspace=-5px");
      auto &padded = PaddedBoxById(fixture, u8"p");
      CHECK(padded.Size() == LayoutSize());
      CHECK(PaddedBoxById(fixture, u8"c").Location() == LayoutPoint(0, -10));
    }

    SECTION("Right-to-left, lspace is from the end")
    {
      auto fixture = PaddedFixture(u8"dir=rtl width=40px lspace=5px");
      CHECK(PaddedBoxById(fixture, u8"c").Location() == LayoutPoint(40 - 20 - 5, 0));
    }

    SECTION("Empty, the baseline is the height")
    {
      ResolverFixture fixture {
        layout::tests::AhemDocument(u8"", u8"<math><mpadded id=p height=7px></mpadded></math>")};
      (void)fixture.Document->UpdateLayout();
      CHECK(PaddedBoxById(fixture, u8"p").FirstLineBaseline() == 7_lu);
    }

    SECTION("Changing an attribute lays it out again")
    {
      auto fixture = PaddedFixture(u8"width=40px");
      fixture.ById(u8"p").SetAttribute(u8"width", u8"60px");
      (void)fixture.Document->UpdateLayout();
      CHECK(PaddedBoxById(fixture, u8"p").Size() == LayoutSize(60, 15));
    }
  }
}
