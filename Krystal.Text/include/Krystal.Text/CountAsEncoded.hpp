#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/Subrange.hpp"
#include "Krystal.Lib/Ranges/Unbounded.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Utils/Tag.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CountResult.hpp"
#include "Krystal.Text/DefaultEncoding.hpp"
#include "Krystal.Text/ErrorHandlers/DefaultHandler.hpp"
#include "Krystal.Text/Impl/EncodingRange.hpp"
#include "Krystal.Text/Impl/IsLossless.hpp"
#include "Krystal.Text/State.hpp"
#include <string_view>

namespace Krys
{
  /// @brief These functions use a variety of means to count the number of code points that will result from
  /// the input code units.

  /// @brief Counts the number of code units that will result from attempting an encode operation on the input
  /// code points.
  /// @param[in] input The input range (of code points) to find out how many code units there are.
  /// @param[in] encoding The encoding to count the input with.
  /// @param[in] errorHandler The error handler to invoke when an encode operation fails.
  /// @param[in,out] state The state that will be used to count code units.
  /// @returns A CountResult that includes information about how many code units are present,
  /// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler) and a
  /// reference to the provided `state`.
  /// @remarks This method will not check any ADL extension points. A combination of implementation techniques
  /// will be used to count code units, with a loop over the `.encode` call into an intermediate, unseen
  /// buffer being the most basic choice.
  template <typename TInput, typename TEncoding, typename TErrorHandler, typename TState>
  constexpr auto BasicCountAsDecoded(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler,
                                     TState &state)
  {
    using TWorkingInput = Krys::Ranges::subrange_for_t<TInput>;
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TResult = CountResult<TWorkingInput, TState>;

    TWorkingInput workingInput(std::forward<TInput>(input));

    std::size_t codeUnitCount = 0;
    std::size_t errorCount = 0;

    if constexpr (Impl::HasTextCountAsEncodedOne<TEncoding, TWorkingInput, TErrorHandler, TState>)
    {
      for (;;)
      {
        auto result = Krys::TextCountAsEncodedOne(Krys::Tag<TUEncoding> {}, std::move(workingInput), encoding,
                                                  errorHandler, state);
        errorCount += result.ErrorCount;
        if (result.ErrorCode != EncodingError::OK)
        {
          return TResult(std::move(result.Input), codeUnitCount, state, result.ErrorCode, errorCount);
        }
        codeUnitCount += result.Count;
        workingInput = std::move(result.Input);
        if (!Krys::IsStateComplete(encoding, state))
        {
          continue;
        }
        if (Krys::Ranges::empty(workingInput))
        {
          break;
        }
      }
    }
    else if constexpr (Impl::HasInternalTextCountAsEncodedOne<TEncoding, TWorkingInput, TErrorHandler,
                                                              TState>)
    {
      for (;;)
      {
        auto result = Krys::InternalTextCountAsEncodedOne(Krys::Tag<TUEncoding> {}, std::move(workingInput),
                                                          encoding, errorHandler, state);
        errorCount += result.ErrorCount;
        if (result.ErrorCode != EncodingError::OK)
        {
          return TResult(std::move(result.Input), codeUnitCount, state, result.ErrorCode, errorCount);
        }
        codeUnitCount += result.Count;
        workingInput = std::move(result.Input);
        if (!Krys::IsStateComplete(encoding, state))
        {
          continue;
        }
        if (Krys::Ranges::empty(workingInput))
        {
          break;
        }
      }
    }
    else
    {
      using TCodeUnit = code_unit_t<TUEncoding>;

      TCodeUnit intermediateStorage[MaxCodeUnits<TUEncoding>] {};
      Span<TCodeUnit, MaxCodeUnits<TUEncoding>> intermediate(intermediateStorage);
      for (;;)
      {
        auto result =
          Krys::EncodeOneIntoRaw(std::move(workingInput), encoding, intermediate, errorHandler, state);
        errorCount += result.ErrorCount;
        if (result.ErrorCode != EncodingError::OK)
        {
          return TResult(std::move(result.Input), codeUnitCount, state, result.ErrorCode, errorCount);
        }
        std::size_t usedSize = static_cast<std::size_t>(
          Krys::Ranges::distance(Krys::Ranges::begin(intermediate), Krys::Ranges::begin(result.Output)));
        codeUnitCount += usedSize;
        workingInput = std::move(result.Input);
        if (Krys::Ranges::empty(workingInput))
        {
          if (!Krys::IsStateComplete(encoding, state))
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
  /// @remarks This method will first check if an ADL Extension Point `TextCountAsEncoded` is callable with
  /// the given arguments. If it is, then that method will be used to do the work after forwarding all four
  /// arguments to that function call. Otherwise, this defers to BasicCountAsDecoded.
  template <typename TInput, typename TEncoding, typename TErrorHandler, typename TState>
  constexpr auto CountAsDecoded(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler,
                                TState &state)
  {
    if constexpr (Impl::HasTextCountAsEncoded<TInput, TEncoding, TErrorHandler, TState>)
    {
      return Krys::TextCountAsEncoded(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::forward<TInput>(input),
                                      std::forward<TEncoding>(encoding),
                                      std::forward<TErrorHandler>(errorHandler), state);
    }
    else if constexpr (Impl::HasInternalTextCountAsEncoded<TInput, TEncoding, TErrorHandler, TState>)
    {
      return Krys::InternalTextCountAsEncoded(Krys::Tag<remove_cvref_t<TEncoding>> {},
                                              std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                              std::forward<TErrorHandler>(errorHandler), state);
    }
    else
    {
      return Krys::BasicCountAsDecoded(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
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
  /// @remarks This method will call CountAsDecoded(Input, Encoding, ErrorHandler, State) with an
  /// `state` created by Krys::CreateEncodeState(Encoding).
  template <typename TInput, typename TEncoding, typename TErrorHandler>
  constexpr auto CountAsDecoded(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TState = encode_state_t<TUEncoding>;

    TState state = Krys::CreateEncodeState(encoding);

    return Krys::CountAsDecoded(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                std::forward<TErrorHandler>(errorHandler), state);
  }

  /// @brief Counts the number of code units that will result from attempting an encode operation.
  /// @param[in] input The input range (of code points) to find out how many code units there are.
  /// @param[in] encoding The encoding to count the input with.
  /// @returns A StatelessCountResult that includes information about how many code units are present,
  /// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler).
  /// @remarks This method will call CountAsDecoded(Input, Encoding, ErrorHandler) by creating an
  /// `ErrorHandler` similar to Handlers::DefaultHandler.
  template <typename TInput, typename TEncoding>
  constexpr auto CountAsDecoded(TInput &&input, TEncoding &&encoding)
  {
    Handlers::DefaultHandler handler {};
    return Krys::CountAsDecoded(std::forward<TInput>(input), std::forward<TEncoding>(encoding), handler);
  }

  /// @brief Counts the number of code units that will result from attempting an encode operation on the input
  /// code points.
  ///
  /// @param[in] input The input range (of code points) to find out how many code units there are.
  ///
  /// @returns A StatelessCountResult that includes information about how many code units are present,
  /// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler).
  ///
  /// @remarks Calls CountAsDecoded(Input, Encoding) with an `encoding` that is derived from
  /// DefaultCodeUnitEncoding.
  template <typename TInput>
  constexpr auto CountAsDecoded(TInput &&input)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TCodePoint = remove_cvref_t<Krys::Ranges::range_value_type_t<TUInput>>;
    if (std::is_constant_evaluated())
    {
      // Use literal encoding instead, if we meet the right criteria
      using TEncoding = default_consteval_code_point_encoding_t<TCodePoint>;
      TEncoding encoding {};
      return Krys::Krys::CountAsDecoded(std::forward<TInput>(input), encoding);
    }
    else
    {
      using TEncoding = default_consteval_code_point_encoding_t<TCodePoint>;
      TEncoding encoding {};
      return Krys::CountAsDecoded(std::forward<TInput>(input), encoding);
    }
  }
}
