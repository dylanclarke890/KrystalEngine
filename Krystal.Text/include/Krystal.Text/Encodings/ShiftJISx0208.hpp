#pragma once

#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Text/DecodeResult.hpp"
#include "Krystal.Text/EncodeResult.hpp"
#include "Krystal.Text/Encodings/EncodingTables/ShiftJISx0208.tables.hpp"
#include "Krystal.Text/Impl/ReplacementUnits.hpp"
#include "Krystal.Text/IsIgnorableErrorHandler.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys
{
  /// @brief The Shift-JIS Encoding (with extensions x0208) for use with most Shift-JIS applications.
  /// Identical version of what is a part of the WHATWG encoding standard for Shift-JIS.
  /// @tparam TCodeUnit The code unit type to use.
  /// @tparam TCodePoint The code point type to use.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_shift_jis_x0208
  {
  public:
    /// @brief Shift-JIS is generally stored as minimum-8-bit values in a sequence.
    using code_unit = TCodeUnit;

    /// @brief Shift-JIS outputs Unicode Scalar Values.
    using code_point = TCodePoint;

    /// @brief Shift-JIS requires no state.
    using state = EmptyState;

    /// @brief The Shift-JIS encoding can put out at most 1 codepoint point per decoding action.
    static constexpr inline std::size_t MaxCodePoints = 1;

    /// @brief The Shift-JIS encoding can put out at most 2 codePoint units per encoding action.
    static constexpr inline std::size_t MaxCodeUnits = 2;

    /// @brief A fixed-size 1-element span with the codepoint unit "?"
    constexpr Span<const code_unit, 1> ReplacementCodeUnits() const noexcept
    {
      return Span<const code_unit, 1>(Impl::QuestionMarkReplacementUnits<code_unit>.data(),
                                      Impl::QuestionMarkReplacementUnits<code_unit>.size());
    }

    /// @brief Marks this encoding as injective for the decode portion of its encoding actions.
    using is_decode_injective = std::true_type;

    /// @brief Decodes a single complete unit of information as codePoint points and produces a result
    /// with the input and output ranges moved past what was successfully read and written; or, produces an
    /// error and returns the input and output ranges untouched.
    /// @param[in] input The input view to read codePoint uunits from.
    /// @param[in] output The output view to write codePoint points into.
    /// @param[in] errorHandler The error handler to invoke if encoding fails.
    /// @param[in, out] state The necessary state information. For this encoding, the state is empty and
    /// means very little.
    /// @returns A DecodeResult object that contains the input range, output range, error
    /// handler, and a reference to the passed-in state.
    /// @remarks To the best ability of the implementation, the iterators will be returned untouched (e.g.,
    /// the input models at least a view and a forward_range). If it is not possible, returned ranges may be
    /// incremented even if an error occurs due to the semantics of any view that models an input_range.
    template <typename TInput, typename TOutput, typename TErrorHandler>
    static constexpr auto DecodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler,
                                    state &s) noexcept
    {
      using TUErrorHandler = remove_cvref_t<TErrorHandler>;
      using TSubInput = Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
      using TSubOutput = Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
      using TResult = DecodeResult<TSubInput, TSubOutput, s>;
      ;
      constexpr bool CallErrorHandler = !IsIgnorableErrorHandler<TUErrorHandler>;

      auto inIt = Krys::Ranges::cbegin(input);
      auto inLast = Krys::Ranges::cend(input);

      if (inIt == inLast)
      {
        // we don't need more, so we can just say the emptiness is a-okay.
        return TResult(std::move(input), std::move(output), s, EncodingError::OK);
      }

      code_unit units[MaxCodeUnits] = {static_cast<code_unit>(*inIt)};
      uchar unit0 = static_cast<uchar>(units[0]);
      auto outIt = Krys::Ranges::begin(output);
      auto outLast = Krys::Ranges::end(output);

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

    /// @brief Encodes a single complete unit of information as codePoint units and produces a result
    /// with the input and output ranges moved past what was successfully read and written; or, produces an
    /// error and returns the input and output ranges untouched.
    ///
    /// @param[in] input The input view to read codePoint points from.
    /// @param[in] output The output view to write codePoint units into.
    /// @param[in] errorHandler The error handler to invoke if encoding fails.
    /// @param[in, out] state The necessary state information. For this encoding, the state is empty and
    /// means very little.
    ///
    /// @returns A EncodeResult object that contains the input range, output range, error
    /// handler, and a reference to the passed-in state\.
    ///
    /// @remarks To the best ability of the implementation, the iterators will be returned untouched (e.g.,
    /// the input models at least a view and a forward_range). If it is not possible, returned ranges may be
    /// incremented even if an error occurs due to the semantics of any view that models an input_range.
    template <typename TInput, typename TOutput, typename TErrorHandler>
    static constexpr auto EncodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler,
                                    state &s) noexcept
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
        return TResult(std::move(input), std::move(output), s, EncodingError::OK);
      }

      char32 codePoint32 = static_cast<char32>(*inIt);
      code_point codePoint = static_cast<code_point>(codePoint32);
      auto outIt = Krys::Ranges::begin(output);
      auto outLast = Krys::Ranges::end(output);

      if (codePoint32 <= 0x80)
      {
        if constexpr (CallErrorHandler)
        {
          if (outIt == outLast)
          {
            // output is empty :(
            basic_shift_jis_x0208 self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self,
              TResult(std::move(input), std::move(output), s, EncodingError::InsufficientOutputSpace),
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

  /// @brief A convenience alais that defaults `basic_shift_jis` to the x0208 version.
  template <typename TCodeUnit, typename TCodePoint = UnicodeCodePoint>
  using basic_shift_jis = basic_shift_jis_x0208<TCodeUnit, TCodePoint>;

  /// @brief An instance of basic_shift_jis_x0208 for ease of use.
  constexpr inline basic_shift_jis_x0208<char> shift_jis_x0208 = {};

  /// @brief A convenience alais that defaults `shift_jis` to the x0208 version.
  using shift_jis_t = basic_shift_jis_x0208<char>;

  /// @brief An instance of basic_shift_jis for ease of use.
  constexpr inline auto &shift_jis = shift_jis_x0208;
}
