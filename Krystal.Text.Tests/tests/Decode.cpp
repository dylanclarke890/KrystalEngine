#include "Krystal.Text/Decode.hpp"
#include "Krystal.Text.Tests/BasicUnicodeStrings.hpp"
#include "Krystal.Text/Encodings/UTF16.hpp"
#include "Krystal.Text/Encodings/UTF32.hpp"
#include "Krystal.Text/Encodings/UTF8.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{

  TEST_CASE("text/decode/core", "basic usages of decode function do not explode")
  {
    // SECTION("execution")
    //{
    //   execution_t encoding {};

    // std::u32string result0 =
    //   decode(basic_source_character_set, encoding, replacement_handler);
    // REQUIRE(result0 == u32_basic_source_character_set);

    // if (contains_unicode_encoding(encoding))
    // {
    //   std::u32string result1 = decode(unicode_sequence_truth_native_endian, encoding,
    //                                              replacement_handler);
    //   REQUIRE(result1 == u32_unicode_sequence_truth_native_endian);
    // }
    // }
    // SECTION("wide_execution")
    //{
    // wide_execution_t encoding {};
    // std::u32string result0 =
    //   decode(w_basic_source_character_set, encoding, replacement_handler);
    // REQUIRE(result0 == u32_basic_source_character_set);

    // if (contains_unicode_encoding(encoding))
    // {
    //   std::u32string result1 = decode(w_unicode_sequence_truth_native_endian, encoding,
    //                                              replacement_handler);
    //   REQUIRE(result1 == u32_unicode_sequence_truth_native_endian);
    // }
    // }
    // SECTION("literal")
    //{
    // literal_t encoding {};
    // std::u32string result0 = decode(basic_source_character_set, literal);
    // REQUIRE(result0 == u32_basic_source_character_set);

    // if (contains_unicode_encoding(encoding))
    // {
    //   std::u32string result1 = decode(unicode_sequence_truth_native_endian, encoding,
    //                                              replacement_handler);
    //   REQUIRE(result1 == u32_unicode_sequence_truth_native_endian);
    // }
    // }
    // SECTION("wide_literal")
    //{
    // wide_literal_t encoding {};
    // std::u32string result0 =
    //   decode(w_basic_source_character_set, encoding, replacement_handler);
    // REQUIRE(result0 == u32_basic_source_character_set);

    // if (contains_unicode_encoding(encoding))
    // {
    //   std::u32string result1 = decode(w_unicode_sequence_truth_native_endian, encoding,
    //                                              replacement_handler);
    //   REQUIRE(result1 == u32_unicode_sequence_truth_native_endian);
    // }
    // }
    //SECTION("utf8")
    //{
    //  std::u32string result0 = Decode<std::u32string>(u8_basic_source_character_set, utf8);
    //  REQUIRE(result0 == u32_basic_source_character_set);
    //  std::u32string result1 = Decode<std::u32string>(u8_unicode_sequence_truth_native_endian, utf8);
    //  REQUIRE(result1 == u32_unicode_sequence_truth_native_endian);
    //}
    // SECTION("basic_utf8<std::byte>")
    //{
    //   std::u32string result0 = Decode<std::u32string>(u8_basic_source_character_set,
    //                                                         basic_utf8<std::byte> {});
    //   REQUIRE(result0 == u32_basic_source_character_set);

    // std::u32string result1 = Decode<std::u32string>(
    //   u8_unicode_sequence_truth_native_endian, basic_utf8<std::byte> {});
    // REQUIRE(result1 == u32_unicode_sequence_truth_native_endian);
    // }
    // SECTION("utf16")
    //{
    // std::u32string result0 =
    //   Decode<std::u32string>(u16_basic_source_character_set, utf16);
    // REQUIRE(result0 == u32_basic_source_character_set);

    // std::u32string result1 =
    //   Decode<std::u32string>(u16_unicode_sequence_truth_native_endian, utf16);
    // REQUIRE(result1 == u32_unicode_sequence_truth_native_endian);
    // }
    // SECTION("utf32")
    //{
    // std::u32string result0 =
    //   Decode<std::u32string>(u32_basic_source_character_set, utf32);
    // REQUIRE(result0 == u32_basic_source_character_set);

    // std::u32string result1 =
    //   Decode<std::u32string>(u32_unicode_sequence_truth_native_endian, utf32);
    // REQUIRE(result1 == u32_unicode_sequence_truth_native_endian);
    // }
  }
}