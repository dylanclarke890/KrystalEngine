#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/Impl/InsertBulk.hpp"
#include "Krystal.Lib/Ranges/Unbounded.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Utils/Tag.hpp"
#include "Krystal.Text/CharTraits.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/DecodeResult.hpp"
#include "Krystal.Text/DefaultEncoding.hpp"
#include "Krystal.Text/ErrorHandlers/DefaultHandler.hpp"
#include "Krystal.Text/Impl/EncodingRange.hpp"
#include "Krystal.Text/Impl/ForwardIfMoveOnly.hpp"
#include "Krystal.Text/Impl/IsLossless.hpp"
#include "Krystal.Text/Impl/ProgressHandler.hpp"
#include "Krystal.Text/Impl/SpanReconstruct.hpp"
#include "Krystal.Text/Impl/UpdateInput.hpp"
#include "Krystal.Text/IsUnicodeCodePoint.hpp"
#include "Krystal.Text/MaxUnits.hpp"
#include "Krystal.Text/State.hpp"
#include <string>
#include <utility>
#include <vector>

namespace Krys
{
  /// @brief These functions convert from a view of input into a view of output using either the inferred or
  /// specified encodings. If no error handler is provided, the equivalent of the Handlers::DefaultHandler is
  /// used by default. If no associated state is provided for either the "to" or "from" encodings, one will be
  /// created with automatic storage duration (as a "stack" variable) for the provided encoding.
  /// @brief Converts from the code units of the given `input` view through the encoding to code points into
  /// the `output` view.
  /// @param[in] input An input_view to read code units from and use in the decode operation that will
  /// produce code points.
  /// @param[in] encoding The encoding that will be used to decode the input's code points into
  /// output code units.
  /// @param[in] output An output_view to write code points to as the result of the decode operation from
  /// the intermediate code units.
  /// @param[in] errorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in,out] state A reference to the associated state for the `encoding` 's decode step.
  /// @result A DecodeResult object that contains references to `state`.
  /// @remarks This function performs the bog-standard, basic loop for decoding. It talks to no ADL extension
  /// points.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler, typename TState>
  constexpr auto BasicDecodeIntoRaw(TInput &&input, TEncoding &&encoding, TOutput &&output,
                                    TErrorHandler &&errorHandler, TState &state)
  {
    static_assert(Impl::DecodeLosslessOrDeliberate<TEncoding, TErrorHandler>, KRYS_TEXT_LOSSY_DECODE_MESSAGE);

    std::size_t errorCount = 0;

    auto firstResult =
      encoding.DecodeOne(std::forward<TInput>(input), std::forward<TOutput>(output), errorHandler, state);
    using TWorkingInput = decltype(firstResult.Input);
    using TWorkingOutput = decltype(firstResult.Output);
    using TResult = DecodeResult<TWorkingInput, TWorkingOutput, TState>;
    if (firstResult.ErrorCode != EncodingError::OK)
    {
      return TResult(std::move(firstResult.Input), std::move(firstResult.Output), state,
                     firstResult.ErrorCode, errorCount);
    }
    errorCount += firstResult.ErrorCount;
    if (Krys::Ranges::empty(firstResult.Input))
    {
      if (Krys::IsStateComplete(encoding, state))
      {
        return TResult(std::move(firstResult.Input), std::move(firstResult.Output), state, EncodingError::OK,
                       errorCount);
      }
    }

    TWorkingInput workingInput = std::move(firstResult.Input);
    TWorkingOutput workingOutput = std::move(firstResult.Output);

    for (;;)
    {
      auto result =
        encoding.DecodeOne(std::move(workingInput), std::move(workingOutput), errorHandler, state);
      errorCount += result.ErrorCount;
      workingInput = std::move(result.Input);
      workingOutput = std::move(result.Output);
      if (result.ErrorCode != EncodingError::OK)
      {
        return TResult(std::move(workingInput), std::move(workingOutput), state, result.ErrorCode,
                       errorCount);
      }
      if (Krys::Ranges::empty(workingInput))
      {
        if (!Krys::IsStateComplete(encoding, state))
        {
          continue;
        }
        break;
      }
    }
    return TResult(std::move(workingInput), std::move(workingOutput), state, EncodingError::OK, errorCount);
  }

