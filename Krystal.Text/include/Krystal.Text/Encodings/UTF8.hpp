#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Utils/LosslessStaticCast.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
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
  /// @brief Internal tag for detecting a UTF-8 type.
  class utf8_tag
  {
  };

  /// @brief The UTF-8 replacement code units for U+FFFD.
  template <typename TChar>
  constexpr inline Array<TChar, 3> UTF8ReplacementUnits {
    {static_cast<TChar>(0xEF), static_cast<TChar>(0xBF), static_cast<TChar>(0xBD)}};

  /// @brief An internal type meant to provide the bulk of the UTF-8 functionality.
  /// @remarks Relies on CRTP.
  template <typename TDerived = void, typename TCodeUnit = uchar8, typename TCodePoint = UnicodeCodePoint,
            typename TDecodeState = EmptyState, typename TEncodeState = EmptyState,
            bool AllowOverlong = false, bool AllowSurrogates = false, bool UseOverlongNullOnly = false>
  class UTF8With : public utf8_tag
  {
    static_assert((sizeof(TCodePoint) * CHAR_BIT) >= 22,
                  "The code point type for a UTF-8 encoding must be at least 22 bits wide");

  private:
    using TSelf = conditional_t<Void<TDerived>, UTF8With, TDerived>;

  public:
    /// @brief Whether or not this encoding that can encode all of Unicode.
    using is_unicode_encoding = std::true_type;

    /// @brief The start of a sequence can be found unambiguously when dropped into the middle of a
    /// sequence or after an error in reading as occurred for encoded text.
    /// @remarks Unicode has definitive bit patterns which resemble start and end sequences. The bit
    /// pattern 0xxxxxxx indicates a lone bit, and 1xxxxxx indicates a potential start bit for UTF-8. In
    /// particular, if 0 is not the first bit, it must be a sequence of 1s followed immediately by a 0
    /// (e.g., 10xxxxxx, 110xxxxx, 1110xxxx, or 11110xxx).
    using self_synchronizing_code = std::true_type;

    using decode_state = TDecodeState;

    using encode_state = TEncodeState;

    using code_unit = TCodeUnit;

    using code_point = TCodePoint;

    using is_decode_injective = std::true_type;

    using is_encode_injective = std::true_type;

    constexpr inline static std::size_t MaxCodePoints = 1;

    constexpr inline static std::size_t MaxCodeUnits = AllowOverlong ? 6 : 4;

    constexpr inline static ::Krys::Text::EncodingId EncodedId =
      AllowSurrogates
        ? ::Krys::Text::EncodingId::wtf8
        : (UseOverlongNullOnly ? ::Krys::Text::EncodingId::mutf8 : ::Krys::Text::EncodingId::utf8);

    constexpr inline static Krys::Text::EncodingId DecodedId =
      AllowSurrogates ? ::Krys::Text::EncodingId::ucs4 : ::Krys::Text::EncodingId::utf32;

    constexpr static Span<const code_unit, 3> ReplacementCodeUnits() noexcept
    {
      return Span<const code_unit, 3>(UTF8ReplacementUnits<code_unit>);
    }

    constexpr static Span<const code_point, 1> ReplacementCodePoints() noexcept
    {
      return Span<const code_point, 1>(::Krys::Text::Unicode::ReplacementAsArray<const code_point>);
    }

    template <typename TInput, typename TOutput, typename TState, typename TInputProgress,
              typename TOutputProgress>
    constexpr static auto SkipInputError(DecodeResult<TInput, TOutput, TState> result,
                                         const TInputProgress &inputProgress,
                                         const TOutputProgress &outputProgress) noexcept
    {
      auto it = std::ranges::cbegin(result.Input);
      auto end = std::ranges::cend(result.Input);

      // there may be more values to skip beyond what was read.
      if (it != end)
      {
        // if there is already some items in the input progress (things irreversibly read), then
        // we are not obligated to do "at least" one skip; barrier it behind an empty check for
        // progress.
        if (std::ranges::empty(inputProgress) && std::ranges::empty(outputProgress))
        {
          ++it;
        }
        for (; it != end; ++it)
        {
          const bool foundGoodUTF8Stop =
            (UseOverlongNullOnly
               ? ::Krys::Text::Unicode::IsLeadMUTF8(static_cast<char8>(*it))
               : (AllowOverlong ? ::Krys::Text::Unicode::IsLeadOverlongUTF8(static_cast<char8>(*it))
                                : ::Krys::Text::Unicode::IsLeadUTF8(static_cast<char8>(*it))));
          if (foundGoodUTF8Stop)
          {
            break;
          }
        }
      }
      using TSubInput = ::Krys::Ranges::csubrange_for_t<TInput>;
      return DecodeResult<TSubInput, TOutput, TState>(TSubInput(std::move(it), std::move(end)),
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
    constexpr static auto EncodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler,
                                    encode_state &s)
    {
      using TSubInput = ::Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
      using TSubOutput = ::Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
      using TResult = ::Krys::Text::EncodeResult<TSubInput, TSubOutput, encode_state>;

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

      const char32 point32 = static_cast<char32>(*inIt);

      constexpr bool CallErrorHandler = !IsIgnorableErrorHandler<TErrorHandler>;
      if constexpr (CallErrorHandler)
      {
        if (point32 > ::Krys::Text::Unicode::LastUnicodeCodePoint)
        {
          TSelf self {};
          return std::forward<TErrorHandler>(errorHandler)(
            self,
            TResult(TSubInput(std::move(inIt), std::move(inLast)),
                    TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
            Span<code_point>(), Span<code_unit>());
        }
        if constexpr (!AllowSurrogates)
        {
          if (::Krys::Text::Unicode::IsSurrogate(point32))
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

      if constexpr (UseOverlongNullOnly)
      {
        if (point32 == U'\0')
        {
          // overlong MUTF-8
          constexpr code_unit payload[] = {static_cast<code_unit>(0b11'000'000u),
                                           static_cast<code_unit>(0b10'000'000u)};
          constexpr std::size_t payloadSize = 2uz;
          std::ranges::advance(inIt);
          for (std::size_t i = 0; i < payloadSize; ++i)
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
                  Span<code_point>(), Span<const code_unit>(payload + i, payloadSize - i));
              }
            }
            *outIt = static_cast<code_unit>(payload[i]);
            std::ranges::advance(outIt);
          }
          return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                         TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
        }
      }

      constexpr uchar8 firstMaskContinuationValues[][2] = {
        {0b01'111'111, ::Krys::Text::Unicode::Start1ByteContinuation},
        {0b00'011'111, ::Krys::Text::Unicode::Start2ByteContinuation},
        {0b00'001'111, ::Krys::Text::Unicode::Start3ByteContinuation},
        {0b00'000'111, ::Krys::Text::Unicode::Start4ByteContinuation},
        {0b00'000'011, ::Krys::Text::Unicode::Start5ByteContinuation},
        {0b00'000'001, ::Krys::Text::Unicode::Start6ByteContinuation},
      };

      std::size_t length = ::Krys::Text::Unicode::UTF8DecodeLength<AllowOverlong>(point32);
      std::size_t lengthIndex = static_cast<std::size_t>(length - 1);
      const auto &firstMaskContinuation = firstMaskContinuationValues[lengthIndex];
      const uchar8 &firstMask = firstMaskContinuation[0];
      const uchar8 &firstContinuation = firstMaskContinuation[1];
      std::size_t currentShift = static_cast<std::size_t>(6 * lengthIndex);
      uchar8 first = firstContinuation | static_cast<uchar8>((point32 >> currentShift) & firstMask);

      if (length == 1)
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
        *outIt = static_cast<code_unit>(first);
        outIt++;
        inIt++;
        return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                       TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
      }

      constexpr std::size_t valuesSize = 6;
      code_unit values[valuesSize] {static_cast<code_unit>(first)};

      currentShift -= 6;
      for (std::size_t i = 1; i < length; ++i)
      {
        values[i] = static_cast<code_unit>(
          ::Krys::Text::Unicode::ContinuationSignature
          | static_cast<uchar8>((point32 >> currentShift) & ::Krys::Text::Unicode::ContinuationMaskValue));
        currentShift -= 6;
      }

      inIt++;
      for (std::size_t i = 0; i < length; ++i)
      {
        if constexpr (CallErrorHandler)
        {
          if (outIt == outLast)
          {
            TSelf self {};
            Span<code_unit> codeUnitProgress(std::addressof(values[i]), length - i);
            return std::forward<TErrorHandler>(errorHandler)(
              self,
              TResult(TSubInput(std::move(inIt), std::move(inLast)),
                      TSubOutput(std::move(outIt), std::move(outLast)), s,
                      EncodingError::InsufficientOutputSpace),
              Span<code_point>(), codeUnitProgress);
          }
        }

        *outIt = values[i];
        outIt++;
      }
      return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                     TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
    }

    template <typename TInput, typename TOutput, typename TErrorHandler>
    constexpr static auto DecodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler,
                                    decode_state &s)
    {
      using TSubInput = ::Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
      using TSubOutput = ::Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
      using TResult = ::Krys::Text::DecodeResult<TSubInput, TSubOutput, decode_state>;

      auto inIt = std::ranges::cbegin(input);
      auto inLast = std::ranges::cend(input);

      if (inIt == inLast)
      {
        // the empty sequence is an OK sequence
        return TResult(TSubInput(std::move(inIt), std::move(inLast)), std::forward<TOutput>(output), s,
                       EncodingError::OK);
      }

      auto outIt = std::ranges::begin(output);
      KRYS_MAYBE_UNUSED auto outLast = std::ranges::end(output);

      const uchar8 unit0 = static_cast<uchar8>(*inIt);
      code_unit units[MaxCodeUnits] {static_cast<code_unit>(unit0)};
      std::size_t length = static_cast<std::size_t>(::Krys::Text::Unicode::UTF8SequenceLength(unit0));

      constexpr bool CallErrorHandler = !IsIgnorableErrorHandler<TErrorHandler>;
      if constexpr (CallErrorHandler)
      {
        if (length > MaxCodeUnits)
        {
          TSelf self {};
          return std::forward<TErrorHandler>(errorHandler)(
            self,
            TResult(TSubInput(std::move(inIt), std::move(inLast)),
                    TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
            Span<code_unit>(units, 1), Span<code_point>());
        }
      }

      if (length == 1)
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
        *outIt = static_cast<code_point>(unit0);
        outIt++;
        inIt++;
        return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                       TSubOutput(std::move(outIt), std::move(outLast)), s);
      }

      if constexpr (CallErrorHandler)
      {
        if constexpr (UseOverlongNullOnly)
        {
          if (static_cast<uchar8>(unit0) != static_cast<uchar8>(0xC0))
          {
            const bool isInvalidCodeUnit = ::Krys::Text::Unicode::MUTF8IsInvalid(static_cast<uchar8>(unit0));
            if (isInvalidCodeUnit || !::Krys::Text::Unicode::IsLeadUTF8(static_cast<uchar8>(unit0)))
            {
              TSelf self {};
              return std::forward<TErrorHandler>(errorHandler)(
                self,
                TResult(TSubInput(std::move(inIt), std::move(inLast)),
                        TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
                Span<code_unit>(units, 1), Span<code_point>());
            }
          }
        }
        else
        {
          const bool isInvalidCodeUnit = ::Krys::Text::Unicode::UTF8IsInvalid(static_cast<uchar8>(unit0));
          if (isInvalidCodeUnit || !::Krys::Text::Unicode::IsLeadUTF8(static_cast<uchar8>(unit0)))
          {
            TSelf self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self,
              TResult(TSubInput(std::move(inIt), std::move(inLast)),
                      TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
              Span<code_unit>(units, 1), Span<code_point>());
          }
        }
      }

      inIt++;

      for (std::size_t i = 1; i < length; ++i)
      {
        if constexpr (CallErrorHandler)
        {
          if (inIt == inLast)
          {
            TSelf self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self,
              TResult(TSubInput(std::move(inIt), std::move(inLast)),
                      TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::IncompleteSequence),
              Span<code_unit>(units, i), Span<code_point>());
          }
        }
        units[i] = ::Krys::LosslessStaticCast<code_unit>(*inIt);
        if (!::Krys::Text::Unicode::IsTrailingUTF8(static_cast<uchar8>(units[i])))
        {
          TSelf self {};
          return std::forward<TErrorHandler>(errorHandler)(
            self,
            TResult(TSubInput(std::move(inIt), std::move(inLast)),
                    TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
            Span<code_unit>(units, i + 1), Span<code_point>());
        }
        inIt++;
      }

      code_point decoded {};
      switch (length)
      {
        case 2:
          decoded =
            ::Krys::Text::Unicode::UTF8Decode(static_cast<uchar8>(units[0]), static_cast<uchar8>(units[1]));
          break;
        case 3:
          decoded = ::Krys::Text::Unicode::UTF8Decode(
            static_cast<uchar8>(units[0]), static_cast<uchar8>(units[1]), static_cast<uchar8>(units[2]));
          break;
        case 4:
          decoded =
            ::Krys::Text::Unicode::UTF8Decode(static_cast<uchar8>(units[0]), static_cast<uchar8>(units[1]),
                                              static_cast<uchar8>(units[2]), static_cast<uchar8>(units[3]));
          break;
        case 5:
          if constexpr (AllowOverlong)
          {
            decoded = ::Krys::Text::Unicode::UTF8Decode(
              static_cast<uchar8>(units[0]), static_cast<uchar8>(units[1]), static_cast<uchar8>(units[2]),
              static_cast<uchar8>(units[3]), static_cast<uchar8>(units[4]));
            break;
          }
          else
          {
            KRYS_FALLTHROUGH;
          }
        case 6:
          if constexpr (AllowOverlong)
          {
            decoded = ::Krys::Text::Unicode::UTF8Decode(
              static_cast<uchar8>(units[0]), static_cast<uchar8>(units[1]), static_cast<uchar8>(units[2]),
              static_cast<uchar8>(units[3]), static_cast<uchar8>(units[4]), static_cast<uchar8>(units[5]));
            break;
          }
          else
          {
            KRYS_FALLTHROUGH;
          }
        default:
          if constexpr (CallErrorHandler)
          {
            TSelf self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self,
              TResult(TSubInput(std::move(inIt), std::move(inLast)),
                      TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
              Span<code_unit>(units, length), Span<code_point>());
          }
          else
          {
            break;
          }
      }

      if constexpr (CallErrorHandler)
      {
        if (static_cast<char32>(decoded) > ::Krys::Text::Unicode::LastUnicodeCodePoint)
        {
          TSelf self {};
          return std::forward<TErrorHandler>(errorHandler)(
            self,
            TResult(TSubInput(std::move(inIt), std::move(inLast)),
                    TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
            Span<code_unit>(units, length), Span<code_point>());
        }
        if constexpr (!AllowOverlong)
        {
          const bool isAllowedOverlongNull = UseOverlongNullOnly ? decoded == U'\0' && length == 2 : false;
          if (isAllowedOverlongNull
              || ::Krys::Text::Unicode::UTF8IsOverlong(static_cast<char32>(decoded), length))
          {
            TSelf self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self,
              TResult(TSubInput(std::move(inIt), std::move(inLast)),
                      TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
              Span<code_unit>(units, length), Span<code_point>());
          }
        }
        if constexpr (!AllowSurrogates)
        {
          if (::Krys::Text::Unicode::IsSurrogate(static_cast<char32>(decoded)))
          {
            TSelf self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self,
              TResult(TSubInput(std::move(inIt), std::move(inLast)),
                      TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
              Span<code_unit>(units, length), Span<code_point>());
          }
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
            Span<code_unit>(units, length), Span<code_point>());
        }
      }
      // then everything is fine
      *outIt = decoded;
      outIt++;

      return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                     TSubOutput(std::move(outIt), std::move(outLast)), s);
    }
  };
}

