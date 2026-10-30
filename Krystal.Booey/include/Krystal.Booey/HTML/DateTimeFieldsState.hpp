#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::html
{
  enum class DateTimeMeridiem : bool
  {
    AM,
    PM
  };

  /// @brief The values of a date or time input's fields, each missing until its field has one.
  struct DateTimeFieldsState
  {
    Maybe<uint32> Year;
    Maybe<uint32> Month;
    Maybe<uint32> Week;
    Maybe<uint32> DayOfMonth;
    Maybe<uint32> Hour;
    Maybe<uint32> Minute;
    Maybe<uint32> Second;
    Maybe<uint32> Millisecond;
    Maybe<DateTimeMeridiem> Meridiem;

    KRYS_NODISCARD Maybe<uint32> Hour23() const noexcept
    {
      if (!Hour || !Meridiem)
      {
        return null;
      }
      return (*Hour % 12) + (*Meridiem == DateTimeMeridiem::PM ? 12 : 0);
    }
  };
}
