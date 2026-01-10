#pragma once

#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Text/Concepts.hpp"
#include "Krystal.Text/Decode/DecodeResult.hpp"
#include "Krystal.Text/Encode/EncodeResult.hpp"
#include "Krystal.Text/Encodings/EncodingTables/ShiftJISx0208.tables.hpp"
#include "Krystal.Text/Unicode.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The Shift-JIS Encoding (with extensions x0208) for use with most Shift-JIS applications.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_shift_jis_x0208
  {
  public:
    using code_unit = TCodeUnit;

    using code_point = TCodePoint;

    using state = EmptyState;

    constexpr static inline std::size_t MaxCodePoints = 1;

    constexpr static inline std::size_t MaxCodeUnits = 2;

    constexpr Span<const code_unit, 1> ReplacementCodeUnits() const noexcept
    {
      return Span<const code_unit, 1>(::Krys::Text::Unicode::ASCIIReplacement<code_unit>.data(),
                                      ::Krys::Text::Unicode::ASCIIReplacement<code_unit>.size());
    }

    using is_decode_injective = std::true_type;

    template <typename TInput, typename TOutput, typename TErrorHandler>
    constexpr static auto DecodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler,
                                    state &s) noexcept
    {
      using TSubInput = ::Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
      using TSubOutput = ::Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
      using TResult = DecodeResult<TSubInput, TSubOutput, s>;
      ;

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
      if (unit0 <= 0x80)
      {
        // Top-Level case 0: it's an ASCII byte
        const code_point codePoint = static_cast<code_point>(unit0);
        if constexpr (CallErrorHandler)
        {
          if (outIt == outLast)
          {
            basic_shift_jis_x0208 self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self,
              TResult(TSubInput(std::move(inIt), std::move(inLast)),
                      TSubOutput(std::move(outIt), std::move(outLast)), s,
                      EncodingError::InsufficientOutputSpace),
              Span<const code_unit, 1>(std::addressof(units[0]), 1), Span<const code_point, 0>());
          }
        }
        *outIt = codePoint;
        ++inIt;
        ++outIt;
        return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                       TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
      }
      else if (unit0 <= 0xDF && unit0 >= 0xA1)
      {
        // Top-Level case 1: it's non-ASCII single-byte
        const code_point codePoint = static_cast<code_point>((0xFF61 - 0xA1) + unit0);
        if constexpr (CallErrorHandler)
        {
          if (outIt == outLast)
          {
            basic_shift_jis_x0208 self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self,
              TResult(TSubInput(std::move(inIt), std::move(inLast)),
                      TSubOutput(std::move(outIt), std::move(outLast)), s,
                      EncodingError::InsufficientOutputSpace),
              Span<const code_unit, 1>(std::addressof(units[0]), 1), Span<const code_point, 0>());
          }
        }
        ++inIt;
        *outIt = codePoint;
        ++outIt;
        return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                       TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
      }
      else if ((unit0 <= 0x9F && unit0 >= 0x81) || (unit0 <= 0xFC && unit0 >= 0xE0))
      {
        // Top-Level case 2: this is a double-byte sequence!
        if constexpr (CallErrorHandler)
        {
          if (inIt == inLast)
          {
            basic_shift_jis_x0208 self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self, TResult(std::move(input), std::move(output), s, EncodingError::IncompleteSequence),
              Span<const code_unit, 1>(std::addressof(units[0]), 1), Span<const code_point, 0>());
          }
        }
        ++inIt;
        units[1] = static_cast<code_unit>(*inIt);
        uchar secondByte = static_cast<uchar>(units[1]);
        uchar lookupOffset = secondByte < 0x7F ? 0x40 : 0x41;
        uchar leadLookupOffset = unit0 < 0xA0 ? 0x81 : 0xC1;
        std::size_t lookupIndex = 0;
        if ((secondByte <= 0x7E && secondByte >= 0x40) || (secondByte <= 0xFC && secondByte >= 0x80))
        {
          lookupIndex = (((unit0 - leadLookupOffset) * 188) + secondByte) - lookupOffset;
          if (lookupIndex <= 10'715 && lookupIndex >= 8'836)
          {
            if constexpr (CallErrorHandler)
            {
              if (outIt == outLast)
              {
                basic_shift_jis_x0208 self {};
                return std::forward<TErrorHandler>(errorHandler)(
                  self,
                  TResult(TSubInput(std::move(inIt), std::move(inLast)),
                          TSubOutput(std::move(outIt), std::move(outLast)), s,
                          EncodingError::InsufficientOutputSpace),
                  Span<const code_unit, 1>(std::addressof(units[0]), 1), Span<const code_point, 0>());
              }
            }
            char32 codePoint = static_cast<char32>(0xE000 - 8'836 + lookupIndex);
            *outIt = static_cast<code_point>(codePoint);
            ++inIt;
            ++outIt;
            return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                           TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
          }
          else
          {
            std::optional<std::uint_least32_t> maybeCode =
              Krys::Text::EncodingTable::shift_jis_x0208_index_to_code_point(lookupIndex);
            if (maybeCode)
            {
              if constexpr (CallErrorHandler)
              {
                if (outIt == outLast)
                {
                  basic_shift_jis_x0208 self {};
                  return std::forward<TErrorHandler>(errorHandler)(
                    self,
                    TResult(TSubInput(std::move(inIt), std::move(inLast)),
                            TSubOutput(std::move(outIt), std::move(outLast)), s,
                            EncodingError::InsufficientOutputSpace),
                    Span<const code_unit, 1>(std::addressof(units[0]), 1), Span<const code_point, 0>());
                }
              }
              const char32 &codePoint = *maybeCode;
              *outIt = static_cast<code_point>(codePoint);
              ++inIt;
              ++outIt;
              return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                             TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
            }
          }
        }
      }

      // Top-Level case 3 (default): unrecognized byte sequence!!
      basic_shift_jis_x0208 self {};
      return std::forward<TErrorHandler>(errorHandler)(
        self,
        TResult(TSubInput(std::move(inIt), std::move(inLast)),
                TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
        Span<const code_unit, 0>(), Span<const code_point, 0>());
    }

    template <typename TInput, typename TOutput, typename TErrorHandler>
    constexpr static auto EncodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler,
                                    state &s) noexcept
    {
      using TSubInput = ::Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
      using TSubOutput = ::Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
      using TResult = EncodeResult<TSubInput, TSubOutput, state>;

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
      if (codePoint32 <= 0x80)
      {
        if constexpr (CallErrorHandler)
        {
          if (outIt == outLast)
          {
            // output is empty :(
            basic_shift_jis_x0208 self {};
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
      else if (codePoint32 == 0x00A5)
      {
        if constexpr (CallErrorHandler)
        {
          if (outIt == outLast)
          {
            // output is empty :(
            basic_shift_jis_x0208 self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self,
              TResult(TSubInput(std::move(inIt), std::move(inLast)),
                      TSubOutput(std::move(outIt), std::move(outLast)), s,
                      EncodingError::InsufficientOutputSpace),
              Span<const code_point, 0>(), Span<const code_unit, 0>());
          }
        }
        ++inIt;
        *outIt = static_cast<code_unit>(0x5C);
        ++outIt;
        return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                       TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
      }
      else if (codePoint32 == 0x203E)
      {
        if constexpr (CallErrorHandler)
        {
          if (outIt == outLast)
          {
            // output is empty :(
            basic_shift_jis_x0208 self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self,
              TResult(TSubInput(std::move(inIt), std::move(inLast)),
                      TSubOutput(std::move(outIt), std::move(outLast)), s,
                      EncodingError::InsufficientOutputSpace),
              Span<const code_point, 0>(), Span<const code_unit, 0>());
          }
        }
        ++inIt;
        *outIt = static_cast<code_unit>(0x7E);
        ++outIt;
        return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                       TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
      }
      else if (codePoint32 >= 0xFF61 && codePoint32 <= 0xFF9F)
      {
        if constexpr (CallErrorHandler)
        {
          if (outIt == outLast)
          {
            // output is empty :(
            basic_shift_jis_x0208 self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self,
              TResult(TSubInput(std::move(inIt), std::move(inLast)),
                      TSubOutput(std::move(outIt), std::move(outLast)), s,
                      EncodingError::InsufficientOutputSpace),
              Span<const code_point, 0>(), Span<const code_unit, 0>());
          }
        }
        ++inIt;
        char32 intermediate = (codePoint32 - 0xFF61);
        *outIt = static_cast<code_unit>(intermediate + 0xA1);
        ++outIt;
        return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                       TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
      }

      if (codePoint == 0x2212)
      {
        codePoint = 0xFF0D;
      }

      std::optional<std::size_t> maybeIndex =
        ::Krys::Text::EncodingTable::shift_jis_x0208_code_point_to_index(codePoint);
      if (maybeIndex)
      {
        std::size_t i = *maybeIndex;
        std::size_t first = i / 188;
        std::size_t firstOffset = first < 0x1F ? 0x81 : 0xC1;
        std::size_t second = i % 188;
        std::size_t secondOffset = second < 0x3F ? 0x40 : 0x41;

        if constexpr (CallErrorHandler)
        {
          if (outIt == outLast)
          {
            // output is empty :(
            basic_shift_jis_x0208 self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self,
              TResult(TSubInput(std::move(inIt), std::move(inLast)),
                      TSubOutput(std::move(outIt), std::move(outLast)), s,
                      EncodingError::InsufficientOutputSpace),
              Span<const code_point, 0>(), Span<const code_unit, 0>());
          }
        }
        uchar unitUC = static_cast<uchar>(first + firstOffset);
        const code_unit unit = static_cast<code_unit>(unitUC);
        *outIt = unit;
        ++outIt;
        if constexpr (CallErrorHandler)
        {
          if (outIt == outLast)
          {
            // output is empty :(
            basic_shift_jis_x0208 self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self,
              TResult(TSubInput(std::move(inIt), std::move(inLast)),
                      TSubOutput(std::move(outIt), std::move(outLast)), s,
                      EncodingError::InsufficientOutputSpace),
              Span<const code_point, 0>(), Span<const code_unit, 1>(std::addressof(unit), 1));
          }
        }
        ++inIt;
        *outIt = static_cast<code_unit>(second + secondOffset);
        ++outIt;
        return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                       TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
      }

      basic_shift_jis_x0208 self {};
      return std::forward<TErrorHandler>(errorHandler)(
        self,
        TResult(TSubInput(std::move(inIt), std::move(inLast)),
                TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
        Span<const code_point, 0>(), Span<const code_unit, 0>());
    }
  };

  /// @brief The Shift-JIS Encoding (with extensions x0208) for use with most Shift-JIS applications.
  constexpr inline basic_shift_jis_x0208<char> shift_jis_x0208 = {};
}
