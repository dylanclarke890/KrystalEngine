#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Ranges/Reconstruct.hpp"
#include "Krystal.Lib/Utils/ReferenceWrapper.hpp"
#include "Krystal.Text/EncodingError.hpp"
#include <array>
#include <cstddef>
#include <system_error>
#include <utility>

namespace Krys
{
  /// @brief The result of validation operations that specifically do not include a reference to the state.
  template <typename TInput>
  class StatelessValidateResult
  {
  public:
    /// @brief The reconstructed input_view object, with its .begin() incremented by the number of code units
    /// successfully read (can be identical to .begin() on original range on failure).
    TInput Input;

    /// @brief Whether or not the specified input is valid or not.
    bool Valid;

    /// @brief Constructs a ValidateResult, defaulting the error code to EncodingError::OK if not provided.
    /// @param[in] input The input range to store.
    /// @param[in] isValid Whether or not the validation succeeded.
    template <typename TArgInput>
    constexpr StatelessValidateResult(TArgInput &&input, bool isValid)
        : Input(std::forward<TArgInput>(input)), Valid(isValid)
    {
    }

    /// @brief A conversion for use in if statements and conditional operators.
    /// @return Whether or not the result is valid or not.
    constexpr explicit operator bool() const noexcept
    {
      return Valid;
    }
  };

  /// @brief The result of validation operations.
  template <typename TInput, typename TState>
  class ValidateResult : public StatelessValidateResult<TInput>
  {
  private:
    using TBase = StatelessValidateResult<TInput>;

  public:
    /// @brief A reference to the state of the associated Encoding used for validating the input.
    ReferenceWrapper<TState> State;

    /// @brief Constructs a ValidateResult, defaulting the error code to EncodingError::OK if not provided.
    /// @param[in] input The input range to store.
    /// @param[in] isValid Whether or not the validation succeeded.
    /// @param[in] state The state related to the encoding that was used to do validation.
    template <typename TArgInput, typename TArgState>
    constexpr ValidateResult(TArgInput &&input, bool isValid, TArgState &&state)
        : TBase(std::forward<TArgInput>(input), isValid), State(std::forward<TArgState>(state))
    {
    }
  };

  /// @brief The result of a transcoding validation operations.
  template <typename TInput, typename TDecodeState, typename TEncodeState>
  class ValidatePivotlessTranscodeResult : public StatelessValidateResult<TInput>
  {
  private:
    using TBase = StatelessValidateResult<TInput>;

  public:
    /// @brief A reference to the state of the associated Encoding used for validating the input.
    ReferenceWrapper<TDecodeState> FromState;

    /// @brief A reference to the state of the associated Encoding used for validating the input.
    ReferenceWrapper<TEncodeState> ToState;

    /// @brief Constructs a ValidateResult, defaulting the error code to EncodingError::OK if not provided.
    /// @param[in] input The input range to store.
    /// @param[in] isValid Whether or not the validation succeeded.
    /// @param[in] fromState The state related to the encoding that was used to do validation.
    /// @param[in] toState The state related to the encoding that was used to do validation.
    template <typename TArgInput, typename TArgFromState, typename TArgToState>
    constexpr ValidatePivotlessTranscodeResult(TArgInput &&input, bool isValid, TArgFromState &&fromState,
                                               TArgToState &&toState)
        : TBase(std::forward<TArgInput>(input), isValid), FromState(std::forward<TArgFromState>(fromState)),
          ToState(std::forward<TArgToState>(toState))
    {
    }
  };

  /// @brief The result of a transcoding validation operations.
  template <typename TInput, typename TDecodeState, typename TEncodeState, typename TPivot>
  class ValidateTranscodeResult : public ValidatePivotlessTranscodeResult<TInput, TDecodeState, TEncodeState>
  {
  private:
    using TBase = ValidatePivotlessTranscodeResult<TInput, TDecodeState, TEncodeState>;

  public:
    /// @brief The range used to hold the intermediate pivot transcoding units.
    TPivot Pivot;

    /// @brief Constructs a pivotTValidate_result.
    /// @param[in] input The input range to store.
    /// @param[in] isValid Whether or not the validation succeeded.
    /// @param[in] fromState The state related to the encoding that was used to do validation.
    /// @param[in] toState The state related to the encoding that was used to do validation.
    /// @param[in] pivot The pivot range to store.
    template <typename TArgInput, typename TArgFromState, typename TArgToState, typename TArgPivot>
    constexpr ValidateTranscodeResult(TArgInput &&input, bool isValid, TArgFromState &&fromState,
                                      TArgToState &&toState, TArgPivot &&pivot)
        : TBase(std::forward<TArgInput>(input), isValid, std::forward<TArgFromState>(fromState),
                std::forward<TArgToState>(toState)),
          Pivot(std::forward<TPivot>(pivot))
    {
    }
  };

  namespace Impl
  {
    template <typename TInput, typename TState>
    constexpr StatelessValidateResult<TInput>
      SliceToStateless(ValidateResult<TInput, TState> &&result) noexcept(
        NoThrowConstructible<StatelessValidateResult<TInput>, ValidateResult<TInput, TState>>)
    {
      return result;
    }

    template <typename TInput, typename TDecodeState, typename TEncodeState>
    constexpr StatelessValidateResult<TInput> SliceToStateless(
      ValidatePivotlessTranscodeResult<TInput, TDecodeState, TEncodeState>
        &&result) noexcept(NoThrowConstructible<StatelessValidateResult<TInput>,
                                                ValidatePivotlessTranscodeResult<TInput, TDecodeState,
                                                                                 TEncodeState>>)
    {
      return result;
    }

    template <typename TInput, typename TDecodeState, typename TEncodeState>
    constexpr ValidateResult<TInput, TDecodeState>
      DropSingleState(ValidatePivotlessTranscodeResult<TInput, TDecodeState, TEncodeState>
                            &&result) noexcept(NoThrowConstructible<ValidateResult<TInput, TDecodeState>,
                                                                    TInput &&, bool &, TDecodeState &>)
    {
      return ValidateResult<TInput, TDecodeState>(std::move(result.Input), std::move(result.Valid),
                                                  result.FromState);
    }

    template <typename TInputRange, typename TState>
    using reconstructTValidate_result_t = ValidateResult<Krys::Ranges::range_reconstruct_t<TInputRange>, TState>;
  }
}
