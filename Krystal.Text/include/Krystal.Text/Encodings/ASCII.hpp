#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Lib/Ranges/Subrange.hpp"
#include "Krystal.Text/DecodeResult.hpp"
#include "Krystal.Text/EncodeResult.hpp"
#include "Krystal.Text/EncodingError.hpp"
#include "Krystal.Text/Impl/ReplacementUnits.hpp"
#include "Krystal.Text/IsIgnorableErrorHandler.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys
{
  /// @brief The American Standard Code for Information Exchange (ASCII) Encoding.
  /// @tparam TCodeUnit The code unit type to work over.
  /// @remarks The most vanilla and unimaginative encoding there is in the world, excluding tons of other
  /// languages, dialects, and even common English idioms and borrowed words. Please don't pick this unless
  /// you have good reason!
  template <typename TCodeUnit, typename TCodePoint = UnicodeCodePoint>
  class basic_ascii
  {
  public:
    /// @brief The individual units that result from an encode operation or are used as input to a decode
    /// operation.
    using code_unit = TCodeUnit;

    /// @brief The individual units that result from a decode operation or as used as input to an encode
    /// operation. For most encodings, this is going to be a Unicode Code Point or a Unicode Scalar Value.
    using code_point = TCodePoint;

    /// @brief The state that can be used between calls to the encoder and decoder.
    /// @remarks It is an empty struct because there is no shift state to preserve between complete units of
    /// encoded information. It is also only `state` and not separately `decode_state` and `encode_state`
    /// because one type suffices for both.
    using state = EmptyState;

    /// @brief Whether or not the decode operation can process all forms of input into code point values.
    /// @remarks ASCII can decode from its 7-bit (unpacked) code units to Unicode Code Points. Since the
    /// conversion is lossless, this property is true.
    using is_decode_injective = std::true_type;

    /// @brief Whether or not the encode operation can process all forms of input into code unit values.
    /// This is not true for ASCII, as many Unicode Code Point and Unicode Scalar Values cannot be
    /// represented in ASCII. Since the conversion is lossy, this property is false.
    using is_encode_injective = std::false_type;

    /// @brief The maximum code units a single complete operation of encoding can produce.
    inline static constexpr const std::size_t MaxCodeUnits = 1;

    /// @brief The maximum number of code points a single complete operation of decoding can produce. This
    /// is 1 for all Unicode Transformation Format (UTF) encodings.
    inline static constexpr const std::size_t MaxCodePoints = 1;

    /// @brief A range of code units representing the values to use when a replacement happen. For ASCII,
    /// this must be '?' instead of the usual Unicode Replacement Character U'�'.
    static constexpr Span<const code_unit, 1> ReplacementCodeUnits() noexcept
    {
      return Impl::QuestionMarkReplacementUnits<code_unit>;
    }

    /// @brief Decodes a single complete unit of information as code points and produces a result with the
    /// input and output ranges moved past what was successfully read and written; or, produces an error and
    /// returns the input and output ranges untouched.
    /// @param[in] input The input view to read code uunits from.
    /// @param[in] output The output view to write code points into.
    /// @param[in] errorHandler The error handler to invoke if encoding fails.
    /// @param[in, out] s The necessary state information. For this encoding, the state is empty and means
    /// very little.
    /// @returns A DecodeResult object that contains the input range, output range, error
    /// handler, and a reference to the passed-in state.
    /// @remarks To the best ability of the implementation, the iterators will be returned untouched (e.g.,
    /// the input models at least a view and a forward_range). If it is not possible, returned ranges may be
    /// incremented even if an error occurs due to the semantics of any view that models an input_range.
    template <typename TInput, typename TOutput, typename TErrorHandler>
    static constexpr auto DecodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler, state &s)
    {
      using TUErrorHandler = remove_cvref_t<TErrorHandler>;
      using TSubInput = Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
      using TSubOutput = Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
      using TResult = DecodeResult<TSubInput, TSubOutput, state>;
      constexpr bool CallErrorHandler = !IsIgnorableErrorHandler<TUErrorHandler>;

      auto inIt = Krys::Ranges::cbegin(input);
      auto inLast = Krys::Ranges::cend(input);
      if (inIt == inLast)
      {
        // an exhausted sequence is fine
        return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                       TSubOutput(std::forward<TOutput>(output)), s, EncodingError::OK);
      }

      auto outIt = Krys::Ranges::begin(output);
      KRYS_MAYBE_UNUSED auto outLast = Krys::Ranges::end(output);

      if constexpr (CallErrorHandler)
      {
        if (outIt == outLast)
        {
          basic_ascii self {};
          return std::forward<TErrorHandler>(errorHandler)(
            self,
            TResult(TSubInput(std::move(inIt), std::move(inLast)),
                    TSubOutput(std::move(outIt), std::move(outLast)), s,
                    EncodingError::InsufficientOutputSpace),
            Span<code_unit>(), Span<code_point>());
        }
      }

      code_unit units[1] {};
      units[0] = *inIt;
      const code_unit &unit = units[0];

      if constexpr (CallErrorHandler)
      {
        if (static_cast<schar>(unit) < static_cast<schar>(0))
        {
          basic_ascii self {};
          return std::forward<TErrorHandler>(errorHandler)(
            self,
            TResult(TSubInput(std::move(inIt), std::move(inLast)),
                    TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
            Span<code_unit>(std::addressof(units[0]), 1), Span<code_point>());
        }
      }

      Krys::Ranges::iter_advance(inIt);

      *outIt = unit;
      Krys::Ranges::iter_advance(outIt);

      return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                     TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
    }

    /// @brief Encodes a single complete unit of information as code units and produces a result with the
    /// input and output ranges moved past what was successfully read and written; or, produces an error and
    /// returns the input and output ranges untouched.
    /// @param[in] input The input view to read code points from.
    /// @param[in] output The output view to write code units into.
    /// @param[in] errorHandler The error handler to invoke if encoding fails.
    /// @param[in, out] s The necessary state information. For this encoding, the state is empty and means
    /// very little.
    /// @returns A EncodeResult object that contains the input range, output range, error
    /// handler, and a reference to the passed-in state.
    /// @remarks To the best ability of the implementation, the iterators will be returned untouched (e.g.,
    /// the input models at least a view and a forward_range). If it is not possible, returned ranges may be
    /// incremented even if an error occurs due to the semantics of any view that models an input_range.
    template <typename TInput, typename TOutput, typename TErrorHandler>
    static constexpr auto EncodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler, state &s)
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
        // an exhausted sequence is fine
        return TResult(TSubInput(std::move(inIt), std::move(inLast)), std::forward<TOutput>(output), s,
                       EncodingError::OK);
      }

      auto outIt = Krys::Ranges::begin(output);
      KRYS_MAYBE_UNUSED auto outLast = Krys::Ranges::end(output);

      if constexpr (CallErrorHandler)
      {
        if (outIt == outLast)
        {
          basic_ascii self {};
          return TResult(std::forward<TErrorHandler>(errorHandler)(
            self,
            TResult(TSubInput(std::move(inIt), std::move(inLast)),
                    TSubOutput(std::move(outIt), std::move(outLast)), s,
                    EncodingError::InsufficientOutputSpace),
            Span<code_point>(), Span<code_unit>()));
        }
      }

      code_point points[1] {};
      points[0] = *inIt;
      const code_point &point = points[0];

      if constexpr (CallErrorHandler)
      {
        if (point > Krys::Impl::Unicode::LastASCIIValue)
        {
          basic_ascii self {};
          return TResult(std::forward<TErrorHandler>(errorHandler)(
            self,
            TResult(TSubInput(std::move(inIt), std::move(inLast)),
                    TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
            Span<code_point>(std::addressof(points[0]), 1), Span<code_unit>()));
        }
      }

      Krys::Ranges::iter_advance(inIt);

      *outIt = static_cast<code_unit>(point);
      Krys::Ranges::iter_advance(outIt);

      return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                     TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
    }
  };

  /// @brief The American Standard Code for Information Exchange (ASCII) Encoding.
  /// @remarks The most vanilla and unimaginative encoding there is in the world, excluding tons of other
  /// languages, dialects, and even common English idioms and borrowed words. Please don't pick this unless
  /// you have good reason!
  using ascii_t = basic_ascii<char>;

  /// @brief An instance of the ascii_t type for ease of use.
  constexpr inline ascii_t ascii = {};
}
