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
#include "Krystal.Text/Impl/SpanReconstruct.hpp"
#include "Krystal.Text/InlineContainers/InlineString.hpp"
#include "Krystal.Text/IsUnicodeCodePoint.hpp"
#include "Krystal.Text/MaxUnits.hpp"
#include "Krystal.Text/State.hpp"

namespace Krys
{
  /// @brief These functions convert from a view of input code points into a view of output code units using
  /// either the inferred or specified encoding. If no error handler is provided, the equivalent of the
  /// Handlers::DefaultHandler is used by default. If no associated state is provided for the encoding, one
  /// will be created with automatic storage duration (as a "stack" variable) for the provided encoding.

  /// @brief Converts a single indivisible unit of work's worth of code points of the given `input` view
  /// through the encoding to code units into the @p output view.
  /// @param[in]     input An input_view to read code points from and use in the EncodeOne operation that will
  /// produce code units.
  /// @param[in]     encoding The encoding that will be used to EncodeOne the input's code points into
  /// output code units.
  /// @param[in]     output An output_view to write code units to as the result of the EncodeOne operation
  /// from the intermediate code points.
  /// @param[in]     errorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in,out] state A reference to the associated state for the `encoding` 's EncodeOne step.
  /// @result A encode_one_result object that contains references to `state`.
  /// @remarks This function takes care of performing some "normalizations" of the output and input range
  /// types (like turning them into a span or string_view if at all recognizable or preferable).
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler, typename TState>
  constexpr auto EncodeOneIntoRaw(TInput &&input, TEncoding &&encoding, TOutput &&output,
                                  TErrorHandler &&errorHandler, TState &state)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TUErrorHandler = remove_cvref_t<TErrorHandler>;

    static_assert(Impl::EncodeLosslessOrDeliberate<TUEncoding, TUErrorHandler>,
                  ZTD_TEXT_LOSSY_ENCODE_MESSAGE_I_);

