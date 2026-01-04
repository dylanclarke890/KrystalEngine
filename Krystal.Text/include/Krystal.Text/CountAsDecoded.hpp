#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Utils/Tag.hpp"
#include "Krystal.Text/CharTraits.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/CountResult.hpp"
#include "Krystal.Text/DecodeOne.hpp"
#include "Krystal.Text/DefaultEncoding.hpp"
#include "Krystal.Text/Impl/EncodingRange.hpp"
#include "Krystal.Text/Impl/IsLossless.hpp"
#include "Krystal.Text/State.hpp"
#include <string_view>

namespace Krys
{
  /// @brief These functions use a variety of means to count the number of code units that will result from
  /// the input code points.
  /// @brief Counts the number of code units that will result from attempting a decode operation.
  /// @param[in] input The input range (of code units) to find out how many code points there are.
  /// @param[in] encoding The encoding to count the input with.
  /// @param[in] errorHandler The error handler to invoke when an encode operation fails.
  /// @param[in,out] state The state that will be used to count code units.
  /// @returns A CountResult that includes information about how many code units are present,
  /// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler) and a
  /// reference to the provided `state`.
  /// @remarks This method does not call ADL extension points. It attempts a combination of implementation
  /// techniques to count code units, with a loop over the `.decode` call into an intermediate, unseen buffer
  /// being the most basic guaranteed implementation attempt.
  template <typename TInput, typename TEncoding, typename TErrorHandler, typename TState>
  constexpr auto BasicCountAsDecoded(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler,
                                     TState &state)
  {
    using TWorkingInput = Krys::Ranges::subrange_for_t<TInput>;
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TResult = CountResult<TWorkingInput, TState>;

    TWorkingInput workingInput(std::forward<TInput>(input));

    std::size_t codePointCount = 0;

    if constexpr (Impl::HasTextCountAsDecodedOne<TWorkingInput, TEncoding, TErrorHandler, TState>)
    {
      std::size_t errorCount = 0;
      for (;;)
      {
        auto result = Krys::TextCountAsDecodedOne(Krys::Tag<TUEncoding> {}, std::move(workingInput), encoding,
                                                  errorHandler, state);
        errorCount += result.ErrorCount;
        if (result.ErrorCode != EncodingError::OK)
        {
          return TResult(std::move(result.Input), codePointCount, state, result.ErrorCode, result.ErrorCount);
        }
        codePointCount += result.Count;
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
      return TResult(std::move(workingInput), codePointCount, state, EncodingError::OK, errorCount);
    }
    else if constexpr (Impl::HasInternalTextCountAsDecodedOne<TWorkingInput, TEncoding, TErrorHandler,
                                                              TState>)
    {
      std::size_t errorCount = 0;
      for (;;)
      {
        auto result = Krys::InternalTextCountAsDecodedOne(Krys::Tag<TUEncoding> {}, std::move(workingInput),
                                                          encoding, errorHandler, state);
        errorCount += result.ErrorCount;
        if (result.ErrorCode != EncodingError::OK)
        {
          return TResult(std::move(result.Input), codePointCount, state, result.ErrorCode, errorCount);
        }
        codePointCount += result.Count;
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
      return TResult(std::move(workingInput), codePointCount, state, EncodingError::OK, errorCount);
    }
    else
    {
      using TCodePoint = code_point_t<TUEncoding>;

      TCodePoint intermediateStorage[MaxCodePoints<TUEncoding>] {};
      Span<TCodePoint, MaxCodePoints<TUEncoding>> intermediate(intermediateStorage);
      std::size_t errorCount = 0;
      for (;;)
      {
        auto result =
          Krys::DecodeOneIntoRaw(std::move(workingInput), encoding, intermediate, errorHandler, state);
        errorCount += result.ErrorCount;
        if (result.ErrorCode != EncodingError::OK)
        {
          return TResult(std::move(result.Input), codePointCount, state, result.ErrorCode, errorCount);
        }
        std::size_t usedSize = static_cast<std::size_t>(
          Krys::Ranges::distance(Krys::Ranges::begin(intermediate), Krys::Ranges::begin(result.Output)));
        codePointCount += usedSize;
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
      return TResult(std::move(workingInput), codePointCount, state, EncodingError::OK, errorCount);
    }
  }

  /// @brief Counts the number of code units that will result from attempting a decode operation.
  ///
  /// @param[in] input The input range (of code units) to find out how many code points there are.
  /// @param[in] encoding The encoding to count the input with.
  /// @param[in] errorHandler The error handler to invoke when an encode operation fails.
  /// @param[in,out] state The state that will be used to count code units.
  ///
  /// @returns A CountResult that includes information about how many code units are present,
  /// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler) and a
  /// reference to the provided `state`.
  ///
  /// @remarks This method will first check if an ADL Extension Point `TextCountAsDecoded` is callable with
  /// the given arguments. If it is, then that method will be used to do the work after forwarding all four
  /// arguments to that function call. Otherwise, it defers to BasicCountAsDecoded.
  template <typename TInput, typename TEncoding, typename TErrorHandler, typename TState>
  constexpr auto CountAsDecoded(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler,
                                TState &state)
  {
    if constexpr (Impl::HasTextCountAsDecoded<TInput, TEncoding, TErrorHandler, TState>)
    {
      return Krys::TextCountAsDecoded(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::forward<TInput>(input),
                                      std::forward<TEncoding>(encoding),
                                      std::forward<TErrorHandler>(errorHandler), state);
    }
    else if constexpr (Impl::HasInternalTextCountAsDecoded<TInput, TEncoding, TErrorHandler, TState>)
    {
      return Krys::InternalTextCountAsDecoded(Krys::Tag<remove_cvref_t<TEncoding>> {},
                                              std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                              std::forward<TErrorHandler>(errorHandler), state);
    }
    else
    {
      return Krys::BasicCountAsDecoded(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                       std::forward<TErrorHandler>(errorHandler), state);
    }
  }

  /// @brief Counts the number of code units that will result from attempting a decode operation.
  /// @param[in] input The input range (of code units) to find out how many code points there are.
  /// @param[in] encoding The encoding to count the input with.
  /// @param[in] errorHandler The error handler to invoke when an encode operation fails.
  /// @returns A StatelessCountResult that includes information about how many code units are present,
  /// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler).
  /// @remarks Calls CountAsDecoded(Input, Encoding, ErrorHandler, State) with an `state` that is
  /// created by CreateDecodeState(Encoding).
  template <typename TInput, typename TEncoding, typename TErrorHandler>
  constexpr auto CountAsDecoded(TInput &&input, TEncoding &&encoding, TErrorHandler &&errorHandler)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TState = decode_state_t<TUEncoding>;

    TState state = Krys::CreateDecodeState(encoding);
    auto statefulResult = Krys::CountAsDecoded(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                               std::forward<TErrorHandler>(errorHandler), state);
    return Impl::SliceToStateless(std::move(statefulResult));
  }

  /// @brief Counts the number of code units that will result from attempting a decode operation.
  ///
  /// @param[in] input The input range (of code units) to find out how many code points there are.
  /// @param[in] encoding The encoding to count the input with.
  ///
  /// @returns A StatelessCountResult that includes information about how many code units are present,
  /// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler).
  ///
  /// @remarks Calls CountAsDecoded(Input, Encoding, ErrorHandler) with an `ErrorHandler` that is
  /// similar to Handlers::DefaultHandler.
  template <typename TInput, typename TEncoding>
  constexpr auto CountAsDecoded(TInput &&input, TEncoding &&encoding)
  {
    Handlers::DefaultHandler handler {};
    return Krys::CountAsDecoded(std::forward<TInput>(input), std::forward<TEncoding>(encoding), handler);
  }

  /// @brief Counts the number of code units that will result from attempting a decode operation.
  /// @param[in] input The input range (of code units) to find out how many code points there are.
  /// @returns A StatelessCountResult that includes information about how many code units are present,
  /// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler).
  /// @remarks Calls CountAsDecoded(Input, Encoding) with an `encoding` that is derived from
  /// DefaultCodeUnitEncoding.
  template <typename TInput>
  constexpr auto CountAsDecoded(TInput &&input)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TCodeUnit = Krys::Ranges::range_value_type_t<TUInput>;
    if (std::is_constant_evaluated())
    {
      // Use literal encoding instead, if we meet the right criteria
      using TEncoding = default_consteval_code_unit_encoding_t<TCodeUnit>;
      TEncoding encoding {};
      return Krys::CountAsDecoded(std::forward<TInput>(input), encoding);
    }
    else
    {
      using TEncoding = default_code_unit_encoding_t<TCodeUnit>;
      TEncoding encoding {};
      return Krys::CountAsDecoded(std::forward<TInput>(input), encoding);
    }
  }
}
