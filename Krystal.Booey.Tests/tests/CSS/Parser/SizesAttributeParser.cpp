#include "Krystal.Booey/CSS/Parser/SizesAttributeParser.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontCascade.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    /// @brief The length @p sizes evaluates to in a 400x300 view.
    KRYS_NODISCARD float Length(utf8_stringview sizes) noexcept
    {
      boo::tests::PaintFixture fixture(u8"<!DOCTYPE html>", {400, 300});
      (void)fixture.Document->UpdateLayout();
      return SizesAttributeParser(sizes, *fixture.Document).Length();
    }
  }

  TEST_CASE("SizesAttributeParser: the first entry whose condition matches",
            "[CSS][Parser][SizesAttributeParser][html:parse-a-sizes-attribute][html:valid-source-size-list]")
  {
    CHECK(Length(u8"") == 400);
    CHECK(Length(u8"50vw") == 200);
    CHECK(Length(u8"120px") == 120);
    CHECK(Length(u8"0") == 0);
    CHECK(Length(u8"(max-width: 300px) 100px, 250px") == 250);
    CHECK(Length(u8"(min-width: 300px) 100px, 250px") == 100);
    CHECK(Length(u8"(min-width: 900px) 100px") == 400);
    CHECK(Length(u8"(min-width: 300px) 5em, 250px") == 80);
  }

  TEST_CASE("SizesAttributeParser: font-relative units measure the view's font, keeping its font selector",
            "[CSS][Parser][SizesAttributeParser][html:parse-a-sizes-attribute]")
  {
    boo::tests::PaintFixture fixture(u8"<!DOCTYPE html>", {400, 300});
    (void)fixture.Document->UpdateLayout();
    auto &fontCascade = fixture.Document->GetRenderView()->Style().GetFontCascade();
    auto *fontSelector = fontCascade.GetFontSelector();

    for (auto unit : {u8"ex", u8"cap", u8"ch", u8"ic"})
    {
      INFO(reinterpret_cast<const char *>(unit));
      auto length = SizesAttributeParser(utf8_string(u8"10") + unit, *fixture.Document).Length();
      // A font's x-height, cap height and widths are within (0, 2] em of its 16px size.
      CHECK(length > 0);
      CHECK(length <= 320);
      CHECK(fontCascade.GetFontSelector() == fontSelector);
    }
  }

  TEST_CASE("SizesAttributeParser: invalid entries are skipped",
            "[CSS][Parser][SizesAttributeParser][html:parse-a-sizes-attribute]")
  {
    CHECK(Length(u8"-10px, 30px") == 30);
    CHECK(Length(u8"10%, 30px") == 30);
    CHECK(Length(u8"auto") == 400);
    CHECK(Length(u8"(bogus) 10px, 30px") == 30);
  }

  TEST_CASE("SizesCalcParser: calc() in sizes", "[CSS][Parser][SizesAttributeParser]")
  {
    CHECK(Length(u8"calc(50vw - 20px)") == 180);
    CHECK(Length(u8"calc((10px + 20px) * 2)") == 60);
    CHECK(Length(u8"calc(100px / 4 + 1px)") == 26);
    CHECK(Length(u8"calc(10px - 20px)") == 0);
    // Invalid: mixed lengths and numbers, division by a length or zero.
    CHECK(Length(u8"calc(10px + 2), 30px") == 30);
    CHECK(Length(u8"calc(10px / 0), 30px") == 30);
    CHECK(Length(u8"calc(10px / 2px), 30px") == 30);
    CHECK(Length(u8"min(10px, 20px), 30px") == 30);
  }
}
