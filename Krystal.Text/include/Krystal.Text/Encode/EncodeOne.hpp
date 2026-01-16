#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/Impl/InsertBulk.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/_detail/IsLossless.hpp"
#include "Krystal.Text/Encode/EncodeResult.hpp"
#include "Krystal.Text/Encodings/DefaultEncoding.hpp"
#include "Krystal.Text/Handlers/DefaultHandler.hpp"
#include "Krystal.Text/_detail/SpanReconstruct.hpp"
#include "Krystal.Text/InlineContainers/InlineBasicString.hpp"
#include "Krystal.Text/IsUnicodeCodePoint.hpp"
#include "Krystal.Text/MaxUnits.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/TypeTraits.hpp"

/// These functions convert from a view of input code points into a view of output code units using either the
/// inferred or specified encodings. If no error handler is provided, DefaultHandler is used by default. If no
/// state is provided for the encoding, it will created automatically as needed.
namespace Krys::Text
{
  /// @brief Converts one code point from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code points from.
  /// @param[in] encoding The encoding for encoding the `input`'s code points into `output` code units.
  /// @param[in] output An output view to write code units to.
  /// @param[in] errorHandler The handler to use for encoding errors.
  /// @param[in,out] state A reference to the associated state for the `encoding`'s encode step.
  /// @result A EncodeResult object that contains references to `state`.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler, typename TState>
  constexpr auto EncodeOneIntoRaw(TInput &&input, TEncoding &&encoding, TOutput &&output,
                                  TErrorHandler &&errorHandler, TState &state)
  {
    static_assert(::Krys::Text::EncodeLosslessOrDeliberate<TEncoding, TErrorHandler>,
                  KRYS_TEXT_LOSSY_ENCODE_MESSAGE);

    return encoding.EncodeOne(std::forward<TInput>(input), std::forward<TOutput>(output),
                              std::forward<TErrorHandler>(errorHandler), state);
  }

  /// @brief Converts one code point from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code points from.
  /// @param[in] encoding The encoding for encoding the `input`'s code points into `output` code units.
  /// @param[in] output An output view to write code units to.
  /// @param[in] errorHandler The handler to use for encoding errors.
  /// @result A StatelessEncodeResult object.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler>
  constexpr auto EncodeOneIntoRaw(TInput &&input, TEncoding &&encoding, TOutput &&output,
                                  TErrorHandler &&errorHandler)
  {
    auto state = ::Krys::Text::CreateEncodeState(encoding);
    auto statefulResult = ::Krys::Text::EncodeOneIntoRaw(
      std::forward<TInput>(input), std::forward<TEncoding>(encoding), std::forward<TOutput>(output),
      std::forward<TErrorHandler>(errorHandler), state);

    return ::Krys::Text::Impl::SliceToStatelessEncode(std::move(statefulResult));
  }

  /// @brief Converts one code point from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code points from.
  /// @param[in] encoding The encoding for encoding the `input`'s code points into `output` code units.
  /// @param[in] output An output view to write code units to.
  /// @result A StatelessEncodeResult object.
  template <typename TInput, typename TEncoding, typename TOutput>
  constexpr auto EncodeOneIntoRaw(TInput &&input, TEncoding &&encoding, TOutput &&output)
  {
    ::Krys::Text::Handlers::DefaultHandler handler {};
    return ::Krys::Text::EncodeOneIntoRaw(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                          std::forward<TOutput>(output), handler);
  }

  /// @brief Converts one code point from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code points from.
  /// @param[in] encoding The encoding for encoding the `input`'s code points into `output` code units.
  /// @param[in] output An output view to write code units to.
  /// @result A StatelessEncodeResult object.
  /// @remarks Creates a default encoding using the underlying value type of `input`.
  template <typename TInput, typename TOutput>
  constexpr auto EncodeOneIntoRaw(TInput &&input, TOutput &&output)
  {
    using TCodePoint = ::Krys::Ranges::range_value_type_t<TInput>;
    using TEncoding =
      conditional_t<std::is_constant_evaluated(), default_consteval_code_point_encoding_t<TCodePoint>,
                    default_code_point_encoding_t<TCodePoint>>;

    TEncoding encoding {};
    return ::Krys::Text::EncodeOneIntoRaw(std::forward<TInput>(input), encoding,
                                          std::forward<TOutput>(output));
  }
}

