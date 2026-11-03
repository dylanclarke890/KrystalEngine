#pragma once

#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::html
{
  /// @brief The validity states a form-associated custom element sets through its internals.
  /// @see https://html.spec.whatwg.org/#validitystateflags
  struct ValidityStateFlags
  {
    bool ValueMissing : 1 {false};
    bool TypeMismatch : 1 {false};
    bool PatternMismatch : 1 {false};
    bool TooLong : 1 {false};
    bool TooShort : 1 {false};
    bool RangeUnderflow : 1 {false};
    bool RangeOverflow : 1 {false};
    bool StepMismatch : 1 {false};
    bool BadInput : 1 {false};
    bool CustomError : 1 {false};

    KRYS_NODISCARD constexpr bool IsValid() const noexcept
    {
      return !ValueMissing && !TypeMismatch && !PatternMismatch && !TooLong && !TooShort && !RangeUnderflow
             && !RangeOverflow && !StepMismatch && !BadInput && !CustomError;
    }
  };
}
