#include "Krystal.Booey/CSS/Calc/Executor.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"
#include <cmath>
#include <numbers>

namespace krys::boo::css::calc::tests
{
  using Catch::Matchers::WithinAbs;

  TEST_CASE("Executor(arithmetic)", "[CSS][Calc][Executor]")
  {
    CHECK(ExecuteOperation<Operator::Sum>(1.0, 2.0) == 3.0);
    CHECK(ExecuteOperation<Operator::Sum>(List<double> {1.0, 2.0, 3.0}) == 6.0);
    CHECK(ExecuteOperation<Operator::Sum>(List<int> {1, 2, 3}, [](const int &v) { return v * 2.0; }) == 12.0);
    CHECK(ExecuteOperation<Operator::Product>(List<double> {2.0, 3.0, 4.0}) == 24.0);
    CHECK(ExecuteOperation<Operator::Negate>(2.0) == -2.0);
    CHECK(ExecuteOperation<Operator::Invert>(4.0) == 0.25);
    CHECK(ExecuteOperation<Operator::Abs>(-4.0) == 4.0);
    CHECK(ExecuteOperation<Operator::Sign>(-4.0) == -1.0);
    CHECK(ExecuteOperation<Operator::Sign>(4.0) == 1.0);
    CHECK(std::signbit(ExecuteOperation<Operator::Sign>(-0.0)));
    CHECK(ExecuteOperation<Operator::Pow>(2.0, 10.0) == 1024.0);
    CHECK(ExecuteOperation<Operator::Sqrt>(16.0) == 4.0);
    CHECK(ExecuteOperation<Operator::Hypot>(List<double> {3.0, 4.0}) == 5.0);
    CHECK(ExecuteOperation<Operator::Hypot>(List<double> {-3.0}) == 3.0);
    CHECK(std::isnan(ExecuteOperation<Operator::Hypot>(List<double> {})));
    CHECK_THAT(ExecuteOperation<Operator::Log>(8.0, Maybe<double> {2.0}), WithinAbs(3.0, 1e-12));
    CHECK_THAT(ExecuteOperation<Operator::Log>(std::numbers::e, Maybe<double> {}), WithinAbs(1.0, 1e-12));
    CHECK_THAT(ExecuteOperation<Operator::Exp>(1.0), WithinAbs(std::numbers::e, 1e-12));
  }

  TEST_CASE("Executor(min, max and clamp)", "[CSS][Calc][Executor]")
  {
    const double nan = Limits<double>::quiet_NaN();

    CHECK(ExecuteOperation<Operator::Min>(List<double> {3.0, 1.0, 2.0}) == 1.0);
    CHECK(std::isnan(ExecuteOperation<Operator::Min>(List<double> {3.0, nan, 2.0})));
    CHECK(std::isnan(ExecuteOperation<Operator::Min>(List<double> {})));
    CHECK(ExecuteOperation<Operator::Max>(List<double> {3.0, 1.0, 2.0}) == 3.0);
    CHECK(ExecuteOperation<Operator::Max>(1.0f, 2.0f) == 2.0f);
    CHECK(std::isnan(ExecuteOperation<Operator::Max>(1.0, nan)));

    CHECK(ExecuteOperation<Operator::Clamp>(0.0, 5.0, 3.0) == 3.0);
    CHECK(ExecuteOperation<Operator::Clamp>(0.0, -5.0, 3.0) == 0.0);
    CHECK(std::isnan(ExecuteOperation<Operator::Clamp>(0.0, nan, 3.0)));

    using Bound = Variant<double, keywords::None>;
    CHECK(ExecuteOperation<Operator::Clamp>(Bound {keywords::None {}}, 5.0, Bound {keywords::None {}})
          == 5.0);
    CHECK(ExecuteOperation<Operator::Clamp>(Bound {keywords::None {}}, 5.0, Bound {3.0}) == 3.0);
    CHECK(ExecuteOperation<Operator::Clamp>(Bound {6.0}, 5.0, Bound {keywords::None {}}) == 6.0);
    CHECK(ExecuteOperation<Operator::Clamp>(Bound {0.0}, 5.0, Bound {4.0}) == 4.0);
  }

