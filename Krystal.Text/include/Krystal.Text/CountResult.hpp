#pragma once

#include "Krystal.Lib/Ranges/Reconstruct.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Utils/ReferenceWrapper.hpp"
#include "Krystal.Text/EncodingError.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"
#include <array>
#include <cstddef>
#include <system_error>
#include <utility>

namespace Krys
{
  /// @brief The result of counting operations that specifically do not include a reference to the state.
  template <typename TInput>
  class StatelessCountResult
  {
  public:
    /// @brief The reconstructed input_view object, with its .begin() incremented by the number of code units
    /// successfully read (can be identical to .begin() on original range on failure).
    TInput Input;

    /// @brief The number of code units or code points counted successfully, so far.
    std::size_t Count;

    /// @brief The kind of error that occured, if any.
    EncodingError ErrorCode;

    /// @brief The number of times an error occurred in the processed input text.
    /// @remarks This number may be greater than one despite `ErrorCode` being EncodingError::OK, as
    /// some error handlers will encounter an error but "fix" the text (e.g. Handlers::SkipHandler or
    /// Handlers::ReplacementHandler).
    std::size_t ErrorCount;

    /// @brief Constructs a StatelessCountResult, defaulting the error code to
    /// EncodingError::OK if not provided.
    /// @param[in] input The input range to store.
    /// @param[in] count The number of code points or code units successfully counted.
    /// @param[in] errorCode The error code for the encode operation, taken as the first of either the encode
    /// or decode operation that failed.
    template <typename TArgInput>
    constexpr StatelessCountResult(TArgInput &&input, std::size_t count,
                                   EncodingError errorCode = EncodingError::OK)
        : StatelessCountResult(std::forward<TArgInput>(input), count, errorCode,
                               errorCode != EncodingError::OK)
    {
    }

    /// @brief Constructs a StatelessCountResult with the provided parameters and
    /// information, including whether or not an error was handled.
    /// @param[in] input The input range to store.
    /// @param[in] count The number of code points or code units successfully counted.
    /// @param[in] errorCode The error code for the encode operation, taken as the first of either the encode
    /// or decode operation that failed.
    /// @param[in] errorCount Whether or not an error was handled. Some error handlers are corrective (see
    /// Handlers::ReplacementHandler), and so the error code is not enough to determine if the handler was
    /// invoked. This allows the value to be provided directly when constructing this result type.
    template <typename TArgInput>
    constexpr StatelessCountResult(TArgInput &&input, std::size_t count, EncodingError errorCode,
                                   std::size_t errorCount)
        : Input(std::forward<TArgInput>(input)), Count(count), ErrorCode(errorCode), ErrorCount(errorCount)
    {
    }

    /// @brief Whether or not any errors were handled.
    /// @returns Simply checks whether `ErrorCount` is greater than 0.
    constexpr bool ErrorsWereHandled() const noexcept
    {
      return this->ErrorCount > 0;
    }
  };

  /// @brief The result of counting operations.
  template <typename TInput, typename TState>
  class CountResult : public StatelessCountResult<TInput>
  {
  private:
    using TBase = StatelessCountResult<TInput>;

  public:
    /// @brief A reference to the state of the associated Encoding used for counting.
    ReferenceWrapper<TState> State;

    /// @brief Constructs a CountResult, defaulting the error code to
    /// EncodingError::OK if not provided.
    /// @param[in] input The input range to store.
    /// @param[in] count The number of code points or code units successfully counted.
    /// @param[in] state The state related to the encoding for the counting operation.
    /// @param[in] errorCode The error code for the encode operation, taken as the first of either the encode
    /// or decode operation that failed.
    template <typename TArgInput, typename TArgState>
    constexpr CountResult(TArgInput &&input, std::size_t count, TArgState &&state,
                          EncodingError errorCode = EncodingError::OK)
        : CountResult(std::forward<TArgInput>(input), count, std::forward<TArgState>(state), errorCode,
                      errorCode != EncodingError::OK ? 1uz : 0uz)
    {
    }

    /// @brief Constructs a CountResult with the provided parameters and information,
    /// including whether or not an error was handled.
    /// @param[in] input The input range to store.
    /// @param[in] count The number of code points or code units successfully counted.
    /// @param[in] state The state related to the encode operation that counted the code units.
    /// @param[in] errorCode The error code for the encode operation, taken as the first of either the encode
    /// or decode operation that failed.
    /// @param[in] errorCount Whether or not an error was handled. Some error handlers are corrective (see
    /// Handlers::ReplacementHandler), and so the error code is not enough to determine if the handler was
    /// invoked. This allows the value to be provided directly when constructing this result type.
    template <typename TArgInput, typename TArgState>
    constexpr CountResult(TArgInput &&input, std::size_t count, TArgState &&state, EncodingError errorCode,
                          std::size_t errorCount)
        : TBase(std::forward<TArgInput>(input), count, errorCode, errorCount),
          State(std::forward<TArgState>(state))
    {
    }
  };

