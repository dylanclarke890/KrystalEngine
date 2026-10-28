#include "Krystal.Booey/MathML/MathMLElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/CSS/Properties/ImmutableStyleProperties.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Rendering/RenderTableCell.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::mathml::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    KRYS_NODISCARD std::string Hints(utf8_string markup) noexcept
    {
      auto document = ResolverFixture::Parse(krys::move(markup));
      auto element = document->GetElementById(u8"e");
      krys_assert(element);
      const auto *style = Downcast<dom::StyledElement>(*element).PresentationalHintStyle();
      return style ? css::tests::ToNarrow(style->AsText(css::DefaultSerialisationContext())) : std::string {};
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "MathMLElement: presentational hints",
                   "[MathML][PresentationalHints]")
  {
    CHECK(Hints(u8"<math><mi id=e mathcolor=red mathbackground=blue dir=rtl></mi></math>")
          == "color: red; background-color: blue; direction: rtl;");
    CHECK(Hints(u8"<math><mi id=e mathsize=12px></mi></math>") == "font-size: 12px;");
    CHECK(Hints(u8"<math><mi id=e mathsize=150%></mi></math>") == "font-size: 150%;");
    CHECK(Hints(u8"<math><mi id=e mathsize=0></mi></math>") == "font-size: 0px;");
    CHECK(Hints(u8"<math><mstyle id=e displaystyle=TRUE></mstyle></math>") == "math-style: normal;");
    CHECK(Hints(u8"<math><mstyle id=e displaystyle=false></mstyle></math>") == "math-style: compact;");
    CHECK(Hints(u8"<math><mstyle id=e displaystyle=maybe></mstyle></math>").empty());
    CHECK(Hints(u8"<math><mstyle id=e scriptlevel=2></mstyle></math>") == "math-depth: 2;");
    CHECK(Hints(u8"<math><mstyle id=e scriptlevel=+1></mstyle></math>") == "math-depth: add(1);");
    CHECK(Hints(u8"<math><mstyle id=e scriptlevel=-1></mstyle></math>") == "math-depth: add(-1);");
    CHECK(Hints(u8"<math><mstyle id=e scriptlevel=x></mstyle></math>").empty());

    SECTION("mathsize takes CSS lengths and percentages, not numbers or font-size keywords")
    {
      for (auto value : {u8"2", u8" 1.5", u8"small", u8"x-large", u8"medium", u8"larger", u8"math", u8"big"})
      {
        CAPTURE(css::tests::ToNarrow(value));
        CHECK(Hints(u8"<math><mi id=e mathsize=\"" + utf8_string(value) + u8"\"></mi></math>").empty());
      }
    }

    SECTION("The deprecated attributes give no hints")
    {
      CHECK(
        Hints(u8"<math><mi id=e fontsize=20px fontweight=bold fontstyle=italic fontfamily=serif color=red "
              u8"background=blue></mi></math>")
          .empty());
    }

    SECTION("mathvariant=normal on <mi> turns off its automatic italics")
    {
      CHECK(Hints(u8"<math><mi id=e mathvariant=NORMAL>x</mi></math>") == "text-transform: none;");
      CHECK(Hints(u8"<math><mi id=e mathvariant=bold>x</mi></math>").empty());
      CHECK(Hints(u8"<math><mn id=e mathvariant=normal>1</mn></math>").empty());
      CHECK(Hints(u8"<math><mstyle id=e mathvariant=normal></mstyle></math>").empty());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "MathMLElement: an <mtd>'s spans", "[MathML][MathMLElement]")
  {
    auto document = ResolverFixture::Parse(u8"<math><mtable><mtr><mtd id=a columnspan=3 rowspan=2></mtd>"
                                           u8"<mtd id=b columnspan=0 rowspan=x></mtd>"
                                           u8"<mtd id=c rowspan=70000></mtd></mtr></mtable>"
                                           u8"<mrow id=d columnspan=3 rowspan=2></mrow></math>");
    auto span = [&](dom::DOMStringView id)
    {
      auto &element = Downcast<MathMLElement>(*document->GetElementById(id));
      return std::pair {element.ColSpan(), element.RowSpan()};
    };

    CHECK(span(u8"a") == std::pair {3u, 2u});
    // Zero and invalid spans are 1.
    CHECK(span(u8"b") == std::pair {1u, 1u});
    // Row spans are clamped as HTML's are.
    CHECK(span(u8"c") == std::pair {1u, 65'534u});
    // Only an <mtd> spans.
    CHECK(span(u8"d") == std::pair {1u, 1u});
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "MathMLElement: an <mtd>'s renderer spans its columns",
                   "[MathML][MathMLElement]")
  {
    boo::tests::PaintFixture fixture {
      u8"<!DOCTYPE html><math><mtable><mtr><mtd id=a columnspan=2></mtd><mtd></mtd></mtr>"
      u8"<mtr><mtd></mtd></mtr></mtable></math>"};
    (void)fixture.Document->UpdateLayout();

    auto &cellElement = fixture.ById(u8"a");
    auto *cell = DynamicDowncast<RenderTableCell>(cellElement.Renderer());
    REQUIRE(cell);
    CHECK(cell->ColSpan() == 2u);

    // Changing the attribute updates the cell.
    REQUIRE_FALSE(cellElement.SetAttribute(u8"columnspan", u8"3").HasException());
    (void)fixture.Document->UpdateLayout();
    CHECK(Downcast<RenderTableCell>(*cellElement.Renderer()).ColSpan() == 3u);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "MathMLElement: which children get renderers",
                   "[MathML][MathMLElement]")
  {
    boo::tests::PaintFixture fixture {
      u8"<!DOCTYPE html><math><mrow id=row>text</mrow><mi id=mi>x</mi></math>"};
    // HTML elements in MathML (the parser takes most of them out of the formula).
    auto &row = fixture.ById(u8"row");
    auto &mi = fixture.ById(u8"mi");
    auto rowSpan = fixture.Document->CreateElement(u8"span").ReleaseValue();
    auto miSpan = fixture.Document->CreateElement(u8"span").ReleaseValue();
    REQUIRE_FALSE(rowSpan->TextContent(u8"html").HasException());
    REQUIRE_FALSE(miSpan->TextContent(u8"html").HasException());
    REQUIRE_FALSE(row.AppendChild(*rowSpan).HasException());
    REQUIRE_FALSE(mi.AppendChild(*miSpan).HasException());
    (void)fixture.Document->UpdateLayout();

    // Text and HTML aren't rendered in most MathML elements...
    REQUIRE(row.Renderer());
    CHECK(row.FirstChild()->Renderer() == nullptr);
    CHECK(rowSpan->Renderer() == nullptr);

    // ...but are in token elements.
    REQUIRE(mi.Renderer());
    CHECK(mi.FirstChild()->Renderer() != nullptr);
    CHECK(miSpan->Renderer() != nullptr);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "MathMLElement: the MathML Core style sheets",
                   "[MathML][UserAgentStyle]")
  {
    ResolverFixture fixture {
      u8"<!DOCTYPE html><style>math { font-size: 20px }</style><math>"
      u8"<semantics><mi id=shown>x</mi><annotation id=hidden>x</annotation></semantics>"
      u8"<maction><mi>x</mi><mi id=action>y</mi></maction>"
      u8"<msub><mi id=base>x</mi><mi id=script>2</mi></msub></math>",
      style::tests::WithView::No};

    // <semantics> and <maction> show only their first child.
    CHECK(fixture.Resolve(fixture.ById(u8"shown")).Style->EffectiveDisplay() != DisplayType::None);
    CHECK(fixture.Resolve(fixture.ById(u8"hidden")).Style->EffectiveDisplay() == DisplayType::None);
    CHECK(fixture.Resolve(fixture.ById(u8"action")).Style->EffectiveDisplay() == DisplayType::None);

    // Scripts are scaled down by their math depth (font-size: math), by 0.71 per level without a MATH table.
    CHECK(fixture.Resolve(fixture.ById(u8"base")).Style->ComputedFontSize() == 20);
    auto scriptSize = fixture.Resolve(fixture.ById(u8"script")).Style->ComputedFontSize();
    CHECK(scriptSize > 14.1f);
    CHECK(scriptSize < 14.3f);
  }
}
