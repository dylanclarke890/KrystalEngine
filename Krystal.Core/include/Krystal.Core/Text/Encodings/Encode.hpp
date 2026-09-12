#pragma once

#include "Krystal.Core/Text/Encodings/EncodingRegistry.hpp"

namespace krys::text
{
  /// @brief The unhandled error that occurred during an Encode operation, or EncodeError::None if the
  /// operation was successful.
  enum class EncodeError : uint8
  {
    /// @brief No error occurred.
    None = 0,

    /// @brief The input was empty.
    EmptyInput,

    /// @brief Valid Unicode code point but not representable in the target encoding.
    UnmappableCodePoint,
  };

  /// @brief The result of an encode operation.
  template <typename TOutput>
  struct EncodeResult
  {
    /// @brief The encoded output.
    TOutput Output {};

    /// @brief The number of code points consumed from the input.
    size_t CodePointsConsumed {0uz};

    /// @brief The number of code units produced in the output.
    size_t CodeUnitsProduced {0uz};

    /// @brief Whether a byte order mark (BOM) was appended to the output when encoding, if the encoding
    /// supports it. This is only applicable to encodings that have a BOM, such as UTF-8, UTF-16 and UTF-32.
    bool AppendedByteOrderMark {false};

    /// @brief The unhandled error that occurred, or EncodeError::None if the operation was successful.
    EncodeError Error = EncodeError::None;
  };

  /// @brief The allocation/error handling policies to use when encoding input.
  struct EncodePolicy
  {
    /// @brief The action to take when an unmappable code point is encountered (e.g., a valid Unicode code
    /// point that cannot be represented in the target encoding).
    EncodingErrorAction OnUnmappableCodePoint = EncodingErrorAction::Stop;

    /// @brief Whether to append a byte order mark (BOM) to the output when encoding, if the encoding supports
    /// it. This is only applicable to encodings that have a BOM, such as UTF-8, UTF-16, and UTF-32.
    AppendBOM AppendByteOrderMark = AppendBOM(false);
  };

  /// @brief The default encoding policy, which handles all errors by replacing the invalid code point with
  /// the appropriate replacement code unit.
  constexpr EncodePolicy DefaultEncodePolicy {.OnUnmappableCodePoint = EncodingErrorAction::Replace,
                                              .AppendByteOrderMark = AppendBOM(false)};

  /// @brief Encodes the input code points into code units using the specified encoding and policy.
  template <Encoding TEncoding, EncodePolicy Policy = DefaultEncodePolicy,
            typename TOutput = std::basic_string<typename TEncoding::code_unit>, CodePointInputRange TInput>
  KRYS_NODISCARD constexpr EncodeResult<TOutput> Encode(TInput &&input) noexcept
  {
    EncodeResult<TOutput> result {};

    if (input.empty())
    {
      result.Error = EncodeError::EmptyInput;
      return result;
    }

    if constexpr (Policy.AppendByteOrderMark && HasByteOrderMark<TEncoding>)
    {
      WriteToOutput(result.Output, TEncoding::BOM);
      result.CodeUnitsProduced += TEncoding::BOM.size();
      result.AppendedByteOrderMark = true;
    }

    for (char32 codePoint : input)
    {
      auto [output, codeUnitsProduced, encodeError] = TEncoding::EncodeOne(codePoint);

      switch (encodeError)
      {
        case EncodeOneError::None:
        {
          break;
        }
        case EncodeOneError::UnmappableCodePoint:
        {
          if constexpr (Policy.OnUnmappableCodePoint == EncodingErrorAction::Stop)
          {
            result.Error = EncodeError::UnmappableCodePoint;
            return result;
          }
          else if constexpr (Policy.OnUnmappableCodePoint == EncodingErrorAction::Skip)
          {
            result.CodePointsConsumed += 1uz;
            continue;
          }
          else if constexpr (Policy.OnUnmappableCodePoint == EncodingErrorAction::Replace)
          {
            break;
          }
        }
        default:
        {
          krys_unreachable();
        }
      }

      result.CodePointsConsumed += 1uz;
      result.CodeUnitsProduced += codeUnitsProduced;
      if constexpr (TEncoding::MaxEncodeCodeUnits == 1uz)
      {
        WriteToOutput(result.Output, output);
      }
      else
      {
        WriteToOutput(result.Output, Span<const typename TEncoding::code_unit>(output.data(), codeUnitsProduced));
      }
    }

    return result;
  }
}