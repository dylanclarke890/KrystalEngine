#include "Krystal.Booey/CSS/Values/ColorSchemeValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    ColorScheme Schemes(std::initializer_list<CustomIdentifier> schemes, bool only = false)
    {
      ColorScheme scheme {.Schemes = schemes};
      if (only)
      {
        scheme.Only = keywords::Only {};
      }
      return scheme;
    }

    CustomIdentifier Ident(utf8_stringview name)
    {
      return CustomIdentifier {CSSOMStringAtom {name}};
    }
  }

  TEST_CASE_METHOD(ValueFixture, "ColorSchemeValue::Create", "[CSS][Values][ColorSchemeValue]")
  {
    auto value = ColorSchemeValue::Create(Schemes({Ident(u8"light"), Ident(u8"dark")}));
    CHECK(value->IsColorSchemeValue());
    CHECK(value->GetColorScheme() == Schemes({Ident(u8"light"), Ident(u8"dark")}));
    CHECK_FALSE(value->GetColorScheme().IsNormal());
  }

  TEST_CASE_METHOD(ValueFixture, "ColorSchemeValue serialisation", "[CSS][Values][ColorSchemeValue]")
  {
    // No schemes means `normal`.
    CHECK(CssText(*ColorSchemeValue::Create(ColorScheme {})) == "normal");
    CHECK(CssText(*ColorSchemeValue::Create(Schemes({Ident(u8"light"), Ident(u8"dark")}))) == "light dark");
    CHECK(CssText(*ColorSchemeValue::Create(Schemes({Ident(u8"dark")}, true))) == "dark only");
    CHECK(CssText(*ColorSchemeValue::Create(Schemes({Ident(u8"my-scheme")}))) == "my-scheme");
  }

  TEST_CASE_METHOD(ValueFixture, "ColorSchemeValue equality and hashing", "[CSS][Values][ColorSchemeValue]")
  {
    auto a = ColorSchemeValue::Create(Schemes({Ident(u8"light"), Ident(u8"dark")}));
    CHECK(*a == *ColorSchemeValue::Create(Schemes({Ident(u8"light"), Ident(u8"dark")})));
    CHECK_FALSE(*a == *ColorSchemeValue::Create(Schemes({Ident(u8"dark"), Ident(u8"light")})));
    CHECK_FALSE(*a == *ColorSchemeValue::Create(Schemes({Ident(u8"light"), Ident(u8"dark")}, true)));
    CHECK_FALSE(*a == *ColorSchemeValue::Create(ColorScheme {}));

    // `ColorSchemeValue` has no hash of its own.
    size_t seed = 0uz;
    CHECK_FALSE(a->AddHash(seed));
  }

  TEST_CASE_METHOD(ValueFixture, "ColorSchemeValue visitation", "[CSS][Values][ColorSchemeValue]")
  {
    CHECK(Children(*ColorSchemeValue::Create(Schemes({Ident(u8"light")}))).empty());
  }
}
