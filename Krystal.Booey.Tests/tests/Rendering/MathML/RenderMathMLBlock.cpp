#include "Krystal.Booey/Rendering/MathML/RenderMathMLBlock.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/OpenType/OpenTypeMathData.hpp"
#include "Krystal.Booey/Rendering/MathML/RenderMathMLMath.hpp"
#include "Krystal.Booey/Rendering/MathML/RenderMathMLToken.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;
  using Length = mathml::MathMLElement::Length;
  using LengthType = mathml::MathMLElement::LengthType;

  namespace
  {
    KRYS_NODISCARD ResolverFixture LaidOut(utf8_stringview body, utf8_stringview style = u8"") noexcept
    {
      ResolverFixture fixture {layout::tests::AhemDocument(style, body)};
      (void)fixture.Document->UpdateLayout();
      return fixture;
    }

    KRYS_NODISCARD RenderBox &BoxById(ResolverFixture &fixture, dom::DOMStringView id) noexcept
    {
      auto *renderer = fixture.ById(id).Renderer();
      REQUIRE(renderer);
      return Downcast<RenderBox>(*renderer);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "ToUserUnits", "[Rendering][MathML][RenderMathMLBlock]")
  {
    auto fixture = LaidOut(u8"<math><mspace id=s style=\"font-size: 18px\"></mspace>"
                           u8"<mspace id=z style=\"font-size: 18px; zoom: 2\"></mspace></math>");
    const auto &style = BoxById(fixture, u8"s").Style();
    auto units = [&](LengthType type, float value, LayoutUnit reference = 30_lu)
    {
      return ToUserUnits(Length {.Type = type, .Value = value}, style, reference);
    };

    CHECK(units(LengthType::Px, 7) == 7_lu);
    CHECK(units(LengthType::In, 1) == 96_lu);
    CHECK(units(LengthType::Cm, 2.54f) == 96_lu);
    CHECK(units(LengthType::Mm, 25.4f) == 96_lu);
    CHECK(units(LengthType::Pt, 72) == 96_lu);
    CHECK(units(LengthType::Pc, 6) == 96_lu);
    CHECK(units(LengthType::Em, 2) == 36_lu);
    // A math unit is an eighteenth of an em.
    CHECK(units(LengthType::MathUnit, 9) == 9_lu);
    CHECK(units(LengthType::Ex, 1) == LayoutUnit(style.MetricsOfPrimaryFont().XHeight().value_or(0)));
    // Percentages and unitless values are relative to the reference, which a failed parse returns.
    CHECK(units(LengthType::Percentage, 50) == 15_lu);
    CHECK(units(LengthType::UnitLess, 2) == 60_lu);
    CHECK(units(LengthType::ParsingFailed, 5) == 30_lu);

    // Physical units are zoomed; em lengths already are, through the font.
    const auto &zoomed = BoxById(fixture, u8"z").Style();
    CHECK(ToUserUnits(Length {.Type = LengthType::Px, .Value = 7}, zoomed, 0_lu) == 14_lu);
    CHECK(ToUserUnits(Length {.Type = LengthType::Em, .Value = 1}, zoomed, 0_lu) == 36_lu);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderMathMLBlock: the renderers of the elements",
                   "[Rendering][MathML][RenderMathMLBlock]")
  {
    auto fixture = LaidOut(u8"<math id=math><mrow id=row>text<mi id=mi>x</mi></mrow><mo id=mo>+</mo>"
                           u8"<mspace id=space></mspace><mpadded id=padded></mpadded><foo id=foo></foo>"
                           u8"<mtable id=table><mtr id=tr><mtd id=td></mtd></mtr></mtable></math>");

    CHECK(Is<RenderMathMLMath>(BoxById(fixture, u8"math")));
    CHECK(BoxById(fixture, u8"row").RenderName() == u8"RenderMathMLRow");
    CHECK(BoxById(fixture, u8"mi").RenderName() == u8"RenderMathMLToken");
    // FIXME: <mo> is made a RenderMathMLOperator in B12d.
    CHECK(Is<RenderMathMLToken>(BoxById(fixture, u8"mo")));
    CHECK(BoxById(fixture, u8"space").RenderName() == u8"RenderMathMLSpace");
    CHECK(BoxById(fixture, u8"padded").RenderName() == u8"RenderMathMLPadded");
    // Unknown elements are laid out as rows.
    CHECK(BoxById(fixture, u8"foo").RenderName() == u8"RenderMathMLRow");
    CHECK(Is<RenderMathMLTable>(BoxById(fixture, u8"table")));
    CHECK(BoxById(fixture, u8"table").IsRenderTable());
    CHECK_FALSE(BoxById(fixture, u8"table").IsRenderMathMLBlock());
    CHECK(BoxById(fixture, u8"tr").IsRenderTableRow());
    CHECK(BoxById(fixture, u8"td").IsRenderTableCell());

    // Only elements are children of a MathML block; the text in <mrow> has no renderer.
    auto &row = BoxById(fixture, u8"row");
    REQUIRE(row.FirstChild());
    CHECK(row.FirstChild() == row.LastChild());
    CHECK(row.FirstChild()->GetNode() == &fixture.ById(u8"mi"));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderMathMLBlock: the render tree",
                   "[Rendering][MathML][RenderMathMLBlock]")
  {
    SECTION("Tokens wrap their text in an anonymous block, which isn't dropped")
    {
      auto fixture = LaidOut(u8"<math><mi id=mi>x</mi></math>");
      auto &token = BoxById(fixture, u8"mi");
      auto *anonymous = DynamicDowncast<RenderBlock>(token.FirstChild());
      REQUIRE(anonymous);
      CHECK(anonymous->IsAnonymous());
      CHECK_FALSE(anonymous->IsRenderMathMLBlock());
      CHECK(anonymous->FirstChild()->IsRenderText());
    }

    SECTION("Padding is dumped, and CSS sizes apply to the content box")
    {
      auto fixture = LaidOut(u8"<math><mspace width=20px height=10px style=\"padding: 1px 2px; width: 50px\">"
                             u8"</mspace></math>");
      CHECK(ExternalRepresentation(fixture.Frame.get()) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x13
  RenderBlock {HTML} at (0,0) size 800x13
    RenderBody {BODY} at (0,0) size 800x13
      RenderMathMLMath {math} at (0,0) size 54x12
        RenderMathMLSpace {mspace} at (0,0) size 54x12 [padding: 1 2 1 2]
)");
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderMathMLTable: centred on the math axis",
                   "[Rendering][MathML][RenderMathMLBlock]")
  {
    auto fixture =
      LaidOut(u8"<math><mtable id=table><mtr><mtd><mspace width=10px height=20px></mspace></mtd>"
              u8"</mtr><mtr><mtd><mspace width=10px height=20px></mspace></mtd></mtr></mtable></math>");
    auto &table = BoxById(fixture, u8"table");

    auto primaryFont = table.Style().GetFontCascade().PrimaryFont();
    REQUIRE(primaryFont->MathData());
    auto axisHeight = LayoutUnit(
      primaryFont->MathData()->GetMathConstant(*primaryFont, OpenTypeMathData::MathConstant::AxisHeight));
    auto baseline = table.FirstLineBaseline();
    REQUIRE(baseline);
    CHECK(*baseline == LayoutUnit((table.LogicalHeight() / 2 + axisHeight).ToInt()));

    // An inline table is wrapped in an anonymous block, whose baseline is the table's.
    auto *wrapper = table.Parent();
    REQUIRE(wrapper);
    CHECK(wrapper->IsAnonymous());
    CHECK(Downcast<RenderBox>(*wrapper).FirstLineBaseline() == baseline);
  }
}
