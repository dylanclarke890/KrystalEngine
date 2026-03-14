#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/Concepts.hpp"
#include "Krystal.Text/Decode/DecodeResult.hpp"
#include "Krystal.Text/Encode/EncodeResult.hpp"
#include "Krystal.Text/EncodingId.hpp"
#include "Krystal.Text/SkipInputError.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text::Impl
{
  class UTF32Tag
  {
  };

  /// @brief An internal type meant to provide the bulk of the UTF-32 functionality.
  /// @remarks Relies on CRTP.
  template <typename TDerived = void, typename TCodeUnit = char32, typename TCodePoint = UnicodeCodePoint,
            bool ValidateDecodableAs = true, bool AllowSurrogates = false>
  class UTF32With : public UTF32Tag
  {
    static_assert((sizeof(TCodePoint) * CHAR_BIT) >= 22,
                  "The code point type for a UTF-32 encoding must be at least 22 bits wide");

  private:
    using TSelf = conditional_t<Void<TDerived>, UTF32With, TDerived>;

  public:
    using is_unicode_encoding = std::true_type;

    using self_synchronizing_code = std::true_type;

    using state = EmptyState;

    using code_unit = TCodeUnit;

    using code_point = TCodePoint;

    using is_decode_injective = std::true_type;

    using is_encode_injective = std::true_type;

    constexpr inline static std::size_t MaxCodePoints = 1;

    constexpr inline static std::size_t MaxCodeUnits = 1;

    constexpr inline static ::Krys::Text::EncodingId EncodedId =
      AllowSurrogates ? ::Krys::Text::EncodingId::ucs4 : ::Krys::Text::EncodingId::utf32;

    constexpr inline static ::Krys::Text::EncodingId DecodedId =
      AllowSurrogates ? ::Krys::Text::EncodingId::ucs4 : ::Krys::Text::EncodingId::utf32;

    template <typename TResult, typename TInputProgress, typename TOutputProgress>
    constexpr static auto SkipInputError(TResult &&result, const TInputProgress &inputProgress,
                                         const TOutputProgress &outputProgress) noexcept
    {
      if constexpr (AllowSurrogates)
      {
        return ::Krys::Text::SkipUTF32WithSurrogatesInputError(std::forward<TResult>(result), inputProgress,
                                                               outputProgress);
      }
      else
      {
        return ::Krys::Text::SkipUTF32InputError(std::forward<TResult>(result), inputProgress,
                                                 outputProgress);
      }
    }

    template <typename TInput, typename TOutput, typename TErrorHandler>
    constexpr static auto DecodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler, state &s)
    {
      using TSubInput = ::Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
      using TSubOutput = ::Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
      using TResult = DecodeResult<TSubInput, TSubOutput, state>;

      auto inIt = std::ranges::cbegin(input);
      auto inLast = std::ranges::cend(input);
      if (inIt == inLast)
      {
        // an exhausted sequence is fine
        return TResult(TSubInput(std::move(inIt), std::move(inLast)), std::forward<TOutput>(output), s,
                       EncodingError::OK);
      }

      auto outIt = std::ranges::begin(output);
      KRYS_MAYBE_UNUSED auto outLast = std::ranges::end(output);

      constexpr bool CallErrorHandler = !IsIgnorableErrorHandler<TErrorHandler>;
      if constexpr (CallErrorHandler)
      {
        if (outIt == outLast)
        {
          TSelf self {};
          return std::forward<TErrorHandler>(errorHandler)(
            self,
            TResult(TSubInput(std::move(inIt), std::move(inLast)),
                    TSubOutput(std::move(outIt), std::move(outLast)), s,
                    EncodingError::InsufficientOutputSpace),
            Span<code_unit>(), Span<code_point>());
        }
      }

      code_unit unit = static_cast<code_unit>(*inIt);
      char32 unit32 = static_cast<char32>(unit);
      if constexpr (ValidateDecodableAs && CallErrorHandler)
      {
        if (unit32 > ::Krys::Text::Unicode::LastUnicodeCodePoint
            || (!AllowSurrogates && ::Krys::Text::Unicode::IsSurrogate(static_cast<char32>(unit))))
        {
          TSelf self {};
          return std::forward<TErrorHandler>(errorHandler)(
            self,
            TResult(TSubInput(std::move(inIt), std::move(inLast)),
                    TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
            Span<code_unit>(), Span<code_point>());
        }
      }

      *outIt = unit;
      outIt++;
      inIt++;

      return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                     TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
    }

    template <typename TInput, typename TOutput, typename TErrorHandler>
    constexpr static auto EncodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler, state &s)
    {
      using TSubInput = ::Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
      using TSubOutput = ::Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
      using TResult = EncodeResult<TSubInput, TSubOutput, state>;

      auto inIt = std::ranges::cbegin(input);
      auto inLast = std::ranges::cend(input);
      if (inIt == inLast)
      {
        // an exhausted sequence is fine
        return TResult(TSubInput(::std::move(inIt), ::std::move(inLast)), ::std::forward<TOutput>(output), s,
                       EncodingError::OK);
      }

      auto outIt = std::ranges::begin(output);
      KRYS_MAYBE_UNUSED auto outLast = std::ranges::end(output);

      char32 point32 = static_cast<char32>(*inIt);

      constexpr bool CallErrorHandler = !IsIgnorableErrorHandler<TErrorHandler>;
      if constexpr (CallErrorHandler)
      {
        if (point32 > ::Krys::Text::Unicode::LastUnicodeCodePoint
            || (!AllowSurrogates && ::Krys::Text::Unicode::IsSurrogate(point32)))
        {
          TSelf self {};
          return std::forward<TErrorHandler>(errorHandler)(
            self,
            TResult(TSubInput(std::move(inIt), std::move(inLast)),
                    TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
            Span<code_point>(), Span<code_unit>());
        }

        if (outIt == outLast)
        {
          TSelf self {};
          return std::forward<TErrorHandler>(errorHandler)(
            self,
            TResult(TSubInput(std::move(inIt), std::move(inLast)),
                    TSubOutput(std::move(outIt), std::move(outLast)), s,
                    EncodingError::InsufficientOutputSpace),
            Span<code_point>(), Span<code_unit>());
        }
      }

      *outIt = static_cast<code_unit>(point32);
      outIt++;
      inIt++;

      return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                     TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
    }
  };
}

namespace Krys::Text
{
  /// @brief UTF-32 Encoding.
  template <typename TCodeUnit, typename TCodePoint = UnicodeCodePoint>
  class basic_utf32
      : public ::Krys::Text::Impl::UTF32With<basic_utf32<TCodeUnit, TCodePoint>, TCodeUnit, TCodePoint>
  {
  };

  /// @brief UTF-32 Encoding.
  using utf32_t = basic_utf32<char32>;

  /// @brief UTF-32 Encoding.
  constexpr inline utf32_t utf32 = {};

  /// @brief Wide UTF-32 Encoding.
  using wide_utf32_t = basic_utf32<wchar_t>;

  /// @brief Wide UTF-32 Encoding.
  constexpr inline wide_utf32_t wide_utf32 = {};
}
