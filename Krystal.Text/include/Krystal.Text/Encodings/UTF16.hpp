#pragma once

#include "Krystal.Lib/Ranges/Range.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/DecodeResult.hpp"
#include "Krystal.Text/EncodeResult.hpp"
#include "Krystal.Text/Impl/ReplacementUnits.hpp"
#include "Krystal.Text/Impl/Unicode.hpp"
#include "Krystal.Text/IsIgnorableErrorHandler.hpp"
#include "Krystal.Text/SkipInputError.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/TextEncodingId.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys
{
  namespace Impl
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
    private:
      using TSelf = conditional_t<Void<TDerived>, UTF16With, TDerived>;

    public:
      /// @brief Whether or not this encoding that can encode all of Unicode.
      using is_unicode_encoding = std::true_type;

      /// @brief The start of a sequence can be found unambiguously when dropped into the middle of a
      /// sequence or after an error in reading as occurred for encoded text.
      /// @remarks Unicode has definitive bit patterns which resemble start and end sequences ("low
      /// surrogate" and "high surrogate" for UTF-16).
      using self_synchronizing_code = std::true_type;

      /// @brief The state that can be used between calls to the encoder and decoder. It is an empty struct
      /// because there is no shift state to preserve between complete units of encoded information.
      using state = EmptyState;

      /// @brief The individual units that result from an encode operation or are used as input to a decode
      /// operation. For UTF-16 formats, this is usually char16_t, but this can change (see
      /// basic_utf16).
      using code_unit = TCodeUnit;

      /// @brief The individual units that result from a decode operation or as used as input to an encode
      /// operation. For most encodings, this is going to be a Unicode Code Point or a Unicode Scalar Value.
      using code_point = TCodePoint;

      /// @brief Whether or not the decode operation can process all forms of input into code point values.
      /// Thsi is true for all Unicode Transformation Formats (UTFs), which can encode and decode without a
      /// loss of information from a valid collection of code units.
      using is_decode_injective = std::true_type;

      /// @brief Whether or not the encode operation can process all forms of input into code unit values.
      /// This is true for all Unicode Transformation Formats (UTFs), which can encode and decode without
      /// loss of information from a valid input code point.
      using is_encode_injective = std::true_type;

      /// @brief The maximum number of code points a single complete operation of decoding can produce. This
      /// is 1 for all Unicode Transformation Format (UTF) encodings.
      inline static constexpr std::size_t MaxCodePoints = 1;

      /// @brief The maximum code units a single complete operation of encoding can produce.
      inline static constexpr std::size_t MaxCodeUnits = 2;

      ///@brief The encoding ID for this type. Used for optimization purposes.
      inline static constexpr Krys::TextEncodingId EncodedId =
        AllowSurrogates ? Krys::TextEncodingId::ucs2 : Krys::TextEncodingId::utf16;

      ///@brief The encoding ID for this type. Used for optimization purposes.
      inline static constexpr Krys::TextEncodingId DecodedId =
        AllowSurrogates ? Krys::TextEncodingId::ucs4 : Krys::TextEncodingId::utf32;

      /// @brief Returns the replacement code units to use for the replacement_handler_t error
      /// handler.
      static constexpr Span<const code_unit, 1> ReplacementCodeUnits() noexcept
      {
        return Span<const code_unit, 1>(Krys::Impl::xFFFDReplacementUnits<code_unit>);
      }

      /// @brief Returns the replacement code point to use for the replacement_handler_t error
      /// handler.
      static constexpr Span<const code_point, 1> ReplacementCodePoints() noexcept
      {
        return Span<const code_point, 1>(Krys::Impl::xFFFDReplacementUnits<code_point>);
      }

      /// @brief Allows an encoding to discard input characters if an error occurs, taking in both the
      /// state and the input sequence (by reference) to modify.
      /// @param[in] result The decode result being processed by the error handler.
      /// @param[in] inputProgress The input that has been read but not committed to consumption.
      /// @param[in] outputProgress The output that has been written but could not be committed due to an
      /// error.
      /// @remarks This will skip every input value until a proper UTF-16 starting byte (single or leading
      /// surrogate).
      template <bool TStrawman = AllowSurrogates, typename TInput, typename TOutput, typename TState,
                typename TInputProgress, typename TOutputProgress>
      requires(!TStrawman)
      static constexpr auto SkipInputError(DecodeResult<TInput, TOutput, TState> result,
                                           const TInputProgress &inputProgress,
                                           const TOutputProgress &outputProgress) noexcept
      {
        auto it = Krys::Ranges::cbegin(result.Input);
        auto last = Krys::Ranges::cend(result.Input);
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
            const auto &itValue = *it;
            const bool isSingleUTF16Unit = !Krys::Impl::Unicode::IsSingleUTF16(itValue);
            if (isSingleUTF16Unit)
            {
              break;
            }
            const bool foundGoodUTF16Stop =
              Krys::Impl::Unicode::IsLeadSurrogate(static_cast<char32>(itValue));
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
              if (Krys::Impl::Unicode::IsTrailSurrogate(static_cast<char32>(*secondSurrogate)))
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

      /// @brief Decodes a single complete unit of information as code points and produces a result with the
      /// input and output ranges moved past what was successfully read and written; or, produces an error
      /// and returns the input and output ranges untouched.
      /// @param[in] input The input view to read code uunits from.
      /// @param[in] output The output view to write code points into.
      /// @param[in] errorHandler The error handler to invoke if encoding fails.
      /// @param[in, out] s The necessary state information. For this encoding, the state is empty and
      /// means very little.
      /// @returns A DecodeResult object that contains the reconstructed input range,
      /// reconstructed output range, error handler, and a reference to the passed-in state.
      /// @remarks To the best ability of the implementation, the iterators will be returned untouched
      /// (e.g., the input models at least a view and a forward_range). If it is not possible, returned
      /// ranges may be incremented even if an error occurs due to the semantics of any view that models an
      /// input_range.
      template <typename TInput, typename TOutput, typename TErrorHandler>
      static constexpr auto DecodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler,
                                      state &s)
      {
        using TUErrorHandler = remove_cvref_t<TErrorHandler>;
        using TSubInput = Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
        using TSubOutput = Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
        using TResult = DecodeResult<TSubInput, TSubOutput, state>;
        constexpr bool CallErrorHandler = !IsIgnorableErrorHandler<TUErrorHandler>;

        auto inIt = ::Krys::Ranges::cbegin(input);
        KRYS_MAYBE_UNUSED auto inLast = ::Krys::Ranges::cend(input);
        if constexpr (CallErrorHandler)
        {
          if (inIt == inLast)
          {
            // an exhausted sequence is fine
            return TResult(TSubInput(std::move(inIt), std::move(inLast)), std::forward<TOutput>(output), s,
                           EncodingError::OK);
          }
        }

        auto outIt = ::Krys::Ranges::begin(output);
        KRYS_MAYBE_UNUSED auto outLast = ::Krys::Ranges::end(output);

        std::array<code_unit, 2> units {};
        const char16_t lead16 = static_cast<char16_t>(*inIt);
        units[0] = static_cast<code_unit>(lead16);

        if constexpr (AllowSurrogates)
        {
          // if this is a singular trailing surrogate, serialize and leave.
          if (Krys::Impl::Unicode::IsSingleUTF16(lead16) || Krys::Impl::Unicode::IsTrailSurrogate(lead16))
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
          if (Krys::Impl::Unicode::IsSingleUTF16(lead16))
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
            if (!Krys::Impl::Unicode::IsLeadSurrogate(lead16))
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
          if (!Krys::Impl::Unicode::IsTrailSurrogate(trail16))
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
          if (!Krys::Impl::Unicode::IsTrailSurrogate(trail16))
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
        char32 point32 = Krys::Impl::Unicode::UTF16CombineSurrogates(lead16, trail16);
        *outIt = static_cast<code_point>(point32);
        ::Krys::Ranges::iter_advance(outIt);
        ::Krys::Ranges::iter_advance(inIt);

        return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                       TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
      }

      /// @brief Encodes a single complete unit of information as code units and produces a result with the
      /// input and output ranges moved past what was successfully read and written; or, produces an error
      /// and returns the input and output ranges untouched.
      /// @param[in] input The input view to read code points from.
      /// @param[in] output The output view to write code units into.
      /// @param[in] errorHandler The error handler to invoke if encoding fails.
      /// @param[in, out] s The necessary state information. For this encoding, the state is empty and
      /// means very little.
      /// @returns A EncodeResult object that contains the reconstructed input range,
      /// reconstructed output range, error handler, and a reference to the passed-in state.
      /// @remarks To the best ability of the implementation, the iterators will be returned untouched
      /// (e.g., the input models at least a view and a forward_range). If it is not possible, returned
      /// ranges may be incremented even if an error occurs due to the semantics of any view that models an
      /// input_range.
      template <typename TInput, typename TOutput, typename TErrorHandler>
      static constexpr auto EncodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler,
                                      state &s)
      {
        using TUErrorHandler = remove_cvref_t<TErrorHandler>;
        using TSubInput = Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
        using TSubOutput = Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
        using TResult = EncodeResult<TSubInput, TSubOutput, state>;
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

        code_point points[1] {};
        points[0] = *inIt;
        const code_point &point = points[0];

        if constexpr (CallErrorHandler)
        {
          if (static_cast<char32>(point) > Krys::Impl::Unicode::LastUnicodeCodePoint)
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
          if (Krys::Impl::Unicode::IsSurrogate(static_cast<char32>(point)))
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
            if (Krys::Impl::Unicode::IsSurrogate(static_cast<char32>(point)))
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

        if (static_cast<char32>(point) <= Krys::Impl::Unicode::LastBMPValue)
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

        auto normal = static_cast<char32>(point) - Krys::Impl::Unicode::NormalizingValue;
        auto lead16 =
          Krys::Impl::Unicode::FirstLeadSurrogate
          + ((normal & Krys::Impl::Unicode::LeadSurrogateBitmask) >> Krys::Impl::Unicode::LeadShiftedBits);
        auto trail16 =
          Krys::Impl::Unicode::FirstTrailSurrogate + (normal & Krys::Impl::Unicode::TrailSurrogateBitmask);

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

    private:
      static_assert((sizeof(code_point) * CHAR_BIT) > 21,
                    "The code point type for a UTF-16 (or similar) encoding must be at least 22 bits wide");
    };
  }

  /// @brief A UTF-16 Encoding that traffics in, specifically, the desired code unit type provided as a
  /// template argument.
  /// @tparam TCodeUnit The code unit type to use.
  /// @tparam TCodePoint The code point type to use.
  /// @remarks This is a strict UTF-16 implementation that does not allow lone, unpaired surrogates either
  /// in or out.
  template <typename TCodeUnit, typename TCodePoint = UnicodeCodePoint>
  class basic_utf16 : public Impl::UTF16With<basic_utf16<TCodeUnit, TCodePoint>, TCodeUnit, TCodePoint>
  {
  };

  /// @brief A UTF-16 Encoding that traffics in char16_t. See basic_utf16 for more details.
  using utf16_t = basic_utf16<char16_t>;

  /// @brief An instance of the UTF-16 encoding for ease of use.
  constexpr inline utf16_t utf16 = {};

  /// @brief A UTF-16 Encoding that traffics in wchar_t. See basic_utf16 for more details.
  using wide_utf16_t = basic_utf16<wchar_t>;

  /// @brief An instance of the UTF-16 that traffics in wchar_t for ease of use.
  constexpr inline wide_utf16_t wide_utf16 = {};
}
