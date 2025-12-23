#pragma once

#include "Krystal.Lib/StronglyTypedValue.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::Text
{
  /// @brief Represents a Unicode scalar value.
  struct UnicodeScalar : public StronglyTypedNumber<UnicodeScalar, uint32>
  {
    using StronglyTypedNumber<UnicodeScalar, uint32>::StronglyTypedNumber;
  };
}