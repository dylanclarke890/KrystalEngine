#pragma once

#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Text/DecodeResult.hpp"
#include "Krystal.Text/EncodeResult.hpp"
#include "Krystal.Text/Encodings/EncodingTables/EUC-KR-UHC.hpp"
#include "Krystal.Text/Impl/ReplacementUnits.hpp"
#include "Krystal.Text/IsIgnorableErrorHandler.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys
{
  /// @brief The EUC-KR (Unified Hangul Code) encoding.
  /// @tparam TCodeUnit The default code unit type to use when outputting encoded units.
  /// @tparam TCodePoint The code point type to use when outputting decoded units.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_euc_kr_uhc
  {
  public:
    /// @brief Shift-JIS is generally stored as minimum-8-bit values in a sequence.
    using code_unit = TCodeUnit;

    /// @brief Shift-JIS outputs Unicode Scalar Values.
    using code_point = TCodePoint;

    /// @brief Shift-JIS requires no state.
    using state = EmptyState;

    /// @brief The Shift-JIS encoding can put out at most 1 codePoint point per decoding action.
    static constexpr inline std::size_t MaxCodePoints = 1;

    /// @brief The Shift-JIS encoding can put out at most 2 codePoint units per encoding action.
    static constexpr inline std::size_t MaxCodeUnits = 2;

    /// @brief A fixed-size 1-element span with the codePoint unit "?"
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
      ;
      constexpr bool CallErrorHandler = !IsIgnorableErrorHandler<TUErrorHandler>;

      auto inIt = Krys::Ranges::cbegin(input);
      auto inLast = Krys::Ranges::cend(input);

      if (inIt == inLast)
      {
        // we don't need more, so we can just say the emptiness is a-okay.
        return TResult(std::move(input), std::move(output), state, EncodingError::OK);
      }

      code_unit units[MaxCodeUnits] = {static_cast<code_unit>(*inIt)};
      uchar unit0 = static_cast<uchar>(units[0]);
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
            basic_euc_kr_uhc self {};
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
      else if ((unit0 <= 0xFE && unit0 >= 0x81))
      {
        // Top-Level case 1: this is a double-byte sequence!
        if constexpr (CallErrorHandler)
        {
          if (inIt == inLast)
          {
            basic_euc_kr_uhc self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self, TResult(std::move(input), std::move(output), state, EncodingError::IncompleteSequence),
              Span<const code_unit, 1>(std::addressof(units[0]), 1), Span<const code_point, 0>());
          }
        }
        ++inIt;
        units[1] = static_cast<code_unit>(*inIt);
        uchar secondByte = static_cast<uchar>(units[1]);
        if (secondByte <= 0xFE && secondByte >= 0x41)
        {
          const std::size_t lookupIndex = ((unit0 - 0x81) * 190) + (secondByte - 0x41);
          const std::optional<std::uint_least32_t> maybeCode =
            ::Krys::Text::EncodingTable::euc_kr_uhc_index_to_code_point(lookupIndex);
          if (maybeCode)
          {
            if constexpr (CallErrorHandler)
            {
              if (outIt == outLast)
              {
                basic_euc_kr_uhc self {};
                return std::forward<TErrorHandler>(errorHandler)(
                  self,
                  TResult(TSubInput(std::move(inIt), std::move(inLast)),
                          TSubOutput(std::move(outIt), std::move(outLast)), state,
                          EncodingError::InsufficientOutputSpace),
                  Span<const code_unit, 1>(std::addressof(units[0]), 1), Span<const code_point, 0>());
              }
            }
            const char32 &codePoint = static_cast<code_point>(*maybeCode);
            *outIt = codePoint;
            ++inIt;
            ++outIt;
            return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                           TSubOutput(std::move(outIt), std::move(outLast)), state, EncodingError::OK);
          }
        }
      }

      // Top-Level case 2 (default): unrecognized byte sequence!!
      basic_euc_kr_uhc self {};
      return std::forward<TErrorHandler>(errorHandler)(
        self,
        TResult(TSubInput(std::move(inIt), std::move(inLast)),
                TSubOutput(std::move(outIt), std::move(outLast)), state, EncodingError::InvalidSequence),
        Span<const code_unit, 1>(std::addressof(units[0]), 1), Span<const code_point, 0>());
    }

    /// @brief Encodes a single complete unit of information as codePoint units and produces a result
    /// with the input and output ranges moved past what was successfully read and written; or, produces an
    /// error and returns the input and output ranges untouched.
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

      char32_t codePoint32 = static_cast<char32_t>(*inIt);
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
            basic_euc_kr_uhc self {};
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

      std::optional<std::size_t> maybeIndex =
        ::Krys::Text::EncodingTable::euc_kr_uhc_code_point_to_index(codePoint32);
      if (maybeIndex)
      {
        const std::size_t i = *maybeIndex;
        std::size_t first = (i / 190) + 0x81;
        std::size_t second = (i % 190) + 0x41;

        if constexpr (CallErrorHandler)
        {
          if (outIt == outLast)
          {
            // output is empty :(
            basic_euc_kr_uhc self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self,
              TResult(TSubInput(std::move(inIt), std::move(inLast)),
                      TSubOutput(std::move(outIt), std::move(outLast)), state,
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
            basic_euc_kr_uhc self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self,
              TResult(TSubInput(std::move(inIt), std::move(inLast)),
                      TSubOutput(std::move(outIt), std::move(outLast)), state,
                      EncodingError::InsufficientOutputSpace),
              Span<const code_point, 0>(), Span<const code_unit, 1>(std::addressof(unit), 1));
          }
        }
        ++outIt;
        ++inIt;
        *outIt = static_cast<code_unit>(second);
        ++outIt;
        return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                       TSubOutput(std::move(outIt), std::move(outLast)), state, EncodingError::OK);
      }

      basic_euc_kr_uhc self {};
      return std::forward<TErrorHandler>(errorHandler)(
        self,
        TResult(TSubInput(std::move(inIt), std::move(inLast)),
                TSubOutput(std::move(outIt), std::move(outLast)), state, EncodingError::InvalidSequence),
        Span<const code_point, 0>(), Span<const code_unit, 0>());
    }
  };

  /// @brief An instance of euc_kr_uhc for ease of use.
  inline constexpr basic_euc_kr_uhc<char> euc_kr_uhc = {};
}
