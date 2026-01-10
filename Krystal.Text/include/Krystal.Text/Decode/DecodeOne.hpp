#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/Impl/InsertBulk.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/Concepts.hpp"
#include "Krystal.Text/Decode/DecodeResult.hpp"
#include "Krystal.Text/detail/IsLossless.hpp"
#include "Krystal.Text/Encodings/DefaultEncoding.hpp"
#include "Krystal.Text/Handlers/DefaultHandler.hpp"
#include "Krystal.Text/detail/SpanReconstruct.hpp"
#include "Krystal.Text/InlineContainers/InlineBasicString.hpp"
#include "Krystal.Text/IsUnicodeCodePoint.hpp"
#include "Krystal.Text/MaxUnits.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/TypeTraits.hpp"

/// These functions convert from a view of input code units into a view of output code points using either the
/// inferred or specified encodings. If no error handler is provided, DefaultHandler is used by default. If no
/// state is provided for the encoding, it will created automatically as needed.
namespace Krys::Text
{
  /// @brief Converts one code unit from the given `input` view into code points in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] encoding The encoding for decoding the `input`'s code units into `output` code points.
  /// @param[in] output An output view to write code points to.
  /// @param[in] errorHandler The handler to use for decoding errors.
  /// @param[in,out] state A reference to the associated state for the `encoding`'s decode step.
  /// @result A DecodeResult object that contains references to `state`.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler, typename TState>
  constexpr auto DecodeOneIntoRaw(TInput &&input, TEncoding &&encoding, TOutput &&output,
                                  TErrorHandler &&errorHandler, TState &state)
  {
    static_assert(::Krys::Text::DecodeLosslessOrDeliberate<TEncoding, TErrorHandler>,
                  KRYS_TEXT_LOSSY_DECODE_MESSAGE);

    return encoding.DecodeOne(std::forward<TInput>(input), std::forward<TOutput>(output), errorHandler,
                              state);
  }

  /// @brief Converts one code unit from the given `input` view into code points in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] encoding The encoding for decoding the `input`'s code units into `output` code points.
  /// @param[in] output An output view to write code points to.
  /// @param[in] errorHandler The handler to use for decoding errors.
  /// @result A StatelessDecodeResult object.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler>
  constexpr auto DecodeOneIntoRaw(TInput &&input, TEncoding &&encoding, TOutput &&output,
                                  TErrorHandler &&errorHandler)
  {
    auto state = ::Krys::Text::CreateDecodeState(encoding);
    auto statefulResult = ::Krys::Text::DecodeOneIntoRaw(
      std::forward<TInput>(input), std::forward<TEncoding>(encoding), std::forward<TOutput>(output),
      std::forward<TErrorHandler>(errorHandler), state);

    return ::Krys::Text::detail_result::SliceToStatelessDecode(std::move(statefulResult));
  }

  /// @brief Converts one code unit from the given `input` view into code points in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] encoding The encoding for decoding the `input`'s code units into `output` code points.
  /// @param[in] output An output view to write code points to.
  /// @result A StatelessDecodeResult object.
  /// @remarks Creates a default handler that is similar to Handlers::DefaultHandler, but marked as careless.
  template <typename TInput, typename TEncoding, typename TOutput>
  constexpr auto DecodeOneIntoRaw(TInput &&input, TEncoding &&encoding, TOutput &&output)
  {
    ::Krys::Text::Handlers::DefaultHandler handler {};
    return ::Krys::Text::DecodeOneIntoRaw(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                          std::forward<TOutput>(output), handler);
  }

  /// @brief Converts one code unit from the given `input` view into code points in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] output An output view to write code points to.
  /// @result A StatelessDecodeResult object.
  /// @remarks Creates a default encoding using the underlying value type of `input`.
  template <typename TInput, typename TOutput>
  constexpr auto DecodeOneIntoRaw(TInput &&input, TOutput &&output)
  {
    using TCodeUnit = ::Krys::Ranges::range_value_type_t<TInput>;
    using TEncoding =
      conditional_t<std::is_constant_evaluated(), default_consteval_code_unit_encoding_t<TCodeUnit>,
                    default_code_unit_encoding_t<TCodeUnit>>;
    TEncoding encoding {};
    return ::Krys::Text::DecodeOneIntoRaw(std::forward<TInput>(input), encoding,
                                          std::forward<TOutput>(output));
  }
}

