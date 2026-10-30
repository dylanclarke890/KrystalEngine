#include "Krystal.Booey/CSS/TypedOM/CSSNumericValue.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSKeywordValue.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSNumericFactory.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSUnitValue.hpp"
#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSMathClamp.hpp"
#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSMathInvert.hpp"
#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSMathMax.hpp"
#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSMathMin.hpp"
#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSMathNegate.hpp"
#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSMathProduct.hpp"
#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSMathSum.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/DOM/Document.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    using Factory = CSSNumericFactory;

    KRYS_NODISCARD std::string Text(const CSSStyleValue &value)
    {
      return ToNarrow(value.ToString());
    }

    /// @brief The value as a numeric value, whose `Equals` overloads are public.
    KRYS_NODISCARD Ref<CSSNumericValue> Numeric(Ref<CSSUnitValue> value)
    {
      return value;
    }

    KRYS_NODISCARD CSSNumberish N(Ref<CSSUnitValue> value)
    {
      return RefPtr<CSSNumericValue> {krys::move(value)};
    }

    KRYS_NODISCARD CSSNumberish N(Ref<CSSNumericValue> value)
    {
      return RefPtr<CSSNumericValue> {krys::move(value)};
    }

    template <typename T>
    KRYS_NODISCARD Ref<T> Value(dom::ExceptionOr<Ref<T>> &&result)
    {
      REQUIRE(result.HasValue());
      return result.ReleaseValue();
    }

    template <typename T>
    KRYS_NODISCARD dom::ExceptionCode Error(dom::ExceptionOr<T> &&result)
    {
      REQUIRE(result.HasException());
      return result.ReleaseException().Code();
    }
  }

  TEST_CASE_METHOD(ValueFixture, "CSSUnitValue: units, serialisation and conversion", "[CSS][TypedOM]")
  {
    auto px = Value(CSSUnitValue::Create(1.5, u8"px"));
    CHECK(px->Value() == 1.5);
    CHECK(ToNarrow(px->Unit()) == "px");
    CHECK(Text(*px) == "1.5px");
    CHECK(px->Type().Length == 1);
    CHECK(px->Type().Matches<CSSNumericBaseType::Length>());

    // `number` and `percent` name the unitless units.
    CHECK(Text(*Value(CSSUnitValue::Create(2, u8"number"))) == "2");
    auto percent = Value(CSSUnitValue::Create(50, u8"percent"));
    CHECK(ToNarrow(percent->Unit()) == "percent");
    CHECK(Text(*percent) == "50%");
    CHECK(percent->Type().Percent == 1);
    CHECK(Factory::Number(3)->Type().MatchesNumber());

    CHECK(Error(CSSUnitValue::Create(1, u8"lemon")) == dom::ExceptionCode::TypeError);
    CHECK(CSSUnitValue::ParseUnit(u8"lemon") == UnitType::Unknown);

    // Conversion within a category, and not across categories.
    auto inches = Value(Factory::In(1)->To(u8"px"));
    CHECK(inches->Value() == Catch::Approx(96));
    CHECK(ToNarrow(inches->Unit()) == "px");
    CHECK(Value(Factory::S(1.5)->To(u8"ms"))->Value() == Catch::Approx(1'500));
    CHECK(Error(Factory::Px(1)->To(u8"deg")) == dom::ExceptionCode::TypeError);
    CHECK(Error(Factory::Px(1)->To(u8"lemon")) == dom::ExceptionCode::SyntaxError);
    CHECK_FALSE(Factory::Em(1)->ConvertTo(UnitType::px));

    CHECK(Numeric(Factory::Px(1))->Equals(*Factory::Px(1)));
    CHECK_FALSE(Numeric(Factory::Px(1))->Equals(*Factory::Px(2)));
    CHECK_FALSE(Numeric(Factory::Px(96))->Equals(*Factory::In(1)));

    // As a CSS value.
    CHECK(CssText(*Factory::Px(1)->ToCSSValue()) == "1px");
  }

  TEST_CASE_METHOD(ValueFixture, "CSSNumericValue: arithmetic", "[CSS][TypedOM]")
  {
    SECTION("values of the same unit combine")
    {
      auto sum = Value(Factory::Px(1)->Add({N(Factory::Px(2)), N(Factory::Px(3))}));
      REQUIRE(Is<CSSUnitValue>(*sum));
      CHECK(Text(*sum) == "6px");
      CHECK(Text(*Value(Factory::Px(5)->Sub({N(Factory::Px(2))}))) == "3px");
      CHECK(Text(*Value(Factory::Px(5)->Mul({2.0, 3.0}))) == "30px");
      CHECK(Text(*Value(Factory::Px(6)->Div({2.0}))) == "3px");
      CHECK(Text(*Value(Factory::Px(6)->Min({N(Factory::Px(2)), N(Factory::Px(4))}))) == "2px");
      CHECK(Text(*Value(Factory::Px(6)->Max({N(Factory::Px(2)), N(Factory::Px(9))}))) == "9px");
    }

    SECTION("values of different units make math values")
    {
      auto sum = Value(Factory::Px(1)->Add({N(Factory::Em(2))}));
      REQUIRE(Is<CSSMathSum>(*sum));
      CHECK(Text(*sum) == "calc(1px + 2em)");
      CHECK(Downcast<CSSMathSum>(*sum).Values().Length() == 2);
      // Adding to a sum extends it.
      CHECK(Text(*Value(sum->Add({N(Factory::Vw(3))}))) == "calc(1px + 2em + 3vw)");
      // Subtracting a unit value negates its number; subtracting a math value negates it.
      CHECK(Text(*Value(Factory::Px(1)->Sub({N(Factory::Em(2))}))) == "calc(1px + -2em)");
      auto difference = Value(Factory::Px(1)->Sub({N(Value(Factory::Em(2)->Add({N(Factory::Vw(3))})))}));
      CHECK(Text(*difference) == "calc(1px - (2em + 3vw))");

      auto product = Value(Factory::Px(2)->Mul({N(Factory::Px(3))}));
      REQUIRE(Is<CSSMathProduct>(*product));
      CHECK(Text(*product) == "calc(2px * 3px)");
      CHECK(product->Type().Length == 2);
      CHECK(Text(*Value(Factory::Px(1)->Div({N(Factory::Em(2))}))) == "calc(1px / 2em)");

      CHECK(Text(*Value(Factory::Px(1)->Min({N(Factory::Em(2))}))) == "min(1px, 2em)");
      CHECK(Text(*Value(Factory::Px(1)->Max({N(Factory::Em(2)), N(Factory::Vw(3))})))
            == "max(1px, 2em, 3vw)");
    }

    SECTION("incompatible types and division by zero are errors")
    {
      CHECK(Error(Factory::Px(1)->Add({N(Factory::S(1))})) == dom::ExceptionCode::TypeError);
      CHECK(Error(Factory::Px(1)->Div({0.0})) == dom::ExceptionCode::RangeError);
      CHECK(Error(Factory::Px(1)->Min({N(Factory::Deg(1))})) == dom::ExceptionCode::TypeError);
    }

    SECTION("a percentage added to a length takes a percent hint")
    {
      auto sum = Value(Factory::Px(1)->Add({N(Factory::Percent(10))}));
      CHECK(sum->Type().Length == 1);
      CHECK(sum->Type().PercentHint == CSSNumericBaseType::Length);
      // So it no longer matches <length> alone.
      CHECK_FALSE(sum->Type().Matches<CSSNumericBaseType::Length>());
    }

    SECTION("equality")
    {
      auto a = Value(Factory::Px(1)->Add({N(Factory::Em(2))}));
      auto b = Value(Factory::Px(1)->Add({N(Factory::Em(2))}));
      auto c = Value(Factory::Px(1)->Add({N(Factory::Em(3))}));
      CHECK(a->Equals(*b));
      CHECK_FALSE(a->Equals(*c));
      CHECK((Numeric(Factory::Px(1))->Equals(List<CSSNumberish> {N(Factory::Px(1)), N(Factory::Px(1))})));
      CHECK_FALSE(
        (Numeric(Factory::Px(1))->Equals(List<CSSNumberish> {N(Factory::Px(1)), N(Factory::Em(1))})));
    }
  }

  TEST_CASE_METHOD(ValueFixture, "CSSMathValue: negation, inversion and clamping", "[CSS][TypedOM]")
  {
    auto negate = CSSMathNegate::Create(N(Factory::Px(1)));
    CHECK(Text(*negate) == "calc(-1px)");
    CHECK(static_cast<const CSSMathValue &>(*negate).GetOperator() == CSSMathOperator::Negate);
    CHECK(negate->Type().Length == 1);

    auto invert = CSSMathInvert::Create(N(Factory::Px(2)));
    CHECK(Text(*invert) == "calc(1 / 2px)");
    CHECK(invert->Type().Length == -1);

    auto clamp = Value(CSSMathClamp::Create(N(Factory::Px(1)), N(Factory::Em(2)), N(Factory::Px(3))));
    CHECK(Text(*clamp) == "clamp(1px, 2em, 3px)");
    CHECK(Error(CSSMathClamp::Create(N(Factory::Px(1)), N(Factory::S(2)), N(Factory::Px(3))))
          == dom::ExceptionCode::TypeError);

    // Of the same unit, a clamp resolves to its sum value.
    auto resolved = Value(CSSMathClamp::Create(N(Factory::Px(1)), N(Factory::Px(5)), N(Factory::Px(3))));
    CHECK(Text(*Value(resolved->To(UnitType::px))) == "3px");

    CHECK(Error(CSSMathSum::Create(List<Ref<CSSNumericValue>> {})) == dom::ExceptionCode::SyntaxError);

    // As CSS values: calc() expressions, serialised as calc() is (its terms sorted).
    auto sum = Value(Factory::Px(1)->Add({N(Factory::Em(2))}));
    CHECK(CssText(*sum->ToCSSValue()) == "calc(2em + 1px)");
  }

  TEST_CASE_METHOD(ValueFixture, "CSSNumericValue: to() and toSum()", "[CSS][TypedOM]")
  {
    auto sum = Value(Factory::Px(1)->Add({N(Factory::In(1)), N(Factory::Em(2))}));

    // A sum of lengths of different kinds has more than one sum value item.
    CHECK(Error(sum->To(UnitType::px)) == dom::ExceptionCode::TypeError);

    auto split = Value(sum->ToSum({u8"px", u8"em"}));
    REQUIRE(split->Values().Length() == 2);
    CHECK(Downcast<CSSUnitValue>(*split->Values().Array()[0]).Value() == Catch::Approx(97));
    CHECK(Text(*split->Values().Array()[1]) == "2em");

    // Without units, the canonical units sorted by name.
    auto canonical = Value(sum->ToSum({}));
    CHECK(Text(*canonical) == "calc(2em + 97px)");

    CHECK(Error(sum->ToSum({u8"px"})) == dom::ExceptionCode::TypeError);
    CHECK(Error(sum->ToSum({u8"lemon"})) == dom::ExceptionCode::SyntaxError);
  }

  TEST_CASE_METHOD(ValueFixture, "CSSNumericValue: parsing", "[CSS][TypedOM]")
  {
    auto document = CreateRef<dom::Document>();

    auto px = Value(CSSNumericValue::Parse(*document, u8"  10px "));
    REQUIRE(Is<CSSUnitValue>(*px));
    CHECK(Text(*px) == "10px");
    CHECK(Text(*Value(CSSNumericValue::Parse(*document, u8"25%"))) == "25%");
    CHECK(Text(*Value(CSSNumericValue::Parse(*document, u8"4"))) == "4");

    auto calc = Value(CSSNumericValue::Parse(*document, u8"calc(1px + 2em)"));
    REQUIRE(Is<CSSMathSum>(*calc));
    CHECK(Text(*calc) == "calc(1px + 2em)");
    CHECK(Is<CSSMathMin>(*Value(CSSNumericValue::Parse(*document, u8"min(1px, 2em)"))));

    CHECK(Error(CSSNumericValue::Parse(*document, u8"")) == dom::ExceptionCode::SyntaxError);
    CHECK(Error(CSSNumericValue::Parse(*document, u8"auto")) == dom::ExceptionCode::SyntaxError);
    CHECK(Error(CSSNumericValue::Parse(*document, u8"1px 2px")) == dom::ExceptionCode::SyntaxError);
    CHECK(Error(CSSNumericValue::Parse(*document, u8"1lemon")) == dom::ExceptionCode::SyntaxError);
    CHECK(Error(CSSNumericValue::Parse(*document, u8"calc(1px + )")) == dom::ExceptionCode::SyntaxError);
  }

  TEST_CASE_METHOD(ValueFixture, "CSSKeywordValue", "[CSS][TypedOM]")
  {
    auto keyword = Value(CSSKeywordValue::Create(u8"auto"));
    CHECK(ToNarrow(keyword->Value()) == "auto");
    CHECK(Text(*keyword) == "auto");
    CHECK(CssText(*keyword->ToCSSValue()) == "auto");
    CHECK(Error(CSSKeywordValue::Create(u8"")) == dom::ExceptionCode::TypeError);
    CHECK(keyword->SetValue(u8"").HasException());
    CHECK(ToNarrow(keyword->Value()) == "auto");

    // An identifier that isn't a keyword is a custom identifier.
    auto custom = CSSKeywordValue::RectifyKeywordish(CSSOMString(u8"my-name"));
    CHECK(CssText(*custom->ToCSSValue()) == "my-name");
    CHECK(CSSKeywordValue::RectifyKeywordish(RefPtr<CSSKeywordValue> {keyword}).get() == keyword.get());
  }
}