  TEST_CASE("Executor(rounding)", "[CSS][Calc][Executor]")
  {
    const double inf = Limits<double>::infinity();

    CHECK(ExecuteOperation<Operator::RoundNearest>(2.5, 1.0) == 3.0);
    CHECK(ExecuteOperation<Operator::RoundNearest>(-2.5, 1.0) == -2.0);
    CHECK(ExecuteOperation<Operator::RoundNearest>(7.0, Maybe<double> {5.0}) == 5.0);
    CHECK(ExecuteOperation<Operator::RoundNearest>(2.4, Maybe<double> {}) == 2.0);
    CHECK(ExecuteOperation<Operator::RoundNearest>(4.0, 2.0) == 4.0);
    CHECK(ExecuteOperation<Operator::RoundUp>(2.1, 1.0) == 3.0);
    CHECK(ExecuteOperation<Operator::RoundDown>(2.9, 1.0) == 2.0);
    CHECK(ExecuteOperation<Operator::RoundToZero>(-2.9, 1.0) == -2.0);
    CHECK(ExecuteOperation<Operator::RoundToZero>(2.9, 1.0) == 2.0);

    // Infinite rounding intervals.
    CHECK(ExecuteOperation<Operator::RoundUp>(1.0, inf) == inf);
    CHECK(std::signbit(ExecuteOperation<Operator::RoundUp>(-1.0, inf)));
    CHECK(ExecuteOperation<Operator::RoundDown>(-1.0, inf) == -inf);
    CHECK(ExecuteOperation<Operator::RoundDown>(1.0, inf) == 0.0);
    CHECK(std::signbit(ExecuteOperation<Operator::RoundNearest>(-1.0, inf)));
    CHECK(std::signbit(ExecuteOperation<Operator::RoundToZero>(-1.0, inf)));
  }

  TEST_CASE("Executor(mod and rem)", "[CSS][Calc][Executor]")
  {
    CHECK(ExecuteOperation<Operator::Mod>(-1.0, 3.0) == 2.0);
    CHECK(ExecuteOperation<Operator::Mod>(5.0, 3.0) == 2.0);
    CHECK(ExecuteOperation<Operator::Mod>(1.0, -3.0) == -2.0);
    CHECK(std::isnan(ExecuteOperation<Operator::Mod>(-1.0, Limits<double>::infinity())));
    CHECK(ExecuteOperation<Operator::Rem>(-1.0, 3.0) == -1.0);
    CHECK(std::isnan(ExecuteOperation<Operator::Rem>(1.0, 0.0)));
  }

  TEST_CASE("Executor(trigonometry)", "[CSS][Calc][Executor]")
  {
    CHECK_THAT(ExecuteOperation<Operator::Sin>(std::numbers::pi / 2.0), WithinAbs(1.0, 1e-12));
    CHECK_THAT(ExecuteOperation<Operator::Cos>(0.0), WithinAbs(1.0, 1e-12));
    CHECK(ExecuteOperation<Operator::Tan>(std::numbers::pi / 2.0) == Limits<double>::infinity());
    CHECK(ExecuteOperation<Operator::Tan>(-std::numbers::pi / 2.0) == -Limits<double>::infinity());
    CHECK_THAT(ExecuteOperation<Operator::Tan>(std::numbers::pi / 4.0), WithinAbs(1.0, 1e-12));
    CHECK_THAT(ExecuteOperation<Operator::Asin>(1.0), WithinAbs(90.0, 1e-12));
    CHECK_THAT(ExecuteOperation<Operator::Acos>(1.0), WithinAbs(0.0, 1e-12));
    CHECK_THAT(ExecuteOperation<Operator::Atan>(1.0), WithinAbs(45.0, 1e-12));
    CHECK_THAT(ExecuteOperation<Operator::Atan2>(1.0, -1.0), WithinAbs(135.0, 1e-12));
  }

  TEST_CASE("Executor(progress and random)", "[CSS][Calc][Executor]")
  {
    CHECK(ExecuteOperation<Operator::Progress>(5.0, 0.0, 10.0) == 0.5);
    CHECK(ExecuteOperation<Operator::Progress>(20.0, 0.0, 10.0) == 1.0);
    CHECK(ExecuteOperation<Operator::Progress>(-20.0, 0.0, 10.0) == 0.0);

    const double nan = Limits<double>::quiet_NaN();
    CHECK(ExecuteOperation<Operator::Random>(0.5, 0.0, 10.0, Maybe<double> {}) == 5.0);
    CHECK(std::isnan(ExecuteOperation<Operator::Random>(0.5, nan, 10.0, Maybe<double> {})));
    CHECK(ExecuteOperation<Operator::Random>(0.5, 10.0, 0.0, Maybe<double> {}) == 10.0);
    CHECK(ExecuteOperation<Operator::Random>(0.5, 0.0, 10.0, Maybe<double> {-1.0}) == 5.0);
    CHECK(std::isnan(ExecuteOperation<Operator::Random>(0.5, 0.0, 10.0, Maybe<double> {nan})));

    // Stepped: the options are 0, 5 and 10.
    CHECK(ExecuteOperation<Operator::Random>(0.5, 0.0, 10.0, Maybe<double> {5.0}) == 5.0);
    CHECK(ExecuteOperation<Operator::Random>(0.99, 0.0, 10.0, Maybe<double> {5.0}) == 10.0);
    CHECK(ExecuteOperation<Operator::Random>(0.0, 0.0, 10.0, Maybe<double> {5.0}) == 0.0);
  }
}
