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
#include "Krystal.Text/Impl/SpanReconstruct.hpp"
#include "Krystal.Text/InlineContainers/InlineBasicString.hpp"
#include "Krystal.Text/InlineContainers/InlineString.hpp"
#include "Krystal.Text/InlineContainers/InlineVector.hpp"
#include "Krystal.Text/IsUnicodeCodePoint.hpp"
#include "Krystal.Text/MaxUnits.hpp"
#include "Krystal.Text/State.hpp"

namespace Krys
{
  /// @brief These functions convert from a view of input into a view of output using either the inferred or
  /// specified encodings. If no error handler is provided, the equivalent of the Handlers::DefaultHandler is
  /// used by default. If no associated state is provided for either the "to" or "from" encodings, one will be
  /// created with automatic storage duration (as a "stack" variable) for the provided encoding.
  /// @brief Converts one indivisible unit of information from the code units of the given `input` view
  /// through the encoding to code points into the `output` view.
  /// @param[in] input An input_view to read code units from and use in the DecodeOne operation that will
  /// produce code points.
  /// @param[in] encoding The encoding that will be used to DecodeOne the input's code points into
  /// output code units.
  /// @param[in] output An output_view to write code points to as the result of the DecodeOne operation from
  /// the intermediate code units.
  /// @param[in] errorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in,out] state A reference to the associated state for the `encoding` 's DecodeOne step.
  /// @result A decode_one_result object that contains references to `state`.
  /// @remarks This function is simply a small wrapper for calling DecodeOne on the `encoding` object.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler, typename TState>
  constexpr auto DecodeOneIntoRaw(TInput &&input, TEncoding &&encoding, TOutput &&output,
                                  TErrorHandler &&errorHandler, TState &state)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TUErrorHandler = remove_cvref_t<TErrorHandler>;

    static_assert(Impl::DecodeLosslessOrDeliberate<TUEncoding, TUErrorHandler>,
                  ZTD_TEXT_LOSSY_DECODE_MESSAGE_I_);

