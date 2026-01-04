#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/Impl/InsertBulk.hpp"
#include "Krystal.Lib/Ranges/Unbounded.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Text/CharTraits.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/DefaultEncoding.hpp"
#include "Krystal.Text/EncodeResult.hpp"
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
#include <vector>

namespace Krys
{
  /// @brief These functions convert from a view of input code points into a view of output code units using
  /// either the inferred or specified encoding. If no error handler is provided, the equivalent of the
  /// Handlers::DefaultHandler is used by default. If no associated state is provided for the encoding, one
  /// will be created with automatic storage duration (as a "stack" variable) for the provided encoding.

  /// @brief Converts the code points of the given `input` view through the encoding to code units into the
  /// `output` view.
  /// @param[in]     input An input_view to read code points from and use in the encode operation that will
  /// produce code units.
  /// @param[in]     encoding The encoding that will be used to encode the input's code points into
  /// output code units.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @param[in]     errorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in,out] state A reference to the associated state for the `encoding` 's encode step.
  /// @result A EncodeResult object that contains references to `state`.
  /// @remarks This function does not attempt to call any extension points for encoding. It simply uses the
  /// encoding and attempts to encode by repeatedly calling the encoding's required `EncodeOne` function.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler, typename TState>
  constexpr auto BasicEncodeIntoRaw(TInput &&input, TEncoding &&encoding, TOutput &&output,
                                    TErrorHandler &&errorHandler, TState &state)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TUErrorHandler = remove_cvref_t<TErrorHandler>;

    static_assert(Impl::EncodeLosslessOrDeliberate<TUEncoding, TUErrorHandler>,
                  ZTD_TEXT_LOSSY_ENCODE_MESSAGE_I_);

    std::size_t errorCount = 0;

    auto firstResult =
      encoding.EncodeOne(std::forward<TInput>(input), std::forward<TOutput>(output), errorHandler, state);
    using TWorkingInput = decltype(firstResult.Input);
    using TWorkingOutput = decltype(firstResult.Output);
    using TResult = EncodeResult<TWorkingInput, TWorkingOutput, TState>;
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
        encoding.EncodeOne(std::move(workingInput), std::move(workingOutput), errorHandler, state);
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

  /// @brief Converts the code points of the given `input` view through the encoding to code units into the
  /// `output` view.
  /// @param[in]     input An input_view to read code points from and use in the encode operation that will
  /// produce code units.
  /// @param[in]     encoding The encoding that will be used to encode the input's code points into
  /// output code units.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @param[in]     errorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in,out] state A reference to the associated state for the `encoding` 's encode step.
  /// @result A EncodeResult object that contains references to `state`.
  /// @remarks This function detects whether or not the ADL extension point `TextEncode` can be called with
  /// the provided parameters. If so, it will use that ADL extension point over the default implementation.
  /// Otherwise, it will loop over the two encodings and attempt to encode by repeatedly calling the
  /// encoding's required `EncodeOne` function.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler, typename TState>
  constexpr auto EncodeIntoRaw(TInput &&input, TEncoding &&encoding, TOutput &&output,
                               TErrorHandler &&errorHandler, TState &state)
  {
    if constexpr (Impl::HasTextEncode<TInput, TEncoding, TOutput, TErrorHandler, TState>)
    {
      return TextEncode(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::forward<TInput>(input),
                              std::forward<TEncoding>(encoding), std::forward<TOutput>(output),
                              std::forward<TErrorHandler>(errorHandler), state);
    }
    else if constexpr (Impl::HasInternalTextEncode<TInput, TEncoding, TOutput, TErrorHandler, TState>)
    {
      return InternalTextEncode(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::forward<TInput>(input),
                                      std::forward<TEncoding>(encoding), std::forward<TOutput>(output),
                                      std::forward<TErrorHandler>(errorHandler), state);
    }
    else
    {
      return Krys::BasicEncodeIntoRaw(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                      std::forward<TOutput>(output),
                                      std::forward<TErrorHandler>(errorHandler), state);
    }
  }

