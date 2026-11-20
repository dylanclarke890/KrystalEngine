#include "Krystal.Booey/CSS/Values/Easing/EasingFunction.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    CSSOMString SerialiseEasing(const EasingFunction &value)
    {
      return Serialise(DefaultSerialisationContext(), value);
    }
  }

  namespace
  {
    EasingFunction Steps(StepsEasingParameters parameters)
    {
      return EasingFunction {StepsEasingFunction {.Parameters = krys::move(parameters)}};
    }

    EasingFunction CubicBezier(const Number<ClosedUnitRange> &x1, const Number<> &y1,
                               const Number<ClosedUnitRange> &x2, const Number<> &y2)
    {
      return EasingFunction {CubicBezierEasingFunction {
        .Parameters = {.Value = {CubicBezierEasingParameters::Coordinate {x1, y1},
                                 CubicBezierEasingParameters::Coordinate {x2, y2}}}}};
    }

    EasingFunction Linear()
    {
      using Stop = LinearEasingParameters::Stop;
      return EasingFunction {
        LinearEasingFunction {.Parameters = {.Stops = {
                                               Stop {.Output = 0_css_number, .Input = {}},
                                               Stop {.Output = 0.5_css_number,
                                                     .Input =
                                                       Stop::Length {
                                                         .Input = 50_css_percentage,
                                                         .Extra = {},
                                                       }},
                                               Stop {.Output = 1_css_number,
                                                     .Input =
                                                       Stop::Length {
                                                         .Input = 80_css_percentage,
                                                         .Extra = 90_css_percentage,
                                                       }},
                                             }}}};
    }

    EasingFunction Spring()
    {
      return EasingFunction {SpringEasingFunction {.Parameters = {
                                                     .Mass = 1_css_number,
                                                     .Stiffness = 100_css_number,
                                                     .Damping = 10_css_number,
                                                     .InitialVelocity = 0_css_number,
                                                   }}};
    }
  }

  TEST_CASE("EasingFunction(Serialise keywords)", "[CSS][Easing][EasingFunction]")
  {
    CHECK(SerialiseEasing({keywords::Linear {}}) == u8"linear");
    CHECK(SerialiseEasing({keywords::Ease {}}) == u8"ease");
    CHECK(SerialiseEasing({keywords::EaseIn {}}) == u8"ease-in");
    CHECK(SerialiseEasing({keywords::EaseOut {}}) == u8"ease-out");
    CHECK(SerialiseEasing({keywords::EaseInOut {}}) == u8"ease-in-out");
    CHECK(SerialiseEasing({keywords::StepStart {}}) == u8"step-start");
    CHECK(SerialiseEasing({keywords::StepEnd {}}) == u8"step-end");
  }

  TEST_CASE("EasingFunction(Serialise functions)", "[CSS][Easing][EasingFunction]")
  {
    CHECK(SerialiseEasing(CubicBezier(0.25_css_number, 0.1_css_number, 0.25_css_number, 1_css_number))
          == u8"cubic-bezier(0.25, 0.1, 0.25, 1)");
    CHECK(SerialiseEasing(Linear()) == u8"linear(0, 0.5 50%, 1 80% 90%)");
    CHECK(SerialiseEasing(Spring()) == u8"spring(1 100 10 0)");
  }

  TEST_CASE("EasingFunction(Serialise steps)", "[CSS][Easing][EasingFunction]")
  {
    using P = StepsEasingParameters;

    CHECK(SerialiseEasing(Steps({P::JumpStart {4_css_integer}})) == u8"steps(4, jump-start)");
    CHECK(SerialiseEasing(Steps({P::JumpBoth {4_css_integer}})) == u8"steps(4, jump-both)");
    CHECK(SerialiseEasing(Steps({P::Start {4_css_integer}})) == u8"steps(4, start)");
    CHECK(SerialiseEasing(Steps({P::JumpNone {2_css_integer}})) == u8"steps(2, jump-none)");

    // `jump-end` and `end` are the default, so the keyword is omitted.
    CHECK(SerialiseEasing(Steps({P::JumpEnd {4_css_integer}})) == u8"steps(4)");
    CHECK(SerialiseEasing(Steps({P::End {4_css_integer}})) == u8"steps(4)");
  }

  TEST_CASE("EasingFunction(Equality)", "[CSS][Easing][EasingFunction]")
  {
    using P = StepsEasingParameters;

    CHECK(Linear() == Linear());
    CHECK(Spring() == Spring());
    CHECK(EasingFunction {keywords::Ease {}} == EasingFunction {keywords::Ease {}});
    CHECK(EasingFunction {keywords::Ease {}} != EasingFunction {keywords::EaseIn {}});

    // `end` and `jump-end` serialise the same, but are different values.
    CHECK(Steps({P::End {4_css_integer}}) != Steps({P::JumpEnd {4_css_integer}}));
    CHECK(Steps({P::End {4_css_integer}}) != Steps({P::End {5_css_integer}}));

    CHECK(CubicBezier(0.25_css_number, 0.1_css_number, 0.25_css_number, 1_css_number)
          != CubicBezier(0.25_css_number, 0.1_css_number, 0.25_css_number, 0.5_css_number));
  }

  TEST_CASE("EasingFunction(SwitchOn)", "[CSS][Easing][EasingFunction]")
  {
    auto isSteps = [](const EasingFunction &function)
    {
      return function.SwitchOn([](const StepsEasingFunction &) { return true; },
                               [](const auto &) { return false; });
    };

    CHECK(isSteps(Steps({StepsEasingParameters::Start {1_css_integer}})));
    CHECK_FALSE(isSteps({keywords::StepStart {}}));
    CHECK_FALSE(isSteps(Spring()));
  }

  TEST_CASE("EasingFunction(Visitation)", "[CSS][Easing][EasingFunction]")
  {
    size_t visited = 0;
    Func<IterationStatus(Value &)> visitor = [&](Value &)
    {
      ++visited;
      return IterationStatus::Continue;
    };

    // Raw numbers have no child values.
    CHECK(VisitValueChildren(visitor, Linear()) == IterationStatus::Continue);
    CHECK(VisitValueChildren(visitor, Spring()) == IterationStatus::Continue);
    CHECK(visited == 0);

    CHECK(CollectComputedStyleDependencies(Linear()).Properties.empty());
  }
}
