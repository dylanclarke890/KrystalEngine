#pragma once

#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Concepts.hpp"
#include "Krystal.Text/Decode/DecodeResult.hpp"
#include "Krystal.Text/Encode/EncodeResult.hpp"
#include "Krystal.Text/Encodings/EncodingTables/Big5-HKSCS.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/Unicode.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The Big5-HKSCS (Hong Kong Supplementary Character Set) encoding, standardized around 2001.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_big5_hkscs
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = {"Big5"_s};
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 5> Aliases = {
      "big5"_s, "big5-hkscs"_s, "cn-big5"_s, "csbig5"_s, "x-x-big5"_s};

    using code_unit = TCodeUnit;

    using code_point = TCodePoint;

    using state = EmptyState;

    constexpr static inline std::size_t MaxCodePoints = 2;

    constexpr static inline std::size_t MaxCodeUnits = 2;

    using is_decode_injective = std::true_type;

    constexpr Span<const code_unit, 1> ReplacementCodeUnits() const noexcept
    {
      return Span<const code_unit, 1>(::Krys::Text::Unicode::ASCIIReplacementAsArray<code_unit>.data(),
                                      ::Krys::Text::Unicode::ASCIIReplacementAsArray<code_unit>.size());
    }

    template <typename TInput, typename TOutput, typename TErrorHandler>
    constexpr static auto DecodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler,
                                    state &s) noexcept
    {
      using TSubInput = ::Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
      using TSubOutput = ::Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
      using TResult = ::Krys::Text::DecodeResult<TSubInput, TSubOutput, state>;

      auto inIt = ::Krys::Ranges::cbegin(input);
      auto inLast = ::Krys::Ranges::cend(input);

      if (inIt == inLast)
      {
        // we don't need more, so we can just say the emptiness is a-okay.
        return TResult(std::move(input), std::move(output), s, EncodingError::OK);
      }

      code_unit units[MaxCodeUnits] = {static_cast<code_unit>(*inIt)};
      uchar unit0 = static_cast<uchar>(units[0]);
      auto outIt = ::Krys::Ranges::begin(output);
      auto outLast = ::Krys::Ranges::end(output);

      constexpr bool CallErrorHandler = !IsIgnorableErrorHandler<TErrorHandler>;
      if (unit0 <= 0x7F)
      {
        // Top-Level case 0: it's an ASCII byte
        const code_point codePoint = static_cast<code_point>(unit0);
        if constexpr (CallErrorHandler)
        {
          if (outIt == outLast)
          {
            basic_big5_hkscs self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self,
              TResult(TSubInput(std::move(inIt), std::move(inLast)),
                      TSubOutput(std::move(outIt), std::move(outLast)), s,
                      EncodingError::InsufficientOutputSpace),
              Span<const code_unit, 0>(), Span<const code_point, 0>());
          }
        }
        *outIt = codePoint;
        ++inIt;
        ++outIt;
        return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                       TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
      }
      else if ((unit0 <= 0xFE && unit0 >= 0x81))
      {
        // Top-Level case 1: this is a double-byte sequence!
        if constexpr (CallErrorHandler)
        {
          if (inIt == inLast)
          {
            basic_big5_hkscs self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self, TResult(std::move(input), std::move(output), s, EncodingError::IncompleteSequence),
              Span<const code_unit, 1>(std::addressof(units[0]), 1), Span<const code_point, 0>());
          }
        }
        ++inIt;
        units[1] = static_cast<code_unit>(*inIt);
        uchar secondByte = static_cast<uchar>(units[1]);
        if ((secondByte <= 0x7E && secondByte >= 0x40) || (secondByte <= 0xFE && secondByte >= 0xA1))
        {
          const uchar secondByteOffset = secondByte < 0x7F ? 0x40 : 0x62;
          const std::size_t lookupIndex = ((unit0 - 0x81uz) * 157uz) + (secondByte - secondByteOffset);
          auto serializeDoubleBytes = [&](const std::size_t doubleCodePointValuesIndex)
          {
            if constexpr (CallErrorHandler)
            {
              if (outIt == outLast)
              {
                basic_big5_hkscs self {};
                return std::forward<TErrorHandler>(errorHandler)(
                  self,
                  TResult(TSubInput(std::move(inIt), std::move(inLast)),
                          TSubOutput(std::move(outIt), std::move(outLast)), s,
                          EncodingError::InsufficientOutputSpace),
                  Span<const code_unit, 1>(std::addressof(units[0]), 1), Span<const code_point, 0>());
              }
            }
            *outIt = DoubleCodePointValues[doubleCodePointValuesIndex][0];
            ++outIt;
            if constexpr (CallErrorHandler)
            {
              if (outIt == outLast)
              {
                basic_big5_hkscs self {};
                return std::forward<TErrorHandler>(errorHandler)(
                  self,
                  TResult(TSubInput(std::move(inIt), std::move(inLast)),
                          TSubOutput(std::move(outIt), std::move(outLast)), s,
                          EncodingError::InsufficientOutputSpace),
                  Span<const code_unit>(std::addressof(units[0]), 1),
                  Span<const code_point>(std::addressof(DoubleCodePointValues[0][0]), 1));
              }
            }
            *outIt = DoubleCodePointValues[doubleCodePointValuesIndex][0];
            ++inIt;
            ++outIt;
            return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                           TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
          };
          switch (lookupIndex)
          {
            case 1'133: return serializeDoubleBytes(0);
            case 1'135: return serializeDoubleBytes(1);
            case 1'164: return serializeDoubleBytes(2);
            case 1'166: return serializeDoubleBytes(3);
            default:    break;
          }

          const std::optional<std::uint_least32_t> maybeCode =
            ::Krys::Text::EncodingTable::big5_hkscs_index_to_code_point(lookupIndex);
          if (maybeCode)
          {
            if constexpr (CallErrorHandler)
            {
              if (outIt == outLast)
              {
                basic_big5_hkscs self {};
                return std::forward<TErrorHandler>(errorHandler)(
                  self,
                  TResult(TSubInput(std::move(inIt), std::move(inLast)),
                          TSubOutput(std::move(outIt), std::move(outLast)), s,
                          EncodingError::InsufficientOutputSpace),
                  Span<const code_unit, 1>(std::addressof(units[0]), 1),
                  Span<const code_point, 1>(std::addressof(DoubleCodePointValues[0][0]), 1));
              }
            }
            const code_point codePoint = static_cast<code_point>(*maybeCode);
            *outIt = codePoint;
            ++inIt;
            ++outIt;
            return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                           TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
          }
        }
      }

      // Top-Level case 2 (default): unrecognized byte sequence!!
      basic_big5_hkscs self {};
      return std::forward<TErrorHandler>(errorHandler)(
        self,
        TResult(TSubInput(std::move(inIt), std::move(inLast)),
                TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
        Span<const code_unit, 1>(std::addressof(units[0]), 1), Span<const code_point, 0>());
    }

    template <typename TInput, typename TOutput, typename TErrorHandler>
    constexpr static auto EncodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler,
                                    state &s) noexcept
    {
      using TSubInput = ::Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
      using TSubOutput = ::Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
      using TResult = ::Krys::Text::EncodeResult<TSubInput, TSubOutput, state>;

      auto inIt = ::Krys::Ranges::cbegin(input);
      auto inLast = ::Krys::Ranges::cend(input);

      if (inIt == inLast)
      {
        // we don't need more, so we can just say the emptiness is a-okay.
        return TResult(std::move(input), std::move(output), s, EncodingError::OK);
      }

      char32 codePoint32 = static_cast<char32>(*inIt);
      code_point codePoint = static_cast<code_point>(codePoint32);
      auto outIt = ::Krys::Ranges::begin(output);
      auto outLast = ::Krys::Ranges::end(output);

      constexpr bool CallErrorHandler = !IsIgnorableErrorHandler<TErrorHandler>;
      if (codePoint32 <= 0x7F)
      {
        if constexpr (CallErrorHandler)
        {
          if (outIt == outLast)
          {
            // output is empty :(
            basic_big5_hkscs self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self, TResult(std::move(input), std::move(output), s, EncodingError::InsufficientOutputSpace),
              Span<const code_point, 0>(), Span<const code_unit, 0>());
          }
        }
        ++inIt;
        *outIt = static_cast<code_unit>(codePoint);
        ++outIt;
        return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                       TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
      }

      std::optional<std::size_t> maybeIndex =
        ::Krys::Text::EncodingTable::big5_hkscs_code_point_to_index(codePoint32);
      if (maybeIndex)
      {
        const std::size_t i = *maybeIndex;
        const std::size_t secondByteBase = (i % 157);
        const std::size_t secondByteOffset = secondByteBase < 0x34 ? 0x40 : 0x62;
        std::size_t first = (i / 157) + 0x81;
        std::size_t second = secondByteBase + secondByteOffset;

        if constexpr (CallErrorHandler)
        {
          if (outIt == outLast)
          {
            // output is empty :(
            basic_big5_hkscs self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self,
              TResult(TSubInput(std::move(inIt), std::move(inLast)),
                      TSubOutput(std::move(outIt), std::move(outLast)), s,
                      EncodingError::InsufficientOutputSpace),
              Span<const code_point, 0>(), Span<const code_unit, 0>());
          }
        }
        const code_unit unit = static_cast<code_unit>(first);
        *outIt = unit;
        if constexpr (CallErrorHandler)
        {
          if (outIt == outLast)
          {
            // output is empty :(
            basic_big5_hkscs self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self,
              TResult(TSubInput(std::move(inIt), std::move(inLast)),
                      TSubOutput(std::move(outIt), std::move(outLast)), s,
                      EncodingError::InsufficientOutputSpace),
              Span<const code_point, 0>(), Span<const code_unit, 1>(std::addressof(unit), 1));
          }
        }
        ++outIt;
        ++inIt;
        *outIt = static_cast<code_unit>(second);
        ++outIt;
        return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                       TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
      }

      basic_big5_hkscs self {};
      return std::forward<TErrorHandler>(errorHandler)(
        self,
        TResult(TSubInput(std::move(inIt), std::move(inLast)),
                TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
        Span<const code_point, 0>(), Span<const code_unit, 0>());
    }

  private:
    constexpr inline static const code_point DoubleCodePointValues[4][2] = {
      {0xCA, 0x304},
      {0xCA, 0x30C},
      {0xEA, 0x304},
      {0xEA, 0x30C},
    };

    static_assert(
      (sizeof(code_point) * CHAR_BIT) >= 16,
      "The code point type for Big5 Hong Kong Supplementary Character Set (big5_hkscs) must be at least 16 "
      "bits wide");
  };

  /// @brief The Big5-HKSCS (Hong Kong Supplementary Character Set) encoding, standardized around 2001.
  constexpr inline basic_big5_hkscs<char> big5_hkscs = {};
}
