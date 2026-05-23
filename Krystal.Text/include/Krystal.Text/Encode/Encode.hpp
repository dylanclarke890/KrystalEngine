#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Utils/Tag.hpp"
#include "Krystal.Text/_detail/IsLossless.hpp"
#include "Krystal.Text/_detail/SpanReconstruct.hpp"
#include "Krystal.Text/_detail/UpdateInput.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/Concepts.hpp"
#include "Krystal.Text/Encode/EncodeResult.hpp"
#include "Krystal.Text/Encodings/DefaultEncoding.hpp"
#include "Krystal.Text/Handlers/DefaultHandler.hpp"
#include "Krystal.Text/Handlers/ProgressHandler.hpp"
#include "Krystal.Text/IsUnicodeCodePoint.hpp"
#include "Krystal.Text/MaxUnits.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/TypeTraits.hpp"
#include <string>
#include <vector>

/// These functions convert from a view of input code points into a view of output code units using either the
/// inferred or specified encodings. If no error handler is provided, DefaultHandler is used by default. If no
/// state is provided for the encoding, it will created automatically as needed.
namespace Krys::Text
{
  /// @brief Converts code points from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code points from.
  /// @param[in] encoding The encoding for encoding the `input`'s code points into `output` code units.
  /// @param[in] output An output view to write code units to.
  /// @param[in] errorHandler The handler to use for encoding errors.
  /// @param[in,out] state A reference to the associated state for the `encoding`'s encode step.
  /// @result A EncodeResult object that contains references to `state`.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler, typename TState>
  constexpr auto BasicEncodeIntoRaw(TInput &&input, TEncoding &&encoding, TOutput &&output,
                                    TErrorHandler &&errorHandler, TState &state)
  {
    static_assert(::Krys::Text::EncodeLosslessOrDeliberate<TEncoding, TErrorHandler>,
                  KRYS_TEXT_LOSSY_ENCODE_MESSAGE);

    auto firstResult =
      encoding.EncodeOne(std::forward<TInput>(input), std::forward<TOutput>(output), errorHandler, state);

    using TWorkingInput = decltype(firstResult.Input);
    using TWorkingOutput = decltype(firstResult.Output);
    using TResult = ::Krys::Text::EncodeResult<TWorkingInput, TWorkingOutput, TState>;

    if (firstResult.ErrorCode != EncodingError::OK)
    {
      return TResult(std::move(firstResult.Input), std::move(firstResult.Output), state,
                     firstResult.ErrorCode, firstResult.ErrorCount);
    }

    if (std::ranges::empty(firstResult.Input))
    {
      if (::Krys::Text::IsStateComplete(encoding, state))
      {
        return TResult(std::move(firstResult.Input), std::move(firstResult.Output), state, EncodingError::OK,
                       firstResult.ErrorCount);
      }
    }

    TWorkingInput workingInput = std::move(firstResult.Input);
    TWorkingOutput workingOutput = std::move(firstResult.Output);

    std::size_t errorCount = firstResult.ErrorCount;
    for (;;)
    {
      auto result =
        encoding.EncodeOne(std::move(workingInput), std::move(workingOutput), errorHandler, state);
      workingInput = std::move(result.Input);
      workingOutput = std::move(result.Output);
      errorCount += result.ErrorCount;

      if (result.ErrorCode != EncodingError::OK)
      {
        return TResult(std::move(workingInput), std::move(workingOutput), state, result.ErrorCode,
                       errorCount);
      }

      if (std::ranges::empty(workingInput))
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

  /// @brief Converts code points from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code points from.
  /// @param[in] encoding The encoding for encoding the `input`'s code points into `output` code units.
  /// @param[in] output An output view to write code units to.
  /// @param[in] errorHandler The handler to use for encoding errors.
  /// @param[in,out] state A reference to the associated state for the `encoding`'s encode step.
  /// @result A EncodeResult object that contains references to `state`.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler, typename TState>
  constexpr auto EncodeIntoRaw(TInput &&input, TEncoding &&encoding, TOutput &&output,
                               TErrorHandler &&errorHandler, TState &state)
  {
    if constexpr (::Krys::Text::HasEncodeText<TInput, TEncoding, TOutput, TErrorHandler, TState>)
    {
      return EncodeText(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::forward<TInput>(input),
                        std::forward<TEncoding>(encoding), std::forward<TOutput>(output),
                        std::forward<TErrorHandler>(errorHandler), state);
    }
    else
    {
      return ::Krys::Text::BasicEncodeIntoRaw(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                              std::forward<TOutput>(output),
                                              std::forward<TErrorHandler>(errorHandler), state);
    }
  }

  /// @brief Converts code points from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code points from.
  /// @param[in] encoding The encoding for encoding the `input`'s code points into `output` code units.
  /// @param[in] output An output view to write code units to.
  /// @param[in] errorHandler The handler to use for encoding errors.
  /// @result A StatelessEncodeResult object that contains references to `state`.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler>
  constexpr auto EncodeIntoRaw(TInput &&input, TEncoding &&encoding, TOutput &&output,
                               TErrorHandler &&errorHandler)
  {
    auto state = ::Krys::Text::CreateEncodeState(encoding);
    auto statefulResult = ::Krys::Text::EncodeIntoRaw(
      std::forward<TInput>(input), std::forward<TEncoding>(encoding), std::forward<TOutput>(output),
      std::forward<TErrorHandler>(errorHandler), state);

    return ::Krys::Text::Impl::SliceToStatelessEncode(std::move(statefulResult));
  }

  /// @brief Converts code points from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code points from.
  /// @param[in] encoding The encoding for encoding the `input`'s code points into `output` code units.
  /// @param[in] output An output view to write code units to.
  /// @result A StatelessEncodeResult object that contains references to `state`.
  template <typename TInput, typename TEncoding, typename TOutput>
  constexpr auto EncodeIntoRaw(TInput &&input, TEncoding &&encoding, TOutput &&output)
  {
    ::Krys::Text::Handlers::DefaultHandler handler {};
    return ::Krys::Text::EncodeIntoRaw(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                       std::forward<TOutput>(output), handler);
  }

  /// @brief Converts code points from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code points from.
  /// @param[in] output An output view to write code units to.
  /// @result A StatelessEncodeResult object that contains references to `state`.
  /// @remarks Creates a default encoding using the underlying value type of `input`.
  template <typename TInput, typename TOutput>
  constexpr auto EncodeIntoRaw(TInput &&input, TOutput &&output)
  {
    using TCodePoint = std::ranges::range_value_t<TInput>;
    using TEncoding =
      conditional_t<std::is_constant_evaluated(), default_consteval_code_point_encoding_t<TCodePoint>,
                    default_code_point_encoding_t<TCodePoint>>;

    TEncoding encoding {};
    return ::Krys::Text::EncodeIntoRaw(std::forward<TInput>(input), encoding, std::forward<TOutput>(output));
  }
}

namespace Krys::Text::detail_encode
{
  template <typename TInput, typename TEncoding, typename TOutputContainer, typename TErrorHandler,
            typename TState>
  constexpr auto IntermediateEncodeToStorage(TInput &&input, TEncoding &&encoding, TOutputContainer &output,
                                             TErrorHandler &&errorHandler, TState &state)
  {
    static_assert(::Krys::Text::EncodeLosslessOrDeliberate<TEncoding, TErrorHandler>,
                  KRYS_TEXT_LOSSY_ENCODE_MESSAGE);

    // Write into temporary, then serialize one-by-one/bulk to output.
    constexpr std::size_t MaxBufferSize = ::Krys::Text::MaxEncodeCodeUnits<TEncoding>;
    using TCodeUnit = code_unit_t<TEncoding>;

    using TIntermediateInput = ::Krys::Text::detail::span_reconstruct_t<TInput, TInput>;
    using TResult = decltype(::Krys::Text::EncodeIntoRaw(
      std::declval<TIntermediateInput>(), encoding, std::declval<Span<TCodeUnit>>(), errorHandler, state));
    using TWorkingInput = remove_cvref_t<decltype(std::declval<TResult>().Input)>;

    // We MUST use a temporary error handler
    ::Krys::Text::Handlers::ProgressHandler<IsIgnorableErrorHandler<TErrorHandler>, TEncoding> handler {};

    TIntermediateInput intermediateInput =
      ::Krys::Text::detail::SpanReconstruct<TInput>(std::forward<TInput>(input));
    TWorkingInput workingInput(std::move(intermediateInput));

    TCodeUnit translationBuffer[MaxBufferSize] {};
    Span<TCodeUnit> initialOutput(translationBuffer, MaxBufferSize);

    for (;;)
    {
      // Ignore "out of output" errors and do our best to recover properly along the way...
      auto result =
        ::Krys::Text::EncodeIntoRaw(std::move(workingInput), encoding, initialOutput, handler, state);
      ::Krys::Ranges::ContainerInsertBulk(output, Span(initialOutput.data(), result.Output.data()));

      if (result.ErrorCode == EncodingError::InsufficientOutputSpace)
      {
        if (handler.CodeUnitsProgressSize() != 0)
        {
          // add any leftover partially-unwritten characters to our output
          ::Krys::Ranges::ContainerInsertBulk(output, handler.ConstCodeUnitsProgress());
          // it's okay, just loop around, we've got S P A C E for more
          workingInput = ::Krys::Text::detail::UpdateInput<TWorkingInput>(std::move(result.Input));
        }
        else if (handler.CodePointsProgressSize() != 0)
        {
          if constexpr (std::ranges::bidirectional_range<TWorkingInput>)
          {
            // we can try to rewind our current input by the amount that was not successfully
            // read. This will allow us to try again, when the buffer
            // has more space in it, and should not result in the same error,
            // unless it was legitimiately an EncodingError::InvalidSequence.
            workingInput = ::Krys::Text::detail::UpdateInput<TWorkingInput>(
              ::Krys::Ranges::reconstruct(std::in_place_type<TWorkingInput>,
                                          std::ranges::prev(std::ranges::begin(std::move(result.Input)),
                                                            handler.CodePointsProgressSize()),
                                          std::ranges::end(result.Input)));
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
          workingInput = ::Krys::Text::detail::UpdateInput<TWorkingInput>(std::move(result.Input));
        }
        handler.Clear();
        continue;
      }
      if (result.ErrorCode != EncodingError::OK)
      {
        // mill result through actual error handler!
        auto errorResult = std::forward<TErrorHandler>(errorHandler)(
          std::forward<TEncoding>(encoding), std::move(result), handler.CodePointsProgress(),
          handler.CodeUnitsProgress());
        return TResult(errorResult);
      }
      if (std::ranges::empty(result.Input))
      {
        if (!::Krys::Text::IsStateComplete(encoding, state))
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
    constexpr std::size_t maxUnits = ::Krys::Text::MaxEncodeCodeUnits<TEncoding>;

    TOutputContainer output {};
    if constexpr (std::ranges::sized_range<TInput>)
    {
      using TSize = decltype(std::ranges::size(input));
      if constexpr (HasReserve<TOutputContainer, TSize>)
      {
        TSize outputSizeHint = static_cast<TSize>(std::ranges::size(input));
        outputSizeHint *= (maxUnits > 3) ? (maxUnits / 4) : maxUnits;
        output.reserve(outputSizeHint);
      }
    }
    auto statefulResult = ::Krys::Text::detail_encode::IntermediateEncodeToStorage(
      std::forward<TInput>(input), std::forward<TEncoding>(encoding), output,
      std::forward<TErrorHandler>(errorHandler), state);
    if constexpr (OutputOnly)
    {
      (void)statefulResult;
      return output;
    }
    else if constexpr (NoState)
    {
      return ::Krys::Text::Impl::ReplaceEncodeResultOutputNoState(std::move(statefulResult),
                                                                  std::move(output));
    }
    else
    {
      return ::Krys::Text::Impl::ReplaceEncodeResultOutput(std::move(statefulResult), std::move(output));
    }
  }
}

namespace Krys::Text
{
  /// @brief Converts code points from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code points from.
  /// @param[in] encoding The encoding for encoding the `input`'s code points into `output` code units.
  /// @param[in] output An output view to write code units to.
  /// @param[in] errorHandler The handler to use for encoding errors.
  /// @param[in,out] state A reference to the associated state for the `encoding`'s encode step.
  /// @result A EncodeResult object that contains references to `state`.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler, typename TState>
  constexpr auto BasicEncodeInto(TInput &&input, TEncoding &&encoding, TOutput &&output,
                                 TErrorHandler &&errorHandler, TState &state)
  {
    auto reconstructedInput = ::Krys::Text::detail::SpanReconstruct<TInput>(std::forward<TInput>(input));
    auto result = ::Krys::Text::BasicEncodeIntoRaw(
      std::move(reconstructedInput), std::forward<TEncoding>(encoding), std::forward<TOutput>(output),
      std::forward<TErrorHandler>(errorHandler), state);

    return ::Krys::Text::EncodeResult(
      ::Krys::Text::detail::SpanReconstruct<TInput>(std::move(result.Input)),
      ::Krys::Text::detail::SpanReconstructMutable<TOutput>(std::move(result.Output)), result.State);
  }

  /// @brief Converts code points from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code points from.
  /// @param[in] encoding The encoding for encoding the `input`'s code points into `output` code units.
  /// @param[in] output An output view to write code units to.
  /// @param[in] errorHandler The handler to use for encoding errors.
  /// @param[in,out] state A reference to the associated state for the `encoding`'s encode step.
  /// @result A EncodeResult object that contains references to `state`.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler, typename TState>
  constexpr auto EncodeInto(TInput &&input, TEncoding &&encoding, TOutput &&output,
                            TErrorHandler &&errorHandler, TState &state)
  {
    auto reconstructedInput = ::Krys::Text::detail::SpanReconstruct<TInput>(std::forward<TInput>(input));
    auto result = ::Krys::Text::EncodeIntoRaw(
      std::move(reconstructedInput), std::forward<TEncoding>(encoding), std::forward<TOutput>(output),
      std::forward<TErrorHandler>(errorHandler), state);

    return ::Krys::Text::EncodeResult(
      ::Krys::Text::detail::SpanReconstruct<TInput>(std::move(result.Input)),
      ::Krys::Text::detail::SpanReconstructMutable<TOutput>(std::move(result.Output)), result.State);
  }

  /// @brief Converts code points from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code points from.
  /// @param[in] encoding The encoding for encoding the `input`'s code points into `output` code units.
  /// @param[in] output An output view to write code units to.
  /// @param[in] errorHandler The handler to use for encoding errors.
  /// @result A StatelessEncodeResult object.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler>
  constexpr auto EncodeInto(TInput &&input, TEncoding &&encoding, TOutput &&output,
                            TErrorHandler &&errorHandler)
  {
    auto state = ::Krys::Text::CreateEncodeState(encoding);
    auto statefulResult = ::Krys::Text::EncodeInto(
      std::forward<TInput>(input), std::forward<TEncoding>(encoding), std::forward<TOutput>(output),
      std::forward<TErrorHandler>(errorHandler), state);

    return ::Krys::Text::Impl::SliceToStatelessEncode(std::move(statefulResult));
  }

  /// @brief Converts code points from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code points from.
  /// @param[in] encoding The encoding for encoding the `input`'s code points into `output` code units.
  /// @param[in] output An output view to write code units to.
  /// @result A StatelessEncodeResult object.
  template <typename TInput, typename TEncoding, typename TOutput>
  constexpr auto EncodeInto(TInput &&input, TEncoding &&encoding, TOutput &&output)
  {
    ::Krys::Text::Handlers::DefaultHandler handler {};
    return ::Krys::Text::EncodeInto(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                    std::forward<TOutput>(output), handler);
  }

  /// @brief Converts code points from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code points from.
  /// @param[in] output An output view to write code units to.
  /// @result A StatelessEncodeResult object.
  /// @remarks Creates a default encoding using the underlying value type of `input`.
  template <typename TInput, typename TOutput>
  constexpr auto EncodeInto(TInput &&input, TOutput &&output)
  {
    using TCodePoint = std::ranges::range_value_t<TInput>;
    using TEncoding =
      conditional_t<std::is_constant_evaluated(), default_consteval_code_point_encoding_t<TCodePoint>,
                    default_code_point_encoding_t<TCodePoint>>;

    TEncoding encoding {};
    return ::Krys::Text::EncodeInto(std::forward<TInput>(input), encoding, std::forward<TOutput>(output));
  }

  /// @brief Converts code points from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code points from.
  /// @param[in] encoding The encoding for encoding the `input`'s code points into `output` code units.
  /// @param[in] output An output view to write code units to.
  /// @param[in] errorHandler The handler to use for encoding errors.
  /// @param[in,out] state A reference to the associated state for the `encoding`'s encode step.
  /// @result A EncodeResult object with an output of type `TOutputContainer`.
  /// @remarks This function detects creates a container of type `TOutputContainer` and uses a typical @c
  /// std::back_inserter or `std::push_back_inserter` to fill in elements as it is written to. The result is
  /// then returned, with the `.Output` value put into the container.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding, typename TErrorHandler,
            typename TState>
  constexpr auto EncodeTo(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler, TState &state)
  {
    using TCodeUnit = code_unit_t<TEncoding>;

    constexpr bool IsVoidContainer = Void<remove_cvref_t<TOutputContainer>>;
    if constexpr ((IsCharTraitable<TCodeUnit> || IsUnicodeCodePoint<TCodeUnit>) && IsVoidContainer)
    {
      using TOutput = std::basic_string<TCodeUnit>;
      return ::Krys::Text::detail_encode::EncodeDispatch<false, true, TOutput>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
    else
    {
      using TOutput = conditional_t<IsVoidContainer, std::vector<TCodeUnit>, TOutputContainer>;
      return ::Krys::Text::detail_encode::EncodeDispatch<false, true, TOutput>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
  }

  /// @brief Converts code points from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code points from.
  /// @param[in] encoding The encoding for encoding the `input`'s code points into `output` code units.
  /// @param[in] output An output view to write code units to.
  /// @param[in] errorHandler The handler to use for encoding errors.
  /// @result A StatelessEncodeResult object with an output of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding, typename TErrorHandler>
  constexpr auto EncodeTo(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler)
  {
    using TCodeUnit = code_unit_t<TEncoding>;
    auto state = ::Krys::Text::CreateEncodeState(encoding);

    constexpr bool IsVoidContainer = Void<remove_cvref_t<TOutputContainer>>;
    if constexpr ((IsCharTraitable<TCodeUnit> || IsUnicodeCodePoint<TCodeUnit>) && IsVoidContainer)
    {
      using TOutput = std::basic_string<TCodeUnit>;
      return ::Krys::Text::detail_encode::EncodeDispatch<false, false, TOutput>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
    else
    {
      using TOutput = conditional_t<IsVoidContainer, std::vector<TCodeUnit>, TOutputContainer>;
      return ::Krys::Text::detail_encode::EncodeDispatch<false, false, TOutput>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
  }

  /// @brief Converts code points from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code points from.
  /// @param[in] encoding The encoding for encoding the `input`'s code points into `output` code units.
  /// @param[in] output An output view to write code units to.
  /// @result A StatelessEncodeResult object with an output of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding>
  constexpr auto EncodeTo(TInput &&input, TEncoding &&encoding)
  {
    ::Krys::Text::Handlers::DefaultHandler handler {};
    return ::Krys::Text::EncodeTo<TOutputContainer>(std::forward<TInput>(input),
                                                    std::forward<TEncoding>(encoding), handler);
  }

  /// @brief Converts code points from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code points from.
  /// @result A StatelessEncodeResult object with an output of type `TOutputContainer`.
  /// @remarks Creates a default encoding using the underlying value type of `input`.
  template <typename TOutputContainer = void, typename TInput>
  constexpr auto EncodeTo(TInput &&input)
  {
    using TCodePoint = std::ranges::range_value_t<TInput>;
    using TEncoding =
      conditional_t<std::is_constant_evaluated(), default_consteval_code_point_encoding_t<TCodePoint>,
                    default_code_point_encoding_t<TCodePoint>>;

    TEncoding encoding {};
    return ::Krys::Text::EncodeTo<TOutputContainer>(std::forward<TInput>(input), encoding);
  }

  /// @brief Converts code points from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code points from.
  /// @param[in] encoding The encoding for encoding the `input`'s code points into `output` code units.
  /// @param[in] errorHandler The handler to use for encoding errors.
  /// @param[in,out] state A reference to the associated state for the `encoding`'s encode step.
  /// @result An object of type `TOutputContainer`.
  /// @remarks This function detects creates a container of type `TOutputContainer` and uses a typical @c
  /// std::back_inserter or `std::push_back_inserter` to fill in elements as it is written to.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding, typename TErrorHandler,
            typename TState>
  constexpr auto Encode(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler, TState &state)
  {
    using TCodeUnit = code_unit_t<TEncoding>;

    constexpr bool IsVoidContainer = Void<remove_cvref_t<TOutputContainer>>;
    if constexpr ((IsCharTraitable<TCodeUnit> || IsUnicodeCodePoint<TCodeUnit>) && IsVoidContainer)
    {
      using TOutput = std::basic_string<TCodeUnit>;
      return ::Krys::Text::detail_encode::EncodeDispatch<true, false, TOutput>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
    else
    {
      using TOutput = conditional_t<IsVoidContainer, std::vector<TCodeUnit>, TOutputContainer>;
      return ::Krys::Text::detail_encode::EncodeDispatch<true, false, TOutput>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
  }

  /// @brief Converts code points from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code points from.
  /// @param[in] encoding The encoding for encoding the `input`'s code points into `output` code units.
  /// @param[in] errorHandler The handler to use for encoding errors.
  /// @result An object of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding, typename TErrorHandler>
  constexpr auto Encode(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler)
  {
    using TCodeUnit = code_unit_t<TEncoding>;
    auto state = ::Krys::Text::CreateEncodeState(encoding);

    constexpr bool IsVoidContainer = Void<remove_cvref_t<TOutputContainer>>;
    if constexpr ((IsCharTraitable<TCodeUnit> || IsUnicodeCodePoint<TCodeUnit>) && IsVoidContainer)
    {
      using TOutput = std::basic_string<TCodeUnit>;
      return ::Krys::Text::detail_encode::EncodeDispatch<true, true, TOutput>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
    else
    {
      using TOutput = conditional_t<IsVoidContainer, std::vector<TCodeUnit>, TOutputContainer>;
      return ::Krys::Text::detail_encode::EncodeDispatch<true, true, TOutput>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
  }

  /// @brief Converts code points from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code points from.
  /// @param[in] encoding The encoding for encoding the `input`'s code points into `output` code units.
  /// @result An object of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding>
  constexpr auto Encode(TInput &&input, TEncoding &&encoding)
  {
    ::Krys::Text::Handlers::DefaultHandler handler {};
    return ::Krys::Text::Encode<TOutputContainer>(std::forward<TInput>(input),
                                                  std::forward<TEncoding>(encoding), handler);
  }

  /// @brief Converts code points from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code points from.
  /// @result An object of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput>
  constexpr auto Encode(TInput &&input)
  {
    using TCodePoint = std::ranges::range_value_t<TInput>;
    using TEncoding =
      conditional_t<std::is_constant_evaluated(), default_consteval_code_point_encoding_t<TCodePoint>,
                    default_code_point_encoding_t<TCodePoint>>;

    TEncoding encoding {};
    return ::Krys::Text::Encode<TOutputContainer>(std::forward<TInput>(input), encoding);
  }
}
