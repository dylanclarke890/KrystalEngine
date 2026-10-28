#include "Krystal.Booey/Rendering/MathML/RenderMathMLOperator.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey.Tests/Rendering/MathML/MathMLTestUtils.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListItems.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListRecorderImpl.hpp"
#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/OpenType/OpenTypeMathData.hpp"
#include "Krystal.Booey/MathML/MathMLOperatorElement.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include <algorithm>
#include <cmath>
#include <format>

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    /// @brief A document whose formulas are in the 10px font `stretchy` (a hundredth of a pixel a unit): its
    /// vertical size variants of U+295C are 10px to 40px tall and its horizontal ones of U+295A 10px to 40px
    /// wide, and both are assembled from 50px.
    KRYS_NODISCARD ResolverFixture StretchyDocument(utf8_stringview body) noexcept
    {
      RegisterMathFont("stretchy");
      ResolverFixture fixture {layout::tests::AhemDocument(u8"math { font-family: stretchy }", body)};
      (void)fixture.Document->UpdateLayout();
      return fixture;
    }

    KRYS_NODISCARD RenderMathMLOperator &OperatorById(ResolverFixture &fixture,
                                                      dom::DOMStringView id) noexcept
    {
      (void)fixture.Document->UpdateLayout();
      auto *renderer = fixture.ById(id).Renderer();
      REQUIRE(renderer);
      return Downcast<RenderMathMLOperator>(*renderer);
    }

    KRYS_NODISCARD RenderBox &BoxById(ResolverFixture &fixture, dom::DOMStringView id) noexcept
    {
      auto *renderer = fixture.ById(id).Renderer();
      REQUIRE(renderer);
      return Downcast<RenderBox>(*renderer);
    }

    /// @brief A document whose formulas are in 10px Ahem, with @p body as its body's content.
    KRYS_NODISCARD utf8_string AhemMath(utf8_stringview body) noexcept
    {
      return layout::tests::AhemDocument(u8"math { font-family: Ahem }", body);
    }

    /// @brief The glyph runs @p fixture's view paints, as "glyphs@x,y", in order.
    KRYS_NODISCARD std::string PaintedRuns(ResolverFixture &fixture) noexcept
    {
      (void)fixture.Document->UpdateLayout();
      display_list::RecorderImpl recorder(FloatSize(800, 600));
      fixture.Frame->View()->Paint(recorder, IntRect(0, 0, 800, 600));
      std::string runs;
      for (const auto &item : recorder.TakeDisplayList()->Items())
      {
        if (auto *glyphs = std::get_if<display_list::DrawGlyphs>(&item))
        {
          runs += std::format("{}{}@{},{}", runs.empty() ? "" : " ", glyphs->Length(),
                              glyphs->LocalAnchor().X(), glyphs->LocalAnchor().Y());
        }
      }
      return runs;
    }

    /// @brief `<mo id=o>` with @p attributes, no spacing and not symmetric, before a 10px wide space (`id=s`)
    /// @p height high and @p depth deep, in `stretchy`.
    KRYS_NODISCARD ResolverFixture StretchedOperator(utf8_stringview attributes, int32 height,
                                                     int32 depth) noexcept
    {
      // The first of repeated attributes is kept, so @p attributes override these.
      utf8_string body = u8"<math><mo id=o ";
      body += attributes;
      body += u8" lspace=0 rspace=0 symmetric=false>&#x295C;</mo><mspace id=s width=10px ";
      auto size = std::format("height={}px depth={}px", height, depth);
      body += utf8_stringview(reinterpret_cast<const char8 *>(size.data()), size.size());
      body += u8"></mspace></math>";
      return StretchyDocument(body);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderMathMLOperator: spacing",
                   "[Rendering][MathML][RenderMathMLOperator]")
  {
    SECTION("lspace and rspace surround the text")
    {
      ResolverFixture fixture {AhemMath(u8"<math><mo id=o lspace=5px rspace=3px>+</mo></math>")};
      auto &op = OperatorById(fixture, u8"o");
      CHECK(op.Size() == LayoutSize(18, 10));
      CHECK(op.MaxPreferredLogicalWidth() == 18_lu);
      CHECK(Downcast<RenderBox>(op.FirstChild())->Location() == LayoutPoint(5, 0));
      CHECK(static_cast<RenderMathMLBlock &>(op).UnembellishedOperator() == &op);
      CHECK_FALSE(op.IsStretchy());
      CHECK(op.RenderName() == u8"RenderMathMLOperator");

      // Changing them lays it out again.
      fixture.ById(u8"o").SetAttribute(u8"lspace", u8"0");
      fixture.ById(u8"o").SetAttribute(u8"rspace", u8"1em");
      auto &changed = OperatorById(fixture, u8"o");
      CHECK(changed.Size() == LayoutSize(20, 10));
      CHECK(Downcast<RenderBox>(changed.FirstChild())->Location() == LayoutPoint(0, 0));
    }

    SECTION("The default spacing is the operator dictionary's")
    {
      // An infix + is surrounded by medium math spaces, four eighteenths of an em.
      ResolverFixture fixture {AhemMath(u8"<math><mn>1</mn><mo id=o>+</mo><mn>2</mn></math>")};
      auto &op = OperatorById(fixture, u8"o");
      CHECK(op.MaxPreferredLogicalWidth() == LayoutUnit(10.0f * 4 / 18) * 2 + 10_lu);
    }

    SECTION("Invisible operators take no room and aren't painted")
    {
      ResolverFixture fixture {AhemMath(u8"<math><mn>1</mn><mo id=o>&#x2062;</mo><mn>2</mn></math>")};
      auto &op = OperatorById(fixture, u8"o");
      CHECK(op.MaxPreferredLogicalWidth() == 0_lu);
      CHECK(op.LogicalWidth() == 0_lu);
      CHECK(PaintedRuns(fixture) == "1@0,8 1@10,8");
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderMathMLOperator: the minus sign",
                   "[Rendering][MathML][RenderMathMLOperator]")
  {
    // A hyphen is drawn as the font's minus sign, from the glyph alone.
    ResolverFixture fixture {layout::tests::AhemDocument(u8"math { font: 20px math }",
                                                         u8"<math><mo id=o lspace=0 rspace=0>-</mo></math>")};
    auto &op = OperatorById(fixture, u8"o");
    auto font = op.Style().GetFontCascade().PrimaryFont();
    auto minus = font->GlyphForCharacter(0x2212);
    REQUIRE(minus != 0);
    auto bounds = font->BoundsForGlyph(minus);
    CHECK(op.LogicalWidth() == LayoutUnit(font->WidthForGlyph(minus)));
    CHECK(op.LogicalHeight() == LayoutUnit(-bounds.Y()) + LayoutUnit(bounds.MaxY()));
    CHECK(static_cast<RenderBox &>(op).FirstLineBaseline()
          == LayoutUnit(std::round(LayoutUnit(-bounds.Y()).ToFloat())));
    // One glyph, the minus sign, and not the text.
    auto runs = PaintedRuns(fixture);
    CHECK(runs.starts_with("1@0,"));
    CHECK(runs.find(' ') == std::string::npos);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderMathMLOperator: vertical stretching",
                   "[Rendering][MathML][RenderMathMLOperator]")
  {
    SECTION("A size variant tall enough for the row")
    {
      auto fixture = StretchedOperator(u8"", 30, 10);
      auto &op = OperatorById(fixture, u8"o");
      CHECK(op.IsStretchy());
      CHECK(op.IsVertical());
      CHECK(op.StretchSize() == 40_lu);
      // The 40px variant, on the row's ascent and descent.
      CHECK(op.Size() == LayoutSize(10, 40));
      CHECK(op.Location() == LayoutPoint(0, 0));
      CHECK(static_cast<RenderBox &>(op).FirstLineBaseline() == 30_lu);
      CHECK(BoxById(fixture, u8"s").Location() == LayoutPoint(10, 0));
      CHECK(op.MaxPreferredLogicalWidth() == 10_lu);
      CHECK(PaintedRuns(fixture) == "1@0,40");
    }

    SECTION("The smallest variant that's large enough")
    {
      auto fixture = StretchedOperator(u8"", 15, 0);
      CHECK(OperatorById(fixture, u8"o").LogicalHeight() == 20_lu);
    }

    SECTION("Between the largest variant and the smallest assembly, the largest variant")
    {
      auto fixture = StretchedOperator(u8"", 45, 0);
      CHECK(OperatorById(fixture, u8"o").LogicalHeight() == 40_lu);
    }

    SECTION("A glyph assembly beyond the variants")
    {
      auto fixture = StretchedOperator(u8"", 80, 20);
      auto &op = OperatorById(fixture, u8"o");
      CHECK(op.Size() == LayoutSize(10, 100));
      CHECK(op.Location() == LayoutPoint(0, 0));
      CHECK(static_cast<RenderBox &>(op).FirstLineBaseline() == 80_lu);
      // The top and bottom pieces, then the extender between them.
      CHECK(PaintedRuns(fixture).starts_with("1@0,20 1@0,100 1@0,"));
    }

    SECTION("An empty row stretches to an em, which needs no stretching")
    {
      auto fixture = StretchyDocument(u8"<math><mo id=o lspace=0 rspace=0>&#x295C;</mo></math>");
      CHECK(OperatorById(fixture, u8"o").Size() == LayoutSize(10, 10));
    }

    SECTION("maxsize and minsize scale the height and depth")
    {
      auto maxed = StretchedOperator(u8"maxsize=50px", 80, 20);
      CHECK(OperatorById(maxed, u8"o").StretchSize() == 50_lu);
      CHECK(OperatorById(maxed, u8"o").LogicalHeight() == 50_lu);

      auto mined = StretchedOperator(u8"minsize=60px", 20, 0);
      CHECK(OperatorById(mined, u8"o").LogicalHeight() == 60_lu);

      // Without minsize, the minimum is an em.
      auto small = StretchedOperator(u8"", 5, 0);
      CHECK(OperatorById(small, u8"o").StretchSize() == 10_lu);
    }

    SECTION("Symmetric operators stretch as far above the axis as below")
    {
      auto fixture = StretchedOperator(u8"symmetric=true", 70, 10);
      auto &op = OperatorById(fixture, u8"o");
      auto font = op.Style().GetFontCascade().PrimaryFont();
      LayoutUnit axis {font->MathData()->GetMathConstant(*font, OpenTypeMathData::MathConstant::AxisHeight)};
      CHECK(op.StretchSize() == std::max(70_lu - axis, 10_lu + axis) * 2);
    }

    SECTION("Fonts without a MATH table stretch brackets from Unicode's pieces")
    {
      ResolverFixture fixture {AhemMath(u8"<math><mo id=o lspace=0 rspace=0 symmetric=false>(</mo>"
                                        u8"<mspace width=10px height=80px depth=20px></mspace></math>")};
      auto &op = OperatorById(fixture, u8"o");
      CHECK(op.IsStretchy());
      CHECK(op.LogicalHeight() == 100_lu);
      CHECK(static_cast<RenderBox &>(op).FirstLineBaseline() == 80_lu);
      // The top, bottom and extender pieces, and not the text.
      auto runs = PaintedRuns(fixture);
      CHECK(runs.starts_with("1@0,"));
      CHECK(std::ranges::count(runs, ' ') >= 2);
    }

    SECTION("Without stretchy it isn't stretched")
    {
      auto fixture = StretchedOperator(u8"", 80, 20);
      fixture.ById(u8"o").SetAttribute(u8"stretchy", u8"false");
      auto &op = OperatorById(fixture, u8"o");
      CHECK_FALSE(op.IsStretchy());
      CHECK(op.Size() == LayoutSize(10, 10));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderMathMLOperator: horizontal stretching",
                   "[Rendering][MathML][RenderMathMLOperator]")
  {
    auto fixture = StretchyDocument(u8"<math><mo id=o lspace=1px rspace=2px>&#x295A;</mo></math>");
    auto &op = OperatorById(fixture, u8"o");
    REQUIRE(op.IsStretchy());
    CHECK_FALSE(op.IsVertical());
    CHECK(op.Size() == LayoutSize(13, 10));

    // The 30px variant.
    op.StretchTo(25_lu);
    CHECK(op.StretchSize() == 25_lu);
    CHECK(op.LogicalWidth() == 25_lu + 3_lu);

    op.StretchTo(100_lu);
    CHECK(op.LogicalWidth() == 100_lu + 3_lu);
    CHECK(op.LogicalHeight() == 10_lu);

    op.SetStretchWidthLocked(true);
    CHECK(op.IsStretchWidthLocked());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderMathMLOperator: large operators",
                   "[Rendering][MathML][RenderMathMLOperator]")
  {
    // The display variant of U+2AFF is at least DisplayOperatorMinHeight (20px) tall, with an italic
    // correction of 30px.
    RegisterMathFont("largeop-displayoperatorminheight2000-2AFF-italiccorrection3000");
    ResolverFixture fixture {layout::tests::AhemDocument(
      u8"math { font-family: largeop-displayoperatorminheight2000-2AFF-italiccorrection3000 }",
      u8"<math id=math display=block><mo id=o lspace=0 rspace=0 largeop=true>&#x2AFF;</mo></math>")};

    auto &op = OperatorById(fixture, u8"o");
    CHECK(op.IsLargeOperatorInDisplayStyle());
    CHECK(op.ItalicCorrection() == 30_lu);
    CHECK(op.LogicalHeight() >= 20_lu);

    // Inline, it's the base glyph.
    fixture.ById(u8"math").SetAttribute(u8"display", u8"inline");
    auto &inlineOp = OperatorById(fixture, u8"o");
    CHECK_FALSE(inlineOp.IsLargeOperatorInDisplayStyle());
    CHECK(inlineOp.ItalicCorrection() == 0_lu);
    CHECK(inlineOp.LogicalHeight() < 20_lu);
  }
}
