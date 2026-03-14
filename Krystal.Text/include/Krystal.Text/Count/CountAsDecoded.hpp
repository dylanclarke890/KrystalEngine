#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Utils/Tag.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/Count/CountResult.hpp"
#include "Krystal.Text/Decode/DecodeOne.hpp"
#include "Krystal.Text/Encodings/DefaultEncoding.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/TypeTraits.hpp"
#include <string_view>

/// @brief These functions use a variety of means to count the number of code units that will result from
/// the input code points.
namespace Krys::Text
{
  /// @brief Counts the number of code units that will result from attempting a decode operation.
  /// @param[in] input The input range (of code units) to find out how many code points there are.
  /// @param[in] encoding The encoding to count the input with.
  /// @param[in] errorHandler The error handler to invoke when an encode operation fails.
  /// @param[in,out] state The state that will be used to count code units.
  /// @returns A CountResult that includes information about how many code units are present,
  /// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler) and a
  /// reference to the provided `state`.
  template <typename TInput, typename TEncoding, typename TErrorHandler, typename TState>
  constexpr auto BasicCountAsDecoded(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler,
                                     TState &state)
  {
    using TWorkingInput = ::Krys::Ranges::subrange_for_t<TInput>;
    using TResult = ::Krys::Text::CountResult<TWorkingInput, TState>;

    TWorkingInput workingInput(std::forward<TInput>(input));
    std::size_t codePointCount = 0;

    if constexpr (::Krys::Text::HasCountTextAsDecodedOne<TWorkingInput, TEncoding, TErrorHandler, TState>)
    {
      std::size_t errorCount = 0;
      for (;;)
      {
        auto result = CountTextAsDecodedOne(Krys::Tag<TEncoding> {}, std::move(workingInput), encoding,
                                            errorHandler, state);
        errorCount += result.ErrorCount;
        if (result.ErrorCode != EncodingError::OK)
        {
          return TResult(std::move(result.Input), codePointCount, state, result.ErrorCode, result.ErrorCount);
        }
        codePointCount += result.Count;
        workingInput = std::move(result.Input);
        if (::std::ranges::empty(workingInput))
        {
          if (!::Krys::Text::IsStateComplete(encoding, state))
          {
            continue;
          }
          break;
        }
      }
      return TResult(std::move(workingInput), codePointCount, state, EncodingError::OK, errorCount);
    }
    else
    {
      using TCodePoint = code_point_t<TEncoding>;

      TCodePoint intermediateStorage[::Krys::Text::MaxCodePoints<TEncoding>] {};
      Span<TCodePoint, ::Krys::Text::MaxCodePoints<TEncoding>> intermediate(intermediateStorage);
      std::size_t errorCount = 0;
      for (;;)
      {
        auto result = ::Krys::Text::DecodeOneIntoRaw(std::move(workingInput), encoding, intermediate,
                                                     errorHandler, state);
        errorCount += result.ErrorCount;
        if (result.ErrorCode != EncodingError::OK)
        {
          return TResult(std::move(result.Input), codePointCount, state, result.ErrorCode, errorCount);
        }
        std::size_t usedSize = static_cast<std::size_t>(
          ::std::distance(::std::ranges::begin(intermediate), ::std::ranges::begin(result.Output)));

        codePointCount += usedSize;
        workingInput = std::move(result.Input);

        if (::std::ranges::empty(workingInput))
        {
          if (!::Krys::Text::IsStateComplete(encoding, state))
          {
            continue;
          }
          break;
        }
      }
      return TResult(std::move(workingInput), codePointCount, state, EncodingError::OK, errorCount);
    }
  }

  /// @brief Counts the number of code units that will result from attempting a decode operation.
  /// @param[in] input The input range (of code units) to find out how many code points there are.
  /// @param[in] encoding The encoding to count the input with.
  /// @param[in] errorHandler The error handler to invoke when an encode operation fails.
  /// @param[in,out] state The state that will be used to count code units.
  /// @returns A CountResult that includes information about how many code units are present,
  /// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler) and a
  /// reference to the provided `state`.
  template <typename TInput, typename TEncoding, typename TErrorHandler, typename TState>
  constexpr auto CountAsDecoded(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler,
                                TState &state)
  {
    if constexpr (::Krys::Text::HasCountTextAsDecoded<TInput, TEncoding, TErrorHandler, TState>)
    {
      return CountTextAsDecoded(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::forward<TInput>(input),
                                std::forward<TEncoding>(encoding), std::forward<TErrorHandler>(errorHandler),
                                state);
    }
    else
    {
      return ::Krys::Text::BasicCountAsDecoded(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                               std::forward<TErrorHandler>(errorHandler), state);
    }
  }

  /// @brief Counts the number of code units that will result from attempting a decode operation.
  /// @param[in] input The input range (of code units) to find out how many code points there are.
  /// @param[in] encoding The encoding to count the input with.
  /// @param[in] errorHandler The error handler to invoke when an encode operation fails.
  /// @returns A StatelessCountResult that includes information about how many code units are present,
  /// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler).
  template <typename TInput, typename TEncoding, typename TErrorHandler>
  constexpr auto CountAsDecoded(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler)
  {
    auto state = ::Krys::Text::CreateDecodeState(encoding);
    auto statefulResult =
      ::Krys::Text::CountAsDecoded(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                   std::forward<TErrorHandler>(errorHandler), state);

    return ::Krys::Text::Impl::SliceToStateless(std::move(statefulResult));
  }

  /// @brief Counts the number of code units that will result from attempting a decode operation.
  /// @param[in] input The input range (of code units) to find out how many code points there are.
  /// @param[in] encoding The encoding to count the input with.
  /// @returns A StatelessCountResult that includes information about how many code units are present,
  /// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler).
  template <typename TInput, typename TEncoding>
  constexpr auto CountAsDecoded(TInput &&input, TEncoding &&encoding)
  {
    ::Krys::Text::Handlers::DefaultHandler handler {};
    return ::Krys::Text::CountAsDecoded(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                        handler);
  }

  /// @brief Counts the number of code units that will result from attempting a decode operation.
  /// @param[in] input The input range (of code units) to find out how many code points there are.
  /// @returns A StatelessCountResult that includes information about how many code units are present,
  /// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler).
  template <typename TInput>
  constexpr auto CountAsDecoded(TInput &&input)
  {
    using TCodeUnit = ::Krys::Ranges::range_value_type_t<remove_cvref_t<TInput>>;
    using TEncoding =
      conditional_t<std::is_constant_evaluated(), default_consteval_code_unit_encoding_t<TCodeUnit>,
                    default_code_unit_encoding_t<TCodeUnit>>;

    TEncoding encoding {};
    return ::Krys::Text::CountAsDecoded(std::forward<TInput>(input), encoding);
  }
}
