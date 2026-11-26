#pragma once

#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Web/Animation/Timing/FillMode.hpp"
#include "Krystal.Web/Animation/Timing/PlaybackDirection.hpp"
#include "Krystal.Web/Dom/Types/DOMString.hpp"

namespace krys::web
{
  /// @brief The timing properties to change on an animation effect: those present are changed.
  /// @see https://drafts.csswg.org/web-animations-1/#dictdef-optionaleffecttiming
  struct OptionalEffectTiming
  {
    /// @brief In milliseconds, or the string `auto`.
    Maybe<Variant<double, DOMString>> Duration;
    Maybe<double> Iterations;
    /// @brief In milliseconds.
    Maybe<double> Delay;
    /// @brief In milliseconds.
    Maybe<double> EndDelay;
    Maybe<double> IterationStart;
    Maybe<DOMString> Easing;
    Maybe<FillMode> Fill;
    Maybe<PlaybackDirection> Direction;
  };
}
