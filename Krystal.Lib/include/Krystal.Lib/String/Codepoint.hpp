#pragma once

#include "Krystal.Lib/StronglyTypedValue.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys
{
  /// @brief Unicode codepoint.
  struct Codepoint : public StronglyTypedNumber<Codepoint, uint32>
  {
    using StronglyTypedNumber<Codepoint, uint32>::StronglyTypedNumber;

    constexpr explicit Codepoint(char value) noexcept : StronglyTypedNumber<Codepoint, uint32>(value)
    {
    }
  };
}