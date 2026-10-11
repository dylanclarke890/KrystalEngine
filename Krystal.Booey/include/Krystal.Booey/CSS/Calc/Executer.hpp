#pragma once

#include "Krystal.Booey/CSS/Calc/Operator.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include <numbers>
#include <numeric>
#include <ranges>

/// @summary Contains maths operations used by css::calc::Tree and style::calc::Tree.
namespace krys::boo::css::calc
{
  template <typename Range>
  concept FloatingPointRange = requires(Range range) {
    { *range.begin() } -> FloatingPoint;
    { *range.end() } -> FloatingPoint;
  };

  template <auto>
  struct OperatorExecutor;

  template <auto Op, typename... Args>
  KRYS_NODISCARD auto ExecuteOperation(Args &&...args) noexcept
  {
    return OperatorExecutor<Op>()(std::forward<Args>(args)...);
  }

  // Helper for rounding functions.
  KRYS_NODISCARD inline Pair<double, double> GetNearestMultiples(double a, double b) noexcept
  {
    if (!std::fmod(a, b))
    {
      return {a, a};
    }

    double lower = std::floor(a / std::abs(b)) * std::abs(b);
    double upper = lower + std::abs(b);

    return {lower, upper};
  }

  template <>
  struct OperatorExecutor<Operator::Sum>
  {
    template <typename Range>
    KRYS_NODISCARD double operator()(Range &&range) const noexcept
    {
      double sum = 0;
      for (double value : range)
      {
        sum += value;
      }

      return sum;
    }

    template <typename T, std::invocable<const T &> Functor>
    KRYS_NODISCARD double operator()(const SmallList<T> &range, Functor &&functor) const noexcept
    {
      return ExecuteOperation<Operator::Sum>(range | std::views::transform(std::forward<Functor>(functor)));
    }

    KRYS_NODISCARD double operator()(double a, double b) const noexcept
    {
      return a + b;
    }
  };

  template <>
  struct OperatorExecutor<Operator::Negate>
  {
    KRYS_NODISCARD double operator()(double a) const noexcept
    {
      return -a;
    }
  };

  template <>
  struct OperatorExecutor<Operator::Product>
  {
    template <typename Range>
    KRYS_NODISCARD double operator()(Range &&range) const noexcept
    {
      double product = 1;
      for (double value : range)
      {
        product *= value;
      }

      return product;
    }

    template <typename T, std::invocable<const T &> Functor>
    KRYS_NODISCARD double operator()(const SmallList<T> &range, Functor &&functor) const noexcept
    {
      return ExecuteOperation<Operator::Product>(range
                                                 | std::views::transform(std::forward<Functor>(functor)));
    }

    KRYS_NODISCARD double operator()(double a, double b) const noexcept
    {
      return a * b;
    }
  };

  template <>
  struct OperatorExecutor<Operator::Invert>
  {
    KRYS_NODISCARD double operator()(double a) const noexcept
    {
      return 1.0 / a;
    }
  };

  template <>
  struct OperatorExecutor<Operator::Min>
  {
    // NOTE: FloatingPoint and related concepts can be used for `min`, as there is no precision loss
    // from the operation staying in a lower precision.

    template <FloatingPointRange R>
    KRYS_NODISCARD auto operator()(R &&range) const noexcept
    {
      if (range.empty())
      {
        return Limits<std::ranges::range_value_t<R>>::quiet_NaN();
      }

      auto &&it = range.begin();
      auto &&end = range.end();

      auto minimum = *it;
      for (++it; it != end; ++it)
      {
        auto value = *it;
        if (std::isnan(value))
        {
          return value;
        }

        minimum = std::min(minimum, value);
      }

      return minimum;
    }

    template <FloatingPoint T>
    KRYS_NODISCARD T operator()(T val, T min) const noexcept
    {
      if (std::isnan(val))
      {
        return val;
      }

      if (std::isnan(min))
      {
        return min;
      }

      return std::min(val, min);
    }

    template <typename T, std::invocable<const T &> Functor>
    KRYS_NODISCARD double operator()(const SmallList<T> &range, Functor &&functor) const noexcept
    {
      return ExecuteOperation<Operator::Min>(range | std::views::transform(std::forward<Functor>(functor)));
    }
  };

  template <>
  struct OperatorExecutor<Operator::Max>
  {
    // NOTE: FloatingPoint and related concepts can be used for `max`, as there is no precision loss
    // from the operation staying in a lower precision.

