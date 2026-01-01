#pragma once

#include "Krystal.Lib/Ranges/Range.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/DecodeResult.hpp"
#include "Krystal.Text/EncodeResult.hpp"
#include "Krystal.Text/IsIgnorableErrorHandler.hpp"
#include "Krystal.Text/State.hpp"

namespace Krys
{
  /// @brief A UTF-32 Encoding that traffics in, specifically, the desired code unit type provided as a
  /// template argument.
  /// @tparam T The code unit type to use.
  /// @remarks This is a strict UTF-32 implementation that does not allow lone, unpaired surrogates either
  /// in or out.
  template <typename TCodeUnit, typename TCodePoint>
  class basic_no_encoding
  {
  public:
    /// @brief The state that can be used between calls to the encoder and decoder. It is an empty struct
    /// because there is no shift state to preserve between complete units of encoded information.
    using state = EmptyState;

    /// @brief The individual units that result from an encode operation or are used as input to a decode
    /// operation.
    using code_unit = TCodeUnit;

    /// @brief The individual units that result from a decode operation or as used as input to an encode
    /// operation. For most encodings, this is going to be a Unicode Code Point or a Unicode Scalar Value.
    using code_point = TCodePoint;

    /// @brief Whether or not the decode operation can process all forms of input into code unit values.
    /// This is always true because this encoding does nothing.
    using is_decode_injective = IntegralConstant<bool, SameType<TCodeUnit, TCodePoint>>;

    /// @brief Whether or not the encode operation can process all forms of input into code unit values.
    /// This is always true because this encoding does nothing.
    using is_encode_injective = IntegralConstant<bool, SameType<TCodeUnit, TCodePoint>>;

    /// @brief The maximum number of code points a single complete operation of decoding can produce. This
    /// is 1 because this encoding does nothing.
    inline static constexpr std::size_t MaxCodePoints = 1;

    /// @brief The maximum code units a single complete operation of encoding can produce. This is 1 because
    /// this encoding does nothing.
    inline static constexpr std::size_t MaxCodeUnits = 1;

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
        return TResult(TSubInput(std::move(inIt), std::move(inLast)), std::forward<TOutput>(output), s,
                       EncodingError::OK);
      }

      auto outIt = Krys::Ranges::begin(output);
      KRYS_MAYBE_UNUSED auto outLast = Krys::Ranges::end(output);

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
      Krys::Ranges::iter_advance(inIt);

      *outIt = static_cast<code_point>(unit);
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
    /// handler, and a reference to the passed-in state\.
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
      Krys::Ranges::iter_advance(inIt);

      *outIt = static_cast<code_unit>(point);
      Krys::Ranges::iter_advance(outIt);

      return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                     TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
    }
  };

  /// @brief A do-nothing encoding for `char` types of input.
  using no_encoding_t = basic_no_encoding<char, UnicodeCodePoint>;

  /// @brief An object for the no_encoding_t type.
  inline constexpr no_encoding_t no_encoding = {};

  /// @brief A do-nothing encoding for UnicodeCodePoint types of input.
  using no_codepoint_encoding_t = basic_no_encoding<UnicodeCodePoint, UnicodeCodePoint>;

  /// @brief An object for the no_codepoint_encoding_t type.
  inline constexpr no_codepoint_encoding_t no_codepoint_encoding = {};
}
