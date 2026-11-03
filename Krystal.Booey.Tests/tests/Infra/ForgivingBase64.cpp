#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Text/Base64.hpp"
#include <string>
#include <string_view>

namespace krys::boo::infra::tests
{
  namespace
  {
    /// @brief Forgiving-base64 decode: Base64Decode, validating padding and ignoring whitespace.
    KRYS_NODISCARD Maybe<std::string> ForgivingDecode(std::string_view input)
    {
      auto decoded =
        Base64Decode(Span<const uint8>(reinterpret_cast<const uint8 *>(input.data()), input.size()),
                     {Base64DecodeOption::ValidatePadding, Base64DecodeOption::IgnoreWhitespace});
      if (!decoded)
      {
        return null;
      }
      return std::string(decoded->begin(), decoded->end());
    }
  }

  TEST_CASE("Forgiving-base64 decode: whitespace, padding and the length",
            "[Infra][Base64][infra:forgiving-base64][infra:forgiving-base64-decode]")
  {
    // 1. All ASCII whitespace is removed, form feed included.
    CHECK(ForgivingDecode(" T\tQ\f=\n=\r") == std::string("M"));

    // 2. One or two = are removed only when the length is a multiple of four.
    CHECK(ForgivingDecode("TQ==") == std::string("M"));
    CHECK(ForgivingDecode("TWE=") == std::string("Ma"));
    CHECK_FALSE(ForgivingDecode("TQ="));
    CHECK_FALSE(ForgivingDecode("TQ==="));
    CHECK_FALSE(ForgivingDecode("TWFu===="));
    CHECK_FALSE(ForgivingDecode("=="));
    CHECK_FALSE(ForgivingDecode("===="));

    // Unpadded input is fine.
    CHECK(ForgivingDecode("TQ") == std::string("M"));
    CHECK(ForgivingDecode("TWE") == std::string("Ma"));
    CHECK(ForgivingDecode("") == std::string());

    // 3. A length that leaves a remainder of 1 fails.
    CHECK_FALSE(ForgivingDecode("T"));
    CHECK_FALSE(ForgivingDecode("TWFuT"));

    // 4. Anything other than +, / and ASCII alphanumerics fails, including = left in the middle.
    CHECK_FALSE(ForgivingDecode("TQ==TQ=="));
    CHECK_FALSE(ForgivingDecode("TW-u"));
    CHECK_FALSE(ForgivingDecode("TW\xc3\xa9u"));
    CHECK(ForgivingDecode("+/+/") == std::string("\xfb\xff\xbf"));

    // 9. Leftover bits are discarded, even when they aren't zero.
    CHECK(ForgivingDecode("TR==") == std::string("M"));
    CHECK(ForgivingDecode("TWF") == std::string("Ma"));
  }

  TEST_CASE("Forgiving-base64 encode: RFC 4648's base64, padded",
            "[Infra][Base64][infra:forgiving-base64-encode]")
  {
    auto encode = [](std::string_view input)
    {
      auto encoded =
        Base64Encode(Span<const uint8>(reinterpret_cast<const uint8 *>(input.data()), input.size()));
      return std::string(encoded.begin(), encoded.end());
    };
    CHECK(encode("M") == "TQ==");
    CHECK(encode("Ma") == "TWE=");
    CHECK(encode("Man") == "TWFu");
    CHECK(ForgivingDecode(encode(std::string_view("\x00\xff\x10", 3))) == std::string("\x00\xff\x10", 3));
  }
}
