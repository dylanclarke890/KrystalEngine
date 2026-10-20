#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys
{
  enum class Base64DecodeOption : uint8
  {
    /// @brief Padding (`=`) must make the input a multiple of four characters, and be at most two.
    ValidatePadding,
    /// @brief ASCII whitespace in the input is skipped.
    IgnoreWhitespace,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::Base64DecodeOption, 2uz);

namespace krys
{
  /// @brief Decodes base64 (the standard alphabet, `+` and `/`); null if @p input isn't valid base64.
  /// @note Valid input is a multiple of four characters, less one or two at the end, or padded with `=`.
  KRYS_NODISCARD Maybe<List<uint8>> Base64Decode(Span<const uint8> input,
                                                 EnumSet<Base64DecodeOption> options = {}) noexcept;
}
