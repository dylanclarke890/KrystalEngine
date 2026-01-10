#include "Krystal.Text/Count/CountAsEncoded.hpp"
#include "Krystal.Text.Tests/BasicUnicodeStrings.hpp"
#include "Krystal.Text/Encodings/UTF16.hpp"
#include "Krystal.Text/Encodings/UTF32.hpp"
#include "Krystal.Text/Encodings/UTF8.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Text;

  TEST_CASE("CountAsEncoded(utf8)", "[Text][CountAsEncoded]")
  {
    auto result0 = CountAsEncoded(u32_basic_source_character_set, utf8);
    std::size_t expected0 = std::size(u8_basic_source_character_set);
    REQUIRE_FALSE(result0.ErrorsWereHandled());
    REQUIRE(result0.Count == expected0);

    auto result1 = CountAsEncoded(u32_unicode_sequence_truth_native_endian, utf8);
    std::size_t expected1 = std::size(u8_unicode_sequence_truth_native_endian);
    REQUIRE_FALSE(result1.ErrorsWereHandled());
    REQUIRE(result1.Count == expected1);
  }

  TEST_CASE("CountAsEncoded(utf16)", "[Text][CountAsEncoded]")
  {
    auto result0 = CountAsEncoded(u32_basic_source_character_set, utf16);
    std::size_t expected0 = std::size(u16_basic_source_character_set);
    REQUIRE_FALSE(result0.ErrorsWereHandled());
    REQUIRE(result0.Count == expected0);

    auto result1 = CountAsEncoded(u32_unicode_sequence_truth_native_endian, utf16);
    std::size_t expected1 = std::size(u16_unicode_sequence_truth_native_endian);
    REQUIRE_FALSE(result1.ErrorsWereHandled());
    REQUIRE(result1.Count == expected1);
  }

  TEST_CASE("CountAsEncoded(utf32)", "[Text][CountAsEncoded]")
  {
    auto result0 = CountAsEncoded(u32_basic_source_character_set, utf32);
    std::size_t expected0 = std::size(u32_basic_source_character_set);
    REQUIRE_FALSE(result0.ErrorsWereHandled());
    REQUIRE(result0.Count == expected0);

    auto result1 = CountAsEncoded(u32_unicode_sequence_truth_native_endian, utf32);
    std::size_t expected1 = std::size(u32_unicode_sequence_truth_native_endian);
    REQUIRE_FALSE(result1.ErrorsWereHandled());
    REQUIRE(result1.Count == expected1);
  }
}