    return encoding.DecodeOne(std::forward<TInput>(input), std::forward<TOutput>(output), errorHandler,
                              state);
  }

  /// @brief Converts one indivisible unit of information from the code units of the given `input` view
  /// through the encoding to code points into the `output` view.
  /// @param[in] input An input_view to read code units from and use in the DecodeOne operation that will
  /// produce code points.
  /// @param[in] encoding The encoding that will be used to DecodeOne the input's code points into
  /// output code units.
  /// @param[in] output An output_view to write code points to as the result of the DecodeOne operation from
  /// the intermediate code units.
  /// @param[in] errorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @result A stateless_decode_one_result object that contains references to `state`.
  /// @remarks Creates a default `state` using CreateDecodeState.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler>
  constexpr auto DecodeOneIntoRaw(TInput &&input, TEncoding &&encoding, TOutput &&output,
                                  TErrorHandler &&errorHandler)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TState = decode_state_t<TUEncoding>;

    TState state = Krys::CreateDecodeState(encoding);
    auto statefulResult =
      Krys::DecodeOneIntoRaw(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                             std::forward<TOutput>(output), std::forward<TErrorHandler>(errorHandler), state);
    return Impl::SliceToStatelessDecode(std::move(statefulResult));
  }

  /// @brief Converts one indivisible unit of information from the code units of the given `input` view
  /// through the encoding to code points into the `output` view.
  /// @param[in] input An input_view to read code units from and use in the DecodeOne operation that will
  /// produce code points.
  /// @param[in] encoding The encoding that will be used to DecodeOne the input's code points into
  /// output code units.
  /// @param[in] output An output_view to write code points to as the result of the DecodeOne operation from
  /// the intermediate code units.
  /// @result A stateless_decode_one_result object that contains references to `state`.
  /// @remarks Creates a default `ErrorHandler` that is similar to Handlers::DefaultHandler, but marked as
  /// careless.
  template <typename TInput, typename TEncoding, typename TOutput>
  constexpr auto DecodeOneIntoRaw(TInput &&input, TEncoding &&encoding, TOutput &&output)
  {
    Handlers::DefaultHandler handler {};
    return Krys::DecodeOneIntoRaw(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                  std::forward<TOutput>(output), handler);
  }

  /// @brief Converts one indivisible unit of information from the code units of the given `input` view
  /// through the encoding to code points into the `output` view.
  /// @param[in] input An input_view to read code units from and use in the DecodeOne operation that will
  /// produce code points.
  /// @param[in] output An output_view to write code points to as the result of the DecodeOne operation from
  /// the intermediate code units.
  /// @result A stateless_decode_one_result object that contains references to `state`.
  /// @remarks Creates a default `encoding` by figuring out the `value_type` of the `input`, then passing that
  /// type into default_code_point_encoding_t. That encoding is that used to DecodeOne the input code
  /// units, by default.
  template <typename TInput, typename TOutput>
  constexpr auto DecodeOneIntoRaw(TInput &&input, TOutput &&output)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TCodeUnit = Krys::Ranges::range_value_type_t<TUInput>;
    if (std::is_constant_evaluated())
    {
      // Use literal encoding instead, if we meet the right criteria
      using TEncoding = default_consteval_code_unit_encoding_t<TCodeUnit>;
      TEncoding encoding {};
      return Krys::DecodeOneIntoRaw(std::forward<TInput>(input), encoding, std::forward<TOutput>(output));
    }
    else
    {
      using TEncoding = default_code_unit_encoding_t<TCodeUnit>;
      TEncoding encoding {};
      return Krys::DecodeOneIntoRaw(std::forward<TInput>(input), encoding, std::forward<TOutput>(output));
    }
  }

  namespace Impl
  {
    template <typename TInput, typename TEncoding, typename TOutputContainer, typename TErrorHandler,
              typename TState>
    constexpr auto IntermediateDecodeOneToStorage(TInput &&input, TEncoding &&encoding,
                                                  TOutputContainer &output, TErrorHandler &&errorHandler,
                                                  TState &state)
    {
      using TUEncoding = remove_cvref_t<TEncoding>;
      constexpr std::size_t maxUnits = MaxCodePoints<TUEncoding> * 2;
      constexpr std::size_t intermediateBufferMax = maxUnits;
      using TIntermediateValue = code_point_t<TUEncoding>;
      using TInitialOutput = Span<TIntermediateValue, intermediateBufferMax>;
      using TOutput = Span<TIntermediateValue>;

      static_assert(Impl::DecodeLosslessOrDeliberate<TEncoding, TErrorHandler>,
                    ZTD_TEXT_LOSSY_DECODE_MESSAGE_I_);

      TIntermediateValue intermediateTranslationBuffer[intermediateBufferMax] {};
      TInitialOutput intermediateInitialOutput(intermediateTranslationBuffer);
      auto result = Krys::DecodeOneIntoRaw(std::forward<TInput>(input), encoding, intermediateInitialOutput,
                                     errorHandler, state);
      TOutput intermediateOutput(intermediateInitialOutput.data(), result.Output.data());
      Krys::Ranges::Impl::ContainerInsertBulk(output, intermediateOutput);
      return result;
    }

    template <bool OutputOnly, bool NoState, typename TOutputContainer, typename TInput, typename TEncoding,
              typename TErrorHandler, typename TState>
    constexpr auto DecodeOneDispatch(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler,
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
        Impl::IntermediateDecodeOneToStorage(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                             output, std::forward<TErrorHandler>(errorHandler), state);
      if constexpr (OutputOnly)
      {
        // We are explicitly discarding this information with this function call.
        (void)statefulResult;
        return output;
      }
      else if constexpr (NoState)
      {
        return Impl::ReplaceDecodeResultOutputNoState(std::move(statefulResult), std::move(output));
      }
      else
      {
        return Impl::ReplaceDecodeResultOutput(std::move(statefulResult), std::move(output));
      }
    }
  }

  /// @brief Converts one indivisible unit of information from the code units of the given `input` view
  /// through the encoding to code points into the `output` view.
  /// @param[in] input An input_view to read code units from and use in the DecodeOne operation that will
  /// produce code points.
  /// @param[in] encoding The encoding that will be used to DecodeOne the input's code points into
  /// output code units.
  /// @param[in] output An output_view to write code points to as the result of the DecodeOne operation from
  /// the intermediate code units.
  /// @param[in] errorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in,out] state A reference to the associated state for the `encoding` 's DecodeOne step.
  /// @result A decode_one_result object that contains references to `state`.
  /// @remarks This function is simply a small wrapper for calling DecodeOne on the `encoding` object.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler, typename TState>
  constexpr auto DecodeOneInto(TInput &&input, TEncoding &&encoding, TOutput &&output,
                               TErrorHandler &&errorHandler, TState &state)
  {
    auto reconstructedInput = Impl::SpanReconstruct<TInput>(std::forward<TInput>(input));
    auto result = Krys::DecodeOneIntoRaw(std::move(reconstructedInput), std::forward<TEncoding>(encoding),
                                         std::forward<TOutput>(output), errorHandler, state);
    using TReconstructedResultInput = Impl::span_reconstruct_t<TInput, TInput>;
    using TReconstructedResultOutput = Impl::span_reconstruct_mutable_t<TOutput, TOutput>;
    return DecodeResult<TReconstructedResultInput, TReconstructedResultOutput, TState>(
      Impl::SpanReconstruct<TInput>(std::move(result.Input)),
      Impl::SpanReconstructMutable<TOutput>(std::move(result.Output)), result.State);
  }

  /// @brief Converts one indivisible unit of information from the code units of the given `input` view
  /// through the encoding to code points into the `output` view.
  /// @param[in] input An input_view to read code units from and use in the DecodeOne operation that will
  /// produce code points.
  /// @param[in] encoding The encoding that will be used to DecodeOne the input's code points into
  /// output code units.
  /// @param[in] output An output_view to write code points to as the result of the DecodeOne operation from
  /// the intermediate code units.
  /// @param[in] errorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @result A stateless_decode_one_result object that contains references to `state`.
  /// @remarks Creates a default `state` using CreateDecodeState.
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler>
  constexpr auto DecodeOneInto(TInput &&input, TEncoding &&encoding, TOutput &&output,
                               TErrorHandler &&errorHandler)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TState = decode_state_t<TUEncoding>;

    TState state = Krys::CreateDecodeState(encoding);
    auto statefulResult =
      Krys::DecodeOneInto(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                    std::forward<TOutput>(output), std::forward<TErrorHandler>(errorHandler), state);
    return Impl::SliceToStatelessDecode(std::move(statefulResult));
  }

  /// @brief Converts one indivisible unit of information from the code units of the given `input` view
  /// through the encoding to code points into the `output` view.
  /// @param[in] input An input_view to read code units from and use in the DecodeOne operation that will
  /// produce code points.
  /// @param[in] encoding The encoding that will be used to DecodeOne the input's code points into
  /// output code units.
  /// @param[in] output An output_view to write code points to as the result of the DecodeOne operation from
  /// the intermediate code units.
  /// @result A stateless_decode_one_result object that contains references to `state`.
  /// @remarks Creates a default `ErrorHandler` that is similar to Handlers::DefaultHandler, but marked as
  /// careless.
  template <typename TInput, typename TEncoding, typename TOutput>
  constexpr auto DecodeOneInto(TInput &&input, TEncoding &&encoding, TOutput &&output)
  {
    Handlers::DefaultHandler handler {};
    return Krys::DecodeOneInto(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                         std::forward<TOutput>(output), handler);
  }

  /// @brief Converts one indivisible unit of information from the code units of the given `input` view
  /// through the encoding to code points into the `output` view.
  /// @param[in] input An input_view to read code units from and use in the DecodeOne operation that will
  /// produce code points.
  /// @param[in] output An output_view to write code points to as the result of the DecodeOne operation from
  /// the intermediate code units.
  /// @result A stateless_decode_one_result object that contains references to `state`.
  /// @remarks Creates a default `encoding` by figuring out the `value_type` of the `input`, then passing that
  /// type into default_code_point_encoding_t. That encoding is that used to DecodeOne the input code
  /// units, by default.
  template <typename TInput, typename TOutput>
  constexpr auto DecodeOneInto(TInput &&input, TOutput &&output)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TCodeUnit = Krys::Ranges::range_value_type_t<TUInput>;
    if (std::is_constant_evaluated())
    {
      // Use literal encoding instead, if we meet the right criteria
      using TEncoding = default_consteval_code_unit_encoding_t<TCodeUnit>;
      TEncoding encoding {};
      return Krys::DecodeOneInto(std::forward<TInput>(input), encoding, std::forward<TOutput>(output));
    }
    else
    {
      using TEncoding = default_code_unit_encoding_t<TCodeUnit>;
      TEncoding encoding {};
      return Krys::DecodeOneInto(std::forward<TInput>(input), encoding, std::forward<TOutput>(output));
    }
  }

  /// @brief Converts one indivisible unit of information from the code units of the given `input` view
  /// through the encoding to code points, stored in an object of `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input_view to read code units from and use in the DecodeOne operation that will
  /// produce code points.
  /// @param[in] encoding The encoding that will be used to DecodeOne the input's code points into
  /// output code units.
  /// @param[in] errorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in,out] state A reference to the associated state for the `encoding` 's DecodeOne step.
  /// @result A decode_one_result object that contains references to `state` and an output of type
  /// `TOutputContainer`.
  /// @remarks This function detects creates a container of type `TOutputContainer` and uses a typical @c
  /// std::back_inserter or `std::push_back_inserter` to fill in elements as it is written to. The result is
  /// then returned, with the `.Output` value put into the container.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding, typename TErrorHandler,
            typename TState>
  constexpr auto DecodeOneTo(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler,
                             TState &state)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TUOutputContainer = remove_cvref_t<TOutputContainer>;
    using TOutputCodePoint = code_point_t<TUEncoding>;
    constexpr bool IsVoidContainer = Void<TUOutputContainer>;
    constexpr bool IsStringable = (IsCharTraitable<TOutputCodePoint> || IsUnicodeCodePoint<TOutputCodePoint>);
    constexpr std::size_t maxUnits = MaxDecodeCodePoints<TUEncoding>;
    if constexpr (IsVoidContainer && IsStringable)
    {
      // prevent instantiation errors with basic_string by boxing it inside of an "if constexpr"
      using TRealOutputContainer = InlineBasicString<TOutputCodePoint, maxUnits>;
      return Impl::DecodeOneDispatch<false, false, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
    else
    {
      using TRealOutputContainer =
        conditional_t<IsVoidContainer, InlineVector<TOutputCodePoint, maxUnits>, TOutputContainer>;
      return Impl::DecodeOneDispatch<false, false, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
  }

  /// @brief Converts one indivisible unit of information from the code units of the given `input` view
  /// through the encoding to code points, stored in an object of `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input_view to read code units from and use in the DecodeOne operation that will
  /// produce code points.
  /// @param[in] encoding The encoding that will be used to DecodeOne the input's code points into
  /// output code units.
  /// @param[in] errorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @result A stateless_decode_one_result object whose output is of type `TOutputContainer`.
  /// @remarks This function creates a `state` using CreateDecodeState.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding, typename TErrorHandler>
  constexpr auto DecodeOneTo(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TState = decode_state_t<TUEncoding>;
    TState state = Krys::CreateDecodeState(encoding);
    return Krys::DecodeOneTo<TOutputContainer>(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                               std::forward<TErrorHandler>(errorHandler), state);
  }

  /// @brief Converts one indivisible unit of information from the code units of the given `input` view
  /// through the encoding to code points, stored in an object of `TOutputContainer` type.
  ///
  /// @tparam TOutputContainer The container type to serialize data into.
  ///
  /// @param[in] input An input_view to read code units from and use in the DecodeOne operation that will
  /// produce code points.
  /// @param[in] encoding The encoding that will be used to DecodeOne the input's code points into
  /// output code units.
  ///
  /// @result A stateless_decode_one_result object whose output is of type `TOutputContainer`.
  ///
  /// @remarks This function creates a `handler` using Handlers::DefaultHandler, but marks it as careless.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding>
  constexpr auto DecodeOneTo(TInput &&input, TEncoding &&encoding)
  {
    Handlers::DefaultHandler handler {};
    return Krys::DecodeOneTo<TOutputContainer>(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                               handler);
  }

  /// @brief Converts one indivisible unit of information from the code units of the given `input` view
  /// through the encoding to code points, stored in an object of `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input_view to read code units from and use in the DecodeOne operation that will
  /// produce code points.
  /// @result A stateless_decode_one_result object whose output is of type `TOutputContainer`.
  /// @remarks This function creates an `encoding` by using the `value_type` of the `input` which is then
  /// passed through the DefaultCodePointEncoding type to get the default desired encoding.
  template <typename TOutputContainer = void, typename TInput>
  constexpr auto DecodeOneTo(TInput &&input)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TCodeUnit = Krys::Ranges::range_value_type_t<TUInput>;
    if (std::is_constant_evaluated())
    {
      // Use literal encoding instead, if we meet the right criteria
      using TEncoding = default_consteval_code_unit_encoding_t<TCodeUnit>;
      TEncoding encoding {};
      return Krys::DecodeOneTo<TOutputContainer>(std::forward<TInput>(input), encoding);
    }
    else
    {
      using TEncoding = default_code_unit_encoding_t<TCodeUnit>;
      TEncoding encoding {};
      return Krys::DecodeOneTo<TOutputContainer>(std::forward<TInput>(input), encoding);
    }
  }

  /// @brief Converts one indivisible unit of information from the code units of the given `input` view
  /// through the encoding to code points, stored in an object of `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input_view to read code units from and use in the DecodeOne operation that will
  /// produce code points.
  /// @param[in] encoding The encoding that will be used to DecodeOne the input's code points into
  /// output code units.
  /// @param[in] errorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in,out] state A reference to the associated state for the `encoding` 's DecodeOne step.
  /// @result An object of type `TOutputContainer` .
  /// @remarks This function detects creates a container of type `TOutputContainer` and uses a typical @c
  /// std::back_inserter or `std::push_back_inserter` to fill in elements as it is written to.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding, typename TErrorHandler,
            typename TState>
  constexpr auto DecodeOne(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler, TState &state)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TUOutputContainer = remove_cvref_t<TOutputContainer>;
    using TOutputCodePoint = code_point_t<TUEncoding>;
    constexpr std::size_t maxUnits = MaxDecodeCodePoints<TUEncoding>;
    constexpr bool IsVoidContainer = Void<TUOutputContainer>;
    constexpr bool IsStringable = (IsCharTraitable<TOutputCodePoint> || IsUnicodeCodePoint<TOutputCodePoint>);

    if constexpr (IsVoidContainer && IsStringable)
    {
      using TRealOutputContainer = InlineBasicString<TOutputCodePoint, maxUnits>;
      return Impl::DecodeOneDispatch<true, false, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
    else
    {
      using TRealOutputContainer =
        conditional_t<IsVoidContainer, InlineVector<TOutputCodePoint, MaxCodePoints<TUEncoding>>,
                      TOutputContainer>;
      return Impl::DecodeOneDispatch<true, false, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding),
        std::forward<TErrorHandler>(errorHandler), state);
    }
  }

  /// @brief Converts one indivisible unit of information from the code units of the given `input` view
  /// through the encoding to code points, stored in an object of `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input_view to read code units from and use in the DecodeOne operation that will
  /// produce code points.
  /// @param[in] encoding The encoding that will be used to DecodeOne the input's code points into
  /// output code units.
  /// @param[in] errorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @result An object of type `TOutputContainer` .
  /// @remarks This function creates a `state` using CreateDecodeState.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding, typename TErrorHandler>
  constexpr auto DecodeOne(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TState = decode_state_t<TUEncoding>;

    TState state = Krys::CreateDecodeState(encoding);
    return Krys::DecodeOne<TOutputContainer>(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                             std::forward<TErrorHandler>(errorHandler), state);
  }

  /// @brief Converts one indivisible unit of information from the code units of the given `input` view
  /// through the encoding to code points, stored in an object of `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input_view to read code units from and use in the DecodeOne operation that will
  /// produce code points.
  /// @param[in] encoding The encoding that will be used to DecodeOne the input's code points into
  /// output code units.
  /// @result An object of type `TOutputContainer` .
  /// @remarks This function creates a `handler` using Handlers::DefaultHandler, but marks it as careless.
  template <typename TOutputContainer = void, typename TInput, typename TEncoding>
  constexpr auto DecodeOne(TInput &&input, TEncoding &&encoding)
  {
    Handlers::DefaultHandler handler {};
    return Krys::DecodeOne<TOutputContainer>(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                             handler);
  }

  /// @brief Converts one indivisible unit of information from the code units of the given `input` view
  /// through the encoding to code points, stored in an object of `TOutputContainer` type.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input_view to read code units from and use in the DecodeOne operation that will
  /// produce code points.
  /// @result An object of type `TOutputContainer` .
  /// @remarks This function creates an `encoding` by using the `value_type` of the `input` which is then
  /// passed through the DefaultCodePointEncoding type to get the default desired encoding.
  template <typename TOutputContainer = void, typename TInput>
  constexpr auto DecodeOne(TInput &&input)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TCodeUnit = Krys::Ranges::range_value_type_t<TUInput>;
    if (std::is_constant_evaluated())
    {
      // Use literal encoding instead, if we meet the right criteria
      using TEncoding = default_consteval_code_unit_encoding_t<TCodeUnit>;
      TEncoding encoding {};
      return Krys::DecodeOne<TOutputContainer>(std::forward<TInput>(input), encoding);
    }
    else
    {
      using TEncoding = default_code_unit_encoding_t<TCodeUnit>;
      TEncoding encoding {};
      return Krys::DecodeOne<TOutputContainer>(std::forward<TInput>(input), encoding);
    }
  }
}
