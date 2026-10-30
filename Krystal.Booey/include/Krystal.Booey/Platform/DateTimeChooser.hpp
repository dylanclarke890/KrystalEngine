#pragma once

#include "Krystal.Core/RefCounted.hpp"

namespace krys::boo
{
  struct DateTimeChooserParameters;

  /// @brief A picker the host shows for a date input: a calendar.
  class DateTimeChooser : public RefCounted<DateTimeChooser>
  {
  public:
    virtual ~DateTimeChooser() noexcept = default;

    /// @brief Closes the chooser; it tells its client it ended.
    virtual void EndChooser() noexcept = 0;

    /// @brief Shows the chooser, or updates it if it's showing.
    virtual void ShowChooser(const DateTimeChooserParameters &parameters) noexcept = 0;
  };
}
