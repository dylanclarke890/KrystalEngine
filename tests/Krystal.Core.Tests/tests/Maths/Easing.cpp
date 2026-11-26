#include "Krystal.Core/Maths/Easing.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Maths/SpringSolver.hpp"
#include "Krystal.Core/Maths/UnitBezier.hpp"

namespace krys::tests
{
  TEST_CASE("UnitBezier", "[Maths][Easing]")
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

  TEST_CASE("SpringSolver", "[Maths][Easing]")
  {
    // Under-damped: it overshoots before settling.
    const SpringSolver bouncy(1, 100, 10, 0);
    CHECK(bouncy.Solve(0) == Catch::Approx(0.0));
    CHECK(bouncy.Solve(10) == Catch::Approx(1.0));

    // Critically damped: it never passes its target.
    const SpringSolver critical(1, 100, 20, 0);
    CHECK(critical.Solve(0) == Catch::Approx(0.0));
    CHECK(critical.Solve(0.2) < 1.0);
    CHECK(critical.Solve(10) == Catch::Approx(1.0));
  }

  TEST_CASE("StepEasing", "[Maths][Easing]")
  {
    CHECK(StepEasing(0.3, 4, StepPosition::JumpEnd) == 0.25);
    CHECK(StepEasing(0.3, 4, StepPosition::End) == 0.25);
    CHECK(StepEasing(0.3, 4, StepPosition::JumpStart) == 0.5);
    CHECK(StepEasing(0.5, 4, StepPosition::JumpNone) == Catch::Approx(2.0 / 3.0));
    CHECK(StepEasing(0.5, 4, StepPosition::JumpBoth) == 0.6);
    CHECK(StepEasing(1.0, 4, StepPosition::JumpEnd) == 1.0);

    // Before the animation, a jump on the boundary isn't taken yet.
    CHECK(StepEasing(0.0, 4, StepPosition::JumpStart) == 0.25);
    CHECK(StepEasing(0.0, 4, StepPosition::JumpStart, true) == 0.0);
  }

  TEST_CASE("LinearEasing", "[Maths][Easing]")
  {
    CHECK(LinearEasing({}, 0.4) == 0.4);

    const LinearEasingPoint points[] {
      {.Value = 0, .Progress = 0}, {.Value = 1, .Progress = 0.5}, {.Value = 0, .Progress = 1}};
    CHECK(LinearEasing(points, 0.25) == 0.5);
    CHECK(LinearEasing(points, 0.75) == 0.5);
    // Beyond the points, the end segments extend.
    CHECK(LinearEasing(points, 1.5) == -1.0);
    CHECK(LinearEasing(points, -0.5) == -1.0);

    // A jump: two points at the same progress take the later value.
    const LinearEasingPoint jump[] {{.Value = 0, .Progress = 0},
                                    {.Value = 0, .Progress = 0.5},
                                    {.Value = 1, .Progress = 0.5},
                                    {.Value = 1, .Progress = 1}};
    CHECK(LinearEasing(jump, 0.5) == 1.0);
  }
}