  /// @brief The result of counting operations.
  template <typename TInput, typename TFromState, typename TToState>
  class CountTranscodeResult : public StatelessCountResult<TInput>
  {
  private:
    using TBase = StatelessCountResult<TInput>;

  public:
    /// @brief A reference to the state of the associated Encoding used for counting which covers the decoding
    /// portion of the transcode operation.
    ReferenceWrapper<TFromState> FromState;

    /// @brief A reference to the state of the associated Encoding used for counting which covers the encoding
    /// portion of the transcode operation.
    ReferenceWrapper<TToState> ToState;

    /// @brief Constructs a CountResult, defaulting the error code to
    /// EncodingError::OK if not provided.
    /// @param[in] input The input range to store.
    /// @param[in] count The number of code points or code units successfully counted.
    /// @param[in] fromState The state related to the encoding for the decode portion of the transcode
    /// counting operation.
    /// @param[in] toState The state related to the encoding for the encode portion of the transcode counting
    /// operation.
    /// @param[in] errorCode The error code for the encode operation, taken as the first of either the encode
    /// or decode operation that failed.
    template <typename TArgInput, typename TArgFromState, typename TArgToState>
    constexpr CountTranscodeResult(TArgInput &&input, std::size_t count, TArgFromState &&fromState,
                                   TArgToState &&toState, EncodingError errorCode = EncodingError::OK)
        : CountTranscodeResult(std::forward<TArgInput>(input), count, std::forward<TArgFromState>(fromState),
                               std::forward<TArgToState>(toState), errorCode,
                               errorCode != EncodingError::OK ? 1uz : 0uz)
    {
    }

    /// @brief Constructs a CountResult with the provided parameters and information,
    /// including whether or not an error was handled.
    /// @param[in] input The input range to store.
    /// @param[in] count The number of code points or code units successfully counted.
    /// @param[in] fromState The state related to the encoding for the decode portion of the transcode
    /// counting operation.
    /// @param[in] toState The state related to the encoding for the encode portion of the transcode counting
    /// operation.
    /// @param[in] errorCode The error code for the encode operation, taken as the first of either the encode
    /// or decode operation that failed.
    /// @param[in] errorCount Whether or not an error was handled. Some error handlers are corrective (see
    /// Handlers::ReplacementHandler), and so the error code is not enough to determine if the handler was
    /// invoked. This allows the value to be provided directly when constructing this result type.
    template <typename TArgInput, typename TArgFromState, typename TArgToState>
    constexpr CountTranscodeResult(TArgInput &&input, std::size_t count, TArgFromState &&fromState,
                                   TArgToState &&toState, EncodingError errorCode, std::size_t errorCount)
        : TBase(std::forward<TArgInput>(input), count, errorCode, errorCount),
          FromState(std::forward<TArgFromState>(fromState)), ToState(std::forward<TArgToState>(toState))
    {
    }
  };

  namespace Impl
  {
    template <typename TInput, typename TState>
    constexpr StatelessCountResult<TInput> SliceToStateless(CountResult<TInput, TState> &&result)
    {
      return result;
    }

    template <typename TInput, typename TFromState, typename TToState>
    constexpr StatelessCountResult<TInput>
      SliceToStateless(CountTranscodeResult<TInput, TFromState, TToState> &&result)
    {
      return result;
    }

    template <typename TInputRange, typename TState>
    using reconstruct_count_result_t = CountResult<Krys::Ranges::reconstruct_t<TInputRange>, TState>;

    template <typename TInputRange, typename TState, typename TInFirst, typename TInLast, typename TArgState>
    constexpr decltype(auto) ReconstructStatelessCountResult(TInFirst &&inFirst, TInLast &&inLast,
                                                             std::size_t count, TArgState &&state,
                                                             EncodingError errorCode, std::size_t errorCount)
    {
      decltype(auto) inRange = Krys::Ranges::Reconstruct(
        std::in_place_type<TInputRange>, std::forward<TInFirst>(inFirst), std::forward<TInLast>(inLast));
      return CountResult<TInputRange, TState>(std::forward<decltype(inRange)>(inRange), count,
                                              std::forward<TArgState>(state), errorCode, errorCount);
    }

    template <typename TInputRange, typename TState, typename TInFirst, typename TInLast, typename TArgState>
    constexpr decltype(auto) ReconstructStatelessCountResult(TInFirst &&inFirst, TInLast &&inLast,
                                                             std::size_t count, TArgState &&state,
                                                             EncodingError errorCode = EncodingError::OK)
    {
      return ReconstructStatelessCountResult<TInputRange, TState>(std::forward<TInFirst>(inFirst),
                                                                  std::forward<TInLast>(inLast), count,
                                                                  std::forward<TArgState>(state), errorCode);
    }
  }
}
