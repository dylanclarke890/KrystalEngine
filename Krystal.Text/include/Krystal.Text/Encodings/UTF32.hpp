#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Ranges/Range.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/DecodeResult.hpp"
#include "Krystal.Text/EncodeResult.hpp"
#include "Krystal.Text/IsIgnorableErrorHandler.hpp"
#include "Krystal.Text/SkipInputError.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/TextEncodingId.hpp"

namespace Krys
{
  namespace Impl
  {
    class UTF32Tag
    {
    };

    /// @brief An internal type meant to provide the bulk of the UTF-32 functionality.
    /// @remarks Relies on CRTP.
    template <typename TDerived = void, typename TCodeUnit = char32, typename TCodePoint = UnicodeCodePoint,
              bool TValidateDecodableAs = true, bool surrogatesAllowed = false>
    class UTF32With : public UTF32Tag
    {
    private:
      using TSelf = conditional_t<Void<TDerived>, UTF32With, TDerived>;

    public:
      /// @brief Whether or not this encoding that can encode all of Unicode.
      using IsUnicodeEncoding = std::true_type;

      /// @brief The start of a sequence can be found unambiguously when dropped into the middle of a
      /// sequence or after an error in reading as occurred for encoded text.
      /// @remarks Unicode has definitive bit patterns which resemble start and end sequences. For UTF-32,
      /// there is only 1 code point per fully encoded character.
      using self_synchronizing_code = std::true_type;

      /// @brief The state that can be used between calls to the encoder and decoder. It is an empty struct
      /// because there is no shift state to preserve between complete units of encoded information.
      using state = EmptyState;

      /// @brief The individual units that result from an encode operation or are used as input to a decode
      /// operation. For UTF-32 formats, this is usually char32, but this can change (see
      /// basic_utf32).
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
      inline static constexpr std::size_t MaxCodeUnits = 1;

      ///@brief The encoding ID for this type. Used for optimization purposes.
      inline static constexpr Krys::TextEncodingId EncodedId =
        surrogatesAllowed ? Krys::TextEncodingId::ucs4 : Krys::TextEncodingId::utf32;

      ///@brief The encoding ID for this type. Used for optimization purposes.
      inline static constexpr Krys::TextEncodingId DecodedId =
        surrogatesAllowed ? Krys::TextEncodingId::ucs4 : Krys::TextEncodingId::utf32;

      /// @brief Allows an encoding to discard input characters if an error occurs, taking in both the
      /// state and the input sequence (by reference) to modify.
      /// @remarks This will skip every input value until a proper UTF-32 unicode scalar value (or code
      /// point) is found.
      template <typename TResult, typename TInputProgress, typename TOutputProgress>
      static constexpr auto SkipInputError(TResult &&result, const TInputProgress &inputProgress,
                                           const TOutputProgress &outputProgress) noexcept
      {
        if constexpr (surrogatesAllowed)
        {
          return Krys::SkipUTF32InputError(std::forward<TResult>(result), inputProgress, outputProgress);
        }
        else
        {
          return Krys::SkipUTF32WithSurrogatesInputError(std::forward<TResult>(result), inputProgress,
                                                         outputProgress);
        }
      }

      /// @brief Decodes a single complete unit of information as code points and produces a result with the
      /// input and output ranges moved past what was successfully read and written; or, produces an error
      /// and returns the input and output ranges untouched.
      /// @param[in] input The input view to read code units from.
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
        auto inLast = ::Krys::Ranges::cend(input);
        if (inIt == inLast)
        {
          // an exhausted sequence is fine
          return TResult(TSubInput(std::move(inIt), std::move(inLast)), std::forward<TOutput>(output), s,
                         EncodingError::OK);
        }

        auto outIt = ::Krys::Ranges::begin(output);
        KRYS_MAYBE_USED auto outLast = ::Krys::Ranges::end(output);

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
        if constexpr (TValidateDecodableAs && CallErrorHandler)
        {
          if (unit32 > Krys::Impl::Unicode::LastUnicodeCodePoint
              || (!surrogatesAllowed && Krys::Impl::Unicode::IsSurrogate(static_cast<char32>(unit))))
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

        char32 point32 = static_cast<char32>(*inIt);

        if constexpr (CallErrorHandler)
        {
          if (point32 > Krys::Impl::Unicode::LastUnicodeCodePoint
              || (!surrogatesAllowed && Krys::Impl::Unicode::IsSurrogate(point32)))
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

        *outIt = static_cast<code_point>(point32);
        ::Krys::Ranges::iter_advance(outIt);
        ::Krys::Ranges::iter_advance(inIt);

        return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                       TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
      }

    private:
      static_assert((sizeof(code_point) * CHAR_BIT) > 21,
                    "The code point type for a UTF-32 (or similar) encoding must be at least 22 bits wide");
    };
  }

  /// @brief A UTF-32 Encoding that traffics in, specifically, the desired code unit type provided as a
  /// template argument.
  /// @tparam TCodeUnit The code unit type to use.
  /// @tparam TCodeUnit The code point type to use.
  /// @remarks This is a strict UTF-32 implementation that does not allow lone, unpaired surrogates either
  /// in or out.
  template <typename TCodeUnit, typename TCodePoint = UnicodeCodePoint>
  class basic_utf32 : public Impl::UTF32With<basic_utf32<TCodeUnit, TCodePoint>, TCodeUnit, TCodePoint>
  {
  };

  /// @brief A UTF-32 Encoding that traffics in char32. See basic_utf32 for more details.
  using utf32_t = basic_utf32<char32>;

  /// @brief An instance of the UTF-32 encoding for ease of use.
  inline constexpr utf32_t utf32 = {};

  /// @brief A UTF-32 Encoding that traffics in wchar_t. See basic_utf32 for more details.
  using wide_utf32_t = basic_utf32<wchar_t>;

  /// @brief An instance of the UTF-32 that traffics in wchar_t for ease of use.
  inline constexpr wide_utf32_t wide_utf32 = {};
}
