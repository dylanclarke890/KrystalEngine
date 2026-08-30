#pragma once

#include "Krystal.HTML/CSS/Values/Easing/CubicBezierEasingFunction.hpp"
#include "Krystal.HTML/CSS/Values/Easing/LinearEasingFunction.hpp"
#include "Krystal.HTML/CSS/Values/Easing/SpringEasingFunction.hpp"
#include "Krystal.HTML/CSS/Values/Easing/StepsEasingFunction.hpp"

namespace Krys::HTML
{
  // `EasingFunction` uses a wrapping a struct to allow forward declaration.
  struct EasingFunction
  {
    Variant<
      // <linear()>
      Keywords::Linear, // Equivalent to linear(0, 1)
      LinearEasingFunction,

      // <cubic-bezier()>
      Keywords::Ease,      // Equivalent to cubic-bezier(0.25, 0.1, 0.25, 1)
      Keywords::EaseIn,    // Equivalent to cubic-bezier(0.42, 0, 1, 1)
      Keywords::EaseOut,   // Equivalent to cubic-bezier(0, 0, 0.58, 1)
      Keywords::EaseInOut, // Equivalent to cubic-bezier(0.42, 0, 0.58, 1)
      CubicBezierEasingFunction,

      // <steps()>
      Keywords::StepStart, // Equivalent to steps(1, start)
      Keywords::StepEnd,   // Equivalent to steps(1, end)
      StepsEasingFunction,

      // <spring()>
      SpringEasingFunction>
      value;

    template <typename... F>
    KRYS_NODISCARD constexpr decltype(auto) switchOn(F &&...f) const noexcept
    {
      return Krys::SwitchOn(value, std::forward<F>(f)...);
    }

    constexpr bool operator==(const EasingFunction &) const noexcept = default;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(Krys::HTML::EasingFunction)
