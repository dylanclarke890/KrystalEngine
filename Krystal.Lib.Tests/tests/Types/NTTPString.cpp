#include "Krystal.Lib/Types/NTTPString.hpp"
#include <catch_all.hpp>

namespace Krys
{
  namespace
  {
    template <NTTPString Str>
    struct Template
    {
      static constexpr auto value = Str;
    };

    template <NTTPString Str>
    struct PartialTemplate
    {
      static constexpr uint32 value = 0u;
    };

    template <>
    struct PartialTemplate<"Test">
    {
      static constexpr uint32 value = 3u;
    };
  }

  TEST_CASE("NTTPString can be constructed from a string literal", "[NTTPString]")
  {
    SECTION("char")
    {
      constexpr NTTPString str {"Hello"};
      STATIC_REQUIRE(str == "Hello");
    }

    SECTION("char8")
    {
      constexpr NTTPString str {u8"Hello"};
      STATIC_REQUIRE(str == u8"Hello");
    }
  }

  TEST_CASE("FixedString comparison operators work correctly", "[NTTPString]")
  {
    constexpr NTTPString str1 {"Hello"};
    constexpr NTTPString str2 {"Hello"};
    constexpr NTTPString str3 {"World!"};

    STATIC_REQUIRE(str1 == str2);
    STATIC_REQUIRE(str1 != str3);
  }

  TEST_CASE("NTTPString can be used as a NTTP parameter", "[NTTPString]")
  {
    STATIC_REQUIRE(Template<"Test">::value == "Test");
  }

  TEST_CASE("NTTPString can be used to specialise a template", "[NTTPString]")
  {
    STATIC_REQUIRE(PartialTemplate<"Hello">::value == 0u);
    STATIC_REQUIRE(PartialTemplate<"Test">::value == 3u);
  }
}