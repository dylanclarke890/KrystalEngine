#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/Impl/InsertBulk.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Utils/Tag.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/Concepts.hpp"
#include "Krystal.Text/Decode/DecodeResult.hpp"
#include "Krystal.Text/detail/IsLossless.hpp"
#include "Krystal.Text/detail/SpanReconstruct.hpp"
#include "Krystal.Text/detail/UpdateInput.hpp"
#include "Krystal.Text/Encodings/DefaultEncoding.hpp"
#include "Krystal.Text/Handlers/DefaultHandler.hpp"
#include "Krystal.Text/Handlers/ProgressHandler.hpp"
#include "Krystal.Text/IsUnicodeCodePoint.hpp"
#include "Krystal.Text/MaxUnits.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/TypeTraits.hpp"
#include <string>
#include <utility>
#include <vector>

/// These functions convert from a view of input code units into a view of output code points using either the
/// inferred or specified encodings. If no error handler is provided, DefaultHandler is used by default. If no
/// state is provided for the encoding, it will created automatically as needed.
namespace Krys::Text
{
  /// @brief Converts from code units from the given `input` view into code points in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] encoding The encoding for decoding the `input`'s code units into `output` code points.
  /// @param[in] output An output view to write code points to.
  /// @param[in] errorHandler The handler to use for decoding errors.
  /// @param[in,out] state A reference to the associated state for the `encoding`'s decode step.
  /// @result A DecodeResult object that contains references to `state`.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler, typename TState>
  constexpr auto BasicDecodeIntoRaw(TInput &&input, TEncoding &&encoding, TOutput &&output,
                                    TErrorHandler &&errorHandler, TState &state)
  {
    static_assert(::Krys::Text::DecodeLosslessOrDeliberate<TEncoding, TErrorHandler>,
                  KRYS_TEXT_LOSSY_DECODE_MESSAGE);

    auto firstResult =
      encoding.DecodeOne(std::forward<TInput>(input), std::forward<TOutput>(output), errorHandler, state);

    using TWorkingInput = decltype(firstResult.Input);
    using TWorkingOutput = decltype(firstResult.Output);
    using TResult = ::Krys::Text::DecodeResult<TWorkingInput, TWorkingOutput, TState>;

    if (firstResult.ErrorCode != EncodingError::OK)
    {
      return TResult(std::move(firstResult.Input), std::move(firstResult.Output), state,
                     firstResult.ErrorCode, firstResult.ErrorCount);
    }

    if (::Krys::Ranges::empty(firstResult.Input) && ::Krys::Text::IsStateComplete(encoding, state))
    {
      return TResult(std::move(firstResult.Input), std::move(firstResult.Output), state, EncodingError::OK,
                     firstResult.ErrorCount);
    }

    TWorkingInput workingInput = std::move(firstResult.Input);
    TWorkingOutput workingOutput = std::move(firstResult.Output);

    std::size_t errorCount = firstResult.ErrorCount;
    for (;;)
    {
      auto result =
        encoding.DecodeOne(std::move(workingInput), std::move(workingOutput), errorHandler, state);
      workingInput = std::move(result.Input);
      workingOutput = std::move(result.Output);
      errorCount += result.ErrorCount;

      if (result.ErrorCode != EncodingError::OK)
      {
        return TResult(std::move(workingInput), std::move(workingOutput), state, result.ErrorCode,
                       errorCount);
      }

      if (::Krys::Ranges::empty(workingInput))
      {
        if (!::Krys::Text::IsStateComplete(encoding, state))
        {
          continue;
        }
        break;
      }
    }

    return TResult(std::move(workingInput), std::move(workingOutput), state, EncodingError::OK, errorCount);
  }

