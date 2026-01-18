#pragma once

#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Utils/Size.hpp"
#include "Krystal.Text/Concepts.hpp"
#include "Krystal.Text/Decode/DecodeResult.hpp"
#include "Krystal.Text/Encode/EncodeResult.hpp"
#include "Krystal.Text/Encodings/EncodingTables/GB18030.tables.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/Unicode.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"
#include <climits>

namespace Krys::Text::Encodings::Impl
{
  /// @brief The GBK replacement code units for U+003F.
  template <typename TChar>
  constexpr inline Array<TChar, 4> GB18030ReplacementUnits {
    {static_cast<TChar>(0x84), static_cast<TChar>(0x31), static_cast<TChar>(0xA4), static_cast<TChar>(0x37)}};

  template <typename TDerived, typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint,
            bool IsGBK = true>
  struct BasicGB18030
  {
    static_assert(IsGBK || (sizeof(TCodePoint) * CHAR_BIT) >= 22,
                  "The size of GB18030's codepoint type must be at least 22 bits.");

    static_assert(!IsGBK || (sizeof(TCodePoint) * CHAR_BIT) >= 16,
                  "The size of GBK's codepoint type must be at least 16 bits.");

    /// @brief Shift-JIS is generally stored as minimum-8-bit values in a sequence.
    using code_unit = TCodeUnit;

    /// @brief Shift-JIS outputs Unicode Scalar Values.
    using code_point = TCodePoint;

    /// @brief Shift-JIS requires no state.
    using state = EmptyState;

    /// @brief The Shift-JIS encoding can put out at most 1 code_point units per decoding action.
    constexpr static inline std::size_t MaxCodePoints = 1;

    /// @brief The Shift-JIS encoding can put out at most 4 code_point units per encoding action.
    constexpr static inline std::size_t MaxCodeUnits = IsGBK ? 2 : 4;

    /// @brief A fixed-size replacement for either the GBK or GB18030 encoding.
    constexpr Span<const code_unit, IsGBK ? 1 : 4> ReplacementCodeUnits() const noexcept
    {
      if constexpr (IsGBK)
      {
        return Span<const code_unit, 1>(&::Krys::Text::Unicode::ASCIIReplacement<code_unit>, 1);
      }
      else
      {
        return Span<const code_unit, 4>(GB18030ReplacementUnits<code_unit>.data(),
                                        GB18030ReplacementUnits<code_unit>.size());
      }
    }

    /// @brief A fixed-size replacement for either the GBK or GB18030 encoding.
    constexpr Span<const code_point, IsGBK ? 1 : 4> ReplacementCodePoints() const noexcept
    {
      if constexpr (IsGBK)
      {
        return Span<const code_point, 1>(&::Krys::Text::Unicode::ASCIIReplacement<code_point>, 1);
      }
      else
      {
        return Span<const code_point, 4>(GB18030ReplacementUnits<code_point>.data(),
                                         GB18030ReplacementUnits<code_point>.size());
      }
    }

    /// @brief Marks this encoding as injective for the decode portion of its encoding actions.
    using is_decode_injective = std::true_type;

    /// @brief Marks this encoding as injective for the encode portion of its encoding actions. This is true
    /// for GB18030, and not true for pure GBK.
    using is_encode_injective = IntegralConstant<bool, !IsGBK>;

    /// @brief GB18030-style encodings can encode all Unicode code points. GBK cannot.
    using is_unicode_encoding = IntegralConstant<bool, !IsGBK>;

