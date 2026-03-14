#pragma once

#include "Krystal.Lib/Ranges/TypeTraits.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Text/Concepts.hpp"
#include "Krystal.Text/Decode/DecodeResult.hpp"
#include "Krystal.Text/Encode/EncodeResult.hpp"
#include "Krystal.Text/Encodings/EncodingTables/Utils/TableTypes.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  template <typename TDerived, EncodingTable::basic_lookup_index_to_code_point_function *TLookupCodePoint,
            EncodingTable::basic_lookup_code_point_to_index_function *TLookupIndex, typename TCodeUnit = char,
            typename TCodePoint = UnicodeCodePoint, typename TState = EmptyState>
  struct SingleByteLookupEncoding
  {
    using code_unit = TCodeUnit;

    using code_point = TCodePoint;

    using state = TState;

    using is_decode_injective = std::true_type;

    constexpr static inline size_t MaxCodePoints = 1;

    constexpr static inline size_t MaxCodeUnits = 1;

    constexpr Span<const code_unit, 1> ReplacementCodeUnits() const noexcept
    {
      return Span<const code_unit, 1>(Unicode::ASCIIReplacement<code_unit>.data(),
                                      Unicode::ASCIIReplacement<code_unit>.size());
    }

    template <typename TInput, typename TOutput, typename TErrorHandler>
    constexpr static auto DecodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler,
                                    state &s) noexcept
    {
      using TSubInput = ::Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
      using TSubOutput = ::Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
      using TResult = ::Krys::Text::DecodeResult<TSubInput, TSubOutput, state>;

      auto inIt = std::ranges::cbegin(input);
      auto inLast = std::ranges::cend(input);

      if (inIt == inLast)
      {
        // we don't need more, so we can just say the emptiness is a-okay.
        return TResult(std::move(input), std::move(output), s, EncodingError::OK);
      }

      code_unit units[MaxCodeUnits] = {static_cast<code_unit>(*inIt)};
      uchar unit0 = static_cast<uchar>(units[0]);
      auto outIt = std::ranges::begin(output);
      auto outLast = std::ranges::end(output);

      std::size_t lookupIndex = static_cast<std::size_t>(unit0);
      const std::optional<std::uint_least32_t> maybeCode = TLookupCodePoint(lookupIndex);
      constexpr bool CallErrorHandler = !IsIgnorableErrorHandler<TErrorHandler>;
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
              Span<const code_unit, 1>(std::addressof(units[0]), 1), Span<const code_point, 0>());
          }
        }
        const code_point codePoint = static_cast<code_point>(*maybeCode);
        *outIt = codePoint;
        ++inIt;
        ++outIt;
        return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                       TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
      }

      TDerived self {};
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
      using TResult = EncodeResult<TSubInput, TSubOutput, state>;

      auto inIt = std::ranges::cbegin(input);
      auto inLast = std::ranges::cend(input);

      if (inIt == inLast)
      {
        // we don't need more, so we can just say the emptiness is a-okay.
        return TResult(std::move(input), std::move(output), s, EncodingError::OK);
      }

      char32 codePoint32 = static_cast<char32>(*inIt);
      auto outIt = std::ranges::begin(output);
      auto outLast = std::ranges::end(output);

      constexpr bool CallErrorHandler = !IsIgnorableErrorHandler<TErrorHandler>;
      const std::optional<std::size_t> maybeIndex = TLookupIndex(codePoint32);
      if (maybeIndex)
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
        const code_unit codeUnit = static_cast<code_unit>(*maybeIndex + 0x80);
        *outIt = codeUnit;
        ++inIt;
        ++outIt;
        return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                       TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
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
