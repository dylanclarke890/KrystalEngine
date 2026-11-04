#pragma once

#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief Who a date chooser tells about the user's choice: a date input. It ends its chooser before it
  /// goes away.
  class DateTimeChooserClient
  {
  public:
    virtual ~DateTimeChooserClient() noexcept = default;

    /// @brief Called when the user picked a value.
    virtual void DidChooseValue(utf8_stringview value) noexcept = 0;

    /// @brief Called when the chooser has ended.
    virtual void DidEndChooser() noexcept = 0;
  };
}
