#include "Krystal.Text/Transcode/Transcode.hpp"
#include "Krystal.Text.Tests/BasicUnicodeStrings.hpp"
#include "Krystal.Text/Encodings/UTF16.hpp"
#include "Krystal.Text/Encodings/UTF32.hpp"
#include "Krystal.Text/Encodings/UTF8.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Text;

  TEST_CASE("Transcode(utf8)", "[Text][Transcode]")
  {
    std::u8string result0 = Transcode<std::u8string>(u8_basic_source_character_set, utf8);
    REQUIRE(result0 == u8_basic_source_character_set);

    std::u8string result1 = Transcode(u8_unicode_sequence_truth_native_endian, utf8);
    REQUIRE(result1 == u8_unicode_sequence_truth_native_endian);
  }

  TEST_CASE("Transcode(utf16)", "[Text][Transcode]")
  {
    std::u16string result0 = Transcode(u16_basic_source_character_set, utf16);
    REQUIRE(result0 == u16_basic_source_character_set);

    std::u16string result1 = Transcode(u16_unicode_sequence_truth_native_endian, utf16);
    REQUIRE(result1 == u16_unicode_sequence_truth_native_endian);
  }

  TEST_CASE("Transcode(utf32)", "[Text][Transcode]")
  {
    std::u32string result0 = Transcode(u32_basic_source_character_set, utf32);
    REQUIRE(result0 == u32_basic_source_character_set);

    std::u32string result1 = Transcode(u32_unicode_sequence_truth_native_endian, utf32);
    REQUIRE(result1 == u32_unicode_sequence_truth_native_endian);
  }

  // SECTION("execution")
  // {
  //   execution_t encoding {};
  //   std::string result =
  //     transcode(basic_source_character_set, encoding, encoding, replacement_handler, replacement_handler);
  //   REQUIRE(result == basic_source_character_set);
  //
  //   if (contains_unicode_encoding(encoding))
  //   {
  //     std::string result1 = transcode(unicode_sequence_truth_native_endian, encoding, encoding,
  //                                     replacement_handler, replacement_handler);
  //     REQUIRE(result1 == unicode_sequence_truth_native_endian);
  //   }
  // }
  // SECTION("wide_execution")
  // {
  //   wide_execution_t encoding {};
  //   std::wstring result0 =
  //     transcode(w_basic_source_character_set, encoding, encoding, replacement_handler,
  //     replacement_handler);
  //   REQUIRE(result0 == w_basic_source_character_set);
  //
  //   if (contains_unicode_encoding(encoding))
  //   {
  //     std::wstring result1 = transcode(w_unicode_sequence_truth_native_endian, encoding, encoding,
  //                                      replacement_handler, replacement_handler);
  //     REQUIRE(result1 == w_unicode_sequence_truth_native_endian);
  //   }
  // }
  // SECTION("literal")
  // {
  //   literal_t encoding {};
  //   std::string result =
  //     transcode(basic_source_character_set, encoding, encoding, replacement_handler, replacement_handler);
  //   REQUIRE(result == basic_source_character_set);
  //
  //   if (contains_unicode_encoding(encoding))
  //   {
  //     std::string result1 = transcode(unicode_sequence_truth_native_endian, encoding, encoding,
  //                                     replacement_handler, replacement_handler);
  //     REQUIRE(result1 == unicode_sequence_truth_native_endian);
  //   }
  // }
  // SECTION("wide_literal")
  // {
  //   wide_literal_t encoding {};
  //   std::wstring result0 =
  //     transcode(w_basic_source_character_set, encoding, encoding, replacement_handler,
  //     replacement_handler);
  //   REQUIRE(result0 == w_basic_source_character_set);
  //
  //   if (contains_unicode_encoding(encoding))
  //   {
  //     std::wstring result1 = transcode(w_unicode_sequence_truth_native_endian, encoding, encoding,
  //                                      replacement_handler, replacement_handler);
  //     REQUIRE(result1 == w_unicode_sequence_truth_native_endian);
  //   }
  // }
}