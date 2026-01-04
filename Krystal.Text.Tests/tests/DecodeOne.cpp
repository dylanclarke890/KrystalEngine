#include "Krystal.Text/DecodeOne.hpp"
#include "Krystal.Lib/Utils/WhatIsThis.hpp"
#include "Krystal.Text/Encodings/UTF16.hpp"
#include "Krystal.Text/Encodings/UTF8.hpp"
#include "Krystal.Text/Encodings/UTF32.hpp"
#include "Krystal.Text.Tests/BasicUnicodeStrings.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  template <typename Encoding, typename Source, typename Expected>
  constexpr void CheckDecodeOne(Encoding &encoding, Source &source, Expected &expected)
  {
    using OutChar = Krys::code_point_t<Encoding>;
    constexpr std::size_t MaxOutSize = Krys::MaxDecodeCodePoints<Encoding>;
    using Output = Krys::InlineBasicString<OutChar, MaxOutSize>;

    auto source_it = source.cbegin();
    auto source_last = source.cend();
    auto expected_it = expected.cbegin();
    auto expected_last = expected.cend();

    for (; expected_it != expected_last && source_it != source_last;)
    {
      const auto result0 = Krys::DecodeOneTo(Krys::Ranges::CreateSubrange(source_it, source_last), encoding,
                                             Krys::Handlers::Replacement);
      const Output &result0_output = result0.Output;
      const bool result0_is_equal_to_expected =
        std::equal(result0_output.cbegin(), result0_output.cend(), expected_it);

      REQUIRE(result0.ErrorCode == Krys::EncodingError::OK);
      REQUIRE_FALSE(result0.ErrorsWereHandled());
      REQUIRE(result0_is_equal_to_expected);

      // make sure that the decode_one branch of functions is also tested
      const Output independent_result0_output = Krys::DecodeOne(
        Krys::Ranges::CreateSubrange(source_it, source_last), encoding, Krys::Handlers::Replacement);
      REQUIRE(result0_output == independent_result0_output);
      expected_it += result0_output.size();
      source_it = result0.Input.begin();
    }
    REQUIRE(source_it == source_last);
    REQUIRE(expected_it == expected_last);
  }

  TEST_CASE(
    "DecodeOne can be used to decode a single bit of text at a time and will produce a fixed container",
    "[Text][DecodeOne]")
  {
    // SECTION("execution")
    //{
    //   CheckDecodeOne(Krys::execution, Krys::Tests::basic_source_character_set,
    //                    Krys::Tests::u32_basic_source_character_set);

    // if (Krys::contains_unicode_encoding(Krys::execution))
    // {
    //   CheckDecodeOne(Krys::execution, Krys::Tests::unicode_sequence_truth_native_endian,
    //                    Krys::Tests::u32_unicode_sequence_truth_native_endian);
    // }
    // }
    // SECTION("wide_execution")
    //{
    // CheckDecodeOne(Krys::wide_execution, Krys::Tests::w_basic_source_character_set,
    //                  Krys::Tests::u32_basic_source_character_set);
    // if (Krys::contains_unicode_encoding(Krys::wide_execution))
    // {
    //   CheckDecodeOne(Krys::wide_execution, Krys::Tests::w_unicode_sequence_truth_native_endian,
    //                    Krys::Tests::u32_unicode_sequence_truth_native_endian);
    // }
    // }
    // SECTION("literal")
    //{
    // CheckDecodeOne(Krys::literal, Krys::Tests::basic_source_character_set,
    //                  Krys::Tests::u32_basic_source_character_set);
    // if (Krys::contains_unicode_encoding(Krys::literal))
    // {
    //   CheckDecodeOne(Krys::literal, Krys::Tests::unicode_sequence_truth_native_endian,
    //                    Krys::Tests::u32_unicode_sequence_truth_native_endian);
    // }
    // }
    // SECTION("wide_literal")
    //{
    // CheckDecodeOne(Krys::wide_literal, Krys::Tests::w_basic_source_character_set,
    //                  Krys::Tests::u32_basic_source_character_set);
    // if (Krys::contains_unicode_encoding(Krys::wide_literal))
    // {
    //   CheckDecodeOne(Krys::wide_literal, Krys::Tests::w_unicode_sequence_truth_native_endian,
    //                    Krys::Tests::u32_unicode_sequence_truth_native_endian);
    // }
    // }
    SECTION("utf8")
    {
      CheckDecodeOne(Krys::utf8, Krys::Tests::u8_basic_source_character_set,
                     Krys::Tests::u32_basic_source_character_set);
      CheckDecodeOne(Krys::utf8, Krys::Tests::u8_unicode_sequence_truth_native_endian,
                     Krys::Tests::u32_unicode_sequence_truth_native_endian);
    }
    SECTION("utf16")
    {
      CheckDecodeOne(Krys::utf16, Krys::Tests::u16_basic_source_character_set,
                     Krys::Tests::u32_basic_source_character_set);
      CheckDecodeOne(Krys::utf16, Krys::Tests::u16_unicode_sequence_truth_native_endian,
                     Krys::Tests::u32_unicode_sequence_truth_native_endian);
    }
    SECTION("utf32")
    {
      CheckDecodeOne(Krys::utf32, Krys::Tests::u32_basic_source_character_set,
                     Krys::Tests::u32_basic_source_character_set);
      CheckDecodeOne(Krys::utf32, Krys::Tests::u32_unicode_sequence_truth_native_endian,
                     Krys::Tests::u32_unicode_sequence_truth_native_endian);
    }
  }
}