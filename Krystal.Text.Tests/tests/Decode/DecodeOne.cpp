#include "Krystal.Text/Decode/DecodeOne.hpp"
#include "Krystal.Text.Tests/BasicUnicodeStrings.hpp"
#include "Krystal.Text/Encodings/UTF16.hpp"
#include "Krystal.Text/Encodings/UTF32.hpp"
#include "Krystal.Text/Encodings/UTF8.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Text;

  template <typename Encoding, typename Source, typename Expected>
  constexpr void CheckDecodeOne(Encoding &encoding, Source &source, Expected &expected)
  {
    using namespace Krys::Ranges;

    constexpr std::size_t MaxOutSize = MaxDecodeCodePoints<Encoding>;
    using OutChar = code_point_t<Encoding>;
    using Output = InlineBasicString<OutChar, MaxOutSize>;

    auto source_it = source.cbegin();
    auto source_last = source.cend();

    auto expected_it = expected.cbegin();
    auto expected_last = expected.cend();

    constexpr auto &handler = Handlers::Replacement;
    for (; expected_it != expected_last && source_it != source_last;)
    {
      const auto result = DecodeOneTo(std::ranges::subrange(source_it, source_last), encoding, handler);
      REQUIRE(result.ErrorCode == EncodingError::OK);
      REQUIRE_FALSE(result.ErrorsWereHandled());

      const Output &output = result.Output;
      REQUIRE(std::equal(output.cbegin(), output.cend(), expected_it));

      // make sure that the DecodeOne branch of functions is also tested
      const Output otherResult = DecodeOne(std::ranges::subrange(source_it, source_last), encoding, handler);
      REQUIRE(output == otherResult);

      expected_it += output.size();
      source_it = result.Input.begin();
    }

    REQUIRE(source_it == source_last);
    REQUIRE(expected_it == expected_last);
  }

  TEST_CASE("DecodeOne(utf8)", "[Text][DecodeOne]")
  {
    CheckDecodeOne(utf8, u8_basic_source_character_set, u32_basic_source_character_set);
    CheckDecodeOne(utf8, u8_unicode_sequence_truth_native_endian, u32_unicode_sequence_truth_native_endian);
  }

  TEST_CASE("DecodeOne(utf16", "[Text][DecodeOne]")
  {
    CheckDecodeOne(utf16, u16_basic_source_character_set, u32_basic_source_character_set);
    CheckDecodeOne(utf16, u16_unicode_sequence_truth_native_endian, u32_unicode_sequence_truth_native_endian);
  }
  TEST_CASE("DecodeOne(utf32", "[Text][DecodeOne]")
  {
    CheckDecodeOne(utf32, u32_basic_source_character_set, u32_basic_source_character_set);
    CheckDecodeOne(utf32, u32_unicode_sequence_truth_native_endian, u32_unicode_sequence_truth_native_endian);
  }
  // TEST_CASE("DecodeOne(execution)", "[Text][DecodeOne]")
  //{
  //   CheckDecodeOne(Krys::execution, Krys::Tests::basic_source_character_set,
  //                    Krys::Tests::u32_basic_source_character_set);

  // if (Krys::contains_unicode_encoding(Krys::execution), "[Text][DecodeOne]")
  // {
  //   CheckDecodeOne(Krys::execution, Krys::Tests::unicode_sequence_truth_native_endian,
  //                    Krys::Tests::u32_unicode_sequence_truth_native_endian);
  // }
  // }
  // TEST_CASE("DecodeOne(wide_execution)", "[Text][DecodeOne]")
  //{
  // CheckDecodeOne(Krys::wide_execution, Krys::Tests::w_basic_source_character_set,
  //                  Krys::Tests::u32_basic_source_character_set);
  // if (Krys::contains_unicode_encoding(Krys::wide_execution))
  // {
  //   CheckDecodeOne(Krys::wide_execution, Krys::Tests::w_unicode_sequence_truth_native_endian,
  //                    Krys::Tests::u32_unicode_sequence_truth_native_endian);
  // }
  // }
  // TEST_CASE("DecodeOne(literal)", "[Text][DecodeOne]")
  //{
  // CheckDecodeOne(Krys::literal, Krys::Tests::basic_source_character_set,
  //                  Krys::Tests::u32_basic_source_character_set);
  // if (Krys::contains_unicode_encoding(Krys::literal))
  // {
  //   CheckDecodeOne(Krys::literal, Krys::Tests::unicode_sequence_truth_native_endian,
  //                    Krys::Tests::u32_unicode_sequence_truth_native_endian);
  // }
  // }
  // TEST_CASE("DecodeOne(wide_literal)", "[Text][DecodeOne]")
  //{
  // CheckDecodeOne(Krys::wide_literal, Krys::Tests::w_basic_source_character_set,
  //                  Krys::Tests::u32_basic_source_character_set);
  // if (Krys::contains_unicode_encoding(Krys::wide_literal))
  // {
  //   CheckDecodeOne(Krys::wide_literal, Krys::Tests::w_unicode_sequence_truth_native_endian,
  //                    Krys::Tests::u32_unicode_sequence_truth_native_endian);
  // }
  // }
}