namespace Krys::Text::detail_encode
{
  template <typename TInput, typename TEncoding, typename TOutputContainer, typename TErrorHandler,
            typename TState>
  constexpr auto IntermediateEncodeOneToStorage(TInput &&input, TEncoding &&encoding,
                                                TOutputContainer &output, TErrorHandler &&errorHandler,
                                                TState &state)
  {
    static_assert(::Krys::Text::EncodeLosslessOrDeliberate<TEncoding, TErrorHandler>,
                  KRYS_TEXT_LOSSY_ENCODE_MESSAGE);

    constexpr std::size_t MaxBufferSize = ::Krys::Text::MaxEncodeCodeUnits<TEncoding>;
    using TCodeUnit = code_unit_t<TEncoding>;

    TCodeUnit translationBuffer[MaxBufferSize] {};
    Span<TCodeUnit> initialOutput(translationBuffer);
    auto result = ::Krys::Text::EncodeOneIntoRaw(std::forward<TInput>(input), encoding, initialOutput,
                                                 std::forward<TErrorHandler>(errorHandler), state);

    ::Krys::Ranges::Impl::ContainerInsertBulk(output, Span(initialOutput.data(), result.Output.data()));
    return result;
  }

  template <bool OutputOnly, bool NoState, typename TOutputContainer, typename TInput, typename TEncoding,
            typename TErrorHandler, typename TState>
  constexpr auto EncodeOneDispatch(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler,
                                   TState &state)
  {
    TOutputContainer output {};
    if constexpr (::Krys::Ranges::HasSizeADL<TInput>)
    {
      using TSize = decltype(::Krys::Ranges::size(input));
      if constexpr (::Krys::Ranges::has_reserve_with_size<TOutputContainer, TSize>)
      {
        constexpr std::size_t maxUnits = ::Krys::Text::MaxEncodeCodeUnits<TEncoding>;
        TSize outputSizeHint = static_cast<TSize>(::Krys::Ranges::size(input));
        outputSizeHint *= (maxUnits > 3) ? (maxUnits / 4) : maxUnits;
        output.reserve(outputSizeHint);
      }
    }

    auto statefulResult = ::Krys::Text::detail_encode::IntermediateEncodeOneToStorage(
      std::forward<TInput>(input), std::forward<TEncoding>(encoding), output,
      std::forward<TErrorHandler>(errorHandler), state);

    if constexpr (OutputOnly)
    {
      // We are explicitly discarding this information with this function call.
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
  /// @brief Converts one code point from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code points from.
  /// @param[in] encoding The encoding for encoding the `input`'s code points into `output` code units.
  /// @param[in] output An output view to write code units to.
  /// @param[in] errorHandler The handler to use for encoding errors.
  /// @param[in,out] state A reference to the associated state for the `encoding`'s encode step.
  /// @result A EncodeResult object that contains references to `state`.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler, typename TState>
  constexpr auto EncodeOneInto(TInput &&input, TEncoding &&encoding, TOutput &&output,
                               TErrorHandler &&errorHandler, TState &state)
  {
    auto reconstructedInput = ::Krys::Text::detail::SpanReconstruct<TInput>(std::forward<TInput>(input));
    auto result =
      ::Krys::Text::EncodeOneIntoRaw(std::move(reconstructedInput), std::forward<TEncoding>(encoding),
                                     std::forward<TOutput>(output), errorHandler, state);
    return ::Krys::Text::EncodeResult(
      ::Krys::Text::detail::SpanReconstruct<TInput>(std::move(result.Input)),
      ::Krys::Text::detail::SpanReconstructMutable<TOutput>(std::move(result.Output)), result.State);
  }

  /// @brief Converts one code point from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code points from.
  /// @param[in] encoding The encoding for encoding the `input`'s code points into `output` code units.
  /// @param[in] output An output view to write code units to.
  /// @param[in] errorHandler The handler to use for encoding errors.
  /// @result A StatelessEncodeResult object.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler>
  constexpr auto EncodeOneInto(TInput &&input, TEncoding &&encoding, TOutput &&output,
                               TErrorHandler &&errorHandler)
  {
    auto state = ::Krys::Text::CreateEncodeState(encoding);
    auto statefulResult = ::Krys::Text::EncodeOneInto(
      std::forward<TInput>(input), std::forward<TEncoding>(encoding), std::forward<TOutput>(output),
      std::forward<TErrorHandler>(errorHandler), state);

    return ::Krys::Text::Impl::SliceToStatelessEncode(std::move(statefulResult));
  }

  /// @brief Converts one code point from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code points from.
  /// @param[in] encoding The encoding for encoding the `input`'s code points into `output` code units.
  /// @param[in] output An output view to write code units to.
  /// @result A StatelessEncodeResult object.
  template <typename TInput, typename TEncoding, typename TOutput>
  constexpr auto EncodeOneInto(TInput &&input, TEncoding &&encoding, TOutput &&output)
  {
    ::Krys::Text::Handlers::DefaultHandler handler {};
    return ::Krys::Text::EncodeOneInto(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                       std::forward<TOutput>(output), handler);
  }

  /// @brief Converts one code point from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code points from.
  /// @param[in] output An output view to write code units to.
  /// @result A StatelessEncodeResult object.
  /// @remarks Creates a default encoding using the underlying value type of `input`.
  template <typename TInput, typename TOutput>
  constexpr auto EncodeOneInto(TInput &&input, TOutput &&output)
  {
    using TCodePoint = ::Krys::Ranges::range_value_type_t<TInput>;
    using TEncoding =
      conditional_t<std::is_constant_evaluated(), default_consteval_code_point_encoding_t<TCodePoint>,
                    default_code_point_encoding_t<TCodePoint>>;

    TEncoding encoding {};
    return ::Krys::Text::EncodeOneInto(std::forward<TInput>(input), encoding, std::forward<TOutput>(output));
  }

  /// @brief Converts one code point from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code points from.
  /// @param[in] encoding The encoding for encoding the `input`'s code points into `output` code units.
  /// @param[in] errorHandler The handler to use for encoding errors.
  /// @param[in,out] state A reference to the associated state for the `encoding`'s encode step.
  /// @result A EncodeResult object with an output of type `TOutputContainer`.
  /// @remarks This function detects creates a container of type `TOutputContainer` and uses a typical @c
  /// std::back_inserter or `std::push_back_inserter` to fill in elements as it is written to. The result is
  /// then returned, with the `.Output` value put into the container.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding, typename TErrorHandler,
            typename TState>
  constexpr auto EncodeOneTo(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler,
                             TState &state)
  {
    using TCodeUnit = code_unit_t<TEncoding>;
    constexpr std::size_t MaxUnits = ::Krys::Text::MaxEncodeCodeUnits<TEncoding>;

    constexpr bool IsVoidContainer = Void<remove_cvref_t<TOutputContainer>>;
    if constexpr ((IsCharTraitable<TCodeUnit> || IsUnicodeCodePoint<TCodeUnit>) && IsVoidContainer)
    {
      using TOutput = InlineBasicString<TCodeUnit, MaxUnits>;
      return ::Krys::Text::detail_encode::EncodeOneDispatch<false, false, TOutput>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
    else
    {
      using TOutput = conditional_t<!IsVoidContainer, TOutputContainer, InlineVector<TCodeUnit, MaxUnits>>;
      return ::Krys::Text::detail_encode::EncodeOneDispatch<false, false, TOutput>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
  }

  /// @brief Converts one code point from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code points from.
  /// @param[in] encoding The encoding for encoding the `input`'s code points into `output` code units.
  /// @param[in] errorHandler The handler to use for encoding errors.
  /// @result A StatelessEncodeResult object with an output of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding, typename TErrorHandler>
  constexpr auto EncodeOneTo(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler)
  {
    using TCodeUnit = code_unit_t<TEncoding>;
    constexpr std::size_t MaxUnits = ::Krys::Text::MaxEncodeCodeUnits<TEncoding>;

    auto state = ::Krys::Text::CreateEncodeState(encoding);
    constexpr bool IsVoidContainer = Void<remove_cvref_t<TOutputContainer>>;
    if constexpr ((IsCharTraitable<TCodeUnit> || IsUnicodeCodePoint<TCodeUnit>) && IsVoidContainer)
    {
      using TOutput = InlineBasicString<TCodeUnit, MaxUnits>;
      return ::Krys::Text::detail_encode::EncodeOneDispatch<false, false, TOutput>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
    else
    {
      using TOutput = conditional_t<!IsVoidContainer, TOutputContainer, InlineVector<TCodeUnit, MaxUnits>>;
      return ::Krys::Text::detail_encode::EncodeOneDispatch<false, false, TOutput>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
  }

  /// @brief Converts one code point from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code points from.
  /// @param[in] encoding The encoding for encoding the `input`'s code points into `output` code units.
  /// @result A StatelessEncodeResult object with an output of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding>
  constexpr auto EncodeOneTo(TInput &&input, TEncoding &&encoding)
  {
    ::Krys::Text::Handlers::DefaultHandler handler {};
    return ::Krys::Text::EncodeOneTo<TOutputContainer>(std::forward<TInput>(input),
                                                       std::forward<TEncoding>(encoding), handler);
  }

  /// @brief Converts one code point from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code points from.
  /// @result A StatelessEncodeResult object with an output of type `TOutputContainer`.
  /// @remarks Creates a default encoding using the underlying value type of `input`.
  template <typename TOutputContainer = void, typename TInput>
  constexpr auto EncodeOneTo(TInput &&input)
  {
    using TCodePoint = ::Krys::Ranges::range_value_type_t<TInput>;
    using TEncoding =
      conditional_t<std::is_constant_evaluated(), default_consteval_code_point_encoding_t<TCodePoint>,
                    default_code_point_encoding_t<TCodePoint>>;

    TEncoding encoding {};
    return ::Krys::Text::EncodeOneTo<TOutputContainer>(std::forward<TInput>(input), encoding);
  }

  /// @brief Converts one code point from the given `input` view into code units in `TOutputContainer`.
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
  constexpr auto EncodeOne(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler, TState &state)
  {
    using TCodeUnit = code_unit_t<TEncoding>;
    constexpr std::size_t MaxUnits = ::Krys::Text::MaxEncodeCodeUnits<TEncoding>;

    constexpr bool IsVoidContainer = Void<remove_cvref_t<TOutputContainer>>;
    if constexpr ((IsCharTraitable<TCodeUnit> || IsUnicodeCodePoint<TCodeUnit>) && IsVoidContainer)
    {
      using TOutput = InlineBasicString<TCodeUnit, MaxUnits>;
      return ::Krys::Text::detail_encode::EncodeOneDispatch<true, false, TOutput>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
    else
    {
      using TOutput = conditional_t<!IsVoidContainer, TOutputContainer, InlineVector<TCodeUnit, MaxUnits>>;
      return ::Krys::Text::detail_encode::EncodeOneDispatch<true, false, TOutput>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
  }

  /// @brief Converts one code point from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code points from.
  /// @param[in] encoding The encoding for encoding the `input`'s code points into `output` code units.
  /// @param[in] errorHandler The handler to use for encoding errors.
  /// @result An object of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding, typename TErrorHandler>
  constexpr auto EncodeOne(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler)
  {
    auto state = ::Krys::Text::CreateEncodeState(encoding);
    return ::Krys::Text::EncodeOne<TOutputContainer>(std::forward<TInput>(input),
                                                     std::forward<TEncoding>(encoding),
                                                     std::forward<TErrorHandler>(errorHandler), state);
  }

  /// @brief Converts one code point from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code points from.
  /// @param[in] encoding The encoding for encoding the `input`'s code points into `output` code units.
  /// @result An object of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding>
  constexpr auto EncodeOne(TInput &&input, TEncoding &&encoding)
  {
    ::Krys::Text::Handlers::DefaultHandler handler {};
    return ::Krys::Text::EncodeOne<TOutputContainer>(std::forward<TInput>(input),
                                                     std::forward<TEncoding>(encoding), handler);
  }

  /// @brief Converts one code point from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code points from.
  /// @result An object of type `TOutputContainer`.
  /// @remarks Creates a default encoding using the underlying value type of `input`.
  template <typename TOutputContainer = void, typename TInput>
  constexpr auto EncodeOne(TInput &&input)
  {
    using TCodePoint = ::Krys::Ranges::range_value_type_t<TInput>;
    using TEncoding =
      conditional_t<std::is_constant_evaluated(), default_consteval_code_point_encoding_t<TCodePoint>,
                    default_code_point_encoding_t<TCodePoint>>;

    TEncoding encoding {};
    return ::Krys::Text::EncodeOne<TOutputContainer>(std::forward<TInput>(input), encoding);
  }
}