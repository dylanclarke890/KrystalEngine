#include "Krystal.Lib/String/FixedString.hpp"
#include <catch_all.hpp>

namespace Krys
{
  namespace
  {
    template <FixedString Str>
    struct Template
    {
      static constexpr auto value = Str;
    };

    template <FixedString Str>
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

  TEST_CASE("FixedString can be constructed from a string literal", "[FixedString]")
  {
    SECTION("char")
    {
      constexpr FixedString str {"Hello"};
      STATIC_REQUIRE(str == "Hello");
    }

    SECTION("char8")
    {
      constexpr FixedString str {u8"Hello"};
      STATIC_REQUIRE(str == u8"Hello");
    }
  }

  TEST_CASE("FixedString comparison operators work correctly", "[FixedString]")
  {
    constexpr FixedString str1 {"Hello"};
    constexpr FixedString str2 {"Hello"};
    constexpr FixedString str3 {"World!"};

    STATIC_REQUIRE(str1 == str2);
    STATIC_REQUIRE(str1 != str3);
  }

  TEST_CASE("FixedString can be used as a NTTP parameter", "[FixedString]")
  {
    STATIC_REQUIRE(Template<"Test">::value == "Test");
  }

  TEST_CASE("FixedString can be used to specialise a template", "[FixedString]")
  {
    STATIC_REQUIRE(PartialTemplate<"Hello">::value == 0u);
    STATIC_REQUIRE(PartialTemplate<"Test">::value == 3u);
  }
}