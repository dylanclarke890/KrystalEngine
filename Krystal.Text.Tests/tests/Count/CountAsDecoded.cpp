#include "Krystal.Text/Count/CountAsDecoded.hpp"
#include "Krystal.Text.Tests/BasicUnicodeStrings.hpp"
#include "Krystal.Text/Encodings/UTF16.hpp"
#include "Krystal.Text/Encodings/UTF32.hpp"
#include "Krystal.Text/Encodings/UTF8.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Text;

  std::size_t expected0 = std::size(u32_basic_source_character_set);
  std::size_t expected1 = std::size(u32_unicode_sequence_truth_native_endian);

  TEST_CASE("CountAsDecoded(utf8)", "[Text][CountAsDecoded]")
  {
    auto result0 = CountAsDecoded(u8_basic_source_character_set, utf8);
    REQUIRE_FALSE(result0.ErrorsWereHandled());
    REQUIRE(result0.Count == expected0);

    auto result1 = CountAsDecoded(u8_unicode_sequence_truth_native_endian, utf8);
    REQUIRE_FALSE(result1.ErrorsWereHandled());
    REQUIRE(result1.Count == expected1);
  }

  TEST_CASE("CountAsDecoded(utf16)", "[Text][CountAsDecoded]")
  {
    auto result0 = CountAsDecoded(u16_basic_source_character_set, utf16);
    REQUIRE_FALSE(result0.ErrorsWereHandled());
    REQUIRE(result0.Count == expected0);

    auto result1 = CountAsDecoded(u16_unicode_sequence_truth_native_endian, utf16);
    REQUIRE_FALSE(result1.ErrorsWereHandled());
    REQUIRE(result1.Count == expected1);
  }

  TEST_CASE("CountAsDecoded(utf32)", "[Text][CountAsDecoded]")
  {
    auto result0 = CountAsDecoded(u32_basic_source_character_set, utf32);
    REQUIRE_FALSE(result0.ErrorsWereHandled());
    REQUIRE(result0.Count == expected0);

    auto result1 = CountAsDecoded(u32_unicode_sequence_truth_native_endian, utf32);
    REQUIRE_FALSE(result1.ErrorsWereHandled());
    REQUIRE(result1.Count == expected1);
  }
}