    /// @brief Decodes a single complete unit of information as codePoint points and produces a result
    /// with the input and output ranges moved past what was successfully read and written; or, produces an
    /// error and returns the input and output ranges untouched.
    /// @param[in] input The input view to read codePoint units from.
    /// @param[in] output The output view to write codePoint points into.
    /// @param[in] errorHandler The error handler to invoke if encoding fails.
    /// @param[in, out] state The necessary state information. For this encoding, the state is empty and
    /// means very little.
    /// @returns A DecodeResult object that contains the input range, output range, error
    /// handler, and a reference to the passed-in state\.
    /// @remarks To the best ability of the implementation, the iterators will be returned untouched (e.g.,
    /// the input models at least a view and a forward_range). If it is not possible, returned ranges may be
    /// incremented even if an error occurs due to the semantics of any view that models an input_range.
    template <typename TInput, typename TOutput, typename TErrorHandler>
    constexpr static auto DecodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler,
                                    state &s) noexcept
    {
      using TSubInput = ::Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
      using TSubOutput = ::Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
      using TResult = ::Krys::Text::DecodeResult<TSubInput, TSubOutput, state>;
      constexpr bool CallErrorHandler = !IsIgnorableErrorHandler<TErrorHandler>;

      auto inIt = ::Krys::Ranges::cbegin(input);
      auto inLast = ::Krys::Ranges::cend(input);

      if (inIt == inLast)
      {
        // we don't need more, so we can just say the emptiness is a-okay.
        return TResult(std::move(input), std::move(output), s, EncodingError::OK);
      }

      std::size_t readUnits = 0;
      code_unit units[4] = {static_cast<code_unit>(*inIt)};
      const uchar unit0 = static_cast<uchar>(units[0]);
      auto outIt = ::Krys::Ranges::begin(output);
      auto outLast = ::Krys::Ranges::end(output);

      if (unit0 <= 0x7F)
      {
        // Top-Level case 0: it's an ASCII byte
        const code_point codePoint = static_cast<code_point>(unit0);
        if constexpr (CallErrorHandler)
        {
          if (outIt == outLast)
          {
            TDerived self {};
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
      else if (unit0 == 0x80)
      {
        // Top-Level case 1: 0x80 case
        if constexpr (CallErrorHandler)
        {
          if (outIt == outLast)
          {
            TDerived self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self,
              TResult(TSubInput(std::move(inIt), std::move(inLast)),
                      TSubOutput(std::move(outIt), std::move(outLast)), s,
                      EncodingError::InsufficientOutputSpace),
              Span<const code_unit, 1>(std::addressof(units[0]), 1), Span<const code_point, 0>());
          }
        }
        const code_point codePoint = static_cast<code_point>(0x20AC);
        *outIt = codePoint;
        ++inIt;
        ++outIt;
        return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                       TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
      }
      else if ((unit0 >= 0x81 && unit0 <= 0xFE))
      {
        // Top-Level case 2: this is a double-byte sequence!
        if constexpr (CallErrorHandler)
        {
          if (inIt == inLast)
          {
            TDerived self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self, TResult(std::move(input), std::move(output), s, EncodingError::IncompleteSequence),
              Span<const code_unit, 1>(std::addressof(units[0]), 1), Span<const code_point, 0>());
          }
        }
        ++inIt;
        ++readUnits;
        units[1] = static_cast<code_unit>(*inIt);
        const uchar secondByte = static_cast<uchar>(units[1]);

        auto lookupAndWriteOut = [&](std::size_t lookupIndex, auto useRangesTypeValue)
        {
          constexpr bool useRanges = decltype(useRangesTypeValue)::value;
          constexpr auto toCodePoint = useRanges
                                         ? &::Krys::Text::EncodingTable::gb18030_ranges_index_to_code_point
                                         : &::Krys::Text::EncodingTable::gb18030_index_to_code_point;
          const std::optional<std::uint_least32_t> maybeCode = toCodePoint(lookupIndex);
          if (maybeCode)
          {
            if constexpr (CallErrorHandler)
            {
              if (outIt == outLast)
              {
                TDerived self {};
                return std::forward<TErrorHandler>(errorHandler)(
                  self,
                  TResult(TSubInput(std::move(inIt), std::move(inLast)),
                          TSubOutput(std::move(outIt), std::move(outLast)), s,
                          EncodingError::InsufficientOutputSpace),
                  Span<const code_unit, 3>(std::addressof(units[0]), readUnits), Span<const code_point, 0>());
              }
            }
            const code_point codePoint = static_cast<code_point>(*maybeCode);
            *outIt = codePoint;
            ++inIt;
            ++outIt;
            return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                           TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
          }
          // Otherwise, we failed.
          TDerived self {};
          return std::forward<TErrorHandler>(errorHandler)(
            self,
            TResult(TSubInput(std::move(inIt), std::move(inLast)),
                    TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
            Span<const code_unit>(std::addressof(units[0]), readUnits), Span<const code_point, 0>());
        };

        if (secondByte >= 0x30 && secondByte <= 0x39)
        {
          // Nested: this is a potential 3/4 byte sequence sequence!
          if constexpr (IsGBK)
          {
            if constexpr (CallErrorHandler)
            {
              // we cannot enter this criteria with GBK; it is not allowed to encode more than 2 bytes!
              TDerived self {};
              return std::forward<TErrorHandler>(errorHandler)(
                self, TResult(std::move(input), std::move(output), s, EncodingError::InvalidSequence),
                Span<const code_unit, 2>(std::addressof(units[0]), readUnits), Span<const code_point, 0>());
            }
          }
          else
          {
            if constexpr (CallErrorHandler)
            {
              if (inIt == inLast)
              {
                TDerived self {};
                return std::forward<TErrorHandler>(errorHandler)(
                  self, TResult(std::move(input), std::move(output), s, EncodingError::IncompleteSequence),
                  Span<const code_unit, 2>(std::addressof(units[0]), readUnits), Span<const code_point, 0>());
              }
            }
            ++inIt;
            ++readUnits;
            units[2] = static_cast<code_unit>(*inIt);
            const uchar thirdByte = static_cast<uchar>(units[2]);
            if constexpr (CallErrorHandler)
            {
              if (thirdByte < 0x81 || thirdByte > 0xFE)
              {
                TDerived self {};
                return std::forward<TErrorHandler>(errorHandler)(
                  self, TResult(std::move(input), std::move(output), s, EncodingError::InvalidSequence),
                  Span<const code_unit, 2>(std::addressof(units[0]), readUnits), Span<const code_point, 0>());
              }
              if (inIt == inLast)
              {
                TDerived self {};
                return std::forward<TErrorHandler>(errorHandler)(
                  self, TResult(std::move(input), std::move(output), s, EncodingError::IncompleteSequence),
                  Span<const code_unit, 2>(std::addressof(units[0]), readUnits), Span<const code_point, 0>());
              }
            }

            ++inIt;
            ++readUnits;
            units[3] = static_cast<code_unit>(*inIt);
            const uchar fourthByte = static_cast<uchar>(units[3]);
            if constexpr (CallErrorHandler)
            {
              if (fourthByte < 0x30 || fourthByte > 0x39)
              {
                TDerived self {};
                return std::forward<TErrorHandler>(errorHandler)(
                  self, TResult(std::move(input), std::move(output), s, EncodingError::InvalidSequence),
                  Span<const code_unit, 3>(std::addressof(units[0]), readUnits), Span<const code_point, 0>());
              }
            }
            const std::size_t lookupIndex = ((unit0 - 0x81) * (10 * 126 * 10))
                                            + ((secondByte - 0x30) * (10 * 126)) + ((thirdByte - 0x81) * 10)
                                            + (fourthByte - 0x30);
            return lookupAndWriteOut(lookupIndex, std::true_type {});
          }
        }
        else
        {
          if ((secondByte >= 0x40 && secondByte <= 0x7E) || (secondByte >= 0x80 && secondByte <= 0xFE))
          {
            const uint_least32_t secondByteOffset = secondByte < 0x7F ? 0x40 : 0x41;
            std::size_t lookupIndex = ((unit0 - 0x81) * 190) + (secondByte - secondByteOffset);
            return lookupAndWriteOut(lookupIndex, std::false_type {});
          }
        }
      }

      // Final case (default): unrecognized byte sequence!!
      TDerived self {};
      return std::forward<TErrorHandler>(errorHandler)(
        self,
        TResult(TSubInput(std::move(inIt), std::move(inLast)),
                TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
        Span<const code_unit>(std::addressof(units[0]), readUnits), Span<const code_point, 0>());
    }

    /// @brief Encodes a single complete unit of information as codePoint units and produces a result with
    /// the input and output ranges moved past what was successfully read and written; or, produces an error
    /// and returns the input and output ranges untouched.
    /// @param[in] input The input view to read codePoint points from.
    /// @param[in] output The output view to write codePoint units into.
    /// @param[in] errorHandler The error handler to invoke if encoding fails.
    /// @param[in, out] state The necessary state information. For this encoding, the state is empty and
    /// means very little.
    /// @returns A EncodeResult object that contains the input range, output range, error
    /// handler, and a reference to the passed-in state\.
    /// @remarks To the best ability of the implementation, the iterators will be returned untouched (e.g.,
    /// the input models at least a view and a forward_range). If it is not possible, returned ranges may be
    /// incremented even if an error occurs due to the semantics of any view that models an input_range.
    template <typename TInput, typename TOutput, typename TErrorHandler>
    constexpr static auto EncodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler,
                                    state &s) noexcept
    {
      using TUErrorHandler = remove_cvref_t<TErrorHandler>;
      using TSubInput = ::Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
      using TSubOutput = ::Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
      using TResult = ::Krys::Text::EncodeResult<TSubInput, TSubOutput, state>;
      constexpr bool CallErrorHandler = !IsIgnorableErrorHandler<TUErrorHandler>;

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
      if (codePoint32 <= 0x7F)
      {
        if constexpr (CallErrorHandler)
        {
          if (outIt == outLast)
          {
            // output is empty :(
            TDerived self {};
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

      if constexpr (CallErrorHandler)
      {
        if (codePoint == 0xE5E5)
        {
          TDerived self {};
          return std::forward<TErrorHandler>(errorHandler)(
            self, TResult(std::move(input), std::move(output), s, EncodingError::InvalidSequence),
            Span<const code_point, 0>(), Span<const code_unit, 0>());
        }
      }
      if constexpr (IsGBK)
      {
        if (codePoint == 0x80)
        {
          if constexpr (CallErrorHandler)
          {
            if (outIt == outLast)
            {
              // output is empty :(
              TDerived self {};
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
      }

      std::optional<std::size_t> maybeLookupGBKIndex =
        ::Krys::Text::EncodingTable::gb18030_code_point_to_index(codePoint32);
      if (maybeLookupGBKIndex)
      {
        if constexpr (CallErrorHandler)
        {
          if (outIt == outLast)
          {
            // output is empty :(
            TDerived self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self, TResult(std::move(input), std::move(output), s, EncodingError::InsufficientOutputSpace),
              Span<const code_point, 0>(), Span<const code_unit, 0>());
          }
        }
        const std::size_t lookupGBKIndex = *maybeLookupGBKIndex;
        const uchar firstByte = static_cast<uchar>((lookupGBKIndex / 190) + 0x81);
        const std::size_t secondByteInitial = (lookupGBKIndex % 190);
        const uchar secondByteOffset = static_cast<uchar>(secondByteInitial < 0x3F ? 0x40 : 0x41);
        const uchar secondByte = static_cast<uchar>(secondByteInitial + secondByteOffset);
        const code_unit secondUnit = static_cast<code_unit>(secondByte);
        *outIt = static_cast<code_unit>(firstByte);
        ++outIt;
        if constexpr (CallErrorHandler)
        {
          if (outIt == outLast)
          {
            // output is empty :(
            TDerived self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self, TResult(std::move(input), std::move(output), s, EncodingError::InsufficientOutputSpace),
              Span<const code_point, 0>(), Span<const code_unit, 1>(std::addressof(secondUnit), 1));
          }
        }
        ++inIt;
        *outIt = secondUnit;
        ++outIt;
        return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                       TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
      }

      if constexpr (!IsGBK)
      {
        std::optional<std::size_t> maybeLookupIndex =
          ::Krys::Text::EncodingTable::gb18030_ranges_code_point_to_index(codePoint32);
        if (maybeLookupIndex)
        {
          const std::size_t lookupIndex = *maybeLookupIndex;
          Array<code_unit, 4> units = {};
          units[0] = static_cast<code_unit>(static_cast<uchar>((lookupIndex / (10 * 126 * 10)) + 0x81));
          const std::size_t lookupIndexUnit1 = lookupIndex % (10 * 126 * 10);
          units[1] = static_cast<code_unit>(static_cast<uchar>((lookupIndexUnit1 / (10 * 126)) + 0x30));
          const std::size_t lookupIndexUnit2 = lookupIndexUnit1 % (10 * 126);
          units[2] = static_cast<code_unit>(static_cast<uchar>((lookupIndexUnit2 / 10) + 0x81));
          units[3] = static_cast<code_unit>(static_cast<uchar>((lookupIndexUnit2 % 10) + 0x30));

          for (std::size_t i = 0; i < units.size(); ++i)
          {
            if constexpr (CallErrorHandler)
            {
              if (outIt == outLast)
              {
                // output is empty :(
                TDerived self {};
                return std::forward<TErrorHandler>(errorHandler)(
                  self,
                  TResult(TSubInput(std::move(inIt), std::move(inLast)),
                          TSubOutput(std::move(outIt), std::move(outLast)), s,
                          EncodingError::InsufficientOutputSpace),
                  Span<const code_point, 0>(),
                  Span<const code_unit>(std::addressof(units[i]), units.size() - i));
              }
            }
            const code_unit unit = static_cast<code_unit>(units[i]);
            *outIt = unit;
            ++outIt;
          }
          ++inIt;
          return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                         TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
        }
      }

      TDerived self {};
      return std::forward<TErrorHandler>(errorHandler)(
        self,
        TResult(TSubInput(std::move(inIt), std::move(inLast)),
                TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
        Span<const code_point, 0>(), Span<const code_unit, 0>());
    }
  };
}
