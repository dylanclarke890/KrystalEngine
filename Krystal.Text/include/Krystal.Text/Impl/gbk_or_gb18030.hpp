#pragma once

#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Lib/Utils/Size.hpp"
#include "Krystal.Text/DecodeResult.hpp"
#include "Krystal.Text/EncodeResult.hpp"
#include "Krystal.Text/Encodings/EncodingTables/GB18030.tables.hpp"
#include "Krystal.Text/Impl/ReplacementUnits.hpp"
#include "Krystal.Text/IsIgnorableErrorHandler.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"
#include <climits>

namespace Krys
{
  namespace Impl
  {
    template <typename TDerived, typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint,
              bool TIsGBK = true>
    struct BasicGB18030
    {
      /// @brief Shift-JIS is generally stored as minimum-8-bit values in a sequence.
      using code_unit = TCodeUnit;

      /// @brief Shift-JIS outputs Unicode Scalar Values.
      using code_point = TCodePoint;

      /// @brief Shift-JIS requires no state.
      using state = EmptyState;

      /// @brief The Shift-JIS encoding can put out at most 1 code_point units per decoding action.
      static constexpr inline std::size_t MaxCodePoints = 1;

      /// @brief The Shift-JIS encoding can put out at most 4 code_point units per encoding action.
      static constexpr inline std::size_t MaxCodeUnits = TIsGBK ? 2 : 4;

      /// @brief A fixed-size replacement for either the GBK or GB18030 encoding.
      constexpr Span<const code_unit, TIsGBK ? 1 : 4> ReplacementCodeUnits() const noexcept
      {
        if constexpr (TIsGBK)
        {
          return Span<const code_unit, 1>(Impl::QuestionMarkReplacementUnits<code_unit>.data(),
                                          Impl::QuestionMarkReplacementUnits<code_unit>.size());
        }
        else
        {
          return Span<const code_unit, 4>(Impl::xFFFDGB18030ReplacementUnits<code_unit>.data(),
                                          Impl::xFFFDGB18030ReplacementUnits<code_unit>.size());
        }
      }

      /// @brief A fixed-size replacement for either the GBK or GB18030 encoding.
      constexpr Span<const code_point, 1> ReplacementCodePoints() const noexcept
      {
        if constexpr (TIsGBK)
        {
          return Span<const code_point, 1>(Impl::QuestionMarkReplacementUnits<code_unit>.data(),
                                           Impl::QuestionMarkReplacementUnits<code_unit>.size());
        }
        else
        {
          return Span<const code_point, 4>(Impl::xFFFDGB18030ReplacementUnits<code_unit>.data(),
                                           Impl::xFFFDGB18030ReplacementUnits<code_unit>.size());
        }
      }

      /// @brief Marks this encoding as injective for the decode portion of its encoding actions.
      using is_decode_injective = std::true_type;

      /// @brief Marks this encoding as injective for the encode portion of its encoding actions. This is true
      /// for GB18030, and not true for pure GBK.
      using is_encode_injective = IntegralConstant<bool, !TIsGBK>;

      /// @brief GB18030-style encodings are Unicode Encodings (can encode all Unicode code points). GBK
      /// cannot.
      using IsUnicodeEncoding = IntegralConstant<bool, !TIsGBK>;