    template <FloatingPointRange R>
    KRYS_NODISCARD auto operator()(R &&range) const noexcept
    {
      if (range.empty())
      {
        return Limits<std::ranges::range_value_t<R>>::quiet_NaN();
      }

      auto &&it = range.begin();
      auto &&end = range.end();

      auto maximum = *it;
      for (++it; it != end; ++it)
      {
        auto value = *it;
        if (std::isnan(value))
        {
          return value;
        }

        maximum = std::max(maximum, value);
      }

      return maximum;
    }

    template <FloatingPoint T>
    KRYS_NODISCARD T operator()(T val, T max) const noexcept
    {
      if (std::isnan(val))
        return val;
      if (std::isnan(max))
        return max;
      return std::max(val, max);
    }

    template <typename T, std::invocable<const T &> Functor>
    KRYS_NODISCARD double operator()(const SmallList<T> &range, Functor &&functor) const noexcept
    {
      return ExecuteOperation<Operator::Max>(range | std::views::transform(std::forward<Functor>(functor)));
    }
  };

  template <>
  struct OperatorExecutor<Operator::Clamp>
  {
    // NOTE: FloatingPoint and related concepts can be used for `clamp`, as there is no precision loss
    // from the operation staying in a lower precision.

    template <FloatingPoint T>
    KRYS_NODISCARD T operator()(T min, T val, T max) const noexcept
    {
      if (std::isnan(min) || std::isnan(val) || std::isnan(max))
      {
        return Limits<T>::quiet_NaN();
      }

      return std::max(min, std::min(val, max));
    }

    template <FloatingPoint T>
    KRYS_NODISCARD T operator()(Variant<T, keywords::None> min, T val,
                                Variant<T, keywords::None> max) const noexcept
    {
      bool minIsNone = std::holds_alternative<keywords::None>(min);
      bool maxIsNone = std::holds_alternative<keywords::None>(max);

      // - clamp(none, VAL, none) is equivalent to just calc(VAL).
      if (minIsNone && maxIsNone)
      {
        return val;
      }

      // - clamp(none, VAL, MAX) is equivalent to min(VAL, MAX)
      if (minIsNone)
      {
        return ExecuteOperation<Operator::Min>(val, std::get<T>(max));
      }

      // - clamp(MIN, VAL, none) is equivalent to max(MIN, VAL)
      if (maxIsNone)
      {
        return ExecuteOperation<Operator::Max>(std::get<T>(min), val);
      }

      return ExecuteOperation<Operator::Clamp>(std::get<T>(min), val, std::get<T>(max));
    }
  };

  template <>
  struct OperatorExecutor<Operator::RoundNearest>
  {
    KRYS_NODISCARD double operator()(double valueToRound, double roundingInterval) const noexcept
    {
      if (!std::isinf(valueToRound) && std::isinf(roundingInterval))
      {
        return std::signbit(valueToRound) ? -0.0 : +0.0;
      }

      auto [lower, upper] = GetNearestMultiples(valueToRound, roundingInterval);
      return std::abs(upper - valueToRound) <= std::abs(roundingInterval) / 2 ? upper : lower;
    }

    KRYS_NODISCARD double operator()(double valueToRound, Maybe<double> roundingInterval) const noexcept
    {
      return ExecuteOperation<Operator::RoundNearest>(valueToRound, roundingInterval.value_or(1.0));
    }
  };

  template <>
  struct OperatorExecutor<Operator::RoundUp>
  {
    KRYS_NODISCARD double operator()(double valueToRound, double roundingInterval) const noexcept
    {
      if (!std::isinf(valueToRound) && std::isinf(roundingInterval))
      {
        if (!valueToRound)
        {
          return valueToRound;
        }

        return std::signbit(valueToRound) ? -0.0 : Limits<double>::infinity();
      }

      return GetNearestMultiples(valueToRound, roundingInterval).second;
    }

    KRYS_NODISCARD double operator()(double valueToRound, Maybe<double> roundingInterval) const noexcept
    {
      return ExecuteOperation<Operator::RoundUp>(valueToRound, roundingInterval.value_or(1.0));
    }
  };

  template <>
  struct OperatorExecutor<Operator::RoundDown>
  {
    KRYS_NODISCARD double operator()(double valueToRound, double roundingInterval) const noexcept
    {
      if (!std::isinf(valueToRound) && std::isinf(roundingInterval))
      {
        if (!valueToRound)
          return valueToRound;
        return std::signbit(valueToRound) ? -Limits<double>::infinity() : +0.0;
      }
      return GetNearestMultiples(valueToRound, roundingInterval).first;
    }