    return encoding.EncodeOne(std::forward<TInput>(input), std::forward<TOutput>(output),
                              std::forward<TErrorHandler>(errorHandler), state);
  }

  /// @brief Converts a single indivisible unit of work's worth of code points of the given `input` view
  /// through the encoding to code units into the `output` view.
  /// @param[in]     input An input_view to read code points from and use in the EncodeOne operation that will
  /// produce code units.
  /// @param[in]     encoding The encoding that will be used to EncodeOne the input's code points into
  /// output code units.
  /// @param[in]     output An output_view to write code units to as the result of the EncodeOne operation
  /// from the intermediate code points.
  /// @param[in]     errorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @result A stateless_encode_one_result object that contains references to `state`.
  /// @remarks Creates a default `state` using Krys::CreateEncodeState.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler>
  constexpr auto EncodeOneIntoRaw(TInput &&input, TEncoding &&encoding, TOutput &&output,
                                  TErrorHandler &&errorHandler)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TState = encode_state_t<TUEncoding>;

    TState state = ::Krys::CreateEncodeState(encoding);
    auto statefulResult =
      Krys::EncodeOneIntoRaw(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                             std::forward<TOutput>(output), std::forward<TErrorHandler>(errorHandler), state);
    return Impl::SliceToStatelessEncode(std::move(statefulResult));
  }

  /// @brief Converts a single indivisible unit of work's worth of code points of the given `input` view
  /// through the encoding to code units into the @p output view.
  ///
  /// @param[in]     input An input_view to read code points from and use in the EncodeOne operation that will
  /// produce code units.
  /// @param[in]     encoding The encoding that will be used to EncodeOne the input's code points into
  /// output code units.
  /// @param[in]     output An output_view to write code units to as the result of the EncodeOne operation
  /// from the intermediate code points.
  ///
  /// @result A stateless_encode_one_result object that contains references to `state`.
  ///
  /// @remarks Creates a default `ErrorHandler` that is similar to Handlers::DefaultHandler, but marked as
  /// careless.
  template <typename TInput, typename TEncoding, typename TOutput>
  constexpr auto EncodeOneIntoRaw(TInput &&input, TEncoding &&encoding, TOutput &&output)
  {
    Handlers::DefaultHandler handler {};
    return Krys::EncodeOneIntoRaw(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                  std::forward<TOutput>(output), handler);
  }

  /// @brief Converts a single indivisible unit of work's worth of code points of the given `input` view
  /// through the encoding to code units into the @p output view.
  ///
  /// @param[in]     input An input_view to read code points from and use in the EncodeOne operation that will
  /// produce code units.
  /// @param[in]     output An output_view to write code units to as the result of the EncodeOne operation
  /// from the intermediate code points.
  ///
  /// @result A stateless_encode_one_result object that contains references to `state`.
  ///
  /// @remarks Creates a default `encoding` by figuring out the `value_type` of the `input`, then passing
  /// that type into default_code_point_encoding_t. That encoding is that used to EncodeOne the input
  /// code points, by default.
  template <typename TInput, typename TOutput>
  constexpr auto EncodeOneIntoRaw(TInput &&input, TOutput &&output)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TCodePoint = Krys::Ranges::range_value_type_t<TUInput>;
    if (std::is_constant_evaluated())
    {
      // Use literal encoding instead, if we meet the right criteria
      using TEncoding = default_consteval_code_point_encoding_t<TCodePoint>;
      TEncoding encoding {};
      return Krys::EncodeOneIntoRaw(std::forward<TInput>(input), encoding, std::forward<TOutput>(output));
    }
    else
    {
      using TEncoding = default_code_point_encoding_t<TCodePoint>;
      TEncoding encoding {};
      return Krys::EncodeOneIntoRaw(std::forward<TInput>(input), encoding, std::forward<TOutput>(output));
    }
  }

  namespace Impl
  {
    template <typename TInput, typename TEncoding, typename TOutputContainer, typename TErrorHandler,
              typename TState>
    constexpr auto IntermediateEncodeOneToStorage(TInput &&input, TEncoding &&encoding,
                                                  TOutputContainer &output, TErrorHandler &&errorHandler,
                                                  TState &state)
    {
      // Well, SHIT. Write into temporary, then serialize one-by-one/bulk to output.
      // I'll admit, this is HELLA work to support...
      using TUEncoding = remove_cvref_t<TEncoding>;
      using TUErrorHandler = remove_cvref_t<TErrorHandler>;
      constexpr std::size_t maxUnits = MaxEncodeCodeUnits<TUEncoding>;
      constexpr std::size_t intermediateBufferMax = maxUnits;
      using TIntermediateValue = code_unit_t<TUEncoding>;
      using TOutput = Span<TIntermediateValue>;

      static_assert(Impl::EncodeLosslessOrDeliberate<TUEncoding, TUErrorHandler>,
                    ZTD_TEXT_LOSSY_ENCODE_MESSAGE_I_);

      TIntermediateValue intermediateTranslationBuffer[intermediateBufferMax] {};

      TOutput intermediateInitialOutput(intermediateTranslationBuffer);
      auto result = Krys::EncodeOneIntoRaw(std::forward<TInput>(input), encoding, intermediateInitialOutput,
                                           std::forward<TErrorHandler>(errorHandler), state);
      TOutput intermediateOutput(intermediateInitialOutput.data(), result.Output.data());
      Krys::Ranges::Impl::ContainerInsertBulk(output, intermediateOutput);
      return result;
    }

    template <bool OutputOnly, bool NoState, typename TOutputContainer, typename TInput, typename TEncoding,
              typename TErrorHandler, typename TState>
    constexpr auto EncodeOneDispatch(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler,
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
        Impl::IntermediateEncodeOneToStorage(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                             output, std::forward<TErrorHandler>(errorHandler), state);
      if constexpr (OutputOnly)
      {
        // We are explicitly discarding this information with this function call.
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

  /// @brief Converts a single indivisible unit of work's worth of code points of the given `input` view
  /// through the encoding to code units into the @p output view.
  /// @param[in]     input An input_view to read code points from and use in the EncodeOne operation that will
  /// produce code units.
  /// @param[in]     encoding The encoding that will be used to EncodeOne the input's code points into
  /// output code units.
  /// @param[in]     output An output_view to write code units to as the result of the EncodeOne operation
  /// from the intermediate code points.
  /// @param[in]     errorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in,out] state A reference to the associated state for the `encoding` 's EncodeOne step.
  /// @result A encode_one_result object that contains references to `state`.
  /// @remarks This function takes care of performing some "normalizations" of the output and input range
  /// types (like turning them into a span or string_view if at all recognizable or preferable).
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler, typename TState>
  constexpr auto EncodeOneInto(TInput &&input, TEncoding &&encoding, TOutput &&output,
                               TErrorHandler &&errorHandler, TState &state)
  {
    auto reconstructedInput = Impl::SpanReconstruct<TInput>(std::forward<TInput>(input));
    auto result = Krys::EncodeOneIntoRaw(std::move(reconstructedInput), std::forward<TEncoding>(encoding),
                                     std::forward<TOutput>(output), errorHandler, state);
    using TReconstructedResultInput = Impl::span_reconstruct_t<TInput, TInput>;
    using TReconstructedResultOutput = Impl::span_reconstruct_mutable_t<TOutput, TOutput>;
    return EncodeResult<TReconstructedResultInput, TReconstructedResultOutput, TState>(
      Impl::SpanReconstruct<TInput>(std::move(result.Input)),
      Impl::SpanReconstructMutable<TOutput>(std::move(result.Output)), result.State);
  }

  /// @brief Converts a single indivisible unit of work's worth of code points of the given `input` view
  /// through the encoding to code units into the `output` view.
  /// @param[in]     input An input_view to read code points from and use in the EncodeOne operation that will
  /// produce code units.
  /// @param[in]     encoding The encoding that will be used to EncodeOne the input's code points into
  /// output code units.
  /// @param[in]     output An output_view to write code units to as the result of the EncodeOne operation
  /// from the intermediate code points.
  /// @param[in]     errorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @result A stateless_encode_one_result object that contains references to `state`.
  /// @remarks Creates a default `state` using Krys::CreateEncodeState.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler>
  constexpr auto EncodeOneInto(TInput &&input, TEncoding &&encoding, TOutput &&output,
                               TErrorHandler &&errorHandler)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TState = encode_state_t<TUEncoding>;

    TState state = ::Krys::CreateEncodeState(encoding);
    auto statefulResult =
      Krys::EncodeOneInto(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                      std::forward<TOutput>(output), std::forward<TErrorHandler>(errorHandler), state);
    return Impl::SliceToStatelessEncode(std::move(statefulResult));
  }

  /// @brief Converts a single indivisible unit of work's worth of code points of the given `input` view
  /// through the encoding to code units into the @p output view.
  ///
  /// @param[in]     input An input_view to read code points from and use in the EncodeOne operation that will
  /// produce code units.
  /// @param[in]     encoding The encoding that will be used to EncodeOne the input's code points into
  /// output code units.
  /// @param[in]     output An output_view to write code units to as the result of the EncodeOne operation
  /// from the intermediate code points.
  ///
  /// @result A stateless_encode_one_result object that contains references to `state`.
  ///
  /// @remarks Creates a default `ErrorHandler` that is similar to Handlers::DefaultHandler, but marked as
  /// careless.
  template <typename TInput, typename TEncoding, typename TOutput>
  constexpr auto EncodeOneInto(TInput &&input, TEncoding &&encoding, TOutput &&output)
  {
    Handlers::DefaultHandler handler {};
    return Krys::EncodeOneInto(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                           std::forward<TOutput>(output), handler);
  }

  /// @brief Converts a single indivisible unit of work's worth of code points of the given `input` view
  /// through the encoding to code units into the @p output view.
  /// @param[in]     input An input_view to read code points from and use in the EncodeOne operation that will
  /// produce code units.
  /// @param[in]     output An output_view to write code units to as the result of the EncodeOne operation
  /// from the intermediate code points.
  /// @result A stateless_encode_one_result object that contains references to `state`.
  /// @remarks Creates a default `encoding` by figuring out the `value_type` of the `input`, then passing
  /// that type into default_code_point_encoding_t. That encoding is that used to EncodeOne the input
  /// code points, by default.
  template <typename TInput, typename TOutput>
  constexpr auto EncodeOneInto(TInput &&input, TOutput &&output)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TCodePoint = Krys::Ranges::range_value_type_t<TUInput>;
    if (std::is_constant_evaluated())
    {
      // Use literal encoding instead, if we meet the right criteria
      using TEncoding = default_consteval_code_point_encoding_t<TCodePoint>;
      TEncoding encoding {};
      return Krys::EncodeOneInto(std::forward<TInput>(input), encoding, std::forward<TOutput>(output));
    }
    else
    {
      using TEncoding = default_code_point_encoding_t<TCodePoint>;
      TEncoding encoding {};
      return Krys::EncodeOneInto(std::forward<TInput>(input), encoding, std::forward<TOutput>(output));
    }
  }

  /// @brief Converts a single indivisible unit of work's worth of code points of the given `input` view
  /// through the encoding to code units in the specified `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in]     input An input_view to read code points from and use in the EncodeOne operation that will
  /// produce code units.
  /// @param[in]     encoding The encoding that will be used to EncodeOne the input's code points into
  /// output code units.
  /// @param[in]     errorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in,out] state A reference to the associated state for the `encoding` 's EncodeOne step.
  /// @result A encode_one_result object that contains references to `state` and an output of type
  /// `TOutputContainer`.
  /// @remarks This function detects creates a container of type `TOutputContainer` and uses a typical @c
  /// std::back_inserter or `std::push_back_inserter` to fill in elements as it is written to. The result is
  /// then returned, with the `.Output` value put into the container.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding, typename TErrorHandler,
            typename TState>
  constexpr auto EncodeOneTo(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler,
                               TState &state)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TUOutputContainer = remove_cvref_t<TOutputContainer>;
    using TOutputCodeUnit = code_unit_t<TUEncoding>;
    constexpr bool IsVoidContainer = Void<TUOutputContainer>;
    constexpr bool IsStringable = (IsCharTraitable<TOutputCodeUnit> || IsUnicodeCodePoint<TOutputCodeUnit>);
    constexpr std::size_t maxUnits = MaxEncodeCodeUnits<TUEncoding>;
    if constexpr (IsVoidContainer && IsStringable)
    {
      using TRealOutputContainer = InlineBasicString<TOutputCodeUnit, maxUnits>;
      return Impl::EncodeOneDispatch<false, false, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
    else
    {
      using TRealOutputContainer =
        conditional_t<IsVoidContainer, InlineVector<TOutputCodeUnit, maxUnits>, TOutputContainer>;
      return Impl::EncodeOneDispatch<false, false, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
  }

  /// @brief Converts a single indivisible unit of work's worth of code points of the given `input` view
  /// through the encoding to code units in the specified `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in]     input An input_view to read code points from and use in the EncodeOne operation that will
  /// produce code units.
  /// @param[in]     encoding The encoding that will be used to EncodeOne the input's code points into
  /// output code units.
  /// @param[in]     errorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @result A stateless_encode_one_result object whose output is of type `TOutputContainer`.
  /// @remarks This function creates a `state` using Krys::CreateEncodeState.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding, typename TErrorHandler>
  constexpr auto EncodeOneTo(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TState = encode_state_t<TUEncoding>;
    using TUOutputContainer = remove_cvref_t<TOutputContainer>;
    using TOutputCodeUnit = code_unit_t<TUEncoding>;
    constexpr bool IsVoidContainer = Void<TUOutputContainer>;
    constexpr bool IsStringable = (IsCharTraitable<TOutputCodeUnit> || IsUnicodeCodePoint<TOutputCodeUnit>);
    constexpr std::size_t maxUnits = MaxEncodeCodeUnits<TUEncoding>;

    TState state = ::Krys::CreateEncodeState(encoding);
    if constexpr (IsVoidContainer && IsStringable)
    {
      using TRealOutputContainer = InlineBasicString<TOutputCodeUnit, maxUnits>;
      return Impl::EncodeOneDispatch<false, false, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
    else
    {
      using TRealOutputContainer =
        conditional_t<IsVoidContainer, InlineVector<TOutputCodeUnit, maxUnits>, TOutputContainer>;
      return Impl::EncodeOneDispatch<false, false, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
  }

  /// @brief Converts a single indivisible unit of work's worth of code points of the given `input` view
  /// through the encoding to code units in the specified `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in]     input An input_view to read code points from and use in the EncodeOne operation that will
  /// produce code units.
  /// @param[in]     encoding The encoding that will be used to EncodeOne the input's code points into
  /// output code units.
  /// @result A stateless_encode_one_result object whose output is of type `TOutputContainer`.
  /// @remarks This function creates a `handler` using Handlers::DefaultHandler, but marks it as careless.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding>
  constexpr auto EncodeOneTo(TInput &&input, TEncoding &&encoding)
  {
    Handlers::DefaultHandler handler {};
    return Krys::EncodeOneTo<TOutputContainer>(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                             handler);
  }

  /// @brief Converts a single indivisible unit of work's worth of code points of the given `input` view
  /// through the encoding to code units in the specified `TOutputContainer` type.
  ///
  /// @tparam TOutputContainer The container type to serialize data into.
  ///
  /// @param[in]     input An input_view to read code points from and use in the EncodeOne operation that will
  /// produce code units.
  ///
  /// @result A stateless_encode_one_result object whose output is of type `TOutputContainer`.
  ///
  /// @remarks This function creates an `encoding` by using the `value_type` of the `input` which is then
  /// passed through the DefaultCodePointEncoding type to get the default desired encoding.
  template <typename TOutputContainer = void, typename TInput>
  constexpr auto EncodeOneTo(TInput &&input)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TCodePoint = Krys::Ranges::range_value_type_t<TUInput>;
    if (std::is_constant_evaluated())
    {
      // Use literal encoding instead, if we meet the right criteria
      using TEncoding = default_consteval_code_point_encoding_t<TCodePoint>;
      TEncoding encoding {};
      return Krys::EncodeOneTo<TOutputContainer>(std::forward<TInput>(input), encoding);
    }
    else
    {
      using TEncoding = default_code_point_encoding_t<TCodePoint>;
      TEncoding encoding {};
      return Krys::EncodeOneTo<TOutputContainer>(std::forward<TInput>(input), encoding);
    }
  }

  /// @brief Converts a single indivisible unit of work's worth of code points of the given `input` view
  /// through the encoding to code units in the specified `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in]     input An input_view to read code points from and use in the EncodeOne operation that will
  /// produce code units.
  /// @param[in]     encoding The encoding that will be used to EncodeOne the input's code points into
  /// output code units.
  /// @param[in]     errorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in,out] state A reference to the associated state for the `encoding` 's EncodeOne step.
  /// @result An object of type `TOutputContainer` .
  /// @remarks This function detects creates a container of type `TOutputContainer` and uses a typical @c
  /// std::back_inserter or `std::push_back_inserter` to fill in elements as it is written to.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding, typename TErrorHandler,
            typename TState>
  constexpr auto EncodeOne(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler, TState &state)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TUOutputContainer = remove_cvref_t<TOutputContainer>;
    using TOutputCodeUnit = code_unit_t<TUEncoding>;
    constexpr bool IsVoidContainer = Void<TUOutputContainer>;
    constexpr bool IsStringable = (IsCharTraitable<TOutputCodeUnit> || IsUnicodeCodePoint<TOutputCodeUnit>);
    constexpr std::size_t maxUnits = MaxEncodeCodeUnits<TUEncoding>;
    if constexpr (IsVoidContainer && IsStringable)
    {
      using TRealOutputContainer = InlineBasicString<TOutputCodeUnit, maxUnits>;
      return Impl::EncodeOneDispatch<true, false, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
    else
    {
      using TRealOutputContainer =
        conditional_t<IsVoidContainer, InlineVector<TOutputCodeUnit, maxUnits>, TOutputContainer>;
      return Impl::EncodeOneDispatch<true, false, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
  }

  /// @brief Converts a single indivisible unit of work's worth of code points of the given `input` view
  /// through the encoding to code units in the specified `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in]     input An input_view to read code points from and use in the EncodeOne operation that will
  /// produce code units.
  /// @param[in]     encoding The encoding that will be used to EncodeOne the input's code points into
  /// output code units.
  /// @param[in]     errorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @result An object of type `TOutputContainer` .
  /// @remarks This function creates a `state` using Krys::CreateEncodeState.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding, typename TErrorHandler>
  constexpr auto EncodeOne(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TState = encode_state_t<TUEncoding>;

    TState state = ::Krys::CreateEncodeState(encoding);
    return Krys::EncodeOne<TOutputContainer>(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                         std::forward<TErrorHandler>(errorHandler), state);
  }

  /// @brief Converts a single indivisible unit of work's worth of code points of the given `input` view
  /// through the encoding to code units in the specified `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in]     input An input_view to read code points from and use in the EncodeOne operation that will
  /// produce code units.
  /// @param[in]     encoding The encoding that will be used to EncodeOne the input's code points into
  /// output code units.
  /// @result An object of type `TOutputContainer` .
  /// @remarks This function creates a `handler` using Handlers::DefaultHandler, but marks it as careless.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding>
  constexpr auto EncodeOne(TInput &&input, TEncoding &&encoding)
  {
    Handlers::DefaultHandler handler {};
    return Krys::EncodeOne<TOutputContainer>(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                         handler);
  }

  /// @brief Converts a single indivisible unit of work's worth of code points of the given `input` view
  /// through the encoding to code units in the specified `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in]     input An input_view to read code points from and use in the EncodeOne operation that will
  /// produce code units.
  /// @result An object of type `TOutputContainer` .
  /// @remarks This function creates an `encoding` by using the `value_type` of the `input` which is then
  /// passed through the DefaultCodePointEncoding type to get the default desired encoding.
  template <typename TOutputContainer = void, typename TInput>
  constexpr auto EncodeOne(TInput &&input)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TCodePoint = Krys::Ranges::range_value_type_t<TUInput>;
    if (std::is_constant_evaluated())
    {
      // Use literal encoding instead, if we meet the right criteria
      using TEncoding = default_consteval_code_point_encoding_t<TCodePoint>;
      TEncoding encoding {};
      return Krys::EncodeOne<TOutputContainer>(std::forward<TInput>(input), encoding);
    }
    else
    {
      using TEncoding = default_code_point_encoding_t<TCodePoint>;
      TEncoding encoding {};
      return Krys::EncodeOne<TOutputContainer>(std::forward<TInput>(input), encoding);
    }
  }
}
