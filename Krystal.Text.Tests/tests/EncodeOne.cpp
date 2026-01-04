#include "Krystal.Text/EncodeOne.hpp"
#include "Krystal.Text.Tests/BasicUnicodeStrings.hpp"
#include "Krystal.Text/Encodings/UTF16.hpp"
#include "Krystal.Text/Encodings/UTF32.hpp"
#include "Krystal.Text/Encodings/UTF8.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  template <typename Encoding, typename Source, typename Expected>
  void check_encode_one(Encoding &encoding, Source &source, Expected &expected)
  {
    using namespace Krys::Ranges;

    constexpr std::size_t MaxOutSize = MaxEncodeCodeUnits<Encoding>;
    using OutChar = code_unit_t<Encoding>;
    using Output = InlineBasicString<OutChar, MaxOutSize>;

    auto source_it = source.cbegin();
    auto source_last = source.cend();

    auto expected_it = expected.cbegin();
    auto expected_last = expected.cend();

    constexpr auto &handler = Handlers::Replacement;
    for (; expected_it != expected_last && source_it != source_last;)
    {
      const auto result = EncodeOneTo(CreateSubrange(source_it, source_last), encoding, handler);
      const Output &output = result.Output;

      REQUIRE(result.ErrorCode == EncodingError::OK);
      REQUIRE_FALSE(result.ErrorsWereHandled());

      REQUIRE(std::equal(output.cbegin(), output.cend(), expected_it));

      // make sure that the encode_one branch of functions is also tested
      const Output otherResult = EncodeOne(CreateSubrange(source_it, source_last), encoding, handler);
      REQUIRE((output == otherResult));

      expected_it += output.size();
      source_it = result.Input.begin();
    }
    REQUIRE(source_it == source_last);
    REQUIRE(expected_it == expected_last);
  }

  TEST_CASE("encode_one can be used to encode one bit of text at a time and produce a fixed container",
            "[text][transcode_one]")
  {
    // SECTION("execution")
    //{
    //   check_encode_one(Krys::execution, u32_basic_source_character_set,
    //                    basic_source_character_set);

    // if (Krys::contains_unicode_encoding(Krys::execution))
    // {
    //   check_encode_one(Krys::execution, u32_unicode_sequence_truth_native_endian,
    //                    unicode_sequence_truth_native_endian);
    // }
    // }
    // SECTION("wide_execution")
    //{
    // check_encode_one(Krys::wide_execution, u32_basic_source_character_set,
    //                  w_basic_source_character_set);
    // if (Krys::contains_unicode_encoding(Krys::wide_execution))
    // {
    //   check_encode_one(Krys::wide_execution, u32_unicode_sequence_truth_native_endian,
    //                    w_unicode_sequence_truth_native_endian);
    // }
    // }
    // SECTION("literal")
    //{
    // check_encode_one(Krys::literal, u32_basic_source_character_set,
    //                  basic_source_character_set);
    // if (Krys::contains_unicode_encoding(Krys::literal))
    // {
    //   check_encode_one(Krys::literal, u32_unicode_sequence_truth_native_endian,
    //                    unicode_sequence_truth_native_endian);
    // }
    // }
    // SECTION("wide_literal")
    //{
    //  check_encode_one(Krys::wide_literal, u32_basic_source_character_set,
    //                   w_basic_source_character_set);
    //  if (Krys::contains_unicode_encoding(Krys::wide_literal))
    //  {
    //    check_encode_one(Krys::wide_literal, u32_unicode_sequence_truth_native_endian,
    //                     w_unicode_sequence_truth_native_endian);
    //  }
    //}
    SECTION("utf8")
    {
      check_encode_one(utf8, u32_basic_source_character_set, u8_basic_source_character_set);
      check_encode_one(utf8, u32_unicode_sequence_truth_native_endian,
                       u8_unicode_sequence_truth_native_endian);
    }
     SECTION("utf16")
    {
       check_encode_one(Krys::utf16, u32_basic_source_character_set,
                        u16_basic_source_character_set);
       check_encode_one(Krys::utf16, u32_unicode_sequence_truth_native_endian,
                        u16_unicode_sequence_truth_native_endian);
     }
    SECTION("utf32")
    {
      check_encode_one(utf32, u32_basic_source_character_set, u32_basic_source_character_set);
      check_encode_one(utf32, u32_unicode_sequence_truth_native_endian,
                       u32_unicode_sequence_truth_native_endian);
    }
  }
}