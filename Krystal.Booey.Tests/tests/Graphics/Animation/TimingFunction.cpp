#include "Krystal.Booey/Graphics/Animation/TimingFunction.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Graphics/UnitBezier.hpp"

namespace krys::boo::tests
{
  namespace
  {
    using Preset = CubicBezierTimingFunction::TimingFunctionPreset;
    using StepPosition = StepsTimingFunction::StepPosition;

    KRYS_NODISCARD std::string Text(const TimingFunction &function) noexcept
    {
      auto text = function.CssText();
      return std::string {text.begin(), text.end()};
    }
  }

  TEST_CASE("UnitBezier", "[Graphics][TimingFunction]")
  {
    UnitBezier ease {0.25, 0.1, 0.25, 1.0};
    CHECK(ease.Solve(0.0, 1e-6) == Catch::Approx(0.0));
    CHECK(ease.Solve(1.0, 1e-6) == Catch::Approx(1.0));
    CHECK(ease.Solve(0.5, 1e-6) == Catch::Approx(0.8024033877).epsilon(1e-5));
    // Outside [0, 1], the curve continues along its end-point gradients.
    CHECK(ease.Solve(-0.5, 1e-6) == Catch::Approx(-0.2));
    UnitBezier linear {0, 0, 1, 1};
    CHECK(linear.Solve(0.3, 1e-6) == Catch::Approx(0.3));
  }

  TEST_CASE("TimingFunction", "[Graphics][TimingFunction]")
  {
    SECTION("linear")
    {
      auto identity = LinearTimingFunction::Create();
      CHECK(identity->TransformProgress(0.4, 1) == 0.4);
      CHECK(Text(*identity) == "linear");
      CHECK(&LinearTimingFunction::Identity() == &LinearTimingFunction::Identity());

      auto function = LinearTimingFunction::Create({{0, 0}, {1, 0.5}, {1, 1}});
      CHECK(function->TransformProgress(0.25, 1) == 0.5);
      CHECK(function->TransformProgress(0.75, 1) == 1);
      CHECK(function->TransformProgress(-1, 1) == -2);
      CHECK(Text(*function) == "linear(0 0%, 1 50%, 1 100%)");

      // A step: two points at the same progress jump to the second one's value.
      auto step = LinearTimingFunction::Create({{0, 0}, {0, 0.5}, {1, 0.5}, {1, 1}});
      CHECK(step->TransformProgress(0.5, 1) == 1);
      CHECK((*function == *static_cast<const TimingFunction &>(*function).Clone()));
      CHECK_FALSE((*function == *identity));
    }

    SECTION("cubic-bezier")
    {
      auto ease = CubicBezierTimingFunction::Create();
      CHECK(ease->GetTimingFunctionPreset() == Preset::Ease);
      CHECK(ease->TransformProgress(0.5, 1) == Catch::Approx(0.8024033877).epsilon(1e-3));
      CHECK(Text(*ease) == "ease");
      CHECK(Text(*CubicBezierTimingFunction::Create(Preset::EaseIn)) == "ease-in");
      CHECK(Text(*CubicBezierTimingFunction::Create(Preset::EaseOut)) == "ease-out");
      CHECK(Text(*CubicBezierTimingFunction::Create(Preset::EaseInOut)) == "ease-in-out");

      auto custom = CubicBezierTimingFunction::Create(0.1, 0.2, 0.3, 0.4);
      CHECK(Text(*custom) == "cubic-bezier(0.1, 0.2, 0.3, 0.4)");
      auto reversed = custom->CreateReversed();
      CHECK(reversed->X1() == Catch::Approx(0.7));
      CHECK(reversed->Y2() == Catch::Approx(0.8));

      // Presets compare by preset; custom curves by their control points.
      CHECK((*CubicBezierTimingFunction::Create(Preset::EaseIn, 0, 0, 0, 0)
             == *CubicBezierTimingFunction::Create(Preset::EaseIn)));
      CHECK_FALSE((*custom == *CubicBezierTimingFunction::Create(0.1, 0.2, 0.3, 0.5)));
      CHECK((*custom == *static_cast<const TimingFunction &>(*custom).Clone()));

      CHECK(CubicBezierTimingFunction::Create(0, 0, 1, 1)->IsLinear());
      CHECK(CubicBezierTimingFunction::Create(0, 0, 1, 1)->TransformProgress(0.3, 1) == 0.3);
      CHECK_FALSE(custom->IsLinear());
    }

    SECTION("steps")
    {
      CHECK(StepsTimingFunction::Create(4, StepPosition::End)->TransformProgress(0.3, 1) == 0.25);
      CHECK(StepsTimingFunction::Create(4, StepPosition::JumpStart)->TransformProgress(0.3, 1) == 0.5);
      CHECK(StepsTimingFunction::Create(4, StepPosition::JumpNone)->TransformProgress(0.3, 1)
            == Catch::Approx(1.0 / 3));
      CHECK(StepsTimingFunction::Create(4, StepPosition::JumpBoth)->TransformProgress(0.3, 1) == 0.4);
      CHECK(StepsTimingFunction::Create(4, StepPosition::End)->TransformProgress(1, 1) == 1);
      // The before flag drops a step at the exact step boundaries.
      CHECK(StepsTimingFunction::Create(4, StepPosition::End)
              ->TransformProgress(0.5, 1, TimingFunction::Before::Yes)
            == 0.25);
      CHECK(StepsTimingFunction::Create(4, StepPosition::Start)->TransformProgress(-0.1, 1) == 0);

      CHECK(Text(*StepsTimingFunction::Create(3, StepPosition::End)) == "steps(3)");
      CHECK(Text(*StepsTimingFunction::Create(3, StepPosition::JumpStart)) == "steps(3, jump-start)");
      CHECK(Text(*StepsTimingFunction::Create(3, std::nullopt)) == "steps(3)");
      CHECK((*StepsTimingFunction::Create(2, std::nullopt)
             == *StepsTimingFunction::Create(2, StepPosition::End)));
      CHECK_FALSE((*StepsTimingFunction::Create(2, StepPosition::Start)
                   == *StepsTimingFunction::Create(2, StepPosition::End)));
      CHECK((*StepsTimingFunction::Create() == *StepsTimingFunction::Create(1, StepPosition::End)));
    }

    SECTION("spring")
    {
      auto spring = SpringTimingFunction::Create(1, 100, 10, 0);
      CHECK(spring->TransformProgress(0, 1) == Catch::Approx(0));
      CHECK(spring->TransformProgress(1, 1) == Catch::Approx(1).margin(0.01));
      CHECK(Text(*spring) == "spring(1 100 10 0)");
      CHECK((*spring == *static_cast<const TimingFunction &>(*spring).Clone()));
      CHECK_FALSE((*spring == *SpringTimingFunction::Create(1, 100, 11, 0)));
    }

    SECTION("downcasts")
    {
      Ref<TimingFunction> function = StepsTimingFunction::Create();
      CHECK(function->IsStepsTimingFunction());
      CHECK(DynamicDowncast<StepsTimingFunction>(*function));
      CHECK_FALSE(DynamicDowncast<LinearTimingFunction>(*function));
    }
  }
}
