#include "Krystal.Text/TranscodeOne.hpp"
#include "Krystal.Text.Tests/BasicUnicodeStrings.hpp"
#include "Krystal.Text/Encodings/UTF16.hpp"
#include "Krystal.Text/Encodings/UTF32.hpp"
#include "Krystal.Text/Encodings/UTF8.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  template <typename FromEncoding, typename ToEncoding, typename Source, typename Expected>
  void check_transcode_one(FromEncoding &from_encoding, ToEncoding &to_encoding, Source &source,
                           Expected &expected)
  {
    using OutChar = code_unit_t<ToEncoding>;
    constexpr std::size_t MaxOutSize = MaxTranscodeCodeUnits<FromEncoding, ToEncoding>;
    using Output = InlineBasicString<OutChar, MaxOutSize>;

    auto source_it = source.cbegin();
    auto source_last = source.cend();

    auto expected_it = expected.cbegin();
    auto expected_last = expected.cend();

    for (; expected_it != expected_last && source_it != source_last;)
    {
      const auto result0 = TranscodeOneTo(Ranges::CreateSubrange(source_it, source_last), from_encoding,
                                          to_encoding, Handlers::Replacement);

      const Output &result0_output = result0.Output;
      const bool result0_is_equal_to_expected =
        std::equal(result0_output.cbegin(), result0_output.cend(), expected_it);

      REQUIRE(result0.ErrorCode == EncodingError::OK);
      REQUIRE_FALSE(result0.ErrorsWereHandled());
      REQUIRE(result0_is_equal_to_expected);

      // make sure that the transcode_one branch of functions is also tested
      const Output independent_result0_output = TranscodeOne(
        Ranges::CreateSubrange(source_it, source_last), from_encoding, to_encoding, Handlers::Replacement);
      REQUIRE(result0_output == independent_result0_output);
      expected_it += result0_output.size();
      source_it = std::move(result0.Input.begin());
    }
    REQUIRE(source_it == source_last);
    REQUIRE(expected_it == expected_last);
  }

  TEST_CASE("transcode_one can roundtrip through data one indivisible unit at a time",
            "[text][transcode_one][roundtrip]")
  {
    // SECTION("execution")
    //{
    //   check_transcode_one(execution, execution, basic_source_character_set,
    //                       basic_source_character_set);

    // if (contains_unicode_encoding(execution))
    // {
    //   check_transcode_one(execution, execution,
    //                       unicode_sequence_truth_native_endian,
    //                       unicode_sequence_truth_native_endian);
    // }
    // }
    // SECTION("wide_execution")
    //{
    // check_transcode_one(wide_execution, wide_execution,
    //                     w_basic_source_character_set, w_basic_source_character_set);
    // if (contains_unicode_encoding(wide_execution))
    // {
    //   check_transcode_one(wide_execution, wide_execution,
    //                       w_unicode_sequence_truth_native_endian,
    //                       w_unicode_sequence_truth_native_endian);
    // }
    // }
    // SECTION("literal")
    //{
    // check_transcode_one(literal, literal, basic_source_character_set,
    //                     basic_source_character_set);
    // if (contains_unicode_encoding(literal))
    // {
    //   check_transcode_one(literal, literal,
    //                       unicode_sequence_truth_native_endian,
    //                       unicode_sequence_truth_native_endian);
    // }
    // }
    // SECTION("wide_literal")
    //{
    // check_transcode_one(wide_literal, wide_literal,
    //                     w_basic_source_character_set, w_basic_source_character_set);
    // if (contains_unicode_encoding(wide_literal))
    // {
    //   check_transcode_one(wide_literal, wide_literal,
    //                       w_unicode_sequence_truth_native_endian,
    //                       w_unicode_sequence_truth_native_endian);
    // }
    // }
    SECTION("utf8")
    {
      check_transcode_one(utf8, utf8, u8_basic_source_character_set, u8_basic_source_character_set);
      check_transcode_one(utf8, utf8, u8_unicode_sequence_truth_native_endian,
                          u8_unicode_sequence_truth_native_endian);
    }
    SECTION("utf16")
    {
      check_transcode_one(utf16, utf16, u16_basic_source_character_set, u16_basic_source_character_set);
      check_transcode_one(utf16, utf16, u16_unicode_sequence_truth_native_endian,
                          u16_unicode_sequence_truth_native_endian);
    }
    SECTION("utf32")
    {
      check_transcode_one(utf32, utf32, u32_basic_source_character_set, u32_basic_source_character_set);
      check_transcode_one(utf32, utf32, u32_unicode_sequence_truth_native_endian,
                          u32_unicode_sequence_truth_native_endian);
    }
  }
}