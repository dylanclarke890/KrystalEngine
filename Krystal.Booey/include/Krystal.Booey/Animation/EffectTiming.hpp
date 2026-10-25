#pragma once

#include "Krystal.Booey/Animation/FillMode.hpp"
#include "Krystal.Booey/Animation/PlaybackDirection.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSNumericValue.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Core/Visitor.hpp"

namespace krys::boo
{
  using OptionalDoubleOrString = Maybe<Variant<double, dom::DOMString>>;
  using DoubleOrCSSNumericValueOrString = Variant<double, RefPtr<css::CSSNumericValue>, dom::DOMString>;

  /// @brief An animation effect's specified timing, as the Web Animations interfaces give and take it.
  /// @see https://drafts.csswg.org/web-animations-1/#dictdef-effecttiming
  struct EffectTiming
  {
    /// @brief In milliseconds, a numeric value (a percentage, for a progress-based timeline), or the string
    /// `auto`.
    DoubleOrCSSNumericValueOrString Duration {dom::DOMString(u8"auto")};
    /// @brief In milliseconds.
    double Delay {0};
    /// @brief In milliseconds.
    double EndDelay {0};
    double IterationStart {0};
    double Iterations {1};
    dom::DOMString Easing {u8"linear"};
    FillMode Fill {FillMode::Auto};
    PlaybackDirection Direction {PlaybackDirection::Normal};

    /// @brief The duration as a number or a string; null for a numeric value.
    KRYS_NODISCARD OptionalDoubleOrString DurationAsDoubleOrString() const noexcept
    {
      return krys::SwitchOn(
        Duration, [](double doubleDuration) -> OptionalDoubleOrString { return doubleDuration; },
        [](const dom::DOMString &stringDuration) -> OptionalDoubleOrString { return stringDuration; },
        [](const RefPtr<css::CSSNumericValue> &) -> OptionalDoubleOrString { return null; });
    }
  };
}