    KRYS_NODISCARD double operator()(double valueToRound, Maybe<double> roundingInterval) const noexcept
    {
      return ExecuteOperation<Operator::RoundDown>(valueToRound, roundingInterval.value_or(1.0));
    }
  };

  template <>
  struct OperatorExecutor<Operator::RoundToZero>
  {
    KRYS_NODISCARD double operator()(double valueToRound, double roundingInterval) const noexcept
    {
      if (!std::isinf(valueToRound) && std::isinf(roundingInterval))
      {
        return std::signbit(valueToRound) ? -0.0 : +0.0;
      }

      auto [lower, upper] = GetNearestMultiples(valueToRound, roundingInterval);
      return std::abs(upper) < std::abs(lower) ? upper : lower;
    }

    KRYS_NODISCARD double operator()(double valueToRound, Maybe<double> roundingInterval) const noexcept
    {
      return ExecuteOperation<Operator::RoundToZero>(valueToRound, roundingInterval.value_or(1.0));
    }
  };

  template <>
  struct OperatorExecutor<Operator::Mod>
  {
    KRYS_NODISCARD double operator()(double a, double b) const noexcept
    {
      // In mod(A, B) only, if B is infinite and A has opposite sign to B
      // (including an oppositely-signed zero), the result is NaN.
      // https://drafts.csswg.org/css-values/#round-infinities
      if (std::isinf(b) && std::signbit(a) != std::signbit(b))
      {
        return Limits<double>::quiet_NaN();
      }

      auto result = std::fmod(a, b);
      // If the result is on opposite side of zero from B, put it between 0 and B.
      // https://drafts.csswg.org/css-values/#round-func
      if (std::signbit(result) != std::signbit(b))
      {
        result += b;
      }

      return result;
    }
  };

  template <>
  struct OperatorExecutor<Operator::Rem>
  {
    KRYS_NODISCARD double operator()(double a, double b) const noexcept
    {
      if (!b)
      {
        return Limits<double>::quiet_NaN();
      }

      return std::fmod(a, b);
    }
  };

  template <>
  struct OperatorExecutor<Operator::Sin>
  {
    KRYS_NODISCARD double operator()(double a) const noexcept
    {
      return std::sin(a);
    }
  };

  template <>
  struct OperatorExecutor<Operator::Cos>
  {
    KRYS_NODISCARD double operator()(double a) const noexcept
    {
      return std::cos(a);
    }
  };

  template <>
  struct OperatorExecutor<Operator::Tan>
  {
    constexpr static const double piOverTwoDouble = std::numbers::pi / 2.0;

    KRYS_NODISCARD double operator()(double a) const noexcept
    {
      double x = std::fmod(a, std::numbers::pi * 2);

      // std::fmod can return negative values.
      x = x < 0 ? std::numbers::pi * 2 + x : x;

      krys_debug_assert(!(x < 0));
      krys_debug_assert(!(x > std::numbers::pi * 2));

      if (x == piOverTwoDouble)
      {
        return Limits<double>::infinity();
      }

      if (x == 3 * piOverTwoDouble)
      {
        return -Limits<double>::infinity();
      }

      return std::tan(x);
    }
  };

  template <>
  struct OperatorExecutor<Operator::Asin>
  {
    KRYS_NODISCARD double operator()(double a) const noexcept
    {
      return rad2deg(std::asin(a));
    }
  };

  template <>
  struct OperatorExecutor<Operator::Acos>
  {
    KRYS_NODISCARD double operator()(double a) const noexcept
    {
      return rad2deg(std::acos(a));
    }
  };

  template <>
  struct OperatorExecutor<Operator::Atan>
  {
    KRYS_NODISCARD double operator()(double a) const noexcept
    {
      return rad2deg(std::atan(a));
    }
  };

  template <>
  struct OperatorExecutor<Operator::Atan2>
  {
    KRYS_NODISCARD double operator()(double a, double b) const noexcept
    {
      return rad2deg(atan2(a, b));
    }
  };

  template <>
  struct OperatorExecutor<Operator::Pow>
  {
    KRYS_NODISCARD double operator()(double a, double b) const noexcept
    {
      return std::pow(a, b);
    }
  };

  template <>
  struct OperatorExecutor<Operator::Sqrt>
  {
    KRYS_NODISCARD double operator()(double a) const noexcept
    {
      return std::sqrt(a);
    }
  };

