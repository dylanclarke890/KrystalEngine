#include "Krystal.Core/Text/Base64.hpp"
#include "catch2/catch_test_macros.hpp"
#include <string_view>

namespace krys::tests
{
  namespace
  {
    KRYS_NODISCARD Maybe<std::string> Decode(std::string_view input, EnumSet<Base64DecodeOption> options = {})
    {
      auto decoded =
        Base64Decode(Span<const uint8>(reinterpret_cast<const uint8 *>(input.data()), input.size()), options);
      if (!decoded)
      {
        return null;
      }
      return std::string(decoded->begin(), decoded->end());
    }
  }

  TEST_CASE("Base64Decode: decodes the standard alphabet", "[Text][Base64]")
  {
    CHECK(Decode("") == std::string());
    CHECK(Decode("TQ==") == std::string("M"));
    CHECK(Decode("TWE=") == std::string("Ma"));
    CHECK(Decode("TWFu") == std::string("Man"));
    CHECK(Decode("TWFuTQ") == std::string("ManM"));
    CHECK(Decode("+/+/") == std::string("\xfb\xff\xbf"));
  }

  TEST_CASE("Base64Decode: rejects invalid input", "[Text][Base64]")
  {
    CHECK_FALSE(Decode("T"));
    CHECK_FALSE(Decode("TWFuT"));
    CHECK_FALSE(Decode("TW=u"));
    CHECK_FALSE(Decode("TW-u"));
    CHECK_FALSE(Decode("="));
    CHECK_FALSE(Decode("TW Fu"));
  }

  TEST_CASE("Base64Decode: options", "[Text][Base64]")
  {
    SECTION("Whitespace can be ignored")
    {
      CHECK(Decode("TW Fu\n", Base64DecodeOption::IgnoreWhitespace) == std::string("Man"));
    }

    SECTION("Padding can be validated")
    {
      CHECK(Decode("TQ=", {}) == std::string("M"));
      CHECK_FALSE(Decode("TQ=", Base64DecodeOption::ValidatePadding));
      CHECK_FALSE(Decode("TQ===", Base64DecodeOption::ValidatePadding));
      CHECK(Decode("TQ==", Base64DecodeOption::ValidatePadding) == std::string("M"));
    }
  }
}