      /// @brief Decodes a single complete unit of information as codePoint points and produces a result
      /// with the input and output ranges moved past what was successfully read and written; or, produces an
      /// error and returns the input and output ranges untouched.
      /// @param[in] input The input view to read codePoint uunits from.
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
      static constexpr auto DecodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler,
                                      state &state) noexcept
      {
        using TUErrorHandler = remove_cvref_t<TErrorHandler>;
        using TSubInput = Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
        using TSubOutput = Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
        using TResult = DecodeResult<TSubInput, TSubOutput, state>;
        constexpr bool CallErrorHandler = !IsIgnorableErrorHandler<TUErrorHandler>;

        auto inIt = Krys::Ranges::cbegin(input);
        auto inLast = Krys::Ranges::cend(input);

        if (inIt == inLast)
        {
          // we don't need more, so we can just say the emptiness is a-okay.
          return TResult(std::move(input), std::move(output), state, EncodingError::OK);
        }

        std::size_t readUnits = 0;
        code_unit units[4] = {static_cast<code_unit>(*inIt)};
        const uchar unit0 = static_cast<uchar>(units[0]);
        auto outIt = Krys::Ranges::begin(output);
        auto outLast = Krys::Ranges::end(output);

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
                        TSubOutput(std::move(outIt), std::move(outLast)), state,
                        EncodingError::InsufficientOutputSpace),
                Span<const code_unit, 0>(), Span<const code_point, 0>());
            }
          }
          *outIt = codePoint;
          ++inIt;
          ++outIt;
          return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                         TSubOutput(std::move(outIt), std::move(outLast)), state, EncodingError::OK);
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
                        TSubOutput(std::move(outIt), std::move(outLast)), state,
                        EncodingError::InsufficientOutputSpace),
                Span<const code_unit, 1>(std::addressof(units[0]), 1), Span<const code_point, 0>());
            }
          }
          const code_point codePoint = static_cast<code_point>(0x20AC);
          *outIt = codePoint;
          ++inIt;
          ++outIt;
          return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                         TSubOutput(std::move(outIt), std::move(outLast)), state, EncodingError::OK);
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
                self, TResult(std::move(input), std::move(output), state, EncodingError::IncompleteSequence),
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
                            TSubOutput(std::move(outIt), std::move(outLast)), state,
                            EncodingError::InsufficientOutputSpace),
                    Span<const code_unit, 3>(std::addressof(units[0]), readUnits),
                    Span<const code_point, 0>());
                }
              }
              const code_point codePoint = static_cast<code_point>(*maybeCode);
              *outIt = codePoint;
              ++inIt;
              ++outIt;
              return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                             TSubOutput(std::move(outIt), std::move(outLast)), state, EncodingError::OK);
            }
            // Otherwise, we failed.
            TDerived self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self,
              TResult(TSubInput(std::move(inIt), std::move(inLast)),
                      TSubOutput(std::move(outIt), std::move(outLast)), state,
                      EncodingError::InvalidSequence),
              Span<const code_unit>(std::addressof(units[0]), readUnits), Span<const code_point, 0>());
          };

          if (secondByte >= 0x30 && secondByte <= 0x39)
          {
            // Nested: this is a potential 3/4 byte sequence sequence!
            if constexpr (TIsGBK)
            {
              if constexpr (CallErrorHandler)
              {
                // we cannot enter this criteria with GBK; it is not allowed to encode more than
                // 2 bytes!
                TDerived self {};
                return std::forward<TErrorHandler>(errorHandler)(
                  self, TResult(std::move(input), std::move(output), state, EncodingError::InvalidSequence),
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
                    self,
                    TResult(std::move(input), std::move(output), state, EncodingError::IncompleteSequence),
                    Span<const code_unit, 2>(std::addressof(units[0]), readUnits),
                    Span<const code_point, 0>());
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
                    self, TResult(std::move(input), std::move(output), state, EncodingError::InvalidSequence),
                    Span<const code_unit, 2>(std::addressof(units[0]), readUnits),
                    Span<const code_point, 0>());
                }
                if (inIt == inLast)
                {
                  TDerived self {};
                  return std::forward<TErrorHandler>(errorHandler)(
                    self,
                    TResult(std::move(input), std::move(output), state, EncodingError::IncompleteSequence),
                    Span<const code_unit, 2>(std::addressof(units[0]), readUnits),
                    Span<const code_point, 0>());
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
                    self, TResult(std::move(input), std::move(output), state, EncodingError::InvalidSequence),
                    Span<const code_unit, 3>(std::addressof(units[0]), readUnits),
                    Span<const code_point, 0>());
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
                  TSubOutput(std::move(outIt), std::move(outLast)), state, EncodingError::InvalidSequence),
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
      static constexpr auto EncodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler,
                                      state &state) noexcept
      {
        using TUErrorHandler = remove_cvref_t<TErrorHandler>;
        using TSubInput = Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
        using TSubOutput = Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
        using TResult = EncodeResult<TSubInput, TSubOutput, state>;
        constexpr bool CallErrorHandler = !IsIgnorableErrorHandler<TUErrorHandler>;

        auto inIt = Krys::Ranges::cbegin(input);
        auto inLast = Krys::Ranges::cend(input);

        if (inIt == inLast)
        {
          // we don't need more, so we can just say the emptiness is a-okay.
          return TResult(std::move(input), std::move(output), state, EncodingError::OK);
        }

        char32 codePoint32 = static_cast<char32>(*inIt);
        code_point codePoint = static_cast<code_point>(codePoint32);
        auto outIt = Krys::Ranges::begin(output);
        auto outLast = Krys::Ranges::end(output);
        if (codePoint32 <= 0x7F)
        {
          if constexpr (CallErrorHandler)
          {
            if (outIt == outLast)
            {
              // output is empty :(
              TDerived self {};
              return std::forward<TErrorHandler>(errorHandler)(
                self,
                TResult(std::move(input), std::move(output), state, EncodingError::InsufficientOutputSpace),
                Span<const code_point, 0>(), Span<const code_unit, 0>());
            }
          }
          ++inIt;
          *outIt = static_cast<code_unit>(codePoint);
          ++outIt;
          return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                         TSubOutput(std::move(outIt), std::move(outLast)), state, EncodingError::OK);
        }

        if constexpr (CallErrorHandler)
        {
          if (codePoint == 0xE5E5)
          {
            TDerived self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self, TResult(std::move(input), std::move(output), state, EncodingError::InvalidSequence),
              Span<const code_point, 0>(), Span<const code_unit, 0>());
          }
        }
        if constexpr (TIsGBK)
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
                  self,
                  TResult(std::move(input), std::move(output), state, EncodingError::InsufficientOutputSpace),
                  Span<const code_point, 0>(), Span<const code_unit, 0>());
              }
            }
            ++inIt;
            *outIt = static_cast<code_unit>(codePoint);
            ++outIt;
            return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                           TSubOutput(std::move(outIt), std::move(outLast)), state, EncodingError::OK);
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
                self,
                TResult(std::move(input), std::move(output), state, EncodingError::InsufficientOutputSpace),
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
                self,
                TResult(std::move(input), std::move(output), state, EncodingError::InsufficientOutputSpace),
                Span<const code_point, 0>(), Span<const code_unit, 1>(std::addressof(secondUnit), 1));
            }
          }
          ++inIt;
          *outIt = secondUnit;
          ++outIt;
          return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                         TSubOutput(std::move(outIt), std::move(outLast)), state, EncodingError::OK);
        }

        if constexpr (!TIsGBK)
        {
          std::optional<std::size_t> maybeLookupIndex =
            Krys::Text::EncodingTable::gb18030_ranges_code_point_to_index(codePoint32);
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
                            TSubOutput(std::move(outIt), std::move(outLast)), state,
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
                           TSubOutput(std::move(outIt), std::move(outLast)), state, EncodingError::OK);
          }
        }

        TDerived self {};
        return std::forward<TErrorHandler>(errorHandler)(
          self,
          TResult(TSubInput(std::move(inIt), std::move(inLast)),
                  TSubOutput(std::move(outIt), std::move(outLast)), state, EncodingError::InvalidSequence),
          Span<const code_point, 0>(), Span<const code_unit, 0>());
      }

    private:
      static_assert(
        (sizeof(code_point) * CHAR_BIT) > (TIsGBK ? 15 : 21),
        "The code point type for a GBK encoding must be at least 16 bits wide. The code point type for a "
        "GB18030 encoding must be at least 22 bits wide");
    };
  }

}
