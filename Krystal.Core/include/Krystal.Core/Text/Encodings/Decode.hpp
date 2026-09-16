#pragma once

#include "Krystal.Core/Text/Encodings/Base.hpp"

namespace krys::text
{
  /// @brief The unhandled error that occurred during a Decode operation, or DecodeError::None if the
  /// operation was successful.
  enum class DecodeError : uint8
  {
    /// @brief No error occurred.
    None = 0,

    /// @brief The input was empty.
    EmptyInput,

    /// @brief Not enough code units were provided to complete a valid sequence.
    IncompleteSequence,

    /// @brief Invalid code unit sequence for the provided encoding.
    InvalidSequence,
  };

  /// @brief The result of a decode operation.
  template <typename TOutput>
  struct DecodeResult
  {
    /// @brief The decoded output.
    TOutput Output {};

    /// @brief The number of code units consumed from the input.
    size_t CodeUnitsConsumed {0uz};

    /// @brief The number of code points produced from the input.
    size_t CodePointsProduced {0uz};

    /// @brief Whether a byte order mark (BOM) was detected at the start of the input when decoding, if the
    /// encoding supports it. This is only applicable to encodings that have a BOM, such as UTF-8, UTF-16 and
    /// UTF-32.
    bool SawByteOrderMark {false};

    /// @brief The error that occurred, or DecodeError::None if the operation was successful.
    DecodeError Error = DecodeError::None;
  };

  /// @brief The allocation/error handling policies to use when decoding input.
  struct DecodePolicy
  {
    /// @brief The action to take when an invalid sequence is encountered (e.g., a code unit sequence that is
    /// not valid for the target encoding).
    EncodingErrorAction OnInvalidSequence = EncodingErrorAction::Stop;

    /// @brief The action to take when an incomplete sequence is encountered (e.g., the input ends before a
    /// complete code unit sequence is read). Decoding will always stop when this error is encountered (the
    /// input has ended), but with slightly differing behavior:
    /// - `Stop` - reports the error and leaves DecodeResult::CodeUnitsConsumed at the start of the invalid
    /// sequence.
    /// - `Skip` - ignores the error, and updates DecodeResult::CodeUnitsConsumed to include the consumed
    /// incomplete sequence before returning,
    /// - `Replace` - replaces the incomplete sequence with the replacement character before returning and
    /// updates DecodeResult::CodeUnitsConsumed to include the consumed incomplete sequence.
    EncodingErrorAction OnIncompleteSequence = EncodingErrorAction::Stop;

    /// @brief Whether to check for a byte order mark (BOM) at the start of the input when decoding, if the
    /// encoding supports it. This is only applicable to encodings that have a BOM, such as UTF-8, UTF-16
    /// and UTF-32. If the input does not start with a BOM, decoding will continue as normal.
    /// `DecodeResult::SawByteOrderMark` will be set to true if a BOM was detected and consumed from the
    /// input.
    CheckForBOM CheckByteOrderMark = CheckForBOM(false);
  };

  /// @brief The default decoding policy, which handles all errors by replacing the invalid code unit
  /// sequences with the appropriate replacement code point.
  constexpr DecodePolicy DefaultDecodePolicy {.OnInvalidSequence = EncodingErrorAction::Replace,
                                              .OnIncompleteSequence = EncodingErrorAction::Replace,
                                              .CheckByteOrderMark = CheckForBOM(false)};

  /// @brief Decodes the input code units into code points using the specified encoding and policy.
  template <Encoding TEncoding, DecodePolicy Policy = DefaultDecodePolicy, typename TOutput = utf32_string,
            CodeUnitInputRange<TEncoding> TInput>
  KRYS_NODISCARD constexpr DecodeResult<TOutput> Decode(TInput &&input) noexcept
  {
    DecodeResult<TOutput> result {};

    if (input.empty())
    {
      result.Error = DecodeError::EmptyInput;
      return result;
    }

    auto current = std::ranges::cbegin(input);
    auto last = std::ranges::cend(input);

    if constexpr (Policy.CheckByteOrderMark && HasByteOrderMark<TEncoding>)
    {
      if (std::ranges::equal(TEncoding::BOM, std::ranges::subrange(current, last)))
      {
        std::ranges::advance(current, TEncoding::BOM.size());
        result.CodeUnitsConsumed += TEncoding::BOM.size();
        result.SawByteOrderMark = true;
      }

      if (current == last)
      {
        result.Error = DecodeError::EmptyInput;
        return result;
      }
    }

    while (current != last)
    {
      auto [output, codeUnitsConsumed, codePointsProduced, decodeError] = TEncoding::DecodeOne(current, last);
      std::ranges::advance(current, codeUnitsConsumed);

      switch (decodeError)
      {
        case DecodeOneError::None:
        {
          break;
        }
        case DecodeOneError::IncompleteSequence:
        {
          if constexpr (Policy.OnIncompleteSequence == EncodingErrorAction::Stop)
          {
            result.Error = DecodeError::IncompleteSequence;
            return result;
          }
          else if constexpr (Policy.OnIncompleteSequence == EncodingErrorAction::Skip)
          {
            result.CodeUnitsConsumed += codeUnitsConsumed;
            return result;
          }
          else if constexpr (Policy.OnIncompleteSequence == EncodingErrorAction::Replace)
          {
            break;
          }
        }
        case DecodeOneError::InvalidSequence:
        {
          if constexpr (Policy.OnInvalidSequence == EncodingErrorAction::Stop)
          {
            result.Error = DecodeError::InvalidSequence;
            return result;
          }
          else if constexpr (Policy.OnInvalidSequence == EncodingErrorAction::Skip)
          {
            result.CodeUnitsConsumed += codeUnitsConsumed;
            continue;
          }
          else if constexpr (Policy.OnInvalidSequence == EncodingErrorAction::Replace)
          {
            break;
          }
        }
        default:
        {
          krys_unreachable();
        }
      }

      result.CodeUnitsConsumed += codeUnitsConsumed;
      result.CodePointsProduced += codePointsProduced;
      if constexpr (TEncoding::MaxDecodeCodePoints == 1uz)
      {
        WriteToOutput(result.Output, output);
      }
      else
      {
        WriteToOutput(result.Output, Span<char32>(output.data(), codePointsProduced));
      }
    }

    return result;
  }
}