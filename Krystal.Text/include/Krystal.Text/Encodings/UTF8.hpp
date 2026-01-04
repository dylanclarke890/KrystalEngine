#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Lib/Ranges/Range.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Utils/Size.hpp"
#include "Krystal.Text/DecodeResult.hpp"
#include "Krystal.Text/EncodeResult.hpp"
#include "Krystal.Text/Impl/Cast.hpp"
#include "Krystal.Text/Impl/ReplacementUnits.hpp"
#include "Krystal.Text/Impl/Unicode.hpp"
#include "Krystal.Text/IsIgnorableErrorHandler.hpp"
#include "Krystal.Text/SkipInputError.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/TextEncodingId.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"
#include <array>

namespace Krys
{
  namespace Impl
  {
    /// @brief Internal tag for detecting a UTF-8 type.
    class utf8_tag
    {
    };

    /// @brief An internal type meant to provide the bulk of the UTF-8 functionality.
    /// @remarks Relies on CRTP.
    template <typename TDerived = void, typename TCodeUnit = uchar8, typename TCodePoint = UnicodeCodePoint,
              typename TDecodeState = EmptyState, typename TEncodeState = EmptyState,
              bool AllowOverlong = false, bool AllowSurrogates = false, bool UseOverlongNullOnly = false>
    class UTF8With : public utf8_tag
    {
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

      /// @brief The state that can be used between calls to the encoder and decoder. It is normally an
      /// empty struct because there is no shift state to preserve between complete units of encoded
      /// information.
      using decode_state = TDecodeState;

      /// @brief The state that can be used between calls to the encoder and decoder. It is normally an
      /// empty struct because there is no shift state to preserve between complete units of encoded
      /// information.
      using encode_state = TEncodeState;

      /// @brief The individual units that result from an encode operation or are used as input to a
      /// decode operation. For UTF-8 formats, this is usually char8_t, but this can change (see
      /// basic_utf8).
      using code_unit = TCodeUnit;

      /// @brief The individual units that result from a decode operation or as used as input to an encode
      /// operation. For most encodings, this is going to be a Unicode Code Point or a Unicode Scalar
      /// Value.
      using code_point = TCodePoint;

      /// @brief Whether or not the decode operation can process all forms of input into code point
      /// values. Thsi is true for all Unicode Transformation Formats (UTFs), which can encode and decode
      /// without a loss of information from a valid collection of code units.
      using is_decode_injective = std::true_type;

      /// @brief Whether or not the encode operation can process all forms of input into code unit values.
      /// This is true for all Unicode Transformation Formats (UTFs), which can encode and decode without
      /// loss of information from a valid input code point.
      using is_encode_injective = std::true_type;

      /// @brief The maximum number of code points a single complete operation of decoding can produce.
      /// This is 1 for all Unicode Transformation Format (UTF) encodings.
      inline static constexpr std::size_t MaxCodePoints = 1;

      /// @brief The maximum code units a single complete operation of encoding can produce. If overlong
      /// sequence allowed, this is 6: otherwise, this is 4.
      inline static constexpr std::size_t MaxCodeUnits = AllowOverlong ? 6 : 4;

      ///@brief The encoding ID for this type. Used for optimization purposes.
      inline static constexpr Krys::TextEncodingId EncodedId =
        AllowSurrogates ? Krys::TextEncodingId::wtf8
                        : (UseOverlongNullOnly ? Krys::TextEncodingId::mutf8 : Krys::TextEncodingId::utf8);

      ///@brief The encoding ID for this type. Used for optimization purposes.
      inline static constexpr Krys::TextEncodingId DecodedId =
        AllowSurrogates ? Krys::TextEncodingId::ucs4 : Krys::TextEncodingId::utf32;

      /// @brief Returns the replacement code units to use for the replacement_handler_t error
      /// handler.
      static constexpr Span<const code_unit, 3> ReplacementCodeUnits() noexcept
      {
        return Span<const code_unit, 3>(Krys::Impl::xFFFDUTF8ReplacementUnits<code_unit>);
      }

      /// @brief Returns the replacement code point to use for the replacement_handler_t error
      /// handler.
      static constexpr Span<const code_point, 1> ReplacementCodePoints() noexcept
      {
        return Span<const code_point, 1>(Krys::Impl::xFFFDReplacementUnits<code_point>);
      }

