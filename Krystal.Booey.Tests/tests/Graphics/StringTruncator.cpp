#include "Krystal.Booey/Graphics/StringTruncator.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/FontCascadeDescription.hpp"

namespace krys::boo::tests
{
  namespace
  {
    /// @brief 10px Ahem: every glyph is a 10px square.
    KRYS_NODISCARD FontCascade Ahem() noexcept
    {
      layout::tests::RegisterAhem();
      FontCascadeDescription description;
      description.SetFamilies({StringAtom {u8"Ahem"}});
      description.SetComputedSize(10);
      FontCascade cascade {krys::move(description)};
      cascade.Update();
      return cascade;
    }

    constexpr utf8_stringview Ellipsis = u8"\u2026";
  }

  TEST_CASE("StringTruncator", "[Graphics][StringTruncator]")
  {
    auto font = Ahem();
    const float ellipsisWidth = StringTruncator::Width(Ellipsis, font);
    REQUIRE(StringTruncator::Width(u8"abcdefghij", font) == 100);

    SECTION("Strings that fit are kept")
    {
      CHECK(StringTruncator::RightTruncate(u8"abcde", 50, font) == u8"abcde");
      CHECK(StringTruncator::CenterTruncate(u8"abcde", 80, font) == u8"abcde");
      CHECK(StringTruncator::RightTruncate(u8"", 0, font).empty());
    }

    SECTION("Right truncation keeps the start and as much as fits")
    {
      float resultWidth = 0;
      auto result = StringTruncator::RightTruncate(u8"abcdefghij", 40 + ellipsisWidth, font, resultWidth);
      CHECK(result == utf8_string(u8"abcd") + utf8_string(Ellipsis));
      CHECK(resultWidth == 40 + ellipsisWidth);
    }

    SECTION("Centre truncation keeps both ends")
    {
      auto result = StringTruncator::CenterTruncate(u8"abcdefghij", 40 + ellipsisWidth, font);
      CHECK(result == utf8_string(u8"ab") + utf8_string(Ellipsis) + utf8_string(u8"ij"));
    }

    SECTION("Left truncation keeps the end")
    {
      float resultWidth = 0;
      auto result = StringTruncator::LeftTruncate(u8"abcdefghij", 40 + ellipsisWidth, font, resultWidth);
      CHECK(result == utf8_string(Ellipsis) + utf8_string(u8"ghij"));
    }

    SECTION("Clipping adds no ellipsis")
    {
      float resultWidth = 0;
      CHECK(StringTruncator::RightClipToCharacter(u8"abcdefghij", 45, font, resultWidth, false) == u8"abcd");
      CHECK(resultWidth == 40);
      CHECK(StringTruncator::RightClipToWord(u8"ab cd efgh", 75, font, resultWidth, false) == u8"ab cd ");
    }

    SECTION("Grapheme clusters aren't split")
    {
      // Each "e" with a combining acute accent is one cluster of three bytes.
      utf8_string accented;
      for (int32 i = 0; i < 6; ++i)
      {
        accented += u8"e\u0301";
      }
      auto result =
        StringTruncator::RightTruncate(accented, StringTruncator::Width(accented, font) - 1, font);
      REQUIRE(result.ends_with(Ellipsis));
      auto kept = utf8_stringview(result).substr(0, result.size() - Ellipsis.size());
      CHECK(kept.size() % 3 == 0);
      CHECK(!kept.empty());
      CHECK(accented.starts_with(kept));
    }
  }
}