  template <>
  struct OperatorExecutor<Operator::Hypot>
  {
    template <typename Range>
    KRYS_NODISCARD double operator()(Range &&range) const noexcept
    {
      if (range.empty())
      {
        return Limits<double>::quiet_NaN();
      }

      if (range.size() == 1)
      {
        return std::abs(*range.begin());
      }

      double sum = 0;
      for (double value : range)
      {
        sum += (value * value);
      }

      return std::sqrt(sum);
    }

    template <typename T, std::invocable<const T &> Functor>
    double operator()(const SmallList<T> &range, Functor &&functor) const noexcept
    {
      return ExecuteOperation<Operator::Hypot>(range | std::views::transform(std::forward<Functor>(functor)));
    }
  };

  template <>
  struct OperatorExecutor<Operator::Log>
  {
    KRYS_NODISCARD double operator()(double a) const noexcept
    {
      return std::log(a);
    }

    KRYS_NODISCARD double operator()(double a, double b) const noexcept
    {
      return std::log(a) / std::log(b);
    }

    KRYS_NODISCARD double operator()(double a, Maybe<double> b) const noexcept
    {
      if (b)
      {
        return ExecuteOperation<Operator::Log>(a, *b);
      }

      return ExecuteOperation<Operator::Log>(a);
    }
  };

  template <>
  struct OperatorExecutor<Operator::Exp>
  {
    KRYS_NODISCARD double operator()(double a) const noexcept
    {
      return std::exp(a);
    }
  };

  template <>
  struct OperatorExecutor<Operator::Abs>
  {
    KRYS_NODISCARD double operator()(double a) const noexcept
    {
      return std::abs(a);
    }
  };

  template <>
  struct OperatorExecutor<Operator::Sign>
  {
    KRYS_NODISCARD double operator()(double a) const noexcept
    {
      if (a > 0)
      {
        return 1;
      }

      if (a < 0)
      {
        return -1;
      }

      return a;
    }
  };

  template <>
  struct OperatorExecutor<Operator::Progress>
  {
    KRYS_NODISCARD double operator()(double progress, double from, double to) const noexcept
    {
      // (progress value - start value) / (end value - start value)
      return ExecuteOperation<Operator::Clamp>(0.0, (progress - from) / (to - from), 1.0);
    }
  };

  template <>
  struct OperatorExecutor<Operator::Random>
  {
    KRYS_NODISCARD double operator()(double randomBaseValue, double min, double max,
                                     Maybe<double> step) const noexcept
    {
      if (std::isnan(min) || std::isnan(max))
      {
        return Limits<double>::quiet_NaN();
      }

      if (std::isinf(min))
      {
        return min;
      }

      // If the max value is less than the min value, it behaves as if it’s equal to the min value.
      if (max < min)
      {
        max = min;
      }

      auto range = max - min;
      if (std::isinf(range))
      {
        return Limits<double>::quiet_NaN();
      }

      if (!step)
      {
        return min + randomBaseValue * range;
      }

      if (std::isnan(*step))
      {
        return Limits<double>::quiet_NaN();
      }

      if (*step <= 0)
      {
        return min + randomBaseValue * range;
      }

      // Let epsilon be step / 1000, or the smallest representable value greater than zero in the numeric
      // type being used if epsilon would round to zero.
      auto epsilon = *step / 1000.0;

      // Let N be the largest integer such that min + N * step is less than or equal to max
      auto N = std::floor(range / *step);
      if (std::isinf(N))
      {
        return min + randomBaseValue * range;
      }

      // If N produces a value that is not within epsilon of max, but N+1 would produce a value within
      // epsilon of max, set N to N+1.
      auto distanceToMax = max - (min + (N * *step));
      if (std::abs(distanceToMax) > epsilon)
      {
        auto distanceToMaxPlus1 = max - (min + ((N + 1) * *step));
        if (std::abs(distanceToMaxPlus1) < epsilon)
        {
          N = N + 1;
        }
      }

      // Let step index be a random integer less than N+1, given R.
      auto stepIndex = OperatorExecutor<Operator::RoundDown> {}(randomBaseValue * (N + 1.0), 1.0);

      // Let value be min + step index * step.
      auto value = min + stepIndex * *step;

      // If step index is N and value is within epsilon of max, return max
      if (stepIndex == N && std::abs(max - value) < epsilon)
      {
        return max;
      }

      // Otherwise, return value.
      return value;
    }
  };
}
