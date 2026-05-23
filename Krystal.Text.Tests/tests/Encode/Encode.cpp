#include "Krystal.Text/Encode/Encode.hpp"
#include "Krystal.Text.Tests/BasicUnicodeStrings.hpp"
#include "Krystal.Text/Encodings/UTF16.hpp"
#include "Krystal.Text/Encodings/UTF32.hpp"
#include "Krystal.Text/Encodings/UTF8.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Text;

  TEST_CASE("Encode(utf8)", "[Text][Encode]")
  {
    std::basic_string<uchar8> result0 = Encode(u32_basic_source_character_set, utf8);
    REQUIRE(result0 == u8_basic_source_character_set);

    std::basic_string<uchar8> result1 = Encode(u32_unicode_sequence_truth_native_endian, utf8);
    REQUIRE(result1 == u8_unicode_sequence_truth_native_endian);
  }

  TEST_CASE("Encode(utf16)", "[Text][Encode]")
  {
    std::u16string result0 = Encode(u32_basic_source_character_set, utf16);
    REQUIRE(result0 == u16_basic_source_character_set);

    std::u16string result1 = Encode(u32_unicode_sequence_truth_native_endian, utf16);
    REQUIRE(result1 == u16_unicode_sequence_truth_native_endian);
  }

  TEST_CASE("Encode(utf32)", "[Text][Encode]")
  {
    std::u32string result0 = Encode(u32_basic_source_character_set, utf32);
    REQUIRE(result0 == u32_basic_source_character_set);

    std::u32string result1 = Encode(u32_unicode_sequence_truth_native_endian, utf32);
    REQUIRE(result1 == u32_unicode_sequence_truth_native_endian);
  }

  // SECTION("execution")
  //{
  //   Krys::execution_t encoding {};
  //   std::string result0 =
  //     Krys::encode(u32_basic_source_character_set, encoding, Krys::replacement_handler);
  //   REQUIRE(result0 == basic_source_character_set);

  // if (Krys::contains_unicode_encoding(encoding))
  // {
  //   std::string result1 = Krys::encode(u32_unicode_sequence_truth_native_endian, encoding,
  //                                           Krys::replacement_handler);
  //   REQUIRE(result1 == unicode_sequence_truth_native_endian);
  // }
  // }
  // SECTION("wide_execution")
  //{
  // Krys::wide_execution_t encoding {};
  // std::wstring result0 =
  //   Krys::encode(u32_basic_source_character_set, encoding, Krys::replacement_handler);
  // REQUIRE(result0 == w_basic_source_character_set);

  // if (Krys::contains_unicode_encoding(encoding))
  // {
  //   std::wstring result1 = Krys::encode(u32_unicode_sequence_truth_native_endian, encoding,
  //                                            Krys::replacement_handler);
  //   REQUIRE(result1 == w_unicode_sequence_truth_native_endian);
  // }
  // }
  // SECTION("literal")
  //{
  // Krys::literal_t encoding {};
  // std::string result0 =
  //   Krys::encode(u32_basic_source_character_set, encoding, Krys::replacement_handler);
  // REQUIRE(result0 == basic_source_character_set);

  // if (Krys::contains_unicode_encoding(encoding))
  // {
  //   std::string result1 = Krys::encode(u32_unicode_sequence_truth_native_endian, encoding,
  //                                           Krys::replacement_handler);
  //   REQUIRE(result1 == unicode_sequence_truth_native_endian);
  // }
  // }
  // SECTION("wide_literal")
  //{
  // Krys::wide_literal_t encoding {};
  // std::wstring result0 = Krys::encode(u32_basic_source_character_set, encoding);
  // REQUIRE(result0 == w_basic_source_character_set);

  // if (Krys::contains_unicode_encoding(encoding))
  // {
  //   std::wstring result1 = Krys::encode(u32_unicode_sequence_truth_native_endian, encoding,
  //                                            Krys::replacement_handler);
  //   REQUIRE(result1 == w_unicode_sequence_truth_native_endian);
  // }
  // }
}