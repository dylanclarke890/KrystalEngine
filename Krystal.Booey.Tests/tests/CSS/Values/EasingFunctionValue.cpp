#include "Krystal.Booey/CSS/Values/EasingFunctionValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/Easing/EasingFunction.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    KRYS_NODISCARD EasingFunction Steps(StepsEasingParameters parameters) noexcept
    {
      return EasingFunction {StepsEasingFunction {.Parameters = krys::move(parameters)}};
    }
  }

  TEST_CASE_METHOD(ValueFixture, "EasingFunctionValue::Create", "[CSS][Values][EasingFunctionValue]")
  {
    EasingFunction easing {keywords::EaseIn {}};
    auto value = EasingFunctionValue::Create(easing);
    CHECK(value->IsEasingFunctionValue());
    CHECK(value->GetEasingFunction() == easing);
  }

  TEST_CASE_METHOD(ValueFixture, "EasingFunctionValue serialisation", "[CSS][Values][EasingFunctionValue]")
  {
    using P = StepsEasingParameters;
    CHECK(CssText(*EasingFunctionValue::Create(EasingFunction {keywords::EaseIn {}})) == "ease-in");
    CHECK(CssText(*EasingFunctionValue::Create(Steps({P::JumpStart {4_css_integer}})))
          == "steps(4, jump-start)");
    CHECK(CssText(*EasingFunctionValue::Create(Steps({P::JumpEnd {4_css_integer}}))) == "steps(4)");
  }

  TEST_CASE_METHOD(ValueFixture, "EasingFunctionValue equality", "[CSS][Values][EasingFunctionValue]")
  {
    using P = StepsEasingParameters;
    auto a = EasingFunctionValue::Create(Steps({P::JumpStart {4_css_integer}}));
    CHECK(*a == *EasingFunctionValue::Create(Steps({P::JumpStart {4_css_integer}})));
    CHECK_FALSE(*a == *EasingFunctionValue::Create(Steps({P::JumpStart {2_css_integer}})));
    CHECK_FALSE(*a == *EasingFunctionValue::Create(Steps({P::JumpBoth {4_css_integer}})));
    CHECK_FALSE(*a == *EasingFunctionValue::Create(EasingFunction {keywords::EaseIn {}}));
  }

  TEST_CASE_METHOD(ValueFixture, "EasingFunctionValue has no children", "[CSS][Values][EasingFunctionValue]")
  {
    using P = StepsEasingParameters;
    CHECK(Children(*EasingFunctionValue::Create(Steps({P::JumpStart {4_css_integer}}))).empty());
  }
}
