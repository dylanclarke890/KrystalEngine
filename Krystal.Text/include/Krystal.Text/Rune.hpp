#pragma once

#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/StronglyTypedValue.hpp"

namespace Krys
{
  /// @brief Represents a valid Unicode codepoint (rune) in the range U+0000 to U+10FFFF.
  struct Rune : public StronglyTypedNumber<Rune, uint32>
  {
    using StronglyTypedNumber<Rune, uint32>::StronglyTypedNumber;

    explicit constexpr Rune(byte value) noexcept
        : StronglyTypedNumber<Rune, uint32>(static_cast<uint32>(value))
    {
    }

    // TODO: Add validation to ensure only valid codepoints are created.
  };
}