  /// @brief Converts from the code units of the given `input` view through the encoding to code points into
  /// the `output` view.
  /// @param[in] input An input_view to read code units from and use in the decode operation that will
  /// produce code points.
  /// @param[in] encoding The encoding that will be used to decode the input's code points into
  /// output code units.
  /// @param[in] output An output_view to write code points to as the result of the decode operation from the
  /// intermediate code units.
  /// @param[in] errorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in,out] state A reference to the associated state for the `encoding` 's decode step.
  /// @result A DecodeResult object that contains references to `state`.
  /// @remarks This function detects whether or not the ADL extension point `TextDecode` can be called with
  /// the provided parameters. If so, it will use that ADL extension point over the default implementation.
  /// Otherwise, it will loop over the two encodings and attempt to decode by repeatedly calling the
  /// encoding's required @c DecodeOne function.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler, typename TState>
  constexpr auto DecodeIntoRaw(TInput &&input, TEncoding &&encoding, TOutput &&output,
                               TErrorHandler &&errorHandler, TState &state)
  {
    if constexpr (Impl::HasTextDecode<TInput, TEncoding, TOutput, TErrorHandler, TState>)
    {
      return TextDecode(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::forward<TInput>(input),
                        std::forward<TEncoding>(encoding), std::forward<TOutput>(output),
                        std::forward<TErrorHandler>(errorHandler), state);
    }
    else if constexpr (Impl::HasInternalTextDecode<TInput, TEncoding, TOutput, TErrorHandler, TState>)
    {
      return InternalTextDecode(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::forward<TInput>(input),
                                std::forward<TEncoding>(encoding), std::forward<TOutput>(output),
                                std::forward<TErrorHandler>(errorHandler), state);
    }
    else
    {
      return Krys::BasicDecodeIntoRaw(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                      std::forward<TOutput>(output),
                                      std::forward<TErrorHandler>(errorHandler), state);
    }
  }

  /// @brief Converts from the code units of the given `input` view through the encoding to code points into
  /// the `output` view.
  /// @param[in] input An input_view to read code units from and use in the decode operation that will
  /// produce code points.
  /// @param[in] encoding The encoding that will be used to decode the input's code points into
  /// output code units.
  /// @param[in] output An output_view to write code points to as the result of the decode operation from the
  /// intermediate code units.
  /// @param[in] errorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @result A StatelessDecodeResult object that contains references to `state`.
  /// @remarks Creates a default `state` using CreateDecodeState.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler>
  constexpr auto DecodeIntoRaw(TInput &&input, TEncoding &&encoding, TOutput &&output,
                               TErrorHandler &&errorHandler)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TState = decode_state_t<TUEncoding>;

