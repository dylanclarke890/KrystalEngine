#pragma once

#include "Krystal.Booey/Page/ScrollBehavior.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include <cmath>

namespace krys::boo
{
  /// @see https://drafts.csswg.org/cssom-view/#dictdef-scrolloptions
  struct ScrollOptions
  {
    Maybe<ScrollBehavior> Behavior {ScrollBehavior::Auto};
  };

  /// @see https://drafts.csswg.org/cssom-view/#dictdef-scrolltooptions
  struct ScrollToOptions : ScrollOptions
  {
    Maybe<double> Left;
    Maybe<double> Top;

    ScrollToOptions() noexcept = default;
    ScrollToOptions(double x, double y) noexcept : Left(x), Top(y)
    {
    }
  };

  /// @brief @p value, with a non-finite value made 0, or @p fallbackValue if there's no value.
  /// @see https://drafts.csswg.org/cssom-view/#normalize-non-finite-values
  KRYS_NODISCARD inline double NormalizeNonFiniteValueOrFallBackTo(Maybe<double> value,
                                                                   double fallbackValue) noexcept
  {
    return value ? (std::isfinite(*value) ? *value : 0) : fallbackValue;
  }

  KRYS_NODISCARD inline ScrollToOptions
    NormalizeNonFiniteCoordinatesOrFallBackTo(const ScrollToOptions &value, double x, double y) noexcept
  {
    auto options = value;
    options.Left = NormalizeNonFiniteValueOrFallBackTo(value.Left, x);
    options.Top = NormalizeNonFiniteValueOrFallBackTo(value.Top, y);
    return options;
  }

  /// @see https://drafts.csswg.org/cssom-view/#enumdef-scrolllogicalposition
  enum class ScrollLogicalPosition : uint8
  {
    Start,
    Center,
    End,
    Nearest
  };

  /// @see https://drafts.csswg.org/cssom-view/#dictdef-scrollintoviewoptions
  struct ScrollIntoViewOptions : ScrollOptions
  {
    Maybe<ScrollLogicalPosition> BlockPosition {ScrollLogicalPosition::Start};
    Maybe<ScrollLogicalPosition> InlinePosition {ScrollLogicalPosition::Nearest};
  };
}
