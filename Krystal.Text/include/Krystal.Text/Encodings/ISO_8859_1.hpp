#pragma once

#include "Krystal.Text/DecodeResult.hpp"
#include "Krystal.Text/EncodeResult.hpp"
#include "Krystal.Text/Impl/ReplacementUnits.hpp"
#include "Krystal.Text/IsIgnorableErrorHandler.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys
{
  /// @brief The ISO/IEC 8859-1 encoding, occasionally referred to as Latin-1 (erroneously). Matches
  /// Unicode's encoding of the first 256 bytes one-to-one.
  /// @tparam TCodeUnit The code unit type to work over.
  /// @tparam TCodePoint The code point type to work over.
  template <typename TCodeUnit, typename TCodePoint = UnicodeCodePoint>
  class basic_iso_8859_1
  {
  public:
    /// @brief The individual units that result from an encode operation or are used as input to a decode
    /// operation.
    using code_unit = TCodeUnit;

    /// @brief The individual units that result from a decode operation or as used as input to an encode
    /// operation. For most encodings, this is going to be a Unicode Code Point or a Unicode Scalar Value.
    using code_point = TCodePoint;

    /// @brief The state that can be used between calls to the encoder and decoder. This is empty for this
    /// encoding.
    using state = EmptyState;

    /// @brief Whether or not the decode operation can process all forms of input into code point values.
    ///
    /// @remarks ISO/IEC 8859-1 can decode from its 8-bit (unpacked) code units to Unicode Code Points.
    /// Since the converion is lossless, this property is true.
    using is_decode_injective = std::true_type;

    /// @brief Whether or not the encode operation can process all forms of input into code unit values.
    /// This is not true for ISO/IEC 8859-1, as many Unicode Code Point and Unicode Scalar Values cannot be
    /// represented in ISO/IEC 8859-1. Since the conversion is lossy, this property is false.
    using is_encode_injective = std::false_type;

    /// @brief The maximum code units a single complete operation of encoding can produce.
    inline static constexpr const std::size_t MaxCodeUnits = 1;

    /// @brief The maximum number of code points a single complete operation of decoding can produce.
    inline static constexpr const std::size_t MaxCodePoints = 1;

    /// @brief A range of code units representing the values to use when a replacement happen. For ISO/IEC
    /// 8859-1, this must be '?' instead of the usual Unicode Replacement Character U'�'.
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
    /// handler, and a reference to the passed-in state\.
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
      auto outLast = Krys::Ranges::end(output);

      if constexpr (CallErrorHandler)
      {
        if (outIt == outLast)
        {
          basic_iso_8859_1 self {};
          return std::forward<TErrorHandler>(errorHandler)(
            self,
            TResult(TSubInput(std::move(inIt), std::move(inLast)),
                    TSubOutput(std::move(outIt), std::move(outLast)), s,
                    EncodingError::InsufficientOutputSpace),
            Span<code_unit>(), Span<code_point>());
        }
      }
      else
      {
        (void)outLast;
      }

      code_unit units[1] {};
      units[0] = *inIt;
      const code_unit &unit = units[0];

      if constexpr (CallErrorHandler)
      {
        if (static_cast<unsigned int>(unit) > static_cast<unsigned int>(0xFF))
        {
          basic_iso_8859_1 self {};
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
    ///
    /// @param[in] input The input view to read code points from.
    /// @param[in] output The output view to write code units into.
    /// @param[in] errorHandler The error handler to invoke if encoding fails.
    /// @param[in, out] s The necessary state information. For this encoding, the state is empty and means
    /// very little.
    ///
    /// @returns A EncodeResult object that contains the input range, output range, error
    /// handler, and a reference to the passed-in state\.
    ///
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
          basic_iso_8859_1 self {};
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
        if (static_cast<unsigned int>(point) > 0xFF)
        {
          basic_iso_8859_1 self {};
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

  /// @brief An instance of the basic_iso_8859_1 type for ease of use.
  inline constexpr basic_iso_8859_1<char> iso_8859_1 = {};
}