  /// @brief Converts from code units from the given `input` view into code points in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] encoding The encoding for decoding the `input`'s code units into `output` code points.
  /// @param[in] output An output view to write code points to.
  /// @param[in] errorHandler The handler to use for decoding errors.
  /// @param[in,out] state A reference to the associated state for the `encoding`'s decode step.
  /// @result A DecodeResult object that contains references to `state`.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler, typename TState>
  constexpr auto DecodeIntoRaw(TInput &&input, TEncoding &&encoding, TOutput &&output,
                               TErrorHandler &&errorHandler, TState &state)
  {
    if constexpr (::Krys::Text::HasDecodeText<TInput, TEncoding, TOutput, TErrorHandler, TState>)
    {
      return DecodeText(::Krys::Tag<remove_cvref_t<TEncoding>> {}, std::forward<TInput>(input),
                        std::forward<TEncoding>(encoding), std::forward<TOutput>(output),
                        std::forward<TErrorHandler>(errorHandler), state);
    }
    else
    {
      return ::Krys::Text::BasicDecodeIntoRaw(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                              std::forward<TOutput>(output),
                                              std::forward<TErrorHandler>(errorHandler), state);
    }
  }

  /// @brief Converts from code units from the given `input` view into code points in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] encoding The encoding for decoding the `input`'s code units into `output` code points.
  /// @param[in] output An output view to write code points to.
  /// @param[in] errorHandler The handler to use for decoding errors.
  /// @result A StatelessDecodeResult object.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler>
  constexpr auto DecodeIntoRaw(TInput &&input, TEncoding &&encoding, TOutput &&output,
                               TErrorHandler &&errorHandler)
  {
    auto state = ::Krys::Text::CreateDecodeState(encoding);
    auto statefulResult = ::Krys::Text::DecodeIntoRaw(
      std::forward<TInput>(input), std::forward<TEncoding>(encoding), std::forward<TOutput>(output),
      std::forward<TErrorHandler>(errorHandler), state);

    return ::Krys::Text::detail_result::SliceToStatelessDecode(std::move(statefulResult));
  }

  /// @brief Converts from code units from the given `input` view into code points in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] encoding The encoding for decoding the `input`'s code units into `output` code points.
  /// @param[in] output An output view to write code points to.
  /// @result A StatelessDecodeResult object.
  template <typename TInput, typename TEncoding, typename TOutput>
  constexpr auto DecodeIntoRaw(TInput &&input, TEncoding &&encoding, TOutput &&output)
  {
    ::Krys::Text::Handlers::DefaultHandler handler {};
    return ::Krys::Text::DecodeIntoRaw(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                       std::forward<TOutput>(output), handler);
  }

  /// @brief Converts from code units from the given `input` view into code points in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] output An output view to write code points to.
  /// @result A StatelessDecodeResult object.
  /// @remarks Creates a default encoding using the underlying value type of `input`.
  template <typename TInput, typename TOutput>
  constexpr auto DecodeIntoRaw(TInput &&input, TOutput &&output)
  {
    using TCodeUnit = ::Krys::Ranges::range_value_type_t<TInput>;
    using TEncoding =
      conditional_t<std::is_constant_evaluated(), default_consteval_code_unit_encoding_t<TCodeUnit>,
                    default_code_unit_encoding_t<TCodeUnit>>;
    TEncoding encoding {};
    return ::Krys::Text::DecodeIntoRaw(std::forward<TInput>(input), encoding, std::forward<TOutput>(output));
  }
}

