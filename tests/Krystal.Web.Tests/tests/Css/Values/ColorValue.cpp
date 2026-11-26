#include "Krystal.Web/Css/Values/ColorValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Css/ColorTestUtils.hpp"
#include "Krystal.Web.Tests/Css/ValueTestUtils.hpp"
#include "Krystal.Web/Css/Values/PrimitiveValue.hpp"
#include "Krystal.Web/Css/Values/ValuePool.hpp"
#include "Krystal.Web/Graphics/Color/ColorSerialisation.hpp"

namespace krys::web::css::tests
{
  TEST_CASE_METHOD(ValueFixture, "ColorValue::Create", "[CSS][Values][ColorValue]")
  {
    SECTION("From a CSS colour")
    {
      auto value = ColorValue::Create(ColorKeyword(ValueId::Red));
      CHECK(Is<ColorValue>(*value));
      CHECK(value->GetColor() == ColorKeyword(ValueId::Red));
    }

    SECTION("From a resolved colour")
    {
      auto value = ColorValue::Create(web::Color {web::Color::blue});
      CHECK(value->GetColor() == Color {ResolvedColor {web::Color::blue}});
    }
  }

  TEST_CASE_METHOD(ValueFixture, "ColorValue serialisation", "[CSS][Values][ColorValue]")
  {
    CHECK(CssText(*ColorValue::Create(ColorKeyword(ValueId::Red))) == "red");
    CHECK(CssText(*ColorValue::Create(ColorKeyword(ValueId::Currentcolor))) == "currentcolor");
    CHECK(CssText(*ColorValue::Create(web::Color {web::Color::blue}))
          == ToNarrow(web::SerialisationForCSS(web::Color::blue)));
  }

  TEST_CASE_METHOD(ValueFixture, "ColorValue equality and hashing", "[CSS][Values][ColorValue]")
  {
    auto a = ColorValue::Create(ColorKeyword(ValueId::Red));
    CHECK(*a == *ColorValue::Create(ColorKeyword(ValueId::Red)));
    CHECK_FALSE(*a == *ColorValue::Create(ColorKeyword(ValueId::Blue)));
    // A keyword and the colour it resolves to are different values.
    CHECK_FALSE(*a == *ColorValue::Create(web::Color {web::Color::red}));
    CHECK(*ColorValue::Create(web::Color {web::Color::red})
          == *ColorValue::Create(web::Color {web::Color::red}));

    // `ColorValue` has no hash of its own.
    size_t seed = 0uz;
    CHECK_FALSE(a->AddHash(seed));
  }

  TEST_CASE_METHOD(ValueFixture, "ColorValue visitation", "[CSS][Values][ColorValue]")
  {
    CHECK(Children(*ColorValue::Create(ColorKeyword(ValueId::Red))).empty());
  }

  TEST_CASE_METHOD(ValueFixture, "ColorValue::GetAbsoluteColor", "[CSS][Values][ColorValue]")
  {
    CHECK(ColorValue::GetAbsoluteColor(*ColorValue::Create(web::Color {web::Color::blue}))
          == web::Color {web::Color::blue});
    CHECK(ColorValue::GetAbsoluteColor(*ColorValue::Create(ColorKeyword(ValueId::Red)))
          == web::Color {web::Color::red});

    // Absolute colour keywords as identifiers.
    CHECK(ColorValue::GetAbsoluteColor(*PrimitiveValue::Create(ValueId::Red))
          == web::Color {web::Color::red});

    // Anything else is invalid.
    CHECK_FALSE(
      ColorValue::GetAbsoluteColor(*ColorValue::Create(ColorKeyword(ValueId::Currentcolor))).IsValid());
    CHECK_FALSE(ColorValue::GetAbsoluteColor(*PrimitiveValue::Create(ValueId::Currentcolor)).IsValid());
    CHECK_FALSE(ColorValue::GetAbsoluteColor(*PrimitiveValue::Create(1.0, UnitType::px)).IsValid());
  }

  TEST_CASE_METHOD(ValueFixture, "ValuePool::CreateColorValue", "[CSS][Values][ColorValue][ValuePool]")
  {
    ValuePool pool;

    SECTION("Common colours are shared by every pool")
    {
      ValuePool other;
      for (const auto &color : {web::Color {web::Color::transparentBlack}, web::Color {web::Color::white},
                                web::Color {web::Color::black}})
      {
        auto first = pool.CreateColorValue(color);
        auto second = other.CreateColorValue(color);
        CHECK(first.get() == second.get());
        CHECK(first->GetColor() == Color {ResolvedColor {color}});
      }

      CHECK(CssText(*pool.CreateColorValue(web::Color::transparentBlack))
            == ToNarrow(web::SerialisationForCSS(web::Color::transparentBlack)));
      CHECK(CssText(*pool.CreateColorValue(web::Color::white))
            == ToNarrow(web::SerialisationForCSS(web::Color::white)));
      CHECK(CssText(*pool.CreateColorValue(web::Color::black))
            == ToNarrow(web::SerialisationForCSS(web::Color::black)));
    }

    SECTION("Other colours are cached per pool")
    {
      auto first = pool.CreateColorValue(web::Color::blue);
      auto second = pool.CreateColorValue(web::Color::blue);
      CHECK(first.get() == second.get());
      CHECK(first->GetColor() == Color {ResolvedColor {web::Color::blue}});

      CHECK(pool.CreateColorValue(web::Color::red).get() != first.get());

      ValuePool other;
      CHECK(other.CreateColorValue(web::Color::blue).get() != first.get());
    }

    SECTION("Drain empties the cache")
    {
      auto first = pool.CreateColorValue(web::Color::blue);
      pool.Drain();
      auto second = pool.CreateColorValue(web::Color::blue);
      CHECK(first.get() != second.get());
      CHECK(*first == *second);
    }
  }
}
