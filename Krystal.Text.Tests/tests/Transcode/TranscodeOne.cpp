#include "Krystal.Text/Transcode/TranscodeOne.hpp"
#include "Krystal.Text.Tests/BasicUnicodeStrings.hpp"
#include "Krystal.Text/Encodings/UTF16.hpp"
#include "Krystal.Text/Encodings/UTF32.hpp"
#include "Krystal.Text/Encodings/UTF8.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Text;

  template <typename FromEncoding, typename ToEncoding, typename Source, typename Expected>
  void CheckTranscodeOne(FromEncoding &from_encoding, ToEncoding &to_encoding, Source &source,
                         Expected &expected)
  {
    using namespace Krys::Ranges;

    constexpr std::size_t MaxOutSize = ::Krys::Text::MaxTranscodeCodeUnits<FromEncoding, ToEncoding>;
    using Output = InlineBasicString<code_unit_t<ToEncoding>, MaxOutSize>;

    auto source_it = source.cbegin();
    auto source_last = source.cend();

    auto expected_it = expected.cbegin();
    auto expected_last = expected.cend();

    auto &handler = ::Krys::Text::Handlers::Replacement;
    for (; expected_it != expected_last && source_it != source_last;)
    {
      const auto result =
        TranscodeOneTo(std::ranges::subrange(source_it, source_last), from_encoding, to_encoding, handler);
      REQUIRE(result.ErrorCode == EncodingError::OK);
      REQUIRE_FALSE(result.ErrorsWereHandled());

      const Output &output = result.Output;
      REQUIRE(std::equal(output.cbegin(), output.cend(), expected_it));

      // make sure that the transcode_one branch of functions is also tested
      const Output otherResult =
        TranscodeOne(std::ranges::subrange(source_it, source_last), from_encoding, to_encoding, handler);
      REQUIRE(output == otherResult);

      expected_it += otherResult.size();
      source_it = std::move(result.Input.begin());
    }
    REQUIRE(source_it == source_last);
    REQUIRE(expected_it == expected_last);
  }

  TEST_CASE("TranscodeOne(utf8)", "[Text][TranscodeOne]")
  {
    CheckTranscodeOne(utf8, utf8, u8_basic_source_character_set, u8_basic_source_character_set);
    CheckTranscodeOne(utf8, utf8, u8_unicode_sequence_truth_native_endian,
                      u8_unicode_sequence_truth_native_endian);
  }

  TEST_CASE("TranscodeOne(utf16)", "[Text][TranscodeOne]")
  {
    CheckTranscodeOne(utf16, utf16, u16_basic_source_character_set, u16_basic_source_character_set);
    CheckTranscodeOne(utf16, utf16, u16_unicode_sequence_truth_native_endian,
                      u16_unicode_sequence_truth_native_endian);
  }

  TEST_CASE("TranscodeOne(utf32)", "[Text][TranscodeOne]")
  {
    CheckTranscodeOne(utf32, utf32, u32_basic_source_character_set, u32_basic_source_character_set);
    CheckTranscodeOne(utf32, utf32, u32_unicode_sequence_truth_native_endian,
                      u32_unicode_sequence_truth_native_endian);
  }

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
}