namespace Krys::Text::detail_decode
{
  template <typename TInput, typename TEncoding, typename TOutputContainer, typename TErrorHandler,
            typename TState>
  constexpr auto IntermediateDecodeToStorage(TInput &&input, TEncoding &&encoding, TOutputContainer &output,
                                             TErrorHandler &&errorHandler, TState &state)
  {
    static_assert(::Krys::Text::DecodeLosslessOrDeliberate<TEncoding, TErrorHandler>,
                  KRYS_TEXT_LOSSY_DECODE_MESSAGE);

    // Write into temporary, then serialize one-by-one/bulk to output.
    constexpr std::size_t MaxBufferSize = ::Krys::Text::MaxDecodeCodePoints<TEncoding>;
    using TCodePoint = code_point_t<TEncoding>;

    using TIntermediateInput = ::Krys::Text::detail::span_reconstruct_t<TInput, TInput>;
    using TResult = decltype(encoding.DecodeOne(std::declval<TIntermediateInput>(),
                                                std::declval<Span<TCodePoint>>(), errorHandler, state));
    using TResultInput = decltype(std::declval<TResult>().Input);
    using TWorkingInput = remove_cvref_t<decltype(std::declval<TResult>().Input)>;

    constexpr bool AssumeValid = ::Krys::Text::IsIgnorableErrorHandler<TErrorHandler>;
    ::Krys::Text::Handlers::ProgressHandler<AssumeValid, TEncoding> progressHandler {};

    TIntermediateInput intermediateInput =
      ::Krys::Text::detail::SpanReconstruct<TInput>(std::forward<TInput>(input));
    TWorkingInput workingInput(std::move(intermediateInput));

    TCodePoint translationBuffer[MaxBufferSize] {};

    for (;;)
    {
      Span<TCodePoint, MaxBufferSize> initialOutput(translationBuffer);

      auto result =
        ::Krys::Text::DecodeIntoRaw(std::move(workingInput), encoding, initialOutput, progressHandler, state);

      ::Krys::Ranges::Impl::ContainerInsertBulk(output, Span(initialOutput.data(), result.Output.data()));

      if (result.ErrorCode == EncodingError::InsufficientOutputSpace)
      {
        if (progressHandler.CodePointsProgressSize() != 0)
        {
          // add any leftover partially-unwritten characters to our output
          ::Krys::Ranges::Impl::ContainerInsertBulk(output, progressHandler.ConstCodePointsProgress());
          // it's okay, just loop around, we've got S P A C E for more
          workingInput = ::Krys::Text::detail::UpdateInput<TWorkingInput>(std::move(result.Input));
        }
        else if (progressHandler.CodeUnitsProgressSize() != 0)
        {
          if constexpr (::Krys::Ranges::BidirectionalRange<TWorkingInput>)
          {
            // we can try to rewind our current input by the amount that was not successfully
            // read. This will allow us to try again, when the buffer has more space in it, and should not
            // result in the same error, unless it was legitimiately an EncodingError::InvalidSequence.
            workingInput = ::Krys::Text::detail::UpdateInput<TWorkingInput>(::Krys::Ranges::reconstruct(
              std::in_place_type<TWorkingInput>,
              ::Krys::Ranges::iter_recede(::Krys::Ranges::begin(result.Input),
                                          progressHandler.CodeUnitsProgressSize()),
              ::Krys::Ranges::end(result.Input)));
          }
          else
          {
            // this is an effectively-impossible case, as we cannot stitch the old input together
            // with the current input. simply bail!!
            return result;
          }
        }
        else
        {
          // it's okay, just loop around, we've got S P A C E for more
          workingInput = ::Krys::Text::detail::UpdateInput<TWorkingInput>(std::move(result.Input));
        }
        progressHandler.Clear();
        continue;
      }

      if (result.ErrorCode != EncodingError::OK)
      {
        // mill result through actual error handler!
        auto errorResult = std::forward<TErrorHandler>(errorHandler)(
          std::forward<TEncoding>(encoding), std::move(result), progressHandler.CodeUnitsProgress(),
          progressHandler.CodePointsProgress());
        return errorResult;
      }

      if (::Krys::Ranges::empty(result.Input))
      {
        if (!::Krys::Text::IsStateComplete(encoding, state))
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
    TOutputContainer output {};
    if constexpr (::Krys::Ranges::HasSizeADL<TInput>)
    {
      using TSize = decltype(::Krys::Ranges::size(input));
      if constexpr (::Krys::Ranges::has_reserve_with_size<TOutputContainer, TSize>)
      {
        auto outputSizeHint = ::Krys::Ranges::size(input) * ::Krys::Text::MaxDecodeCodePoints<TEncoding> / 2;
        output.reserve(outputSizeHint);
      }
    }

    auto statefulResult = ::Krys::Text::detail_decode::IntermediateDecodeToStorage(
      std::forward<TInput>(input), std::forward<TEncoding>(encoding), output,
      std::forward<TErrorHandler>(errorHandler), state);

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

namespace Krys::Text
{
  /// @brief Converts code units of the given `input` view into code points in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] encoding The encoding for decoding the `input`'s code units into `output` code points.
  /// @param[in] output An output view to write code points to.
  /// @param[in] errorHandler The handler to use for decoding errors.
  /// @param[in,out] state A reference to the associated state for the `encoding`'s decode step.
  /// @result A DecodeResult object that contains references to `state`.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler, typename TState>
  constexpr auto BasicDecodeInto(TInput &&input, TEncoding &&encoding, TOutput &&output,
                                 TErrorHandler &&errorHandler, TState &state)
  {
    auto reconstructedInput = ::Krys::Text::detail::SpanReconstruct<TInput>(std::forward<TInput>(input));
    auto result =
      ::Krys::Text::BasicDecodeIntoRaw(std::move(reconstructedInput), std::forward<TEncoding>(encoding),
                                       std::forward<TOutput>(output), errorHandler, state);

    return ::Krys::Text::DecodeResult(
      ::Krys::Text::detail::SpanReconstruct<TInput>(std::move(result.Input)),
      ::Krys::Text::detail::SpanReconstructMutable<TOutput>(std::move(result.Output)), result.State);
  }

  /// @brief Converts code units of the given `input` view into code points in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] encoding The encoding for decoding the `input`'s code units into `output` code points.
  /// @param[in] output An output view to write code points to.
  /// @param[in] errorHandler The handler to use for decoding errors.
  /// @param[in,out] state A reference to the associated state for the `encoding`'s decode step.
  /// @result A DecodeResult object that contains references to `state`.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler, typename TState>
  constexpr auto DecodeInto(TInput &&input, TEncoding &&encoding, TOutput &&output,
                            TErrorHandler &&errorHandler, TState &state)
  {
    auto reconstructedInput = ::Krys::Text::detail::SpanReconstruct<TInput>(std::forward<TInput>(input));
    auto result =
      ::Krys::Text::DecodeIntoRaw(std::move(reconstructedInput), std::forward<TEncoding>(encoding),
                                  std::forward<TOutput>(output), errorHandler, state);

    return ::Krys::Text::DecodeResult(
      ::Krys::Text::detail::SpanReconstruct<TInput>(std::move(result.Input)),
      ::Krys::Text::detail::SpanReconstructMutable<TOutput>(std::move(result.Output)), result.State);
  }

  /// @brief Converts code units of the given `input` view into code points in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] encoding The encoding for decoding the `input`'s code units into `output` code points.
  /// @param[in] output An output view to write code points to.
  /// @param[in] errorHandler The handler to use for decoding errors.
  /// @result A StatelessDecodeResult object.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler>
  constexpr auto DecodeInto(TInput &&input, TEncoding &&encoding, TOutput &&output,
                            TErrorHandler &&errorHandler)
  {
    auto state = ::Krys::Text::CreateDecodeState(encoding);
    auto statefulResult = ::Krys::Text::DecodeInto(
      std::forward<TInput>(input), std::forward<TEncoding>(encoding), std::forward<TOutput>(output),
      std::forward<TErrorHandler>(errorHandler), state);

    return ::Krys::Text::detail_result::SliceToStatelessDecode(std::move(statefulResult));
  }

  /// @brief Converts code units of the given `input` view into code points in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] encoding The encoding for decoding the `input`'s code units into `output` code points.
  /// @param[in] output An output view to write code points to.
  /// @result A StatelessDecodeResult object.
  template <typename TInput, typename TEncoding, typename TOutput>
  constexpr auto DecodeInto(TInput &&input, TEncoding &&encoding, TOutput &&output)
  {
    ::Krys::Text::Handlers::DefaultHandler handler {};
    return ::Krys::Text::DecodeInto(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                    std::forward<TOutput>(output), handler);
  }

  /// @brief Converts code units of the given `input` view into code points in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] output An output view to write code points to.
  /// @result A StatelessDecodeResult object.
  /// @remarks Creates a default encoding using the underlying value type of `input`.
  template <typename TInput, typename TOutput>
  constexpr auto DecodeInto(TInput &&input, TOutput &&output)
  {
    using TCodeUnit = ::Krys::Ranges::range_value_type_t<TInput>;
    using TEncoding =
      conditional_t<std::is_constant_evaluated(), default_consteval_code_unit_encoding_t<TCodeUnit>,
                    default_code_unit_encoding_t<TCodeUnit>>;

    TEncoding encoding {};
    return ::Krys::Text::DecodeInto(std::forward<TInput>(input), encoding, std::forward<TOutput>(output));
  }

  /// @brief Converts code units from `input` to code points in the specified `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] encoding The encoding for decoding the `input`'s code units into `output` code points.
  /// @param[in] errorHandler The handler to use for decoding errors.
  /// @param[in,out] state A reference to the associated state for the `encoding`'s decode step.
  /// @result A DecodeResult that contains references to `state` and an output of type `TOutputContainer`.
  /// @remarks This function detects creates a container of type `TOutputContainer` and uses a typical @c
  /// std::back_inserter or `std::push_back_inserter` to fill in elements as it is written to. The result is
  /// then returned, with the `.Output` value put into the container.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding, typename TErrorHandler,
            typename TState>
  constexpr auto DecodeTo(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler, TState &state)
  {
    using TCodePoint = code_point_t<TEncoding>;

    constexpr bool IsVoidContainer = Void<remove_cvref_t<TOutputContainer>>;
    if constexpr ((IsCharTraitable<TCodePoint> || IsUnicodeCodePoint<TCodePoint>) && IsVoidContainer)
    {
      using TOutput = std::basic_string<TCodePoint>;
      return ::Krys::Text::detail_decode::DecodeDispatch<false, false, TOutput>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
    else
    {
      using TOutput = conditional_t<!IsVoidContainer, TOutputContainer, std::vector<TCodePoint>>;
      return ::Krys::Text::detail_decode::DecodeDispatch<false, false, TOutput>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
  }

  /// @brief Converts code units from `input` to code points in the specified `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] encoding The encoding for decoding the `input`'s code units into `output` code points.
  /// @param[in] errorHandler The handler to use for decoding errors.
  /// @result A StatelessDecodeResult object whose output is of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding, typename TErrorHandler>
  constexpr auto DecodeTo(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler)
  {
    using TCodePoint = code_point_t<TEncoding>;
    auto state = ::Krys::Text::CreateDecodeState(encoding);

    constexpr bool IsVoidContainer = Void<remove_cvref_t<TOutputContainer>>;
    if constexpr ((IsCharTraitable<TCodePoint> || IsUnicodeCodePoint<TCodePoint>) && IsVoidContainer)
    {
      // prevent instantiation errors with basic_string by boxing it inside of an "if constexpr"
      using TOutput = std::basic_string<TCodePoint>;
      return ::Krys::Text::detail_decode::DecodeDispatch<false, true, TOutput>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
    else
    {
      using TOutput = conditional_t<!IsVoidContainer, TOutputContainer, std::vector<TCodePoint>>;
      return ::Krys::Text::detail_decode::DecodeDispatch<false, true, TOutput>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
  }

  /// @brief Converts code units from `input` to code points in the specified `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] encoding The encoding for decoding the `input`'s code units into `output` code points.
  /// @result A StatelessDecodeResult object whose output is of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding>
  constexpr auto DecodeTo(TInput &&input, TEncoding &&encoding)
  {
    ::Krys::Text::Handlers::DefaultHandler handler {};
    return ::Krys::Text::DecodeTo<TOutputContainer>(std::forward<TInput>(input),
                                                    std::forward<TEncoding>(encoding), handler);
  }

  /// @brief Converts code units from `input` to code points in the specified `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @result A StatelessDecodeResult object whose output is of type `TOutputContainer`.
  /// @remarks Creates a default encoding using the underlying value type of `input`.
  template <typename TOutputContainer = void, typename TInput>
  constexpr auto DecodeTo(TInput &&input)
  {
    using TCodeUnit = ::Krys::Ranges::range_value_type_t<TInput>;
    using TEncoding =
      conditional_t<std::is_constant_evaluated(), default_consteval_code_unit_encoding_t<TCodeUnit>,
                    default_code_unit_encoding_t<TCodeUnit>>;

    TEncoding encoding {};
    return ::Krys::Text::DecodeTo<TOutputContainer>(std::forward<TInput>(input), encoding);
  }

  /// @brief Converts code units from `input` to code points in the specified `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] encoding The encoding for decoding the `input`'s code units into `output` code points.
  /// @param[in] errorHandler The handler to use for decoding errors.
  /// @param[in,out] state A reference to the associated state for the `encoding`'s decode step.
  /// @result An object of type `TOutputContainer`.
  /// @remarks This function detects creates a container of type `TOutputContainer` and uses a typical @c
  /// std::back_inserter or `std::push_back_inserter` to fill in elements as it is written to.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding, typename TErrorHandler,
            typename TState>
  constexpr auto Decode(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler, TState &state)
  {
    using TCodePoint = code_point_t<TEncoding>;
    constexpr bool IsVoidContainer = Void<remove_cvref_t<TOutputContainer>>;

    if constexpr ((IsCharTraitable<TCodePoint> || IsUnicodeCodePoint<TCodePoint>) && IsVoidContainer)
    {
      using TOutput = std::basic_string<TCodePoint>;
      return ::Krys::Text::detail_decode::DecodeDispatch<true, false, TOutput>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
    else
    {
      using TOutput = conditional_t<!IsVoidContainer, TOutputContainer, std::vector<TCodePoint>>;
      return ::Krys::Text::detail_decode::DecodeDispatch<true, false, TOutput>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
  }

  /// @brief Converts code units from `input` to code points in the specified `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] encoding The encoding for decoding the `input`'s code units into `output` code points.
  /// @param[in] errorHandler The handler to use for decoding errors.
  /// @result An object of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding, typename TErrorHandler>
  constexpr auto Decode(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler)
  {
    using TCodePoint = code_point_t<TEncoding>;
    auto state = ::Krys::Text::CreateDecodeState(encoding);

    constexpr bool IsVoidContainer = Void<remove_cvref_t<TOutputContainer>>;
    if constexpr ((IsCharTraitable<TCodePoint> || IsUnicodeCodePoint<TCodePoint>) && IsVoidContainer)
    {
      using TOutput = std::basic_string<TCodePoint>;
      return ::Krys::Text::detail_decode::DecodeDispatch<true, true, TOutput>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
    else
    {
      using TOutput = conditional_t<!IsVoidContainer, TOutputContainer, std::vector<TCodePoint>>;
      return ::Krys::Text::detail_decode::DecodeDispatch<true, true, TOutput>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
  }

  /// @brief Converts code units from `input` to code points in the specified `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] encoding The encoding for decoding the `input`'s code units into `output` code points.
  /// @result An object of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding>
  constexpr auto Decode(TInput &&input, TEncoding &&encoding)
  {
    ::Krys::Text::Handlers::DefaultHandler handler {};
    return ::Krys::Text::Decode<TOutputContainer>(std::forward<TInput>(input),
                                                  std::forward<TEncoding>(encoding), handler);
  }

  /// @brief Converts code units from `input` to code points in the specified `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @result An object of type `TOutputContainer`.
  /// @remarks Creates a default encoding using the underlying value type of `input`.
  template <typename TOutputContainer = void, typename TInput>
  constexpr auto Decode(TInput &&input)
  {
    using TCodeUnit = ::Krys::Ranges::range_value_type_t<TInput>;
    using TEncoding =
      conditional_t<std::is_constant_evaluated(), default_consteval_code_unit_encoding_t<TCodeUnit>,
                    default_code_unit_encoding_t<TCodeUnit>>;

    TEncoding encoding {};
    return ::Krys::Text::Decode<TOutputContainer>(std::forward<TInput>(input), encoding);
  }
}
