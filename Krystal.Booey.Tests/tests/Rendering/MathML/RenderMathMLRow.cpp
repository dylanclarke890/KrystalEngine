#include "Krystal.Booey/Rendering/MathML/RenderMathMLRow.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey/Rendering/MathML/RenderMathMLMath.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    constexpr const char8 *TwoSpaces = u8"<mspace id=a width=20px height=10px depth=5px></mspace><mspace "
                                       u8"id=b width=30px height=20px></mspace>";

    KRYS_NODISCARD ResolverFixture LaidOutRow(utf8_stringview body, utf8_stringview style = u8"") noexcept
    {
      ResolverFixture fixture {layout::tests::AhemDocument(style, body)};
      (void)fixture.Document->UpdateLayout();
      return fixture;
    }

    KRYS_NODISCARD RenderBox &RowBoxById(ResolverFixture &fixture, dom::DOMStringView id) noexcept
    {
      auto *renderer = fixture.ById(id).Renderer();
      REQUIRE(renderer);
      return Downcast<RenderBox>(*renderer);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderMathMLRow: children on a common baseline",
                   "[Rendering][MathML][RenderMathMLRow]")
  {
    utf8_string body = u8"<math><mrow id=row>";
    body += TwoSpaces;
    body += u8"</mrow></math>";
    auto fixture = LaidOutRow(body);

    auto &row = RowBoxById(fixture, u8"row");
    CHECK(row.Size() == LayoutSize(50, 25));
    CHECK(RowBoxById(fixture, u8"a").Location() == LayoutPoint(0, 10));
    CHECK(RowBoxById(fixture, u8"b").Location() == LayoutPoint(20, 0));
    // The baseline is the first child's.
    CHECK(row.FirstLineBaseline() == 20_lu);

    // The preferred widths are the children's.
    CHECK(row.MinPreferredLogicalWidth() == 50_lu);
    CHECK(row.MaxPreferredLogicalWidth() == 50_lu);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderMathMLRow: empty rows, margins and right-to-left",
                   "[Rendering][MathML][RenderMathMLRow]")
  {
    SECTION("An empty row has no size and no baseline")
    {
      auto fixture = LaidOutRow(u8"<math><mrow id=row></mrow></math>");
      auto &row = RowBoxById(fixture, u8"row");
      CHECK(row.Size() == LayoutSize());
      CHECK_FALSE(row.FirstLineBaseline());
    }

    SECTION("Margins separate the children")
    {
      auto fixture =
        LaidOutRow(u8"<math><mrow id=row><mspace id=a width=20px height=10px style=\"margin: 0 5px\">"
                   u8"</mspace><mspace id=b width=30px height=10px></mspace></mrow></math>");
      CHECK(RowBoxById(fixture, u8"row").Size() == LayoutSize(60, 10));
      CHECK(RowBoxById(fixture, u8"a").Location() == LayoutPoint(5, 0));
      CHECK(RowBoxById(fixture, u8"b").Location() == LayoutPoint(30, 0));
      CHECK(RowBoxById(fixture, u8"row").MaxPreferredLogicalWidth() == 60_lu);
    }

    SECTION("Right-to-left rows start on the right")
    {
      utf8_string body = u8"<math dir=rtl><mrow id=row>";
      body += TwoSpaces;
      body += u8"</mrow></math>";
      auto fixture = LaidOutRow(body);
      CHECK(RowBoxById(fixture, u8"a").Location() == LayoutPoint(30, 10));
      CHECK(RowBoxById(fixture, u8"b").Location() == LayoutPoint(0, 0));
    }

    SECTION("Border and padding surround the content")
    {
      utf8_string body = u8"<math><mrow id=row style=\"padding: 1px 2px 3px 4px; border: 1px solid\">";
      body += TwoSpaces;
      body += u8"</mrow></math>";
      auto fixture = LaidOutRow(body);
      auto &row = RowBoxById(fixture, u8"row");
      CHECK(row.Size() == LayoutSize(50 + 2 + 4 + 2, 25 + 1 + 3 + 2));
      CHECK(RowBoxById(fixture, u8"b").Location() == LayoutPoint(20 + 5, 2));
      CHECK(row.FirstLineBaseline() == 22_lu);
      CHECK(row.MaxPreferredLogicalWidth() == 58_lu);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderMathMLRow: CSS sizes",
                   "[Rendering][MathML][RenderMathMLRow]")
  {
    utf8_string body = u8"<math><mrow id=row style=\"width: 80px; height: 40px\">";
    body += TwoSpaces;
    body += u8"</mrow><mrow id=rtl dir=rtl style=\"width: 80px\">";
    body += TwoSpaces;
    body += u8"</mrow></math>";
    auto fixture = LaidOutRow(body);

    auto &row = RowBoxById(fixture, u8"row");
    CHECK(row.Size() == LayoutSize(80, 40));
    CHECK(row.MaxPreferredLogicalWidth() == 80_lu);
    // The content stays at the start.
    CHECK(RowBoxById(fixture, u8"b").Location() == LayoutPoint(20, 0));

    // Right-to-left content is moved to the end.
    auto *rtlSpace = Downcast<RenderBox>(RowBoxById(fixture, u8"rtl").FirstChild());
    CHECK(rtlSpace->Location() == LayoutPoint(30 + 30, 10));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderMathMLMath: inline and display formulas",
                   "[Rendering][MathML][RenderMathMLRow]")
  {
    SECTION("Inline formulas sit on the line's baseline")
    {
      auto fixture =
        LaidOutRow(u8"x<math id=math><mspace width=20px height=30px depth=5px></mspace></math>x");
      CHECK(ExternalRepresentation(fixture.Frame.get()) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x35
  RenderBlock {HTML} at (0,0) size 800x35
    RenderBody {BODY} at (0,0) size 800x35
      RenderText {#text} at (0,22) size 10x10
        text run at (0,22) width 10: "x"
      RenderMathMLMath {math} at (10,0) size 20x35
        RenderMathMLSpace {mspace} at (0,0) size 20x35
      RenderText {#text} at (30,22) size 10x10
        text run at (30,22) width 10: "x"
)");
    }

    SECTION("Display formulas are centred")
    {
      utf8_string body = u8"<math id=math display=block>";
      body += TwoSpaces;
      body += u8"</math>";
      auto fixture = LaidOutRow(body);
      auto &math = RowBoxById(fixture, u8"math");
      CHECK(math.Size() == LayoutSize(800, 25));
      CHECK(RowBoxById(fixture, u8"a").Location() == LayoutPoint(375, 10));
      CHECK(RowBoxById(fixture, u8"b").Location() == LayoutPoint(395, 0));
    }

    SECTION("Display formulas wider than the line grow")
    {
      auto fixture = LaidOutRow(u8"<div style=\"width: 30px\"><math id=math display=block>"
                                u8"<mspace id=a width=50px height=10px></mspace></math></div>");
      CHECK(RowBoxById(fixture, u8"math").Size() == LayoutSize(50, 10));
      CHECK(RowBoxById(fixture, u8"a").Location() == LayoutPoint(0, 0));
    }

    SECTION("Right-to-left display formulas are centred too")
    {
      utf8_string body = u8"<math id=math display=block dir=rtl>";
      body += TwoSpaces;
      body += u8"</math>";
      auto fixture = LaidOutRow(body);
      CHECK(RowBoxById(fixture, u8"b").Location() == LayoutPoint(375, 0));
      CHECK(RowBoxById(fixture, u8"a").Location() == LayoutPoint(405, 10));
    }
  }
}