namespace Krys::Text
{
  /// @brief A UTF-8 Encoding.
  template <typename TCodeUnit, typename TCodePoint = UnicodeCodePoint>
  class basic_utf8
      : public ::Krys::Text::Impl::UTF8With<basic_utf8<TCodeUnit, TCodePoint>, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = "UTF-8"_s;
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 6> Aliases = {
      "unicode-1-1-utf-8"_s, "unicode11utf8"_s, "unicode20utf8"_s, "utf-8"_s, "utf8"_s, "x-unicode20utf8"_s};
  };

  /// @brief A UTF-8 Encoding.
  using utf8_t = basic_utf8<uchar8>;

  /// @brief UTF-8 Encoding.
  constexpr inline utf8_t utf8 = {};

  /// @brief UTF-8 Encoding for compatibility with older codeBases.
  using compat_utf8_t = basic_utf8<char>;

  /// @brief UTF-8 Encoding for compatibility with older codeBases.
  constexpr inline compat_utf8_t compat_utf8 = {};

  /// @brief "Wobbly Transformation Format 8" (WTF-8) Encoding.
  /// @remarks This type as a maximum of 4 input code points and a maximum of 1 output code point. Unpaired
  /// surrogates are allowed in this type, which may be useful for dealing with legacy storage and
  /// implementations of the Windows Filesystem (modern Windows no longer lets non-Unicode filenames
  /// through). For a strict, Unicode-compliant UTF-8 Encoding, see basic_utf8 .
  template <typename TCodeUnit, typename TCodePoint = UnicodeCodePoint>
  class basic_wtf8 : public Impl::UTF8With<basic_wtf8<TCodeUnit, TCodePoint>, TCodeUnit, TCodePoint,
                                           EmptyState, EmptyState, false, true, false>
  {
  };

  /// @brief "Wobbly Transformation Format 8" (WTF-8) Encoding.
  using wtf8_t = basic_wtf8<uchar8>;

  /// @brief "Wobbly Transformation Format 8" (WTF-8) Encoding.
  constexpr inline wtf8_t wtf8 = {};

  /// @brief "Wobbly Transformation Format 8" (WTF-8) Encoding for compatibility with older codeBases.
  using compat_wtf8_t = basic_wtf8<char>;

  /// @brief "Wobbly Transformation Format 8" (WTF-8) Encoding for compatibility with older codeBases.
  constexpr inline compat_wtf8_t compat_wtf8 = {};

  /// @brief Modified UTF-8 Encoding.
  /// @remarks This type as a maximum of 6 input code points and a maximum of 1 output code point. Null
  /// values are encoded as an overlong sequence to specifically avoid problems with C-style strings, which
  /// is useful for working with bad implementations sitting on top of POSIX or other Operating System APIs.
  /// For a strict, Unicode-compliant UTF-8 Encoding, see basic_utf8 .
  template <typename TCodeUnit, typename TCodePoint = UnicodeCodePoint>
  class basic_mutf8 : public Impl::UTF8With<basic_mutf8<TCodeUnit, TCodePoint>, TCodeUnit, TCodePoint,
                                            EmptyState, EmptyState, true, false, true>
  {
  };

  /// @brief Modified UTF-8 Encoding.
  using mutf8_t = basic_mutf8<uchar8>;

  /// @brief Modified UTF-8 Encoding.
  constexpr inline mutf8_t mutf8 = {};

  /// @brief Modified UTF-8 Encoding for compatibility with older codeBases.
  using compat_mutf8_t = basic_mutf8<char>;

  /// @brief Modified UTF-8 Encoding for compatibility with older codeBases.
  constexpr inline compat_mutf8_t compat_mutf8 = {};
}