      /// @brief Allows an encoding to discard input characters if an error occurs, taking in both the
      /// state and the input sequence to modify through the result type.
      /// @remarks This will skip every input value until a proper starting byte is found.
      template <typename TInput, typename TOutput, typename TState, typename TInputProgress,
                typename TOutputProgress>
      static constexpr auto SkipInputError(DecodeResult<TInput, TOutput, TState> result,
                                           const TInputProgress &inputProgress,
                                           const TOutputProgress &outputProgress) noexcept
      {
        auto it = Krys::Ranges::cbegin(result.Input);
        auto last = Krys::Ranges::cend(result.Input);
        // there may be more values to skip beyond what was read.
        if (it != last)
        {
          // if there is already some items in the input progress (things irreversibly read), then
          // we are not obligated to do "at least" one skip; barrier it behind an empty check for
          // progress.
          if (::Krys::Ranges::empty(inputProgress) && ::Krys::Ranges::empty(outputProgress))
          {
            ++it;
          }
          for (; it != last; ++it)
          {
            const bool foundGoodUTF8Stop =
              (UseOverlongNullOnly
                 ? Krys::Impl::Unicode::IsLeadMUTF8(static_cast<char8>(*it))
                 : (AllowOverlong ? Krys::Impl::Unicode::IsLeadOverlongUTF8(static_cast<char8>(*it))
                                  : Krys::Impl::Unicode::IsLeadUTF8(static_cast<char8>(*it))));
            if (foundGoodUTF8Stop)
            {
              break;
            }
          }
        }
        using TSubInput = Krys::Ranges::csubrange_for_t<TInput>;
        return DecodeResult<TSubInput, TOutput, TState>(TSubInput(std::move(it), std::move(last)),
                                                        std::move(result.Output), result.State,
                                                        result.ErrorCode, result.ErrorCount);
      }

      /// @brief Allows an encoding to discard input characters if an error occurs, taking in both the
      /// state and the input sequence (by reference) to modify.
      /// @remarks This will skip every input value until a proper UTF-32 unicode scalar value (or code
      /// point) is found.
      template <typename TInput, typename TOutput, typename TState, typename TInputProgress,
                typename TOutputProgress>
      static constexpr auto SkipInputError(EncodeResult<TInput, TOutput, TState> result,
                                           const TInputProgress &inputProgress,
                                           const TOutputProgress &outputProgress) noexcept
      {
        if constexpr (AllowSurrogates)
        {
          return Krys::SkipUTF32WithSurrogatesInputError(std::move(result), inputProgress, outputProgress);
        }
        else
        {
          return Krys::SkipUTF32InputError(std::move(result), inputProgress, outputProgress);
        }
      }

      /// @brief Encodes a single complete unit of information as code units and produces a result with
      /// the input and output ranges moved past what was successfully read and written; or, produces an
      /// error and returns the input and output ranges untouched.
      /// @param[in] input The input view to read code points from.
      /// @param[in] output The output view to write code units into.
      /// @param[in] errorHandler The error handler to invoke if encoding fails.
      /// @param[in, out] s The necessary state information. For this encoding, the state is empty and
      /// means very little.
      /// @returns A EncodeResult object that contains the reconstructed input range,
      /// reconstructed output range, error handler, and a reference to the passed-in state.
      /// @remarks To the best ability of the implementation, the iterators will be returned untouched
      /// (e.g., the input models at least a view and a forward_range). If it is not possible, returned
      /// ranges may be incremented even if an error occurs due to the semantics of any view that models
      /// an input_range.
      template <typename TInput, typename TOutput, typename TErrorHandler>
      static constexpr auto EncodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler,
                                      encode_state &s)
      {
        using TUErrorHandler = remove_cvref_t<TErrorHandler>;
        using TSubInput = Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
        using TSubOutput = Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
        using TResult = EncodeResult<TSubInput, TSubOutput, encode_state>;
        constexpr bool CallErrorHandler = !IsIgnorableErrorHandler<TUErrorHandler>;

        auto inIt = ::Krys::Ranges::cbegin(input);
        auto inLast = ::Krys::Ranges::cend(input);

        if (inIt == inLast)
        {
          // an exhausted sequence is fine
          return TResult(TSubInput(std::move(inIt), std::move(inLast)), std::forward<TOutput>(output), s,
                         EncodingError::OK);
        }

        auto outIt = ::Krys::Ranges::begin(output);
        KRYS_MAYBE_UNUSED auto outLast = ::Krys::Ranges::end(output);

        const char32 point32 = static_cast<char32>(*inIt);

        if constexpr (CallErrorHandler)
        {
          if (point32 > Krys::Impl::Unicode::LastUnicodeCodePoint)
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
            if (Krys::Impl::Unicode::IsSurrogate(point32))
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
            ::Krys::Ranges::iter_advance(inIt);
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
              ::Krys::Ranges::iter_advance(outIt);
            }
            return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                           TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
          }
        }

