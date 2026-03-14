#pragma once

#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/Concepts.hpp"
#include "Krystal.Text/Decode/DecodeResult.hpp"
#include "Krystal.Text/Encode/EncodeResult.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief A no-op encoding; simply casts code units to code points and vice versa.
  template <typename TCodeUnit, typename TCodePoint>
  class basic_no_encoding
  {
  public:
    using state = EmptyState;

    using code_unit = TCodeUnit;

    using code_point = TCodePoint;

    using is_decode_injective = IntegralConstant<bool, SameType<TCodeUnit, TCodePoint>>;

    using is_encode_injective = IntegralConstant<bool, SameType<TCodeUnit, TCodePoint>>;

    constexpr inline static std::size_t MaxCodePoints = 1;

    constexpr inline static std::size_t MaxCodeUnits = 1;

    template <typename TInput, typename TOutput, typename TErrorHandler>
    constexpr static auto DecodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler, state &s)
    {
      using TSubInput = ::Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
      using TSubOutput = ::Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
      using TResult = ::Krys::Text::DecodeResult<TSubInput, TSubOutput, state>;

      auto inIt = ::std::ranges::cbegin(input);
      auto inLast = ::std::ranges::cend(input);
      if (inIt == inLast)
      {
        // an exhausted sequence is fine
        return TResult(TSubInput(std::move(inIt), std::move(inLast)), std::forward<TOutput>(output), s,
                       EncodingError::OK);
      }

      auto outIt = ::std::ranges::begin(output);
      KRYS_MAYBE_UNUSED auto outLast = ::std::ranges::end(output);

      constexpr bool CallErrorHandler = !IsIgnorableErrorHandler<TErrorHandler>;
      if constexpr (CallErrorHandler)
      {
        if (outIt == outLast)
        {
          basic_no_encoding self {};
          return std::forward<TErrorHandler>(errorHandler)(
            self,
            TResult(TSubInput(std::move(inIt), std::move(inLast)),
                    TSubOutput(std::move(outIt), std::move(outLast)), s,
                    EncodingError::InsufficientOutputSpace),
            Span<code_unit>(), Span<code_point>());
        }
      }

      code_unit unit = *inIt;
      ::Krys::Ranges::iter_advance(inIt);

      *outIt = static_cast<code_point>(unit);
      ::Krys::Ranges::iter_advance(outIt);

      return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                     TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
    }

    template <typename TInput, typename TOutput, typename TErrorHandler>
    constexpr static auto EncodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler, state &s)
    {
      using TSubInput = ::Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
      using TSubOutput = ::Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
      using TResult = ::Krys::Text::EncodeResult<TSubInput, TSubOutput, state>;

      auto inIt = ::std::ranges::cbegin(input);
      auto inLast = ::std::ranges::cend(input);
      if (inIt == inLast)
      {
        // an exhausted sequence is fine
        return TResult(TSubInput(std::move(inIt), std::move(inLast)), std::forward<TOutput>(output), s,
                       EncodingError::OK);
      }

      auto outIt = ::std::ranges::begin(output);
      KRYS_MAYBE_UNUSED auto outLast = ::std::ranges::end(output);

      constexpr bool CallErrorHandler = !IsIgnorableErrorHandler<TErrorHandler>;
      if constexpr (CallErrorHandler)
      {
        if (outIt == outLast)
        {
          basic_no_encoding self {};
          return std::forward<TErrorHandler>(errorHandler)(
            self,
            TResult(TSubInput(std::move(inIt), std::move(inLast)),
                    TSubOutput(std::move(outIt), std::move(outLast)), s,
                    EncodingError::InsufficientOutputSpace),
            Span<code_point>(), Span<code_unit>());
        }
      }

      code_point points[1] {};
      points[0] = *inIt;
      const code_point &point = points[0];
      ::Krys::Ranges::iter_advance(inIt);

      *outIt = static_cast<code_unit>(point);
      ::Krys::Ranges::iter_advance(outIt);

      return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                     TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
    }
  };

  /// @brief A do-nothing encoding for `char` types of input.
  using no_encoding_t = basic_no_encoding<char, UnicodeCodePoint>;

  /// @brief An object for the no_encoding_t type.
  constexpr inline no_encoding_t no_encoding = {};
}
