#pragma once

#include "Krystal.Booey/Animation/FillMode.hpp"
#include "Krystal.Booey/Animation/PlaybackDirection.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo
{
  /// @brief The timing properties to change on an animation effect: those present are changed.
  /// @see https://drafts.csswg.org/web-animations-1/#dictdef-optionaleffecttiming
  struct OptionalEffectTiming
  {
    /// @brief In milliseconds, or the string `auto`.
    Maybe<Variant<double, dom::DOMString>> Duration;
    Maybe<double> Iterations;
    /// @brief In milliseconds.
    Maybe<double> Delay;
    /// @brief In milliseconds.
    Maybe<double> EndDelay;
    Maybe<double> IterationStart;
    Maybe<dom::DOMString> Easing;
    Maybe<FillMode> Fill;
    Maybe<PlaybackDirection> Direction;
  };
}
