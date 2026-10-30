#include "Krystal.Booey/CSS/Values/ColorValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    Color Keyword(ValueId id)
    {
      return Color {KeywordColor {id}};
    }
  }

  TEST_CASE_METHOD(ValueFixture, "ColorValue::Create", "[CSS][Values][ColorValue]")
  {
    SECTION("From a CSS colour")
    {
      auto value = ColorValue::Create(Keyword(ValueId::Red));
      CHECK(value->IsColorValue());
      CHECK(value->GetColor() == Keyword(ValueId::Red));
    }

    SECTION("From a resolved colour")
    {
      auto value = ColorValue::Create(boo::Color {boo::Color::blue});
      CHECK(value->GetColor() == Color {ResolvedColor {boo::Color::blue}});
    }
  }

  TEST_CASE_METHOD(ValueFixture, "ColorValue serialisation", "[CSS][Values][ColorValue]")
  {
    CHECK(CssText(*ColorValue::Create(Keyword(ValueId::Red))) == "red");
    CHECK(CssText(*ColorValue::Create(Keyword(ValueId::Currentcolor))) == "currentcolor");
    CHECK(CssText(*ColorValue::Create(boo::Color {boo::Color::blue}))
          == ToNarrow(boo::SerialisationForCSS(boo::Color::blue)));
  }

  TEST_CASE_METHOD(ValueFixture, "ColorValue equality and hashing", "[CSS][Values][ColorValue]")
  {
    auto a = ColorValue::Create(Keyword(ValueId::Red));
    CHECK(*a == *ColorValue::Create(Keyword(ValueId::Red)));
    CHECK_FALSE(*a == *ColorValue::Create(Keyword(ValueId::Blue)));
    // A keyword and the colour it resolves to are different values.
    CHECK_FALSE(*a == *ColorValue::Create(boo::Color {boo::Color::red}));
    CHECK(*ColorValue::Create(boo::Color {boo::Color::red})
          == *ColorValue::Create(boo::Color {boo::Color::red}));

    // `ColorValue` has no hash of its own.
    size_t seed = 0uz;
    CHECK_FALSE(a->AddHash(seed));
  }

  TEST_CASE_METHOD(ValueFixture, "ColorValue visitation", "[CSS][Values][ColorValue]")
  {
    CHECK(Children(*ColorValue::Create(Keyword(ValueId::Red))).empty());
  }

  TEST_CASE_METHOD(ValueFixture, "ColorValue::GetAbsoluteColor", "[CSS][Values][ColorValue]")
  {
    CHECK(ColorValue::GetAbsoluteColor(*ColorValue::Create(boo::Color {boo::Color::blue}))
          == boo::Color {boo::Color::blue});
    CHECK(ColorValue::GetAbsoluteColor(*ColorValue::Create(Keyword(ValueId::Red)))
          == boo::Color {boo::Color::red});

    // Absolute colour keywords as identifiers.
    CHECK(ColorValue::GetAbsoluteColor(*PrimitiveValue::Create(ValueId::Red))
          == boo::Color {boo::Color::red});

    // Anything else is invalid.
    CHECK_FALSE(ColorValue::GetAbsoluteColor(*ColorValue::Create(Keyword(ValueId::Currentcolor))).IsValid());
    CHECK_FALSE(ColorValue::GetAbsoluteColor(*PrimitiveValue::Create(ValueId::Currentcolor)).IsValid());
    CHECK_FALSE(ColorValue::GetAbsoluteColor(*PrimitiveValue::Create(1.0, UnitType::px)).IsValid());
  }

  TEST_CASE_METHOD(ValueFixture, "ValuePool::CreateColorValue", "[CSS][Values][ColorValue][ValuePool]")
  {
    ValuePool pool;

    SECTION("Common colours are shared by every pool")
    {
      ValuePool other;
      for (auto color : {boo::Color {boo::Color::transparentBlack}, boo::Color {boo::Color::white},
                         boo::Color {boo::Color::black}})
      {
        auto first = pool.CreateColorValue(color);
        auto second = other.CreateColorValue(color);
        CHECK(first.get() == second.get());
        CHECK(first->GetColor() == Color {ResolvedColor {color}});
      }

      CHECK(CssText(*pool.CreateColorValue(boo::Color::transparentBlack))
            == ToNarrow(boo::SerialisationForCSS(boo::Color::transparentBlack)));
      CHECK(CssText(*pool.CreateColorValue(boo::Color::white))
            == ToNarrow(boo::SerialisationForCSS(boo::Color::white)));
      CHECK(CssText(*pool.CreateColorValue(boo::Color::black))
            == ToNarrow(boo::SerialisationForCSS(boo::Color::black)));
    }

    SECTION("Other colours are cached per pool")
    {
      auto first = pool.CreateColorValue(boo::Color::blue);
      auto second = pool.CreateColorValue(boo::Color::blue);
      CHECK(first.get() == second.get());
      CHECK(first->GetColor() == Color {ResolvedColor {boo::Color::blue}});

      CHECK(pool.CreateColorValue(boo::Color::red).get() != first.get());

      ValuePool other;
      CHECK(other.CreateColorValue(boo::Color::blue).get() != first.get());
    }

    SECTION("Drain empties the cache")
    {
      auto first = pool.CreateColorValue(boo::Color::blue);
      pool.Drain();
      auto second = pool.CreateColorValue(boo::Color::blue);
      CHECK(first.get() != second.get());
      CHECK(*first == *second);
    }
  }
}
