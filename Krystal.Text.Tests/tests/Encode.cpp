#include "Krystal.Text/Encode.hpp"
#include "Krystal.Text.Tests/BasicUnicodeStrings.hpp"
#include "Krystal.Text/Encodings/UTF16.hpp"
#include "Krystal.Text/Encodings/UTF32.hpp"
#include "Krystal.Text/Encodings/UTF8.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  TEST_CASE("text/encode/core", "basic usages of encode function do not explode")
  {
    // SECTION("execution")
    //{
    //   Krys::execution_t encoding {};
    //   std::string result0 =
    //     Krys::encode(Krys::Tests::u32_basic_source_character_set, encoding, Krys::replacement_handler);
    //   REQUIRE(result0 == Krys::Tests::basic_source_character_set);

    // if (Krys::contains_unicode_encoding(encoding))
    // {
    //   std::string result1 = Krys::encode(Krys::Tests::u32_unicode_sequence_truth_native_endian, encoding,
    //                                           Krys::replacement_handler);
    //   REQUIRE(result1 == Krys::Tests::unicode_sequence_truth_native_endian);
    // }
    // }
    // SECTION("wide_execution")
    //{
    // Krys::wide_execution_t encoding {};
    // std::wstring result0 =
    //   Krys::encode(Krys::Tests::u32_basic_source_character_set, encoding, Krys::replacement_handler);
    // REQUIRE(result0 == Krys::Tests::w_basic_source_character_set);

    // if (Krys::contains_unicode_encoding(encoding))
    // {
    //   std::wstring result1 = Krys::encode(Krys::Tests::u32_unicode_sequence_truth_native_endian, encoding,
    //                                            Krys::replacement_handler);
    //   REQUIRE(result1 == Krys::Tests::w_unicode_sequence_truth_native_endian);
    // }
    // }
    // SECTION("literal")
    //{
    // Krys::literal_t encoding {};
    // std::string result0 =
    //   Krys::encode(Krys::Tests::u32_basic_source_character_set, encoding, Krys::replacement_handler);
    // REQUIRE(result0 == Krys::Tests::basic_source_character_set);

    // if (Krys::contains_unicode_encoding(encoding))
    // {
    //   std::string result1 = Krys::encode(Krys::Tests::u32_unicode_sequence_truth_native_endian, encoding,
    //                                           Krys::replacement_handler);
    //   REQUIRE(result1 == Krys::Tests::unicode_sequence_truth_native_endian);
    // }
    // }
    // SECTION("wide_literal")
    //{
    // Krys::wide_literal_t encoding {};
    // std::wstring result0 = Krys::encode(Krys::Tests::u32_basic_source_character_set, encoding);
    // REQUIRE(result0 == Krys::Tests::w_basic_source_character_set);

    // if (Krys::contains_unicode_encoding(encoding))
    // {
    //   std::wstring result1 = Krys::encode(Krys::Tests::u32_unicode_sequence_truth_native_endian, encoding,
    //                                            Krys::replacement_handler);
    //   REQUIRE(result1 == Krys::Tests::w_unicode_sequence_truth_native_endian);
    // }
    // }
    //SECTION("utf8")
    //{
    //  Krys::utf8_t encoding {};
    //  std::basic_string<uchar8> result0 = Krys::Encode(Krys::Tests::u32_basic_source_character_set, encoding);
    //  REQUIRE(result0 == Krys::Tests::u8_basic_source_character_set);

    //  std::basic_string<uchar8> result1 =
    //    Krys::Encode(Krys::Tests::u32_unicode_sequence_truth_native_endian, encoding);
    //  REQUIRE(result1 == Krys::Tests::u8_unicode_sequence_truth_native_endian);
    //}
    //SECTION("utf16")
    //{
    //  Krys::utf16_t encoding {};
    //  std::u16string result0 = Krys::Encode(Krys::Tests::u32_basic_source_character_set, encoding);
    //  REQUIRE(result0 == Krys::Tests::u16_basic_source_character_set);

    //  std::u16string result1 = Krys::Encode(Krys::Tests::u32_unicode_sequence_truth_native_endian, encoding);
    //  REQUIRE(result1 == Krys::Tests::u16_unicode_sequence_truth_native_endian);
    //}
    //SECTION("utf32")
    //{
    //  Krys::utf32_t encoding {};
    //  std::u32string result0 = Krys::Encode(Krys::Tests::u32_basic_source_character_set, encoding);
    //  REQUIRE(result0 == Krys::Tests::u32_basic_source_character_set);

    //  std::u32string result1 = Krys::Encode(Krys::Tests::u32_unicode_sequence_truth_native_endian, encoding);
    //  REQUIRE(result1 == Krys::Tests::u32_unicode_sequence_truth_native_endian);
    //}
  }
}