        constexpr uchar8 firstMaskContinuationValues[][2] = {
          {0b01'111'111, Krys::Impl::Unicode::Start1ByteContinuation},
          {0b00'011'111, Krys::Impl::Unicode::Start2ByteContinuation},
          {0b00'001'111, Krys::Impl::Unicode::Start3ByteContinuation},
          {0b00'000'111, Krys::Impl::Unicode::Start4ByteContinuation},
          {0b00'000'011, Krys::Impl::Unicode::Start5ByteContinuation},
          {0b00'000'001, Krys::Impl::Unicode::Start6ByteContinuation},
        };

        std::size_t length = Krys::Impl::Unicode::UTF8DecodeLength<AllowOverlong>(point32);
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
          ::Krys::Ranges::iter_advance(outIt);
          ::Krys::Ranges::iter_advance(inIt);
          return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                         TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
        }

        constexpr std::size_t valuesSize = 6;
        code_unit values[valuesSize] {static_cast<code_unit>(first)};

        currentShift -= 6;
        for (std::size_t i = 1; i < length; ++i)
        {
          values[i] = static_cast<code_unit>(
            Krys::Impl::Unicode::ContinuationSignature
            | static_cast<uchar8>((point32 >> currentShift) & Krys::Impl::Unicode::ContinuationMaskValue));
          currentShift -= 6;
        }

        ::Krys::Ranges::iter_advance(inIt);
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
          ::Krys::Ranges::iter_advance(outIt);
        }
        return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                       TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
      }

      /// @brief Decodes a single complete unit of information as code points and produces a result with
      /// the input and output ranges moved past what was successfully read and written; or, produces an
      /// error and returns the input and output ranges untouched.
      /// @param[in] input The input view to read code uunits from.
      /// @param[in] output The output view to write code points into.
      /// @param[in] errorHandler The error handler to invoke if encoding fails.
      /// @param[in, out] s The necessary state information. For this encoding, the state is empty and
      /// means very little.
      /// @returns A DecodeResult object that contains the reconstructed input range,
      /// reconstructed output range, error handler, and a reference to the passed-in state.
      /// @remarks To the best ability of the implementation, the iterators will be returned untouched
      /// (e.g., the input models at least a view and a forward_range). If it is not possible, returned
      /// ranges may be incremented even if an error occurs due to the semantics of any view that models
      /// an input_range.
      template <typename TInput, typename TOutput, typename TErrorHandler>
      static constexpr auto DecodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler,
                                      decode_state &s)
      {
        using TUErrorHandler = remove_cvref_t<TErrorHandler>;
        using TSubInput = Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
        using TSubOutput = Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
        using TResult = DecodeResult<TSubInput, TSubOutput, decode_state>;
        constexpr bool CallErrorHandler = !IsIgnorableErrorHandler<TUErrorHandler>;

        auto inIt = ::Krys::Ranges::cbegin(input);
        auto inLast = ::Krys::Ranges::cend(input);

        if (inIt == inLast)
        {
          // the empty sequence is an OK sequence
          return TResult(TSubInput(std::move(inIt), std::move(inLast)), std::forward<TOutput>(output), s,
                         EncodingError::OK);
        }

        auto outIt = ::Krys::Ranges::begin(output);
        KRYS_MAYBE_UNUSED auto outLast = ::Krys::Ranges::end(output);

        const uchar8 unit0 = static_cast<uchar8>(*inIt);
        code_unit units[MaxCodeUnits] {static_cast<code_unit>(unit0)};
        std::size_t length = static_cast<std::size_t>(Krys::Impl::Unicode::UTF8SequenceLength(unit0));

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
          ::Krys::Ranges::iter_advance(outIt);
          ::Krys::Ranges::iter_advance(inIt);
          return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                         TSubOutput(std::move(outIt), std::move(outLast)), s);
        }

        if constexpr (CallErrorHandler)
        {
          if constexpr (UseOverlongNullOnly)
          {
            if (static_cast<uchar8>(unit0) != static_cast<uchar8>(0xC0))
            {
              const bool isInvalidCodeUnit = Krys::Impl::Unicode::MUTF8IsInvalid(static_cast<uchar8>(unit0));
              if (isInvalidCodeUnit || !Krys::Impl::Unicode::IsLeadUTF8(static_cast<uchar8>(unit0)))
              {
                TSelf self {};
                return std::forward<TErrorHandler>(errorHandler)(
                  self,
                  TResult(TSubInput(std::move(inIt), std::move(inLast)),
                          TSubOutput(std::move(outIt), std::move(outLast)), s,
                          EncodingError::InvalidSequence),
                  Span<code_unit>(units, 1), Span<code_point>());
              }
            }
          }
          else
          {
            const bool isInvalidCodeUnit = Krys::Impl::Unicode::UTF8IsInvalid(static_cast<uchar8>(unit0));
            if (isInvalidCodeUnit || !Krys::Impl::Unicode::IsLeadUTF8(static_cast<uchar8>(unit0)))
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

        ::Krys::Ranges::iter_advance(inIt);

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
                        TSubOutput(std::move(outIt), std::move(outLast)), s,
                        EncodingError::IncompleteSequence),
                Span<code_unit>(units, i), Span<code_point>());
            }
          }
          units[i] = Impl::StaticCastIfLossless<code_unit>(*inIt);
          if (!Krys::Impl::Unicode::IsTrailingUTF8(static_cast<uchar8>(units[i])))
          {
            TSelf self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self,
              TResult(TSubInput(std::move(inIt), std::move(inLast)),
                      TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
              Span<code_unit>(units, i + 1), Span<code_point>());
          }
          ::Krys::Ranges::iter_advance(inIt);
        }

        code_point decoded {};
        switch (length)
        {
          case 2:
            decoded =
              Krys::Impl::Unicode::UTF8Decode(static_cast<uchar8>(units[0]), static_cast<uchar8>(units[1]));
            break;
          case 3:
            decoded = Krys::Impl::Unicode::UTF8Decode(
              static_cast<uchar8>(units[0]), static_cast<uchar8>(units[1]), static_cast<uchar8>(units[2]));
            break;
          case 4:
            decoded =
              Krys::Impl::Unicode::UTF8Decode(static_cast<uchar8>(units[0]), static_cast<uchar8>(units[1]),
                                              static_cast<uchar8>(units[2]), static_cast<uchar8>(units[3]));
            break;
          case 5:
            if constexpr (AllowOverlong)
            {
              decoded = Krys::Impl::Unicode::UTF8Decode(
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
              decoded = Krys::Impl::Unicode::UTF8Decode(
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
          if (static_cast<char32>(decoded) > Krys::Impl::Unicode::LastUnicodeCodePoint)
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
                || Krys::Impl::Unicode::UTF8IsOverlong(static_cast<char32>(decoded), length))
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
            if (Krys::Impl::Unicode::IsSurrogate(static_cast<char32>(decoded)))
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
        ::Krys::Ranges::iter_advance(outIt);

        return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                       TSubOutput(std::move(outIt), std::move(outLast)), s);
      }

    private:
      static_assert((sizeof(code_point) * CHAR_BIT) > 21,
                    "The code point type for a UTF-8 (or similar) encoding must be at least 22 bits wide");
    };
  }

  /// @brief A UTF-8 Encoding that traffics in, specifically, the desired code unit type provided as a
  /// template argument.
  /// @tparam TCodeUnit The code unit type to use.
  /// @tparam TCodePoint The code point type to use.
  /// @remarks This type as a maximum of 4 input code points and a maximum of 1 output code point. It
  /// strictly follows the Unicode Specification for allowed conversions. For overlong sequences (e.g.,
  /// similar to Android or Java UTF-8 implementations) and other quirks, see basic_mutf8 or
  /// basic_wtf8 .
  template <typename TCodeUnit, typename TCodePoint = UnicodeCodePoint>
  class basic_utf8 : public Impl::UTF8With<basic_utf8<TCodeUnit, TCodePoint>, TCodeUnit, TCodePoint>
  {
  };

  /// @brief A UTF-8 Encoding that traffics in uchar8. See basic_utf8 for more details.
  using utf8_t = basic_utf8<uchar8>;

  /// @brief An instance of the UTF-8 encoding for ease of use.
  constexpr inline utf8_t utf8 = {};

  /// @brief A UTF-8 Encoding that traffics in char, for compatibility purposes with older codebases. See
  /// basic_utf8 for more details.
  using compat_utf8_t = basic_utf8<char>;

  /// @brief An instance of the compatibility UTF-8 encoding for ease of use.
  constexpr inline compat_utf8_t compat_utf8 = {};

  /// @brief A "Wobbly Transformation Format 8" (WTF-8) Encoding that traffics in, specifically, the desired
  /// code unit type provided as a template argument.
  /// @tparam TCodeUnit The code unit type to use.
  /// @tparam TCodePoint The code point type to use.
  /// @remarks This type as a maximum of 4 input code points and a maximum of 1 output code point. Unpaired
  /// surrogates are allowed in this type, which may be useful for dealing with legacy storage and
  /// implementations of the Windows Filesystem (modern Windows no longer lets non-Unicode filenames
  /// through). For a strict, Unicode-compliant UTF-8 Encoding, see basic_utf8 .
  template <typename TCodeUnit, typename TCodePoint = UnicodeCodePoint>
  class basic_wtf8 : public Impl::UTF8With<basic_wtf8<TCodeUnit, TCodePoint>, TCodeUnit, TCodePoint,
                                           EmptyState, EmptyState, false, true, false>
  {
  };

  /// @brief A "Wobbly Transformation Format 8" (WTF-8) Encoding that traffics in `char8_t`. See
  /// basic_wtf8 for more details.
  using wtf8_t = basic_wtf8<uchar8>;

  /// @brief An instance of the WTF-8 type for ease of use.
  constexpr inline wtf8_t wtf8 = {};

  /// @brief A "Wobbly Transformation Format 8" (WTF-8) Encoding that traffics in `char` for compatibility
  /// purposes. See basic_wtf8 for more details.
  using compat_wtf8_t = basic_wtf8<char>;

  /// @brief An instance of the Compatibility WTF-8 type for ease of use.
  constexpr inline compat_wtf8_t compat_wtf8 = {};

  /// @brief A Modified UTF-8 Encoding that traffics in, specifically, the desired code unit type provided
  /// as a template argument.
  /// @tparam TCodeUnit The code unit type to use.
  /// @tparam TCodePoint The code point type to use.
  /// @remarks This type as a maximum of 6 input code points and a maximum of 1 output code point. Null
  /// values are encoded as an overlong sequence to specifically avoid problems with C-style strings, which
  /// is useful for working with bad implementations sitting on top of POSIX or other Operating System APIs.
  /// For a strict, Unicode-compliant UTF-8 Encoding, see basic_utf8 .
  template <typename TCodeUnit, typename TCodePoint = UnicodeCodePoint>
  class basic_mutf8 : public Impl::UTF8With<basic_mutf8<TCodeUnit, TCodePoint>, TCodeUnit, TCodePoint,
                                            EmptyState, EmptyState, true, false, true>
  {
  };

  /// @brief A Modified UTF-8 Encoding that traffics in char8_t. See basic_mutf8 for more
  /// details.
  using mutf8_t = basic_mutf8<uchar8>;

  /// @brief An instance of the MUTF-8 type for ease of use.
  constexpr inline mutf8_t mutf8 = {};

  /// @brief A Modified UTF-8 Encoding that traffics in char8_t. See basic_mutf8 for more
  /// details.
  using compat_mutf8_t = basic_mutf8<char>;

  /// @brief An instance of the MUTF-8 type for ease of use.
  constexpr inline compat_mutf8_t compat_mutf8 = {};
}
