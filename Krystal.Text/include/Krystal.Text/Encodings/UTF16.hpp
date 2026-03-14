#pragma once

#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/Concepts.hpp"
#include "Krystal.Text/Decode/DecodeResult.hpp"
#include "Krystal.Text/Encode/EncodeResult.hpp"
#include "Krystal.Text/EncodingId.hpp"
#include "Krystal.Text/SkipInputError.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/Unicode.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text::Impl
{
  class UTF16Tag
  {
  };

  /// @brief An internal type meant to provide the bulk of the UTF-16 functionality.
  /// @remarks Relies on CRTP.
  template <typename TDerived = void, typename TCodeUnit = char16_t, typename TCodePoint = UnicodeCodePoint,
            bool AllowSurrogates = false>
  class UTF16With : public UTF16Tag
  {
    static_assert((sizeof(TCodePoint) * CHAR_BIT) >= 22,
                  "The code point type for a UTF-16 encoding must be at least 22 bits wide");

  private:
    using TSelf = conditional_t<Void<TDerived>, UTF16With, TDerived>;

  public:
    using is_unicode_encoding = std::true_type;

    using self_synchronizing_code = std::true_type;

    using state = EmptyState;

    using code_unit = TCodeUnit;

    using code_point = TCodePoint;

    using is_decode_injective = std::true_type;

    using is_encode_injective = std::true_type;

    constexpr inline static std::size_t MaxCodePoints = 1;

    constexpr inline static std::size_t MaxCodeUnits = 2;

    constexpr inline static Krys::Text::EncodingId EncodedId =
      AllowSurrogates ? Krys::Text::EncodingId::ucs2 : Krys::Text::EncodingId::utf16;

    constexpr inline static Krys::Text::EncodingId DecodedId =
      AllowSurrogates ? Krys::Text::EncodingId::ucs4 : Krys::Text::EncodingId::utf32;

    constexpr static Span<const code_unit, 1> ReplacementCodeUnits() noexcept
    {
      return Span<const code_unit, 1>(::Krys::Text::Unicode::ReplacementAsArray<code_unit>);
    }

    constexpr static Span<const code_point, 1> ReplacementCodePoints() noexcept
    {
      return Span<const code_point, 1>(::Krys::Text::Unicode::ReplacementAsArray<code_point>);
    }

    template <bool TStrawman = AllowSurrogates, typename TInput, typename TOutput, typename TState,
              typename TInputProgress, typename TOutputProgress>
    requires(!TStrawman)
    constexpr static auto SkipInputError(DecodeResult<TInput, TOutput, TState> result,
                                         const TInputProgress &inputProgress,
                                         const TOutputProgress &outputProgress) noexcept
    {
      auto it = ::std::ranges::cbegin(result.Input);
      auto last = ::std::ranges::cend(result.Input);
      if (it != last)
      {
        // if there is already some items in the input progress (things irreversibly read), then
        // we are not obligated to do "at least" one skip; barrier it behind an empty check for
        // progress.
        if (::std::ranges::empty(inputProgress) && ::std::ranges::empty(outputProgress))
        {
          ++it;
        }
        for (; it != last; ++it)
        {
          const auto &itValue = *it;
          const bool isSingleUTF16Unit = !::Krys::Text::Unicode::IsSingleUTF16(itValue);
          if (isSingleUTF16Unit)
          {
            break;
          }
          const bool foundGoodUTF16Stop =
            ::Krys::Text::Unicode::IsLeadSurrogate(static_cast<char32>(itValue));
          if constexpr (::Krys::Ranges::ForwardIterator<decltype(it)>)
          {
            // if we have forward-or-better iterators, we can
            // do a search for a more appropriate sequence rather than stop
            if (it == last)
            {
              break;
            }
            auto secondSurrogate = it;
            ++secondSurrogate;
            if (::Krys::Text::Unicode::IsTrailSurrogate(static_cast<char32>(*secondSurrogate)))
            {
              break;
            }
            it = std::move(secondSurrogate);
          }
          else
          {
            // otherwise, just stop here; it's the best we can do without losing data.
            if (foundGoodUTF16Stop)
            {
              break;
            }
          }
        }
      }
      using TSubInput = ::Krys::Ranges::csubrange_for_t<TInput>;
      return ::Krys::Text::DecodeResult<TSubInput, TOutput, TState>(TSubInput(std::move(it), std::move(last)),
                                                                    std::move(result.Output), result.State,
                                                                    result.ErrorCode, result.ErrorCount);
    }

    template <typename TInput, typename TOutput, typename TState, typename TInputProgress,
              typename TOutputProgress>
    constexpr static auto SkipInputError(EncodeResult<TInput, TOutput, TState> result,
                                         const TInputProgress &inputProgress,
                                         const TOutputProgress &outputProgress) noexcept
    {
      if constexpr (AllowSurrogates)
      {
        return ::Krys::Text::SkipUTF32WithSurrogatesInputError(std::move(result), inputProgress,
                                                               outputProgress);
      }
      else
      {
        return ::Krys::Text::SkipUTF32InputError(std::move(result), inputProgress, outputProgress);
      }
    }

    template <typename TInput, typename TOutput, typename TErrorHandler>
    constexpr static auto DecodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler, state &s)
    {
      using TSubInput = ::Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
      using TSubOutput = ::Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
      using TResult = DecodeResult<TSubInput, TSubOutput, state>;

      auto inIt = ::std::ranges::cbegin(input);
      KRYS_MAYBE_UNUSED auto inLast = ::std::ranges::cend(input);

      constexpr bool CallErrorHandler = !IsIgnorableErrorHandler<TErrorHandler>;
      if constexpr (CallErrorHandler)
      {
        if (inIt == inLast)
        {
          // an exhausted sequence is fine
          return TResult(TSubInput(std::move(inIt), std::move(inLast)), std::forward<TOutput>(output), s,
                         EncodingError::OK);
        }
      }

      auto outIt = ::std::ranges::begin(output);
      KRYS_MAYBE_UNUSED auto outLast = ::std::ranges::end(output);

      std::array<code_unit, 2> units {};
      const char16_t lead16 = static_cast<char16_t>(*inIt);
      units[0] = static_cast<code_unit>(lead16);

      if constexpr (AllowSurrogates)
      {
        // if this is a singular trailing surrogate, serialize and leave.
        if (::Krys::Text::Unicode::IsSingleUTF16(lead16) || ::Krys::Text::Unicode::IsTrailSurrogate(lead16))
        {
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
          *outIt = static_cast<code_point>(lead16);
          ::Krys::Ranges::iter_advance(inIt);
          ::Krys::Ranges::iter_advance(outIt);

          return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                         TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
        }
      }
      else
      {
        if (::Krys::Text::Unicode::IsSingleUTF16(lead16))
        {
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
          *outIt = static_cast<code_point>(lead16);
          ::Krys::Ranges::iter_advance(inIt);
          ::Krys::Ranges::iter_advance(outIt);

          return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                         TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
        }
        if constexpr (CallErrorHandler)
        {
          if (!::Krys::Text::Unicode::IsLeadSurrogate(lead16))
          {
            TSelf self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self,
              TResult(TSubInput(std::move(inIt), std::move(inLast)),
                      TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
              Span<code_unit>(), Span<code_point>());
          }
        }
      }

      if constexpr (CallErrorHandler)
      {
        if (inIt == inLast)
        {
          TSelf self {};
          return std::forward<TErrorHandler>(errorHandler)(
            self,
            TResult(TSubInput(std::move(inIt), std::move(inLast)),
                    TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::IncompleteSequence),
            Span<code_unit>(), Span<code_point>());
        }
      }

      ::Krys::Ranges::iter_advance(inIt);
      const char16_t trail16 = static_cast<char16_t>(*inIt);
      units[1] = static_cast<code_unit>(trail16);
      if constexpr (AllowSurrogates)
      {
        // if this is a single surrogate followed by a not-proper value, just serialize it as-is and
        // leave it alone
        if (!::Krys::Text::Unicode::IsTrailSurrogate(trail16))
        {
          // write only the one leading unit, leave the trailing unit inside.
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
                Span<code_unit>(units.data(), 1), Span<code_point>());
            }
          }
          *outIt = static_cast<code_point>(lead16);
          ::Krys::Ranges::iter_advance(outIt);

          return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                         TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
        }
      }
      else if constexpr (CallErrorHandler)
      {
        if (!::Krys::Text::Unicode::IsTrailSurrogate(trail16))
        {
          TSelf self {};
          return std::forward<TErrorHandler>(errorHandler)(
            self,
            TResult(TSubInput(std::move(inIt), std::move(inLast)),
                    TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
            Span<code_unit>(units.data(), 1), Span<code_point>());
        }
      }

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
            Span<code_unit>(units.data(), 1), Span<code_point>());
        }
      }
      char32 point32 = ::Krys::Text::Unicode::UTF16CombineSurrogates(lead16, trail16);
      *outIt = static_cast<code_point>(point32);
      ::Krys::Ranges::iter_advance(outIt);
      ::Krys::Ranges::iter_advance(inIt);

      return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                     TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
    }

    template <typename TInput, typename TOutput, typename TErrorHandler>
    constexpr static auto EncodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler, state &s)
    {
      using TSubInput = ::Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
      using TSubOutput = ::Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
      using TResult = EncodeResult<TSubInput, TSubOutput, state>;

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

      code_point points[1] {};
      points[0] = *inIt;
      const code_point &point = points[0];

      constexpr bool CallErrorHandler = !IsIgnorableErrorHandler<TErrorHandler>;
      if constexpr (CallErrorHandler)
      {
        if (static_cast<char32>(point) > ::Krys::Text::Unicode::LastUnicodeCodePoint)
        {
          TSelf self {};
          return std::forward<TErrorHandler>(errorHandler)(
            self,
            TResult(TSubInput(std::move(inIt), std::move(inLast)),
                    TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
            Span<code_point>(), Span<code_unit>());
        }
      }

      if constexpr (AllowSurrogates)
      {
        if (::Krys::Text::Unicode::IsSurrogate(static_cast<char32>(point)))
        {
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
                Span<code_point>(), Span<code_unit>());
            }
          }

          *outIt = static_cast<code_unit>(static_cast<char16>(static_cast<char32>(point)));
          ::Krys::Ranges::iter_advance(outIt);
          ::Krys::Ranges::iter_advance(inIt);

          return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                         TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
        }
      }
      else
      {
        if constexpr (CallErrorHandler)
        {
          if (::Krys::Text::Unicode::IsSurrogate(static_cast<char32>(point)))
          {
            TSelf self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self,
              TResult(TSubInput(std::move(inIt), std::move(inLast)),
                      TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
              Span<code_point>(), Span<code_unit>());
          }
        }
      }

      if (static_cast<char32>(point) <= ::Krys::Text::Unicode::LastBMPValue)
      {
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
              Span<code_point>(), Span<code_unit>());
          }
        }

        *outIt = static_cast<code_unit>(static_cast<char16>(static_cast<char32>(point)));
        ::Krys::Ranges::iter_advance(outIt);
        ::Krys::Ranges::iter_advance(inIt);
        return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                       TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
      }

      auto normal = static_cast<char32>(point) - ::Krys::Text::Unicode::NormalizingValue;
      auto lead16 =
        ::Krys::Text::Unicode::FirstLeadSurrogate
        + ((normal & ::Krys::Text::Unicode::LeadSurrogateBitmask) >> ::Krys::Text::Unicode::LeadShiftedBits);
      auto trail16 =
        ::Krys::Text::Unicode::FirstTrailSurrogate + (normal & ::Krys::Text::Unicode::TrailSurrogateBitmask);

      code_unit lead = static_cast<code_unit>(lead16);
      code_unit trail = static_cast<code_unit>(trail16);

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
            Span<code_point>(), Span<code_unit>());
        }
      }

      ::Krys::Ranges::iter_advance(inIt);
      *outIt = lead;
      ::Krys::Ranges::iter_advance(outIt);

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
            Span<code_point>(), Span<code_unit>(std::addressof(trail), 1));
        }
      }
      *outIt = trail;
      ::Krys::Ranges::iter_advance(outIt);

      return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                     TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
    }
  };
}

namespace Krys::Text
{
  /// @brief UTF-16 Encoding.
  template <typename TCodeUnit, typename TCodePoint = UnicodeCodePoint>
  class basic_utf16
      : public ::Krys::Text::Impl::UTF16With<basic_utf16<TCodeUnit, TCodePoint>, TCodeUnit, TCodePoint>
  {
  };

  /// @brief UTF-16 Encoding.
  using utf16_t = basic_utf16<char16_t>;

  /// @brief UTF-16 Encoding.
  constexpr inline utf16_t utf16 = {};

  /// @brief Wide UTF-16 Encoding.
  using wide_utf16_t = basic_utf16<wchar_t>;

  /// @brief Wide UTF-16 Encoding.
  constexpr inline wide_utf16_t wide_utf16 = {};
}