    TState state = Krys::CreateDecodeState(encoding);
    auto statefulResult =
      Krys::DecodeIntoRaw(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                          std::forward<TOutput>(output), std::forward<TErrorHandler>(errorHandler), state);
    return ::Krys::Text::detail_result::SliceToStatelessDecode(std::move(statefulResult));
  }

  /// @brief Converts from the code units of the given `input` view through the encoding to code points into
  /// the `output` view.
  /// @param[in] input An input_view to read code units from and use in the decode operation that will
  /// produce code points.
  /// @param[in] encoding The encoding that will be used to decode the input's code points into
  /// output code units.
  /// @param[in] output An output_view to write code points to as the result of the decode operation from the
  /// intermediate code units.
  /// @result A StatelessDecodeResult object that contains references to `state`.
  /// @remarks Creates a default `ErrorHandler` that is similar to Handlers::DefaultHandler, but marked as
  /// careless.
  template <typename TInput, typename TEncoding, typename TOutput>
  constexpr auto DecodeIntoRaw(TInput &&input, TEncoding &&encoding, TOutput &&output)
  {
    Handlers::DefaultHandler handler {};
    return Krys::DecodeIntoRaw(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                               std::forward<TOutput>(output), handler);
  }

  /// @brief Converts from the code units of the given `input` view through the encoding to code points into
  /// the `output` view.
  /// @param[in] input An input_view to read code units from and use in the decode operation that will
  /// produce code points.
  /// @param[in] output An output_view to write code points to as the result of the decode operation from the
  /// intermediate code units.
  /// @result A StatelessDecodeResult object that contains references to `state`.
  /// @remarks Creates a default `encoding` by figuring out the `value_type` of the `input`, then passing that
  /// type into default_code_point_encoding_t. That encoding is that used to decode the input code units,
  /// by default.
  template <typename TInput, typename TOutput>
  constexpr auto DecodeIntoRaw(TInput &&input, TOutput &&output)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TCodeUnit = Krys::Ranges::range_value_type_t<TUInput>;
    if (std::is_constant_evaluated())
    {
      // Use literal encoding instead, if we meet the right criteria
      using TEncoding = default_consteval_code_unit_encoding_t<TCodeUnit>;
      TEncoding encoding {};
      return Krys::DecodeIntoRaw(std::forward<TInput>(input), encoding, std::forward<TOutput>(output));
    }
    else
    {
      using TEncoding = default_code_unit_encoding_t<TCodeUnit>;
      TEncoding encoding {};
      return Krys::DecodeIntoRaw(std::forward<TInput>(input), encoding, std::forward<TOutput>(output));
    }
  }

  namespace Impl
  {
    template <typename TInput, typename TEncoding, typename TOutputContainer, typename TErrorHandler,
              typename TState>
    constexpr auto IntermediateDecodeToStorage(TInput &&input, TEncoding &&encoding, TOutputContainer &output,
                                               TErrorHandler &&errorHandler, TState &state)
    {
      // Well, SHIT. Write into temporary, then serialize one-by-one/bulk to output.
      // I'll admit, this is HELLA work to support...
      constexpr std::size_t maxUnits = MaxDecodeCodePoints<TEncoding>;
      constexpr std::size_t intermediateBufferMax = maxUnits;
      using TIntermediateValue = code_point_t<TEncoding>;
      using TIntermediateInput = Impl::span_reconstruct_t<TInput, TInput>;
      using TInitialOutput = Span<TIntermediateValue, intermediateBufferMax>;
      using TOutput = Span<TIntermediateValue>;
      using TResult = decltype(encoding.DecodeOne(std::declval<TIntermediateInput>(), std::declval<TOutput>(),
                                                  errorHandler, state));
      using TResultInput = decltype(std::declval<TResult>().Input);
      using TWorkingInput = span_reconstruct_t<TResultInput, TResultInput>;

      static_assert(Impl::DecodeLosslessOrDeliberate<TEncoding, TErrorHandler>,
                    KRYS_TEXT_LOSSY_DECODE_MESSAGE);

      // We MUST use a temporary error handler
      // as well as a pass-throuugh handler if we end up with lots of intermediary input
      Impl::ProgressHandler<IsIgnorableErrorHandler<TErrorHandler>, TEncoding> intermediateHandler {};

      TIntermediateInput intermediateInput = Impl::SpanReconstruct<TInput>(std::forward<TInput>(input));
      TWorkingInput workingInput(std::move(intermediateInput));
      TIntermediateValue intermediateTranslationBuffer[intermediateBufferMax] {};

      for (;;)
      {
        // Ignore "out of output" errors and do our best to recover properly along the way...
        TInitialOutput intermediateInitialOutput(intermediateTranslationBuffer);
        auto result = Krys::DecodeIntoRaw(std::move(workingInput), encoding, intermediateInitialOutput,
                                          intermediateHandler, state);
        TOutput intermediateOutput(intermediateInitialOutput.data(), result.Output.data());
        Krys::Ranges::Impl::ContainerInsertBulk(output, intermediateOutput);
        if (result.ErrorCode == EncodingError::InsufficientOutputSpace)
        {
          if (intermediateHandler.CodePointsProgressSize() != 0)
          {
            // add any leftover partially-unwritten characters to our output
            Krys::Ranges::Impl::ContainerInsertBulk(output, intermediateHandler.ConstCodePointsProgress());
            // it's okay, just loop around, we've got S P A C E for more
            workingInput = Impl::UpdateInput<TWorkingInput>(std::move(result.Input));
          }
          else if (intermediateHandler.CodeUnitsProgressSize() != 0)
          {
            if constexpr (Krys::Ranges::BidirectionalRange<TWorkingInput>)
            {
              // we can try to rewind our current input by the amount that was not successfully
              // read. This will allow us to try again, when the buffer
              // has more space in it, and should not result in the same error,
              // unless it was legitimiately an EncodingError::InvalidSequence.
              workingInput = Impl::UpdateInput<TWorkingInput>(Krys::Ranges::reconstruct(
                std::in_place_type<TWorkingInput>,
                Krys::Ranges::iter_recede(Krys::Ranges::begin(result.Input),
                                          intermediateHandler.CodeUnitsProgressSize()),
                Krys::Ranges::end(result.Input)));
            }
            else
            {
              // this is an effectively-impossible case, as we cannot stitch the old input together
              // with the current input.
              // simply bail!!
              return result;
            }
          }
          else
          {
            // it's okay, just loop around, we've got S P A C E for more
            workingInput = Impl::UpdateInput<TWorkingInput>(std::move(result.Input));
          }
          intermediateHandler.clear();
          continue;
        }
        if (result.ErrorCode != EncodingError::OK)
        {
          // mill result through actual error handler!
          auto errorResult = std::forward<TErrorHandler>(errorHandler)(
            std::forward<TEncoding>(encoding), std::move(result), intermediateHandler.CodeUnitsProgress(),
            intermediateHandler.CodePointsProgress());
          return errorResult;
        }
        if (Krys::Ranges::empty(result.Input))
        {
          if (!Krys::IsStateComplete(encoding, state))
          {
            continue;
          }
          return result;
        }
      }
    }

    template <bool OutputOnly, bool NoState, typename TOutputContainer, typename TInput, typename TEncoding,
              typename TErrorHandler, typename TState>
    constexpr auto DecodeDispatch(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler,
                                  TState &state)
    {
      using TUEncoding = remove_cvref_t<TEncoding>;
      constexpr std::size_t maxUnits = MaxDecodeCodePoints<TUEncoding>;

      TOutputContainer output {};
      if constexpr (Krys::Ranges::has_adl_size<TInput>)
      {
        using TSize = decltype(Krys::Ranges::size(input));
        if constexpr (Krys::Ranges::has_reserve_with_size<TOutputContainer, TSize>)
        {
          auto outputSizeHint = Krys::Ranges::size(input);
          outputSizeHint *= (maxUnits / 2);
          output.reserve(outputSizeHint);
        }
      }
      auto statefulResult =
        Impl::IntermediateDecodeToStorage(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                          output, std::forward<TErrorHandler>(errorHandler), state);
      if constexpr (OutputOnly)
      {
        (void)statefulResult;
        return output;
      }
      else if constexpr (NoState)
      {
        return ::Krys::Text::detail_result::ReplaceStatelessDecodeResultOutput(std::move(statefulResult),
                                                                               std::move(output));
      }
      else
      {
        return ::Krys::Text::detail_result::ReplaceDecodeResultOutput(std::move(statefulResult),
                                                                      std::move(output));
      }
    }

  }

  /// @brief Converts code units of the given `input` view into code points in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] encoding The encoding for decoding the `input`'s code points into `output` code units.
  /// @param[in] output An output view to write code points to.
  /// @param[in] errorHandler The error handlers for the from and to encodings, respectively.
  /// @param[in,out] state A reference to the associated state for the `encoding`'s decode step.
  /// @result A DecodeResult object that contains references to `state`.
  /// @remarks This function performs the bog-standard, basic loop for decoding. It talks to no ADL extension
  /// points.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler, typename TState>
  constexpr auto BasicDecodeInto(TInput &&input, TEncoding &&encoding, TOutput &&output,
                                 TErrorHandler &&errorHandler, TState &state)
  {
    auto reconstructedInput = Impl::SpanReconstruct<TInput>(std::forward<TInput>(input));
    auto result = Krys::BasicDecodeIntoRaw(std::move(reconstructedInput), std::forward<TEncoding>(encoding),
                                           std::forward<TOutput>(output), errorHandler, state);
    using TReconstructedResultInput = Impl::span_reconstruct_t<TInput, TInput>;
    using TReconstructedResultOutput = Impl::span_reconstruct_mutable_t<TOutput, TOutput>;
    using TResult = DecodeResult<TReconstructedResultInput, TReconstructedResultOutput, TState>;
    return TResult(Impl::SpanReconstruct<TInput>(std::move(result.Input)),
                   Impl::SpanReconstructMutable<TOutput>(std::move(result.Output)), result.State);
  }

  /// @brief Converts code units of the given `input` view into code points in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] encoding The encoding for decoding the `input`'s code points into `output` code units.
  /// @param[in] output An output view to write code points to.
  /// @param[in] errorHandler The error handlers for the from and to encodings, respectively.
  /// @param[in,out] state A reference to the associated state for the `encoding`'s decode step.
  /// @result A DecodeResult object that contains references to `state`.
  /// @remarks This function performs the bog-standard, basic loop for decoding. It talks to no ADL extension
  /// points.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler, typename TState>
  constexpr auto DecodeInto(TInput &&input, TEncoding &&encoding, TOutput &&output,
                            TErrorHandler &&errorHandler, TState &state)
  {
    auto reconstructedInput = Impl::SpanReconstruct<TInput>(std::forward<TInput>(input));
    auto result = Krys::DecodeIntoRaw(std::move(reconstructedInput), std::forward<TEncoding>(encoding),
                                      std::forward<TOutput>(output), errorHandler, state);
    using TReconstructedResultInput = Impl::span_reconstruct_t<TInput, TInput>;
    using TReconstructedResultOutput = Impl::span_reconstruct_mutable_t<TOutput, TOutput>;
    using TResult = DecodeResult<TReconstructedResultInput, TReconstructedResultOutput, TState>;
    return TResult(Impl::SpanReconstruct<TInput>(std::move(result.Input)),
                   Impl::SpanReconstructMutable<TOutput>(std::move(result.Output)), result.State);
  }

  /// @brief Converts code units of the given `input` view into code points in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] encoding The encoding for decoding the `input`'s code points into `output` code units.
  /// @param[in] output An output view to write code points to.
  /// @param[in] errorHandler The error handlers for the from and to encodings, respectively.
  /// @result A StatelessDecodeResult object that contains references to `state`.
  /// @remarks Creates a default `state` using CreateDecodeState.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler>
  constexpr auto DecodeInto(TInput &&input, TEncoding &&encoding, TOutput &&output,
                            TErrorHandler &&errorHandler)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TState = decode_state_t<TUEncoding>;

    TState state = Krys::CreateDecodeState(encoding);
    auto statefulResult =
      Krys::DecodeInto(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                       std::forward<TOutput>(output), std::forward<TErrorHandler>(errorHandler), state);
    return ::Krys::Text::detail_result::SliceToStatelessDecode(std::move(statefulResult));
  }

  /// @brief Converts code units of the given `input` view into code points in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] encoding The encoding for decoding the `input`'s code points into `output` code units.
  /// @param[in] output An output view to write code points to.
  /// @result A StatelessDecodeResult object that contains references to `state`.
  ///
  /// @remarks Creates a default `ErrorHandler` that is similar to Handlers::DefaultHandler, but marked as
  /// careless.
  template <typename TInput, typename TEncoding, typename TOutput>
  constexpr auto DecodeInto(TInput &&input, TEncoding &&encoding, TOutput &&output)
  {
    Handlers::DefaultHandler handler {};
    return Krys::DecodeInto(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                            std::forward<TOutput>(output), handler);
  }

  /// @brief Converts from the code units of the given `input` view through the encoding to code points into
  /// the `output` view.
  /// @param[in] input An input_view to read code units from and use in the decode operation that will
  /// produce code points.
  /// @param[in] output An output_view to write code points to as the result of the decode operation from the
  /// intermediate code units.
  /// @result A StatelessDecodeResult object that contains references to `state`.
  /// @remarks Creates a default `encoding` by figuring out the `value_type` of the `input`, then passing that
  /// type into default_code_point_encoding_t. That encoding is that used to decode the input code units,
  /// by default.
  template <typename TInput, typename TOutput>
  constexpr auto DecodeInto(TInput &&input, TOutput &&output)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TCodeUnit = Krys::Ranges::range_value_type_t<TUInput>;
    if (std::is_constant_evaluated())
    {
      // Use literal encoding instead, if we meet the right criteria
      using TEncoding = default_consteval_code_unit_encoding_t<TCodeUnit>;
      TEncoding encoding {};
      return Krys::DecodeInto(std::forward<TInput>(input), encoding, std::forward<TOutput>(output));
    }
    else
    {
      using TEncoding = default_code_unit_encoding_t<TCodeUnit>;
      TEncoding encoding {};
      return Krys::DecodeInto(std::forward<TInput>(input), encoding, std::forward<TOutput>(output));
    }
  }

  /// @brief Converts the code units of the given `input` view through the encoding to code points the
  /// specified `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input_view to read code units from and use in the decode operation that will
  /// produce code points.
  /// @param[in] encoding The encoding that will be used to decode the input's code points into
  /// output code units.
  /// @param[in] errorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in,out] state A reference to the associated state for the `encoding` 's decode step.
  /// @result A DecodeResult object that contains references to `state` and an output of type
  /// `TOutputContainer`.
  /// @remarks This function detects creates a container of type `TOutputContainer` and uses a typical @c
  /// std::back_inserter or `std::push_back_inserter` to fill in elements as it is written to. The result is
  /// then returned, with the `.Output` value put into the container.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding, typename TErrorHandler,
            typename TState>
  constexpr auto DecodeTo(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler, TState &state)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TUOutputContainer = remove_cvref_t<TOutputContainer>;
    using TOutputCodePoint = code_point_t<TUEncoding>;
    constexpr bool IsVoidContainer = Void<TUOutputContainer>;
    constexpr bool IsStringable = (IsCharTraitable<TOutputCodePoint> || IsUnicodeCodePoint<TOutputCodePoint>);
    if constexpr (IsVoidContainer && IsStringable)
    {
      // prevent instantiation errors with basic_string by boxing it inside of an "if constexpr"
      using TRealOutputContainer = std::basic_string<TOutputCodePoint>;
      return Impl::DecodeDispatch<false, false, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
    else
    {
      using TRealOutputContainer =
        conditional_t<IsVoidContainer, std::vector<TOutputCodePoint>, TOutputContainer>;
      return Impl::DecodeDispatch<false, false, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
  }

  /// @brief Converts the code units of the given `input` view through the encoding to code points the
  /// specified `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input_view to read code units from and use in the decode operation that will
  /// produce code points.
  /// @param[in] encoding The encoding that will be used to decode the input's code points into
  /// output code units.
  /// @param[in] errorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @result A StatelessDecodeResult object whose output is of type `TOutputContainer`.
  /// @remarks This function creates a `state` using CreateDecodeState.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding, typename TErrorHandler>
  constexpr auto DecodeTo(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TUOutputContainer = remove_cvref_t<TOutputContainer>;
    using TOutputCodePoint = code_point_t<TUEncoding>;
    using TState = decode_state_t<TUEncoding>;
    TState state = Krys::CreateDecodeState(encoding);
    constexpr bool IsVoidContainer = Void<TUOutputContainer>;
    constexpr bool IsStringable = (IsCharTraitable<TOutputCodePoint> || IsUnicodeCodePoint<TOutputCodePoint>);
    if constexpr (IsVoidContainer && IsStringable)
    {
      // prevent instantiation errors with basic_string by boxing it inside of an "if constexpr"
      using TRealOutputContainer = std::basic_string<TOutputCodePoint>;
      return Impl::DecodeDispatch<false, true, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
    else
    {
      using TRealOutputContainer =
        conditional_t<IsVoidContainer, std::vector<TOutputCodePoint>, TOutputContainer>;
      return Impl::DecodeDispatch<false, true, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
  }

  /// @brief Converts the code units of the given `input` view through the encoding to code points the
  /// specified `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input_view to read code units from and use in the decode operation that will
  /// produce code points.
  /// @param[in] encoding The encoding that will be used to decode the input's code points into
  /// output code units.
  /// @result A StatelessDecodeResult object whose output is of type `TOutputContainer`.
  /// @remarks This function creates a `handler` using Handlers::DefaultHandler, but marks it as careless.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding>
  constexpr auto DecodeTo(TInput &&input, TEncoding &&encoding)
  {
    Handlers::DefaultHandler handler {};
    return ::Krys::DecodeTo<TOutputContainer>(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                              handler);
  }

  /// @brief Converts the code units of the given `input` view through the encoding to code points the
  /// specified`TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input_view to read code units from and use in the decode operation that will produce
  /// code points.
  /// @result A StatelessDecodeResult object whose output is of type `TOutputContainer`.
  /// @remarks This function creates an `encoding` by using the `value_type` of the `input` which is then
  /// passed through the DefaultCodePointEncoding type to get the default desired encoding.
  template <typename TOutputContainer = void, typename TInput>
  constexpr auto DecodeTo(TInput &&input)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TCodeUnit = ::Krys::Ranges::range_value_type_t<TUInput>;
    if (std::is_constant_evaluated())
    {
      // Use literal encoding instead, if we meet the right criteria
      using TEncoding = default_consteval_code_unit_encoding_t<TCodeUnit>;
      TEncoding encoding {};
      return ::Krys::DecodeTo<TOutputContainer>(std::forward<TInput>(input), encoding);
    }
    else
    {
      using TEncoding = default_code_unit_encoding_t<TCodeUnit>;
      TEncoding encoding {};
      return ::Krys::DecodeTo<TOutputContainer>(std::forward<TInput>(input), encoding);
    }
  }

  /// @brief Converts the code units of the given `input` view through the encoding to code points the
  /// specified `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input_view to read code units from and use in the decode operation that will
  /// produce code points.
  /// @param[in] encoding The encoding that will be used to decode the input's code points into
  /// output code units.
  /// @param[in] errorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in,out] state A reference to the associated state for the `encoding` 's decode step.
  /// @result An object of type `TOutputContainer` .
  /// @remarks This function detects creates a container of type `TOutputContainer` and uses a typical @c
  /// std::back_inserter or `std::push_back_inserter` to fill in elements as it is written to.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding, typename TErrorHandler,
            typename TState>
  constexpr auto Decode(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler, TState &state)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TUOutputContainer = remove_cvref_t<TOutputContainer>;
    using TOutputCodePoint = code_point_t<TUEncoding>;
    constexpr bool IsVoidContainer = Void<TUOutputContainer>;
    constexpr bool IsStringable = (IsCharTraitable<TOutputCodePoint> || IsUnicodeCodePoint<TOutputCodePoint>);
    if constexpr (IsVoidContainer && IsStringable)
    {
      // prevent instantiation errors with basic_string by boxing it inside of an "if constexpr"
      using TRealOutputContainer = std::basic_string<TOutputCodePoint>;
      return Impl::DecodeDispatch<true, false, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
    else
    {
      using TRealOutputContainer =
        conditional_t<IsVoidContainer, std::vector<TOutputCodePoint>, TOutputContainer>;
      return Impl::DecodeDispatch<true, false, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
  }

  /// @brief Converts the code units of the given `input` view through the encoding to code points the
  /// specified `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input_view to read code units from and use in the decode operation that will
  /// produce code points.
  /// @param[in] encoding The encoding that will be used to decode the input's code points into
  /// output code units.
  /// @param[in] errorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @result An object of type `TOutputContainer` .
  /// @remarks This function creates a `state` using CreateDecodeState.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding, typename TErrorHandler>
  constexpr auto Decode(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TState = decode_state_t<TUEncoding>;
    using TUOutputContainer = remove_cvref_t<TOutputContainer>;
    using TOutputCodePoint = code_point_t<TUEncoding>;

    TState state = Krys::CreateDecodeState(encoding);
    constexpr bool IsVoidContainer = Void<TUOutputContainer>;
    constexpr bool IsStringable = (IsCharTraitable<TOutputCodePoint> || IsUnicodeCodePoint<TOutputCodePoint>);
    if constexpr (IsVoidContainer && IsStringable)
    {
      // prevent instantiation errors with basic_string by boxing it inside of an "if constexpr"
      using TRealOutputContainer = std::basic_string<TOutputCodePoint>;
      return Impl::DecodeDispatch<true, true, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
    else
    {
      using TRealOutputContainer =
        conditional_t<IsVoidContainer, std::vector<TOutputCodePoint>, TOutputContainer>;
      return Impl::DecodeDispatch<true, true, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
  }

  /// @brief Converts the code units of the given `input` view through the encoding to code points the
  /// specified `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input_view to read code units from and use in the decode operation that will
  /// produce code points.
  /// @param[in] encoding The encoding that will be used to decode the input's code points into
  /// output code units.
  /// @result An object of type `TOutputContainer` .
  /// @remarks This function creates a `handler` using Handlers::DefaultHandler, but marks it as careless.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding>
  constexpr auto Decode(TInput &&input, TEncoding &&encoding)
  {
    Handlers::DefaultHandler handler {};
    return Krys::Decode<TOutputContainer>(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                          handler);
  }

  /// @brief Converts the code units of the given `input` view through the encoding to code points the
  /// specified `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input_view to read code units from and use in the decode operation that will
  /// produce code points.
  /// @result An object of type `TOutputContainer` .
  /// @remarks This function creates an `encoding` by using the `value_type` of the `input` which is then
  /// passed through the DefaultCodePointEncoding type to get the default desired encoding.
  template <typename TOutputContainer = void, typename TInput>
  constexpr auto Decode(TInput &&input)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TCodeUnit = Krys::Ranges::range_value_type_t<TUInput>;
    if (std::is_constant_evaluated())
    {
      // Use literal encoding instead, if we meet the right criteria
      using TEncoding = default_consteval_code_unit_encoding_t<TCodeUnit>;
      TEncoding encoding {};
      return Krys::Decode<TOutputContainer>(std::forward<TInput>(input), encoding);
    }
    else
    {
      using TEncoding = default_code_unit_encoding_t<TCodeUnit>;
      TEncoding encoding {};
      return Krys::Decode<TOutputContainer>(std::forward<TInput>(input), encoding);
    }
  }
}