  /// @brief Converts the code points of the given `input` view through the encoding to code units into the
  /// `output` view.
  /// @param[in]     input An input_view to read code points from and use in the encode operation that will
  /// produce code units.
  /// @param[in]     encoding The encoding that will be used to encode the input's code points into
  /// output code units.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @param[in]     errorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @result A stateless_encode_result object that contains references to `state`.
  /// @remarks Creates a default `state` using Krys::CreateEncodeState.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler>
  constexpr auto EncodeIntoRaw(TInput &&input, TEncoding &&encoding, TOutput &&output,
                               TErrorHandler &&errorHandler)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TState = encode_state_t<TUEncoding>;

    TState state = ::Krys::CreateEncodeState(encoding);
    auto statefulResult =
      Krys::EncodeIntoRaw(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                          std::forward<TOutput>(output), std::forward<TErrorHandler>(errorHandler), state);
    return Impl::SliceToStatelessEncode(std::move(statefulResult));
  }

  /// @brief Converts the code points of the given `input` view through the encoding to code units into the
  /// `output` view.
  /// @param[in]     input An input_view to read code points from and use in the encode operation that will
  /// produce code units.
  /// @param[in]     encoding The encoding that will be used to encode the input's code points into
  /// output code units.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @result A stateless_encode_result object that contains references to `state`.
  /// @remarks Creates a default `ErrorHandler` that is similar to Handlers::DefaultHandler, but marked as
  /// careless.
  template <typename TInput, typename TEncoding, typename TOutput>
  constexpr auto EncodeIntoRaw(TInput &&input, TEncoding &&encoding, TOutput &&output)
  {
    Handlers::DefaultHandler handler {};
    return Krys::EncodeIntoRaw(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                               std::forward<TOutput>(output), handler);
  }

  /// @brief Converts the code points of the given `input` view through the encoding to code units into the
  /// `output` view.
  /// @param[in]     input An input_view to read code points from and use in the encode operation that will
  /// produce code units.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @result A stateless_encode_result object that contains references to `state`.
  /// @remarks Creates a default `encoding` by figuring out the `value_type` of the `input`, then passing
  /// that type into default_code_point_encoding_t. That encoding is that used to encode the input
  /// code points, by default.
  template <typename TInput, typename TOutput>
  constexpr auto EncodeIntoRaw(TInput &&input, TOutput &&output)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TCodePoint = Krys::Ranges::range_value_type_t<TUInput>;
    if (std::is_constant_evaluated())
    {
      // Use literal encoding instead, if we meet the right criteria
      using TEncoding = default_consteval_code_point_encoding_t<TCodePoint>;
      TEncoding encoding {};
      return Krys::EncodeIntoRaw(std::forward<TInput>(input), encoding, std::forward<TOutput>(output));
    }
    else
    {
      using TEncoding = default_code_point_encoding_t<TCodePoint>;
      TEncoding encoding {};
      return Krys::EncodeIntoRaw(std::forward<TInput>(input), encoding, std::forward<TOutput>(output));
    }
  }

  namespace Impl
  {
    template <typename TInput, typename TEncoding, typename TOutputContainer, typename TErrorHandler,
              typename TState>
    constexpr auto IntermediateEncodeToStorage(TInput &&input, TEncoding &&encoding, TOutputContainer &output,
                                               TErrorHandler &&errorHandler, TState &state)
    {
      // Well, SHIT. Write into temporary, then serialize one-by-one/bulk to output.
      // I'll admit, this is HELLA work to support...
      using TUEncoding = remove_cvref_t<TEncoding>;
      using TUErrorHandler = remove_cvref_t<TErrorHandler>;
      constexpr std::size_t intermediateBufferMax = MaxEncodeCodeUnits<TUEncoding>;
      using TIntermediateValue = code_unit_t<TUEncoding>;
      using TIntermediateInput = Impl::span_reconstruct_t<TInput, TInput>;
      using TInitialOutput = Span<TIntermediateValue>;
      using TOutput = Span<TIntermediateValue>;
      using TResult = decltype(Krys::EncodeIntoRaw(std::declval<TIntermediateInput>(), encoding,
                                                   std::declval<TOutput>(), errorHandler, state));
      using TWorkingInput = remove_cvref_t<decltype(std::declval<TResult>().Input)>;

      static_assert(Impl::EncodeLosslessOrDeliberate<TUEncoding, TUErrorHandler>,
                    ZTD_TEXT_LOSSY_ENCODE_MESSAGE_I_);

      // We MUST use a temporary error handler
      // as well as a pass-throuugh handler if we end up with lots of intermediary input
      Impl::ProgressHandler<IsIgnorableErrorHandler<TUErrorHandler>, TUEncoding> intermediateHandler {};

      TIntermediateInput intermediateInput = Impl::SpanReconstruct<TInput>(std::forward<TInput>(input));
      TWorkingInput workingInput(std::move(intermediateInput));
      TIntermediateValue intermediateTranslationBuffer[intermediateBufferMax] {};

      for (;;)
      {
        // Ignore "out of output" errors and do our best to recover properly along the way...
        TInitialOutput intermediateInitialOutput(intermediateTranslationBuffer, intermediateBufferMax);
        auto result = Krys::EncodeIntoRaw(std::move(workingInput), encoding, intermediateInitialOutput,
                                          intermediateHandler, state);
        TOutput intermediateOutput(intermediateInitialOutput.data(), result.Output.data());
        Krys::Ranges::Impl::ContainerInsertBulk(output, intermediateOutput);
        if (result.ErrorCode == EncodingError::InsufficientOutputSpace)
        {
          if (intermediateHandler.CodeUnitsProgressSize() != 0)
          {
            // add any leftover partially-unwritten characters to our output
            Krys::Ranges::Impl::ContainerInsertBulk(output, intermediateHandler.ConstCodeUnitsProgress());
            // it's okay, just loop around, we've got S P A C E for more
            workingInput = Impl::UpdateInput<TWorkingInput>(std::move(result.Input));
          }
          else if (intermediateHandler.CodePointsProgressSize() != 0)
          {
            if constexpr (Krys::Ranges::BidirectionalRange<TWorkingInput>)
            {
              // we can try to rewind our current input by the amount that was not successfully
              // read. This will allow us to try again, when the buffer
              // has more space in it, and should not result in the same error,
              // unless it was legitimiately an EncodingError::InvalidSequence.
              workingInput = Impl::UpdateInput<TWorkingInput>(Krys::Ranges::reconstruct(
                std::in_place_type<TWorkingInput>,
                Krys::Ranges::iter_recede(Krys::Ranges::begin(std::move(result.Input)),
                                          intermediateHandler.CodePointsProgressSize()),
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
            std::forward<TEncoding>(encoding), std::move(result), intermediateHandler.CodePointsProgress(),
            intermediateHandler.CodeUnitsProgress());
          return TResult(errorResult);
        }
        if (Krys::Ranges::empty(result.Input))
        {
          if (!Krys::IsStateComplete(encoding, state))
          {
            continue;
          }
          return TResult(result);
        }
      }
    }

    template <bool OutputOnly, bool NoState, typename TOutputContainer, typename TInput, typename TEncoding,
              typename TErrorHandler, typename TState>
    constexpr auto EncodeDispatch(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler,
                                  TState &state)
    {
      using TUEncoding = remove_cvref_t<TEncoding>;
      constexpr std::size_t maxUnits = MaxEncodeCodeUnits<TUEncoding>;

      TOutputContainer output {};
      if constexpr (Krys::Ranges::has_adl_size<TInput>)
      {
        using TSize = decltype(Krys::Ranges::size(input));
        if constexpr (Krys::Ranges::has_reserve_with_size<TOutputContainer, TSize>)
        {
          TSize outputSizeHint = static_cast<TSize>(Krys::Ranges::size(input));
          outputSizeHint *= (maxUnits > 3) ? (maxUnits / 4) : maxUnits;
          output.reserve(outputSizeHint);
        }
      }
      auto statefulResult =
        Impl::IntermediateEncodeToStorage(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                          output, std::forward<TErrorHandler>(errorHandler), state);
      if constexpr (OutputOnly)
      {
        (void)statefulResult;
        return output;
      }
      else if constexpr (NoState)
      {
        return Impl::ReplaceEncodeResultOutputNoState(std::move(statefulResult), std::move(output));
      }
      else
      {
        return Impl::ReplaceEncodeResultOutput(std::move(statefulResult), std::move(output));
      }
    }
  }

  /// @brief Converts the code points of the given `input` view through the encoding to code units into the
  /// `output` view.
  /// @param[in]     input An input_view to read code points from and use in the encode operation that will
  /// produce code units.
  /// @param[in]     encoding The encoding that will be used to encode the input's code points into
  /// output code units.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @param[in]     errorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in,out] state A reference to the associated state for the `encoding` 's encode step.
  /// @result A EncodeResult object that contains references to `state`.
  /// @remarks This function detects whether or not the ADL extension point `TextEncode` can be called with
  /// the provided parameters. If so, it will use that ADL extension point over the default implementation.
  /// Otherwise, it will loop over the two encodings and attempt to encode by repeatedly calling the
  /// encoding's required `EncodeOne` function.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler, typename TState>
  constexpr auto BasicEncodeInto(TInput &&input, TEncoding &&encoding, TOutput &&output,
                                 TErrorHandler &&errorHandler, TState &state)
  {
    auto reconstructedInput = Impl::SpanReconstruct<TInput>(std::forward<TInput>(input));
    auto result = Krys::BasicEncodeIntoRaw(std::move(reconstructedInput), std::forward<TEncoding>(encoding),
                                           std::forward<TOutput>(output),
                                           std::forward<TErrorHandler>(errorHandler), state);
    using TReconstructedResultInput = Impl::span_reconstruct_t<TInput, TInput>;
    using TReconstructedResultOutput = Impl::span_reconstruct_mutable_t<TOutput, TOutput>;
    using TResult = EncodeResult<TReconstructedResultInput, TReconstructedResultOutput, TState>;
    return TResult(Impl::SpanReconstruct<TInput>(std::move(result.Input)),
                   Impl::SpanReconstructMutable<TOutput>(std::move(result.Output)), result.State);
  }

  /// @brief Converts the code points of the given `input` view through the encoding to code units into the
  /// `output` view.
  /// @param[in]     input An input_view to read code points from and use in the encode operation that will
  /// produce code units.
  /// @param[in]     encoding The encoding that will be used to encode the input's code points into
  /// output code units.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @param[in]     errorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in,out] state A reference to the associated state for the `encoding` 's encode step.
  /// @result A EncodeResult object that contains references to `state`.
  /// @remarks This function detects whether or not the ADL extension point `TextEncode` can be called with
  /// the provided parameters. If so, it will use that ADL extension point over the default implementation.
  /// Otherwise, it will loop over the two encodings and attempt to encode by repeatedly calling the
  /// encoding's required `EncodeOne` function.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler, typename TState>
  constexpr auto EncodeInto(TInput &&input, TEncoding &&encoding, TOutput &&output,
                            TErrorHandler &&errorHandler, TState &state)
  {
    auto reconstructedInput = Impl::SpanReconstruct<TInput>(std::forward<TInput>(input));
    auto result =
      Krys::EncodeIntoRaw(std::move(reconstructedInput), std::forward<TEncoding>(encoding),
                          std::forward<TOutput>(output), std::forward<TErrorHandler>(errorHandler), state);
    using TReconstructedResultInput = Impl::span_reconstruct_t<TInput, TInput>;
    using TReconstructedResultOutput = Impl::span_reconstruct_mutable_t<TOutput, TOutput>;
    using TResult = EncodeResult<TReconstructedResultInput, TReconstructedResultOutput, TState>;
    return TResult(Impl::SpanReconstruct<TInput>(std::move(result.Input)),
                   Impl::SpanReconstructMutable<TOutput>(std::move(result.Output)), result.State);
  }

  /// @brief Converts the code points of the given `input` view through the encoding to code units into the
  /// `output` view.
  /// @param[in]     input An input_view to read code points from and use in the encode operation that will
  /// produce code units.
  /// @param[in]     encoding The encoding that will be used to encode the input's code points into
  /// output code units.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @param[in]     errorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @result A stateless_encode_result object that contains references to `state`.
  /// @remarks Creates a default `state` using Krys::CreateEncodeState.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler>
  constexpr auto EncodeInto(TInput &&input, TEncoding &&encoding, TOutput &&output,
                            TErrorHandler &&errorHandler)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TState = encode_state_t<TUEncoding>;

    TState state = ::Krys::CreateEncodeState(encoding);
    auto statefulResult =
      Krys::EncodeInto(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                       std::forward<TOutput>(output), std::forward<TErrorHandler>(errorHandler), state);
    return Impl::SliceToStatelessEncode(std::move(statefulResult));
  }

  /// @brief Converts the code points of the given `input` view through the encoding to code units into the
  /// `output` view.
  /// @param[in]     input An input_view to read code points from and use in the encode operation that will
  /// produce code units.
  /// @param[in]     encoding The encoding that will be used to encode the input's code points into
  /// output code units.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @result A stateless_encode_result object that contains references to `state`.
  /// @remarks Creates a default `ErrorHandler` that is similar to Handlers::DefaultHandler, but marked as
  /// careless.
  template <typename TInput, typename TEncoding, typename TOutput>
  constexpr auto EncodeInto(TInput &&input, TEncoding &&encoding, TOutput &&output)
  {
    Handlers::DefaultHandler handler {};
    return Krys::EncodeInto(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                            std::forward<TOutput>(output), handler);
  }

  /// @brief Converts the code points of the given `input` view through the encoding to code units into the
  /// `output` view.
  /// @param[in]     input An input_view to read code points from and use in the encode operation that will
  /// produce code units.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @result A stateless_encode_result object that contains references to `state`.
  /// @remarks Creates a default `encoding` by figuring out the `value_type` of the `input`, then passing
  /// that type into default_code_point_encoding_t. That encoding is that used to encode the input
  /// code points, by default.
  template <typename TInput, typename TOutput>
  constexpr auto EncodeInto(TInput &&input, TOutput &&output)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TCodePoint = Krys::Ranges::range_value_type_t<TUInput>;
    if (std::is_constant_evaluated())
    {
      // Use literal encoding instead, if we meet the right criteria
      using TEncoding = default_consteval_code_point_encoding_t<TCodePoint>;
      TEncoding encoding {};
      return Krys::EncodeInto(std::forward<TInput>(input), encoding, std::forward<TOutput>(output));
    }
    else
    {
      using TEncoding = default_code_point_encoding_t<TCodePoint>;
      TEncoding encoding {};
      return Krys::EncodeInto(std::forward<TInput>(input), encoding, std::forward<TOutput>(output));
    }
  }

  /// @brief Converts the code points of the given `input` view through the encoding to code units in the
  /// specified `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in]     input An input_view to read code points from and use in the encode operation that will
  /// produce code units.
  /// @param[in]     encoding The encoding that will be used to encode the input's code points into
  /// output code units.
  /// @param[in]     errorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in,out] state A reference to the associated state for the `encoding` 's encode step.
  /// @result A EncodeResult object that contains references to `state` and an output of type
  /// `TOutputContainer`.
  /// @remarks This function detects creates a container of type `TOutputContainer` and uses a typical @c
  /// std::back_inserter or `std::push_back_inserter` to fill in elements as it is written to. The result is
  /// then returned, with the `.Output` value put into the container.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding, typename TErrorHandler,
            typename TState>
  constexpr auto EncodeTo(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler, TState &state)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TUOutputContainer = remove_cvref_t<TOutputContainer>;
    using TOutputCodeUnit = code_unit_t<TUEncoding>;
    constexpr bool IsVoidContainer = Void<TUOutputContainer>;
    constexpr bool IsStringable = (IsCharTraitable<TOutputCodeUnit> || IsUnicodeCodePoint<TOutputCodeUnit>);
    if constexpr (IsVoidContainer && IsStringable)
    {
      // prevent instantiation errors with basic_string by boxing it inside of an "if constexpr"
      using TRealOutputContainer = std::basic_string<TOutputCodeUnit>;
      return Impl::EncodeDispatch<false, false, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
    else
    {
      using TRealOutputContainer =
        conditional_t<IsVoidContainer, std::vector<TOutputCodeUnit>, TOutputContainer>;
      return Impl::EncodeDispatch<false, false, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
  }

  /// @brief Converts the code points of the given `input` view through the encoding to code units in the
  /// specified `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in]     input An input_view to read code points from and use in the encode operation that will
  /// produce code units.
  /// @param[in]     encoding The encoding that will be used to encode the input's code points into
  /// output code units.
  /// @param[in]     errorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @result A stateless_encode_result object whose output is of type `TOutputContainer`.
  /// @remarks This function creates a `state` using Krys::CreateEncodeState.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding, typename TErrorHandler>
  constexpr auto EncodeTo(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TState = encode_state_t<TUEncoding>;
    using TUOutputContainer = remove_cvref_t<TOutputContainer>;
    using TOutputCodeUnit = code_unit_t<TUEncoding>;

    TState state = ::Krys::CreateEncodeState(encoding);
    constexpr bool IsVoidContainer = Void<TUOutputContainer>;
    constexpr bool IsStringable = (IsCharTraitable<TOutputCodeUnit> || IsUnicodeCodePoint<TOutputCodeUnit>);
    if constexpr (IsVoidContainer && IsStringable)
    {
      // prevent instantiation errors with basic_string by boxing it inside of an "if constexpr"
      using TRealOutputContainer = std::basic_string<TOutputCodeUnit>;
      return Impl::EncodeDispatch<false, true, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
    else
    {
      using TRealOutputContainer =
        conditional_t<IsVoidContainer, std::vector<TOutputCodeUnit>, TOutputContainer>;
      return Impl::EncodeDispatch<false, true, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
  }

  /// @brief Converts the code points of the given `input` view through the encoding to code units in the
  /// specified `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in]     input An input_view to read code points from and use in the encode operation that will
  /// produce code units.
  /// @param[in]     encoding The encoding that will be used to encode the input's code points into
  /// output code units.
  /// @result A stateless_encode_result object whose output is of type `TOutputContainer`.
  /// @remarks This function creates a `handler` using Handlers::DefaultHandler, but marks it as careless.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding>
  constexpr auto EncodeTo(TInput &&input, TEncoding &&encoding)
  {
    Handlers::DefaultHandler handler {};
    return Krys::EncodeTo<TOutputContainer>(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                            handler);
  }

  /// @brief Converts the code points of the given `input` view through the encoding to code units in the
  /// specified `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in]     input An input_view to read code points from and use in the encode operation that will
  /// produce code units.
  /// @result A stateless_encode_result object whose output is of type `TOutputContainer`.
  /// @remarks This function creates an `encoding` by using the `value_type` of the `input` which is then
  /// passed through the DefaultCodePointEncoding type to get the default desired encoding.
  template <typename TOutputContainer = void, typename TInput>
  constexpr auto EncodeTo(TInput &&input)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TCodePoint = Krys::Ranges::range_value_type_t<TUInput>;
    if (std::is_constant_evaluated())
    {
      // Use literal encoding instead, if we meet the right criteria
      using TEncoding = default_consteval_code_point_encoding_t<TCodePoint>;
      TEncoding encoding {};
      return Krys::EncodeTo<TOutputContainer>(std::forward<TInput>(input), encoding);
    }
    else
    {
      using TEncoding = default_code_point_encoding_t<TCodePoint>;
      TEncoding encoding {};
      return Krys::EncodeTo<TOutputContainer>(std::forward<TInput>(input), encoding);
    }
  }

  /// @brief Converts the code points of the given `input` view through the encoding to code units in the
  /// specified `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in]     input An input_view to read code points from and use in the encode operation that will
  /// produce code units.
  /// @param[in]     encoding The encoding that will be used to encode the input's code points into
  /// output code units.
  /// @param[in]     errorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in,out] state A reference to the associated state for the `encoding` 's encode step.
  /// @result An object of type `TOutputContainer` .
  /// @remarks This function detects creates a container of type `TOutputContainer` and uses a typical @c
  /// std::back_inserter or `std::push_back_inserter` to fill in elements as it is written to.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding, typename TErrorHandler,
            typename TState>
  constexpr auto Encode(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler, TState &state)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TUOutputContainer = remove_cvref_t<TOutputContainer>;
    using TOutputCodeUnit = code_unit_t<TUEncoding>;
    constexpr bool IsVoidContainer = Void<TUOutputContainer>;
    constexpr bool IsStringable = (IsCharTraitable<TOutputCodeUnit> || IsUnicodeCodePoint<TOutputCodeUnit>);
    if constexpr (IsVoidContainer && IsStringable)
    {
      // prevent instantiation errors with basic_string by boxing it inside of an "if constexpr"
      using TRealOutputContainer = std::basic_string<TOutputCodeUnit>;
      return Impl::EncodeDispatch<true, false, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
    else
    {
      using TRealOutputContainer =
        conditional_t<IsVoidContainer, std::vector<TOutputCodeUnit>, TOutputContainer>;
      return Impl::EncodeDispatch<true, false, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
  }

  /// @brief Converts the code points of the given `input` view through the encoding to code units in the
  /// specified `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in]     input An input_view to read code points from and use in the encode operation that will
  /// produce code units.
  /// @param[in]     encoding The encoding that will be used to encode the input's code points into
  /// output code units.
  /// @param[in]     errorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @result An object of type `TOutputContainer` .
  /// @remarks This function creates a `state` using Krys::CreateEncodeState.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding, typename TErrorHandler>
  constexpr auto Encode(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TState = encode_state_t<TUEncoding>;
    using TUOutputContainer = remove_cvref_t<TOutputContainer>;
    using TOutputCodeUnit = code_unit_t<TUEncoding>;

    TState state = ::Krys::CreateEncodeState(encoding);
    constexpr bool IsVoidContainer = Void<TUOutputContainer>;
    constexpr bool IsStringable = (IsCharTraitable<TOutputCodeUnit> || IsUnicodeCodePoint<TOutputCodeUnit>);
    if constexpr (IsVoidContainer && IsStringable)
    {
      // prevent instantiation errors with basic_string by boxing it inside of an "if constexpr"
      using TRealOutputContainer = std::basic_string<TOutputCodeUnit>;
      return Impl::EncodeDispatch<true, true, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
    else
    {
      using TRealOutputContainer =
        conditional_t<IsVoidContainer, std::vector<TOutputCodeUnit>, TOutputContainer>;
      return Impl::EncodeDispatch<true, true, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
  }

  /// @brief Converts the code points of the given `input` view through the encoding to code units in the
  /// specified `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in]     input An input_view to read code points from and use in the encode operation that will
  /// produce code units.
  /// @param[in]     encoding The encoding that will be used to encode the input's code points into
  /// output code units.
  /// @result An object of type `TOutputContainer` .
  /// @remarks This function creates a `handler` using Handlers::DefaultHandler, but marks it as careless.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding>
  constexpr auto Encode(TInput &&input, TEncoding &&encoding)
  {
    Handlers::DefaultHandler handler {};
    return Krys::Encode<TOutputContainer>(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                          handler);
  }

  /// @brief Converts the code points of the given `input` view through the encoding to code units in the
  /// specified `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in]     input An input_view to read code points from and use in the encode operation that will
  /// produce code units.
  /// @result An object of type `TOutputContainer` .
  /// @remarks This function creates an `encoding` by using the `value_type` of the `input` which is then
  /// passed through the DefaultCodePointEncoding type to get the default desired encoding.
  template <typename TOutputContainer = void, typename TInput>
  constexpr auto Encode(TInput &&input)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TCodePoint = Krys::Ranges::range_value_type_t<TUInput>;
    if (std::is_constant_evaluated())
    {
      // Use literal encoding instead, if we meet the right criteria
      using TEncoding = default_consteval_code_point_encoding_t<TCodePoint>;
      TEncoding encoding {};
      return Krys::Encode<TOutputContainer>(std::forward<TInput>(input), encoding);
    }
    else
    {
      using TEncoding = default_code_point_encoding_t<TCodePoint>;
      TEncoding encoding {};
      return Krys::Encode<TOutputContainer>(std::forward<TInput>(input), encoding);
    }
  }
}