namespace Krys::Text::detail_decode
{
  template <typename TInput, typename TEncoding, typename TOutputContainer, typename TErrorHandler,
            typename TState>
  constexpr auto IntermediateDecodeOneToStorage(TInput &&input, TEncoding &&encoding,
                                                TOutputContainer &output, TErrorHandler &&errorHandler,
                                                TState &state)
  {
    static_assert(::Krys::Text::DecodeLosslessOrDeliberate<TEncoding, TErrorHandler>,
                  KRYS_TEXT_LOSSY_DECODE_MESSAGE);

    using TCodePoint = code_point_t<TEncoding>;
    constexpr std::size_t MaxBufferSize = ::Krys::Text::MaxDecodeCodePoints<TEncoding>;
    TCodePoint translationBuffer[MaxBufferSize] {};
    Span<TCodePoint, MaxBufferSize> initialOutput(translationBuffer);

    auto result = ::Krys::Text::DecodeOneIntoRaw(std::forward<TInput>(input), encoding, initialOutput,
                                                 errorHandler, state);
    ::Krys::Ranges::Impl::ContainerInsertBulk(output, Span {initialOutput.data(), result.Output.data()});
    return result;
  }

  template <bool OutputOnly, bool NoState, typename TOutputContainer, typename TInput, typename TEncoding,
            typename TErrorHandler, typename TState>
  constexpr auto DecodeOneDispatch(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler,
                                   TState &state)
  {
    TOutputContainer output {};
    if constexpr (::Krys::Ranges::HasSizeADL<TInput>)
    {
      using TSize = decltype(::Krys::Ranges::size(input));
      if constexpr (::Krys::Ranges::has_reserve_with_size<TOutputContainer, TSize>)
      {
        output.reserve(::Krys::Ranges::size(input) * (::Krys::Text::MaxDecodeCodePoints<TEncoding> / 2));
      }
    }

    auto statefulResult = ::Krys::Text::detail_decode::IntermediateDecodeOneToStorage(
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
  /// @brief Converts one code unit from the given `input` view into code points in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] encoding The encoding for decoding the `input`'s code units into `output` code points.
  /// @param[in] output An output view to write code points to.
  /// @param[in] errorHandler The handler to use for decoding errors.
  /// @param[in,out] state A reference to the associated state for the `encoding`'s decode step.
  /// @result A DecodeResult object that contains references to `state`.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler, typename TState>
  constexpr auto DecodeOneInto(TInput &&input, TEncoding &&encoding, TOutput &&output,
                               TErrorHandler &&errorHandler, TState &state)
  {
    auto reconstructedInput = ::Krys::Text::detail::SpanReconstruct<TInput>(std::forward<TInput>(input));
    auto result =
      ::Krys::Text::DecodeOneIntoRaw(std::move(reconstructedInput), std::forward<TEncoding>(encoding),
                                     std::forward<TOutput>(output), errorHandler, state);

    return ::Krys::Text::DecodeResult(
      ::Krys::Text::detail::SpanReconstruct<TInput>(std::move(result.Input)),
      ::Krys::Text::detail::SpanReconstructMutable<TOutput>(std::move(result.Output)), result.State);
  }

  /// @brief Converts one code unit from the given `input` view into code points in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] encoding The encoding for decoding the `input`'s code units into `output` code points.
  /// @param[in] output An output view to write code points to.
  /// @param[in] errorHandler The handler to use for decoding errors.
  /// @result A StatelessDecodeResult object.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler>
  constexpr auto DecodeOneInto(TInput &&input, TEncoding &&encoding, TOutput &&output,
                               TErrorHandler &&errorHandler)
  {
    auto state = ::Krys::Text::CreateDecodeState(encoding);
    auto statefulResult = ::Krys::Text::DecodeOneInto(
      std::forward<TInput>(input), std::forward<TEncoding>(encoding), std::forward<TOutput>(output),
      std::forward<TErrorHandler>(errorHandler), state);

    return ::Krys::Text::detail_result::SliceToStatelessDecode(std::move(statefulResult));
  }

  /// @brief Converts one code unit from the given `input` view into code points in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] encoding The encoding for decoding the `input`'s code units into `output` code points.
  /// @param[in] output An output view to write code points to.
  /// @result A StatelessDecodeResult object.
  template <typename TInput, typename TEncoding, typename TOutput>
  constexpr auto DecodeOneInto(TInput &&input, TEncoding &&encoding, TOutput &&output)
  {
    ::Krys::Text::Handlers::DefaultHandler handler {};
    return ::Krys::Text::DecodeOneInto(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                       std::forward<TOutput>(output), handler);
  }

  /// @brief Converts one code unit from the given `input` view into code points in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] output An output view to write code points to.
  /// @result A StatelessDecodeResult object.
  /// @remarks Creates a default encoding using the underlying value type of `input`.
  template <typename TInput, typename TOutput>
  constexpr auto DecodeOneInto(TInput &&input, TOutput &&output)
  {
    using TCodeUnit = ::Krys::Ranges::range_value_type_t<TInput>;
    using TEncoding =
      conditional_t<std::is_constant_evaluated(), default_consteval_code_unit_encoding_t<TCodeUnit>,
                    default_code_unit_encoding_t<TCodeUnit>>;

    TEncoding encoding {};
    return ::Krys::Text::DecodeOneInto(std::forward<TInput>(input), encoding, std::forward<TOutput>(output));
  }

  /// @brief Converts one code unit from the given `input` view into code points in `TOutputContainer`.
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
  constexpr auto DecodeOneTo(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler,
                             TState &state)
  {
    using TCodePoint = code_point_t<TEncoding>;
    constexpr std::size_t MaxCodePoints = ::Krys::Text::MaxDecodeCodePoints<TEncoding>;

    constexpr bool IsVoidContainer = Void<remove_cvref_t<TOutputContainer>>;
    if constexpr ((IsCharTraitable<TCodePoint> || IsUnicodeCodePoint<TCodePoint>) && IsVoidContainer)
    {
      // prevent instantiation errors with basic_string by boxing it inside of an "if constexpr"
      using TOutput = InlineBasicString<TCodePoint, MaxCodePoints>;
      return ::Krys::Text::detail_decode::DecodeOneDispatch<false, false, TOutput>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
    else
    {
      using TOutput =
        conditional_t<!IsVoidContainer, TOutputContainer, InlineVector<TCodePoint, MaxCodePoints>>;
      return ::Krys::Text::detail_decode::DecodeOneDispatch<false, false, TOutput>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
  }

  /// @brief Converts one code unit from the given `input` view into code points in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] encoding The encoding for decoding the `input`'s code units into `output` code points.
  /// @param[in] errorHandler The handler to use for decoding errors.
  /// @result A DecodeResult that contains references to `state` and an output of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding, typename TErrorHandler>
  constexpr auto DecodeOneTo(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler)
  {
    auto state = ::Krys::Text::CreateDecodeState(encoding);
    return ::Krys::Text::DecodeOneTo<TOutputContainer>(std::forward<TInput>(input),
                                                       std::forward<TEncoding>(encoding),
                                                       std::forward<TErrorHandler>(errorHandler), state);
  }

  /// @brief Converts one code unit from the given `input` view into code points in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] encoding The encoding for decoding the `input`'s code units into `output` code points.
  /// @param[in] errorHandler The handler to use for decoding errors.
  /// @result A DecodeResult that contains references to `state` and an output of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding>
  constexpr auto DecodeOneTo(TInput &&input, TEncoding &&encoding)
  {
    ::Krys::Text::Handlers::DefaultHandler handler {};
    return ::Krys::Text::DecodeOneTo<TOutputContainer>(std::forward<TInput>(input),
                                                       std::forward<TEncoding>(encoding), handler);
  }

  /// @brief Converts one code unit from the given `input` view into code points in the `output` view.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @result A StatelessDecodeResult with an output of type `TOutputContainer`.
  /// @remarks Creates a default encoding using the underlying value type of `input`.
  template <typename TOutputContainer = void, typename TInput>
  constexpr auto DecodeOneTo(TInput &&input)
  {
    using TCodeUnit = ::Krys::Ranges::range_value_type_t<TInput>;
    using TEncoding =
      conditional_t<std::is_constant_evaluated(), default_consteval_code_unit_encoding_t<TCodeUnit>,
                    default_code_unit_encoding_t<TCodeUnit>>;
    TEncoding encoding {};
    return ::Krys::Text::DecodeOneTo<TOutputContainer>(std::forward<TInput>(input), encoding);
  }

  /// @brief Converts one code unit from the given `input` view into code points in `TOutputContainer`.
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
  constexpr auto DecodeOne(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler, TState &state)
  {
    using TCodePoint = code_point_t<TEncoding>;
    constexpr std::size_t MaxCodePoints = ::Krys::Text::MaxDecodeCodePoints<TEncoding>;

    constexpr bool IsVoidContainer = Void<remove_cvref_t<TOutputContainer>>;
    if constexpr ((IsCharTraitable<TCodePoint> || IsUnicodeCodePoint<TCodePoint>) && IsVoidContainer)
    {
      using TOutput = InlineBasicString<TCodePoint, MaxCodePoints>;
      return ::Krys::Text::detail_decode::DecodeOneDispatch<true, false, TOutput>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
    else
    {
      using TOutput =
        conditional_t<!IsVoidContainer, TOutputContainer, InlineVector<TCodePoint, MaxCodePoints>>;
      return ::Krys::Text::detail_decode::DecodeOneDispatch<true, false, TOutput>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
  }

  /// @brief Converts one code unit from the given `input` view into code points in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] encoding The encoding for decoding the `input`'s code units into `output` code points.
  /// @param[in] errorHandler The handler to use for decoding errors.
  /// @result An object of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding, typename TErrorHandler>
  constexpr auto DecodeOne(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler)
  {
    auto state = ::Krys::Text::CreateDecodeState(encoding);
    return ::Krys::Text::DecodeOne<TOutputContainer>(std::forward<TInput>(input),
                                                     std::forward<TEncoding>(encoding),
                                                     std::forward<TErrorHandler>(errorHandler), state);
  }

  /// @brief Converts one code unit from the given `input` view into code points in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] encoding The encoding for decoding the `input`'s code units into `output` code points.
  /// @result An object of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding>
  constexpr auto DecodeOne(TInput &&input, TEncoding &&encoding)
  {
    ::Krys::Text::Handlers::DefaultHandler handler {};
    return ::Krys::Text::DecodeOne<TOutputContainer>(std::forward<TInput>(input),
                                                     std::forward<TEncoding>(encoding), handler);
  }

  /// @brief Converts one code unit from the given `input` view into code points in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @result An object of type `TOutputContainer`.
  /// @remarks Creates a default encoding using the underlying value type of `input`.
  template <typename TOutputContainer = void, typename TInput>
  constexpr auto DecodeOne(TInput &&input)
  {
    using TCodeUnit = ::Krys::Ranges::range_value_type_t<TInput>;
    using TEncoding =
      conditional_t<std::is_constant_evaluated(), default_consteval_code_unit_encoding_t<TCodeUnit>,
                    default_code_unit_encoding_t<TCodeUnit>>;

    TEncoding encoding {};
    return ::Krys::Text::DecodeOne<TOutputContainer>(std::forward<TInput>(input), encoding);
  }
}
