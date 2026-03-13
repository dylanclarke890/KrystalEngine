#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/Subrange.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Utils/Tag.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/Count/CountResult.hpp"
#include "Krystal.Text/Encode/EncodeOne.hpp"
#include "Krystal.Text/Encodings/DefaultEncoding.hpp"
#include "Krystal.Text/Handlers/DefaultHandler.hpp"
#include "Krystal.Text/State.hpp"
#include <string_view>

/// @brief These functions use a variety of means to count the number of code points that will result from
/// the input code units.
namespace Krys::Text
{

  /// @brief Counts the number of code units that will result from attempting an encode operation on the input
  /// code points.
  /// @param[in] input The input range (of code points) to find out how many code units there are.
  /// @param[in] encoding The encoding to count the input with.
  /// @param[in] errorHandler The error handler to invoke when an encode operation fails.
  /// @param[in,out] state The state that will be used to count code units.
  /// @returns A CountResult that includes information about how many code units are present,
  /// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler) and a
  /// reference to the provided `state`.
  template <typename TInput, typename TEncoding, typename TErrorHandler, typename TState>
  constexpr auto BasicCountAsEncoded(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler,
                                     TState &state)
  {
    using TWorkingInput = ::Krys::Ranges::subrange_for_t<TInput>;
    using TResult = ::Krys::Text::CountResult<TWorkingInput, TState>;

    TWorkingInput workingInput(std::forward<TInput>(input));
    std::size_t codeUnitCount = 0;
    std::size_t errorCount = 0;

    if constexpr (::Krys::Text::HasCountTextAsEncodedOne<TEncoding, TWorkingInput, TErrorHandler, TState>)
    {
      for (;;)
      {
        auto result = CountTextAsEncodedOne(::Krys::Tag<TEncoding> {}, std::move(workingInput), encoding,
                                            errorHandler, state);
        errorCount += result.ErrorCount;
        if (result.ErrorCode != EncodingError::OK)
        {
          return TResult(std::move(result.Input), codeUnitCount, state, result.ErrorCode, errorCount);
        }

        codeUnitCount += result.Count;
        workingInput = std::move(result.Input);

        if (!::Krys::Text::IsStateComplete(encoding, state))
        {
          continue;
        }

        if (::Krys::Ranges::empty(workingInput))
        {
          break;
        }
      }
    }
    else
    {
      using TCodeUnit = code_unit_t<TEncoding>;

      TCodeUnit intermediateStorage[::Krys::Text::MaxCodeUnits<TEncoding>] {};
      Span<TCodeUnit, ::Krys::Text::MaxCodeUnits<TEncoding>> intermediate(intermediateStorage);
      for (;;)
      {
        auto result =
          Krys::Text::EncodeOneIntoRaw(std::move(workingInput), encoding, intermediate, errorHandler, state);
        errorCount += result.ErrorCount;

        if (result.ErrorCode != EncodingError::OK)
        {
          return TResult(std::move(result.Input), codeUnitCount, state, result.ErrorCode, errorCount);
        }

        std::size_t usedSize = static_cast<std::size_t>(
          ::std::distance(::std::ranges::begin(intermediate), ::std::ranges::begin(result.Output)));

        codeUnitCount += usedSize;
        workingInput = std::move(result.Input);

        if (::Krys::Ranges::empty(workingInput))
        {
          if (!::Krys::Text::IsStateComplete(encoding, state))
          {
            continue;
          }
          break;
        }
      }
    }
    return TResult(std::move(workingInput), codeUnitCount, state, EncodingError::OK, errorCount);
  }

  /// @brief Counts the number of code units that will result from attempting an encode operation on the input
  /// code points.
  /// @param[in] input The input range (of code points) to find out how many code units there are.
  /// @param[in] encoding The encoding to count the input with.
  /// @param[in] errorHandler The error handler to invoke when an encode operation fails.
  /// @param[in,out] state The state that will be used to count code units.
  /// @returns A CountResult that includes information about how many code units are present,
  /// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler) and a
  /// reference to the provided `state`.
  template <typename TInput, typename TEncoding, typename TErrorHandler, typename TState>
  constexpr auto CountAsEncoded(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler,
                                TState &state)
  {
    if constexpr (::Krys::Text::HasCountTextAsEncoded<TInput, TEncoding, TErrorHandler, TState>)
    {
      return CountTextAsEncoded(::Krys::Tag<remove_cvref_t<TEncoding>> {}, std::forward<TInput>(input),
                                std::forward<TEncoding>(encoding), std::forward<TErrorHandler>(errorHandler),
                                state);
    }
    else
    {
      return ::Krys::Text::BasicCountAsEncoded(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                               std::forward<TErrorHandler>(errorHandler), state);
    }
  }

  /// @brief Counts the number of code units that will result from attempting an encode operation on the input
  /// code points.
  /// @param[in] input The input range (of code points) to find out how many code units there are.
  /// @param[in] encoding The encoding to count the input with.
  /// @param[in] errorHandler The error handler to invoke when an encode operation fails.
  /// @returns A StatelessCountResult that includes information about how many code units are present,
  /// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler).
  template <typename TInput, typename TEncoding, typename TErrorHandler>
  constexpr auto CountAsEncoded(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler)
  {
    auto state = ::Krys::Text::CreateEncodeState(encoding);
    return ::Krys::Text::CountAsEncoded(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                        std::forward<TErrorHandler>(errorHandler), state);
  }

  /// @brief Counts the number of code units that will result from attempting an encode operation.
  /// @param[in] input The input range (of code points) to find out how many code units there are.
  /// @param[in] encoding The encoding to count the input with.
  /// @returns A StatelessCountResult that includes information about how many code units are present,
  /// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler).
  template <typename TInput, typename TEncoding>
  constexpr auto CountAsEncoded(TInput &&input, TEncoding &&encoding)
  {
    ::Krys::Text::Handlers::DefaultHandler handler {};
    return Krys::Text::CountAsEncoded(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                      handler);
  }

  /// @brief Counts the number of code units that will result from attempting an encode operation on the input
  /// code points.
  /// @param[in] input The input range (of code points) to find out how many code units there are.
  /// @returns A StatelessCountResult that includes information about how many code units are present,
  /// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler).
  template <typename TInput>
  constexpr auto CountAsEncoded(TInput &&input)
  {
    using TCodePoint = remove_cvref_t<::Krys::Ranges::range_value_type_t<remove_cvref_t<TInput>>>;
    using TEncoding =
      conditional_t<std::is_constant_evaluated(), default_consteval_code_point_encoding_t<TCodePoint>,
                    default_code_point_encoding_t<TCodePoint>>;

    TEncoding encoding {};
    return ::Krys::Text::CountAsEncoded(std::forward<TInput>(input), encoding);
  }
}
