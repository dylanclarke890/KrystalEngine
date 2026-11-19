#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::i18n
{
  /// @brief Which fields of a date or time `Locale::FormatDateTime` shows.
  enum class DateTimeKind : uint8
  {
    Date,
    Month,
    Week,
    Time,
    DateTime,
  };

  /// @brief A date or time for `Locale::FormatDateTime`: the fields its kind shows, the others unused.
  struct DateTimeFields
  {
    DateTimeKind Kind {DateTimeKind::Date};
    int32 Year {0};
    /// @brief The month, from 0 for January.
    int32 Month {0};
    /// @brief The day of the month, from 1.
    int32 Day {1};
    /// @brief The ISO week of the year, from 1.
    int32 Week {1};
    int32 Hour {0};
    int32 Minute {0};
    int32 Second {0};
    int32 Millisecond {0};
  };
}
