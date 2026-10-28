#include "Krystal.Booey/Rendering/MathML/MathVariant.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::tests
{
  TEST_CASE("MathVariantMapCodePoint: italics", "[Rendering][MathVariant]")
  {
    auto italic = [](char32 codePoint)
    {
      return MathVariantMapCodePoint(codePoint, MathVariant::Italic);
    };

    CHECK(italic(U'A') == U'\U0001D434');
    CHECK(italic(U'a') == U'\U0001D44E');
    CHECK(italic(U'z') == U'\U0001D467');
    // The italic h is outside the mathematical block.
    CHECK(italic(U'h') == U'ℎ');
    CHECK(italic(U'ı') == U'\U0001D6A4');
    CHECK(italic(U'α') == U'\U0001D6FC');
    CHECK(italic(U'Α') == U'\U0001D6E2');
    CHECK(italic(U'∇') == U'\U0001D6FB');

    // Digits, symbols and characters already transformed have no italics.
    CHECK(italic(U'1') == U'1');
    CHECK(italic(U'+') == U'+');
    CHECK(italic(U'\U0001D44E') == U'\U0001D44E');
    CHECK(italic(U'΢') == U'΢');
  }

  TEST_CASE("MathVariantMapCodePoint: the other variants", "[Rendering][MathVariant]")
  {
    CHECK(MathVariantMapCodePoint(U'a', MathVariant::Normal) == U'a');
    CHECK(MathVariantMapCodePoint(U'A', MathVariant::Bold) == U'\U0001D400');
    CHECK(MathVariantMapCodePoint(U'1', MathVariant::DoubleStruck) == U'\U0001D7D9');
    CHECK(MathVariantMapCodePoint(U'R', MathVariant::DoubleStruck) == U'ℝ');
    CHECK(MathVariantMapCodePoint(U'Ϝ', MathVariant::Bold) == U'\U0001D7CA');
    CHECK(MathVariantMapCodePoint(U'ب', MathVariant::Initial) == U'\U0001EE21');
    // Arabic letters without the variant, and Latin letters in an Arabic variant, are unchanged.
    CHECK(MathVariantMapCodePoint(U'ا', MathVariant::Initial) == U'ا');
    CHECK(MathVariantMapCodePoint(U'a', MathVariant::Looped) == U'a');
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderText: text-transform: math-auto",
                   "[Rendering][RenderText][MathML]")
  {
    PaintFixture fixture {u8"<!DOCTYPE html><math><mi id=x>x</mi><mi id=sin>sin</mi><mi id=normal "
                          u8"mathvariant=normal>x</mi><mn id=n>x</mn></math>"
                          u8"<span id=span style=\"text-transform: math-auto\">h</span>"};
    (void)fixture.Document->UpdateLayout();

    auto text = [&](dom::DOMStringView id) -> std::string
    {
      auto *renderer = fixture.ById(id).FirstChild()->Renderer();
      REQUIRE(renderer);
      return css::tests::ToNarrow(Downcast<RenderText>(*renderer).Text());
    };

    // A single letter in <mi> is italic.
    CHECK(text(u8"x") == css::tests::ToNarrow(u8"\U0001D465"));
    // Longer identifiers aren't.
    CHECK(text(u8"sin") == "sin");
    // mathvariant=normal turns it off.
    CHECK(text(u8"normal") == "x");
    // Other tokens aren't transformed.
    CHECK(text(u8"n") == "x");
    // The transform applies to any text.
    CHECK(text(u8"span") == css::tests::ToNarrow(u8"ℎ"));
  }
}
