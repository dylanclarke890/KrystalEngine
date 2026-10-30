#include "Krystal.Booey/CSS/Calc/CalcType.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericCategory.hpp"
#include "Krystal.Booey/CSS/Values/UnitType.hpp"

namespace krys::boo::css::calc::tests
{
  TEST_CASE("CalcType(determining types)", "[CSS][Calc][CalcType]")
  {
    CHECK(Type::DetermineType(UnitType::Number) == Type::CreateNumber());
    CHECK(Type::DetermineType(UnitType::Integer) == Type::CreateNumber());
    CHECK(Type::DetermineType(UnitType::px) == Type::CreateLength());
    CHECK(Type::DetermineType(UnitType::cqmax) == Type::CreateLength());
    CHECK(Type::DetermineType(UnitType::turn) == Type::CreateAngle());
    CHECK(Type::DetermineType(UnitType::ms) == Type::CreateTime());
    CHECK(Type::DetermineType(UnitType::khz) == Type::CreateFrequency());
    CHECK(Type::DetermineType(UnitType::x) == Type::CreateResolution());
    CHECK(Type::DetermineType(UnitType::fr) == Type::CreateFlex());
    CHECK(Type::DetermineType(UnitType::Percentage) == Type::CreatePercent());

    CHECK_FALSE(Type::DeterminePercentHint(Category::Length));
    CHECK(*Type::DeterminePercentHint(Category::LengthPercentage) == PercentHint::Length);
    CHECK(*Type::DeterminePercentHint(Category::AnglePercentage) == PercentHint::Angle);
  }

  TEST_CASE("CalcType(adding types)", "[CSS][Calc][CalcType]")
  {
    CHECK(Type::Add(Type::CreateLength(), Type::CreateLength()) == Type::CreateLength());
    CHECK_FALSE(Type::Add(Type::CreateLength(), Type::CreateAngle()));
    CHECK(Type::Add(Type::CreateNumber(), Type::CreateNumber()) == Type::CreateNumber());

    // A percentage added to a length applies the length percent hint.
    auto lengthPercentage = Type::Add(Type::CreateLength(), Type::CreatePercent());
    REQUIRE(lengthPercentage);
    CHECK(lengthPercentage->Length == 1);
    CHECK(lengthPercentage->Percent == 0);
    CHECK(*lengthPercentage->PercentHint == PercentHint::Length);

    // Percent hints must agree.
    auto withAngleHint = Type::Add(Type::CreateAngle(), Type::CreatePercent());
    REQUIRE(withAngleHint);
    CHECK_FALSE(Type::Add(*lengthPercentage, *withAngleHint));
    CHECK_FALSE(Type::Add(Maybe<Type> {}, Type::CreateLength()));
    CHECK(Type::ConsistentType(Type::CreateTime(), Type::CreateTime()) == Type::CreateTime());
  }

  TEST_CASE("CalcType(multiplying and inverting types)", "[CSS][Calc][CalcType]")
  {
    auto area = Type::Multiply(Type::CreateLength(), Type::CreateLength());
    REQUIRE(area);
    CHECK(area->Length == 2);

    auto inverted = Type::Invert(Type::CreateLength());
    CHECK(inverted.Length == -1);
    CHECK(Type::Multiply(Type::CreateLength(), inverted) == Type::CreateNumber());

    // Exponents that would overflow fail instead of wrapping.
    CHECK_FALSE(Type::Multiply(Type {.Length = 100}, Type {.Length = 100}));
    CHECK_FALSE(Type::Multiply(Type {.Length = -100}, Type {.Length = -100}));
    CHECK(Type::Multiply(Type {.Length = 127}, Type {}) == Type {.Length = 127});
  }

  TEST_CASE("CalcType(making types consistent)", "[CSS][Calc][CalcType]")
  {
    Type hinted = Type::CreateLength();
    hinted.ApplyPercentHint(PercentHint::Length);

    auto consistent = Type::MadeConsistent(Type::CreateNumber(), hinted);
    REQUIRE(consistent);
    CHECK(*consistent->PercentHint == PercentHint::Length);

    Type angleHinted = Type::CreateAngle();
    angleHinted.ApplyPercentHint(PercentHint::Angle);
    CHECK_FALSE(Type::MadeConsistent(hinted, angleHinted));

    CHECK(TransformType<OutputTransform::None>(Type::CreateFlex()) == Type::CreateFlex());
    CHECK(TransformType<OutputTransform::AngleMadeConsistent>(hinted)->Angle == 1);
    CHECK_FALSE(TransformType<OutputTransform::NumberMadeConsistent>(Maybe<Type> {}));

    CHECK(MergeTypes<MergePolicy::Same>(Type::CreateTime(), Type::CreateTime()) == Type::CreateTime());
    CHECK_FALSE(MergeTypes<MergePolicy::Same>(Type::CreateTime(), Type::CreateLength()));
    CHECK(MergeTypes<MergePolicy::Consistent>(Type::CreateLength(), Type::CreatePercent()));
    CHECK_FALSE(MergeTypes<MergePolicy::Consistent>(Maybe<Type> {Type::CreateLength()}, Maybe<Type> {}));
  }

  TEST_CASE("CalcType(percent hints)", "[CSS][Calc][CalcType]")
  {
    Type type = Type::CreatePercent();
    type.ApplyPercentHint(PercentHint::Angle);
    CHECK(type.Angle == 1);
    CHECK(type.Percent == 0);
    CHECK(*type.PercentHint == PercentHint::Angle);
    CHECK(type[PercentHint::Angle] == 1);
    CHECK(type[BaseType::Angle] == 1);
  }

  TEST_CASE("CalcType(matching categories)", "[CSS][Calc][CalcType]")
  {
    CHECK(Type::CreateNumber().Matches(Category::Number));
    CHECK(Type::CreateNumber().Matches(Category::Integer));
    CHECK(Type::CreateLength().Matches(Category::Length));
    CHECK_FALSE(Type::CreateLength().Matches(Category::Angle));
    CHECK(Type::CreateLength().Matches(Category::LengthPercentage));
    CHECK(Type::CreatePercent().Matches(Category::LengthPercentage));
    CHECK_FALSE(Type::CreateAngle().Matches(Category::LengthPercentage));

    Type hinted = Type::CreateLength();
    hinted.ApplyPercentHint(PercentHint::Length);
    CHECK_FALSE(hinted.Matches(Category::Length));
    CHECK(hinted.Matches(Category::LengthPercentage));

    CHECK(ValidateType<AllowedTypes::Any>(Type::CreateLength()));
    CHECK(ValidateType<AllowedTypes::Number>(Type::CreateNumber()));
    CHECK_FALSE(ValidateType<AllowedTypes::Number>(Type::CreateLength()));
    CHECK(ValidateType<AllowedTypes::NumberOrAngle>(Type::CreateAngle()));
  }

  TEST_CASE("CalcType(calculation category)", "[CSS][Calc][CalcType]")
  {
    CHECK(Type::CreateNumber().CalculationCategory() == Category::Number);
    CHECK(Type::CreateLength().CalculationCategory() == Category::Length);
    CHECK(Type::CreatePercent().CalculationCategory() == Category::Percentage);
    CHECK(Type::CreateResolution().CalculationCategory() == Category::Resolution);

    Type hinted = Type::CreateAngle();
    hinted.ApplyPercentHint(PercentHint::Angle);
    CHECK(hinted.CalculationCategory() == Category::AnglePercentage);

    CHECK_FALSE(Type {.Length = 2}.CalculationCategory());
    CHECK_FALSE(Type {.Length = 1, .Time = 1}.CalculationCategory());
  }
}
