#pragma once

#include "Krystal.Booey/CSS/Values/Easing/CubicBezierEasingFunction.hpp"
#include "Krystal.Booey/CSS/Values/Easing/LinearEasingFunction.hpp"
#include "Krystal.Booey/CSS/Values/Easing/SpringEasingFunction.hpp"
#include "Krystal.Booey/CSS/Values/Easing/StepsEasingFunction.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::css
{
  // `EasingFunction` uses a wrapping a struct to allow forward declaration.
  struct EasingFunction
  {
    Variant<
      // <linear()>
      keywords::Linear, // Equivalent to linear(0, 1)
      LinearEasingFunction,
      // <cubic-bezier()>
      keywords::Ease,      // Equivalent to cubic-bezier(0.25, 0.1, 0.25, 1)
      keywords::EaseIn,    // Equivalent to cubic-bezier(0.42, 0, 1, 1)
      keywords::EaseOut,   // Equivalent to cubic-bezier(0, 0, 0.58, 1)
      keywords::EaseInOut, // Equivalent to cubic-bezier(0.42, 0, 0.58, 1)
      CubicBezierEasingFunction,
      // <steps()>
      keywords::StepStart, // Equivalent to steps(1, start)
      keywords::StepEnd,   // Equivalent to steps(1, end)
      StepsEasingFunction,
      // <spring()>
      SpringEasingFunction>
      Value;

    template <typename... F>
    KRYS_NODISCARD constexpr decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(Value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD constexpr bool operator==(const EasingFunction &) const noexcept = default;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::css::EasingFunction)
