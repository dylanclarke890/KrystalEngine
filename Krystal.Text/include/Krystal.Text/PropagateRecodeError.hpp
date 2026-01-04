#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Text/DecodeResult.hpp"
#include "Krystal.Text/EncodeResult.hpp"
#include "Krystal.Text/RecodeResult.hpp"

namespace Krys
{
  /// @brief Transcoding helper. Takes the given `fromEncoding`, `toEncoding`,
  /// `TEncodeErrorHandler`, and `TDecodeErrorHandler` and launders the EncodeResult through the
  /// `TEncodeErrorHandler`. The transformed encode result is then transformed to a
  /// DecodeResult before transforming that into the desired RecodeResult type.
  /// @tparam TResult The exact recode result type to use.
  /// @param[in] output The output view to be writing into.
  /// @param[in] fromEncoding The desired encoding that performs the decode portion of the transcoding
  /// step.
  /// @param[in] toEncoding The desired encoding that performs the decode portion of the transcoding
  /// step.
  /// @param[in] TEncodeResult The result value that has an error in it.
  /// @param[in] TEncodeErrorHandler The error handler to mill the `result` and
  /// other relevant information through.
  /// @param[in] TDecodeErrorHandler The error handler to mill the `TEncodeErrorHandler`'s  invoked
  /// result and other relevant information through.
  /// @param[in] toState The current state of the encoding step of the recode operation.
  /// @param[in] fromInputProgress Any unread input characters in any intermediate between the (failed)
  /// encode and decode operations.
  /// @param[in] fromOutputProgress Any unread intermediate output characters in any intermediates
  /// between the (failed) encode and decode operations.
  /// @param[in] toInputProgress Any unread intermediate input characters in any intermediate between
  /// the (failed) encode and decode operations.
  /// @param[in] toOutputProgress Any unread output characters in any intermediates between the (failed)
  /// encode and decode operations.
  /// @remarks This function is a helper whose sole purpose is to ensure that the other half of error
  /// handling is called by recode-style functions written by the end user (e.g., writing overriding hooks
  /// for recode). This function attempts to take care of any unread/unwritten characters and
  /// other minor points in its pursuit of properly making sure the error manifests on the other side.
  template <typename TResult, typename TOutput, typename TFromEncoding, typename TToEncoding,
            typename TEncodeErrorHandler, typename TDecodeErrorHandler, typename TToState,
            typename TFromInputProgress, typename TFromOutputProgress, typename TToInputProgress,
            typename TToOutputProgress, typename TResultInput, typename TIntermediate, typename TFromState>
  constexpr auto PropagateRecodeEncodeError(
    TOutput &&output, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
    EncodeResult<TResultInput, TIntermediate, TFromState> &&TEncodeResult,
    TEncodeErrorHandler &&TEncodeErrorHandler, TDecodeErrorHandler &&TDecodeErrorHandler, TToState &toState,
    TFromInputProgress &&fromInputProgress, TFromOutputProgress &&fromOutputProgress,
    TToInputProgress &&toInputProgress, TToOutputProgress &&toOutputProgress) noexcept
  {
    // first, run the encode error handler
    auto encodeErrorResult = std::forward<TEncodeErrorHandler>(TEncodeErrorHandler)(
      std::forward<TFromEncoding>(fromEncoding), std::move(TEncodeResult),
      std::forward<TFromInputProgress>(fromInputProgress),
      std::forward<TFromOutputProgress>(fromOutputProgress));
    // then, run the decode error handler
    DecodeResult<TIntermediate, remove_cvref_t<TOutput>, TToState> TDecodeResult(
      std::move(encodeErrorResult.Output), std::forward<TOutput>(output), toState,
      encodeErrorResult.ErrorCode, encodeErrorResult.ErrorCount);
    auto decodeErrorResult = std::forward<TDecodeErrorHandler>(TDecodeErrorHandler)(
      std::forward<TToEncoding>(toEncoding), std::move(TDecodeResult),
      std::forward<TToInputProgress>(toInputProgress), std::forward<TToOutputProgress>(toOutputProgress));
    // compose into the final result type
    return TResult(std::move(encodeErrorResult.Input), std::move(decodeErrorResult.Output),
                   encodeErrorResult.State, decodeErrorResult.State, decodeErrorResult.ErrorCode,
                   decodeErrorResult.ErrorCount, std::move(encodeErrorResult.Output),
                   encodeErrorResult.ErrorCode, encodeErrorResult.ErrorCount);
  }

  /// @brief Transcoding helper. Takes the given `fromEncoding`, `toEncoding`, `TEncodeErrorHandler`, and
  /// `TDecodeErrorHandler` and launders the EncodeResult through the `TEncodeErrorHandler`. The transformed
  /// encode result is then transformed to a DecodeResult before transforming that into the desired
  /// RecodeResult type.
  /// @tparam TResult The exact recode result type to use.
  ///
  /// @param[in] output The output view to be writing into.
  /// @param[in] fromEncoding The desired encoding that performs the decode portion of the transcoding
  /// step.
  /// @param[in] toEncoding The desired encoding that performs the decode portion of the transcoding
  /// step.
  /// @param[in] TEncodeResult The result value that has an error in it.
  /// @param[in] TEncodeErrorHandler The error handler to mill the `result` and
  /// other relevant information through.
  /// @param[in] TDecodeErrorHandler The error handler to mill the `TEncodeErrorHandler`'s  invoked
  /// result and other relevant information through.
  /// @param[in] toState The current state of the encoding step of the recode operation.
  /// @param[in] fromInputProgress Any unread input characters in any intermediate between the (failed)
  /// encode and decode operations.
  /// @param[in] fromOutputProgress Any unread intermediate output characters in any intermediates
  /// between the (failed) encode and decode operations.
  /// @param[in] toInputProgress Any unread intermediate input characters in any intermediate between
  /// the (failed) encode and decode operations.
  /// @param[in] toOutputProgress Any unread output characters in any intermediates between the (failed)
  /// encode and decode operations.
  ///
  /// @remarks This function is a helper whose sole purpose is to ensure that the other half of error
  /// handling is called by recode-style functions written by the end user (e.g., writing overriding hooks
  /// for recode). This function attempts to take care of any unread/unwritten characters and
  /// other minor points in its pursuit of properly making sure the error manifests on the other side.
  template <typename TOutput, typename TFromEncoding, typename TToEncoding, typename TEncodeErrorHandler,
            typename TDecodeErrorHandler, typename TToState, typename TFromInputProgress,
            typename TFromOutputProgress, typename TToInputProgress, typename TToOutputProgress,
            typename TResultInput, typename TResultIntermediate, typename TFromState>
  constexpr auto PropagateRecodeEncodeError(
    TOutput &&output, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
    EncodeResult<TResultInput, TResultIntermediate, TFromState> &&TEncodeResult,
    TEncodeErrorHandler &&TEncodeErrorHandler, TDecodeErrorHandler &&TDecodeErrorHandler, TToState &toState,
    TFromInputProgress &&fromInputProgress, TFromOutputProgress &&fromOutputProgress,
    TToInputProgress &&toInputProgress, TToOutputProgress &&toOutputProgress) noexcept
  {
    using TResult =
      RecodeResult<TResultInput, remove_cvref_t<TOutput>, TFromState, TToState, TResultIntermediate>;
    return Krys::PropagateRecodeEncodeError<TResult>(
      std::forward<TOutput>(output), std::move(TEncodeResult), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TEncodeErrorHandler>(TEncodeErrorHandler),
      std::forward<TDecodeErrorHandler>(TDecodeErrorHandler), toState,
      std::forward<TToInputProgress>(fromInputProgress), std::forward<TToOutputProgress>(fromOutputProgress),
      std::forward<TToInputProgress>(toInputProgress), std::forward<TToOutputProgress>(toOutputProgress));
  }

  /// @brief Transcoding helper. Takes the given `toEncoding` and `TDecodeErrorHandler` and launders
  /// the failed EncodeResult through it, producing a DecodeResult and transforming
  /// that into the desired RecodeResult type.
  /// @tparam TResult The exact recode result type to use.
  /// @param[in] toEncoding The desired encoding that performs the decode portion of the transcoding
  /// step.
  /// @param[in] TEncodeResult The encode result value that has an error in it.
  /// @param[in] TDecodeResult The decode result value that has not yet been processed by the decode error
  /// handler.
  /// @param[in] TDecodeErrorHandler The error handler to mill the `result` and other relevant
  /// information through.
  /// @param[in] toInputProgress Any unread output characters in any intermediate between the (failed)
  /// encode and decode operations.
  /// @param[in] toOutputProgress Any unread output characters in any intermediates between the (failed)
  /// encode and decode operations.
  /// @remarks This function is a helper whose sole purpose is to ensure that the other half of error
  /// handling is called by recode-style functions written by the end user (e.g., writing overriding hooks
  /// for recode). This function attempts to take care of any unread/unwritten characters and
  /// other minor points in its pursuit of properly making sure the error manifests on the other side.
  template <typename TResult, typename TToEncoding, typename TDecodeErrorHandler, typename TToInputProgress,
            typename TToOutputProgress, typename TResultInput, typename TResultPivot, typename TFromState,
            typename TResultIntermediate, typename TResultOutput, typename TToState>
  constexpr auto
    PropagateRecodeDecodeError(TToEncoding &&toEncoding,
                               EncodeResult<TResultInput, TResultPivot, TFromState> &&TEncodeResult,
                               DecodeResult<TResultIntermediate, TResultOutput, TToState> &&TDecodeResult,
                               TDecodeErrorHandler &&TDecodeErrorHandler, TToInputProgress &&toInputProgress,
                               TToOutputProgress &&toOutputProgress) noexcept
  {
    // just run the decode error handler only
    auto decodeErrorResult = std::forward<TDecodeErrorHandler>(TDecodeErrorHandler)(
      std::forward<TToEncoding>(toEncoding), std::move(TDecodeResult),
      std::forward<TToInputProgress>(toInputProgress), std::forward<TToOutputProgress>(toOutputProgress));
    return TResult(std::move(TEncodeResult.Input), std::move(decodeErrorResult.Output), TEncodeResult.State,
                   decodeErrorResult.State, decodeErrorResult.ErrorCode, decodeErrorResult.ErrorCount,
                   std::move(TEncodeResult.Output), TEncodeResult.ErrorCode, TEncodeResult.ErrorCount);
  }

  /// @brief Takes the given `toEncoding` and `TDecodeErrorHandler` and launders the failed
  /// EncodeResult through it, producing a DecodeResult and transforming that into
  /// the desired RecodeResult type.
  /// @param[in] toEncoding The desired encoding that performs the decode portion of the transcoding
  /// step.
  /// @param[in] TEncodeResult The result value that has an error on it.
  /// @param[in] TDecodeResult The result value that has an error on it.
  /// @param[in] TDecodeErrorHandler The error handler to mill the `result` and other relevant
  /// information through.
  /// @param[in] toInputProgress Any unread output characters in any intermediate between the (failed)
  /// encode and decode operations.
  /// @param[in] toOutputProgress Any unread output characters in any intermediates between the (failed)
  /// encode and decode operations.
  /// @remarks This function is a helper whose sole purpose is to ensure that the other half of error
  /// handling is called by recode-style functions written by the end user (e.g., writing overriding hooks
  /// for recode). This function attempts to take care of any unread/unwritten characters and
  /// other minor points in its pursuit of properly making sure the error manifests on the other side.
  /// Unlike it's counterpart, this function does not take an `TResult` template parameter and instead
  /// deduces the returned recode result type from inputs.
  template <typename TToEncoding, typename TDecodeErrorHandler, typename TToInputProgress,
            typename TToOutputProgress, typename TResultInput, typename TResultPivot, typename TFromState,
            typename TResultIntermediate, typename TResultOutput, typename TToState>
  constexpr auto
    PropagateRecodeDecodeError(TToEncoding &&toEncoding,
                               EncodeResult<TResultInput, TResultPivot, TFromState> &&TEncodeResult,
                               DecodeResult<TResultIntermediate, TResultOutput, TToState> &&TDecodeResult,
                               TDecodeErrorHandler &&TDecodeErrorHandler, TToInputProgress &&toInputProgress,
                               TToOutputProgress &&toOutputProgress) noexcept
  {
    using TResult = RecodeResult<TResultInput, TResultOutput, TFromState, TToState, TResultPivot>;
    return Krys::PropagateRecodeDecodeError<TResult>(
      std::forward<TToEncoding>(toEncoding), std::move(TEncodeResult), std::move(TDecodeResult),
      std::forward<TDecodeErrorHandler>(TDecodeErrorHandler), std::forward<TToInputProgress>(toInputProgress),
      std::forward<TToOutputProgress>(toOutputProgress));
  }

  /// @brief Transcoding helper. Takes the given `fromEncoding`, `toEncoding`, `TEncodeErrorHandler`, and
  /// `TDecodeErrorHandler` and launders the EncodeResult through the `TEncodeErrorHandler`. The transformed
  /// encode result is then transformed to a DecodeResult before transforming that into the desired
  /// RecodeResult type.
  /// @tparam TResult The exact recode result type to use.
  /// @param[in] input The input view to be reading from.
  /// @param[in] output The output view to be writing into.
  /// @param[in] fromEncoding The desired encoding that performs the decode portion of the transcoding
  /// step.
  /// @param[in] toEncoding The desired encoding that performs the decode portion of the transcoding
  /// step.
  /// @param[in] TEncodeResult The result value that has an error in it.
  /// @param[in] TEncodeErrorHandler The error handler to mill the `result` and
  /// other relevant information through.
  /// @param[in] TDecodeErrorHandler The error handler to mill the `TEncodeErrorHandler`'s  invoked
  /// result and other relevant information through.
  /// @param[in] toState The current state of the encoding step of the recode operation.
  /// @param[in] fromInputProgress Any unread input characters in any intermediate between the (failed)
  /// encode and decode operations.
  /// @param[in] fromOutputProgress Any unread intermediate output characters in any intermediates
  /// between the (failed) encode and decode operations.
  /// @param[in] toInputProgress Any unread intermediate input characters in any intermediate between
  /// the (failed) encode and decode operations.
  /// @param[in] toOutputProgress Any unread output characters in any intermediates between the (failed)
  /// encode and decode operations.
  /// @remarks This function is a helper whose sole purpose is to ensure that the other half of error
  /// handling is called by recode-style functions written by the end user (e.g., writing overriding hooks
  /// for recode). This function attempts to take care of any unread/unwritten characters and
  /// other minor points in its pursuit of properly making sure the error manifests on the other side.
  template <typename TResult, typename TInput, typename TOutput, typename TFromEncoding, typename TToEncoding,
            typename TEncodeErrorHandler, typename TDecodeErrorHandler, typename TToState,
            typename TFromInputProgress, typename TFromOutputProgress, typename TToInputProgress,
            typename TToOutputProgress, typename TResultInput, typename TIntermediate, typename TFromState>
  constexpr auto PropagateRecodeEncodeErrorWith(
    TInput &&input, TOutput &&output, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
    EncodeResult<TResultInput, TIntermediate, TFromState> &&TEncodeResult,
    TEncodeErrorHandler &&TEncodeErrorHandler, TDecodeErrorHandler &&TDecodeErrorHandler, TToState &toState,
    TFromInputProgress &&fromInputProgress, TFromOutputProgress &&fromOutputProgress,
    TToInputProgress &&toInputProgress, TToOutputProgress &&toOutputProgress) noexcept
  {
    // first, run the encode error handler
    auto encodeErrorResult = std::forward<TEncodeErrorHandler>(TEncodeErrorHandler)(
      std::forward<TFromEncoding>(fromEncoding), std::move(TEncodeResult),
      std::forward<TFromInputProgress>(fromInputProgress),
      std::forward<TFromOutputProgress>(fromOutputProgress));
    // then, run the decode error handler
    DecodeResult<TIntermediate, remove_cvref_t<TOutput>, TToState> TDecodeResult(
      std::move(encodeErrorResult.Output), std::forward<TOutput>(output), toState,
      encodeErrorResult.ErrorCode, encodeErrorResult.ErrorCount);
    auto decodeErrorResult = std::forward<TDecodeErrorHandler>(TDecodeErrorHandler)(
      std::forward<TToEncoding>(toEncoding), std::move(TDecodeResult),
      std::forward<TToInputProgress>(toInputProgress), std::forward<TToOutputProgress>(toOutputProgress));
    // compose into the final result type
    return TResult(std::forward<TInput>(input), std::move(decodeErrorResult.Output), encodeErrorResult.State,
                   decodeErrorResult.State, decodeErrorResult.ErrorCode, decodeErrorResult.ErrorCount,
                   std::move(encodeErrorResult.Output), encodeErrorResult.ErrorCode,
                   encodeErrorResult.ErrorCount);
  }

  /// @brief Transcoding helper. Takes the given `fromEncoding`, `toEncoding`, `TEncodeErrorHandler`, and
  /// `TDecodeErrorHandler` and launders the EncodeResult through the `TEncodeErrorHandler`. The transformed
  /// encode result is then transformed to a DecodeResult before transforming that into the desired
  /// RecodeResult type.
  /// @tparam TResult The exact recode result type to use.
  /// @param[in] input The input view to be reading from.
  /// @param[in] output The output view to be writing into.
  /// @param[in] fromEncoding The desired encoding that performs the decode portion of the transcoding
  /// step.
  /// @param[in] toEncoding The desired encoding that performs the decode portion of the transcoding
  /// step.
  /// @param[in] TEncodeResult The result value that has an error in it.
  /// @param[in] TEncodeErrorHandler The error handler to mill the `result` and
  /// other relevant information through.
  /// @param[in] TDecodeErrorHandler The error handler to mill the `TEncodeErrorHandler`'s  invoked
  /// result and other relevant information through.
  /// @param[in] toState The current state of the encoding step of the recode operation.
  /// @param[in] fromInputProgress Any unread input characters in any intermediate between the (failed)
  /// encode and decode operations.
  /// @param[in] fromOutputProgress Any unread intermediate output characters in any intermediates
  /// between the (failed) encode and decode operations.
  /// @param[in] toInputProgress Any unread intermediate input characters in any intermediate between
  /// the (failed) encode and decode operations.
  /// @param[in] toOutputProgress Any unread output characters in any intermediates between the (failed)
  /// encode and decode operations.
  /// @remarks This function is a helper whose sole purpose is to ensure that the other half of error
  /// handling is called by recode-style functions written by the end user (e.g., writing overriding hooks
  /// for recode). This function attempts to take care of any unread/unwritten characters and
  /// other minor points in its pursuit of properly making sure the error manifests on the other side.
  template <typename TInput, typename TOutput, typename TFromEncoding, typename TToEncoding,
            typename TEncodeErrorHandler, typename TDecodeErrorHandler, typename TToState,
            typename TFromInputProgress, typename TFromOutputProgress, typename TToInputProgress,
            typename TToOutputProgress, typename TResultInput, typename TResultIntermediate,
            typename TFromState>
  constexpr auto PropagateRecodeEncodeErrorWith(
    TInput &&input, TOutput &&output, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
    EncodeResult<TResultInput, TResultIntermediate, TFromState> &&TEncodeResult,
    TEncodeErrorHandler &&TEncodeErrorHandler, TDecodeErrorHandler &&TDecodeErrorHandler, TToState &toState,
    TFromInputProgress &&fromInputProgress, TFromOutputProgress &&fromOutputProgress,
    TToInputProgress &&toInputProgress, TToOutputProgress &&toOutputProgress) noexcept
  {
    using TResult =
      RecodeResult<TResultInput, remove_cvref_t<TOutput>, TFromState, TToState, TResultIntermediate>;
    return Krys::PropagateRecodeEncodeErrorWith<TResult>(
      std::forward<TInput>(input), std::forward<TOutput>(output), std::move(TEncodeResult),
      std::forward<TFromEncoding>(fromEncoding), std::forward<TToEncoding>(toEncoding),
      std::forward<TEncodeErrorHandler>(TEncodeErrorHandler),
      std::forward<TDecodeErrorHandler>(TDecodeErrorHandler), toState,
      std::forward<TToInputProgress>(fromInputProgress), std::forward<TToOutputProgress>(fromOutputProgress),
      std::forward<TToInputProgress>(toInputProgress), std::forward<TToOutputProgress>(toOutputProgress));
  }

  /// @brief Transcoding helper. Takes the given `toEncoding` and `TDecodeErrorHandler` and launders
  /// the failed EncodeResult through it, producing a DecodeResult and transforming
  /// that into the desired RecodeResult type.
  /// @tparam TResult The exact recode result type to use.
  /// @param[in] input The input view to be read from.
  /// @param[in] toEncoding The desired encoding that performs the decode portion of the transcoding
  /// step.
  /// @param[in] TEncodeResult The result value that has an error on it.
  /// @param[in] TDecodeResult The result value that has an error on it.
  /// @param[in] TDecodeErrorHandler The error handler to mill the `result` and other relevant
  /// information through.
  /// @param[in] toInputProgress Any unread output characters in any intermediate between the (failed)
  /// encode and decode operations.
  /// @param[in] toOutputProgress Any unread output characters in any intermediates between the (failed)
  /// encode and decode operations.
  /// @remarks This function is a helper whose sole purpose is to ensure that the other half of error
  /// handling is called by recode-style functions written by the end user (e.g., writing overriding hooks
  /// for recode). This function attempts to take care of any unread/unwritten characters and
  /// other minor points in its pursuit of properly making sure the error manifests on the other side.
  template <typename TResult, typename TInput, typename TToEncoding, typename TDecodeErrorHandler,
            typename TToInputProgress, typename TToOutputProgress, typename TResultInput,
            typename TResultPivot, typename TFromState, typename TResultIntermediate, typename TResultOutput,
            typename TToState>
  constexpr auto
    PropagateRecodeDecodeErrorWith(TInput &&input, TToEncoding &&toEncoding,
                                   EncodeResult<TResultInput, TResultPivot, TFromState> &&TEncodeResult,
                                   DecodeResult<TResultIntermediate, TResultOutput, TToState> &&TDecodeResult,
                                   TDecodeErrorHandler &&TDecodeErrorHandler,
                                   TToInputProgress &&toInputProgress,
                                   TToOutputProgress &&toOutputProgress) noexcept
  {
    // just run the decode error handler only
    auto decodeErrorResult = std::forward<TDecodeErrorHandler>(TDecodeErrorHandler)(
      std::forward<TToEncoding>(toEncoding), std::move(TDecodeResult),
      std::forward<TToInputProgress>(toInputProgress), std::forward<TToOutputProgress>(toOutputProgress));
    return TResult(std::forward<TInput>(input), std::move(decodeErrorResult.Output), TEncodeResult.State,
                   decodeErrorResult.State, decodeErrorResult.ErrorCode, decodeErrorResult.ErrorCount,
                   std::move(TEncodeResult.Output), TEncodeResult.ErrorCode, TEncodeResult.ErrorCount);
  }

  /// @brief Takes the given `toEncoding` and `TDecodeErrorHandler` and launders the failed
  /// EncodeResult through it, producing a DecodeResult and transforming that into
  /// the desired RecodeResult type.
  /// @param[in] input The input view to be read from.
  /// @param[in] toEncoding The desired encoding that performs the decode portion of the transcoding
  /// step.
  /// @param[in] TEncodeResult The result value that has an error on it.
  /// @param[in] TDecodeResult The result value that has an error on it.
  /// @param[in] TDecodeErrorHandler The error handler to mill the `result` and other relevant
  /// information through.
  /// @param[in] toInputProgress Any unread output characters in any intermediate between the (failed)
  /// encode and decode operations.
  /// @param[in] toOutputProgress Any unread output characters in any intermediates between the (failed)
  /// encode and decode operations.
  /// @remarks This function is a helper whose sole purpose is to ensure that the other half of error
  /// handling is called by recode-style functions written by the end user (e.g., writing overriding hooks
  /// for recode). This function attempts to take care of any unread/unwritten characters and
  /// other minor points in its pursuit of properly making sure the error manifests on the other side.
  /// Unlike it's counterpart, this function does not take an `TResult` template parameter and instead
  /// deduces the returned recode result type from inputs.
  template <typename TInput, typename TToEncoding, typename TDecodeErrorHandler, typename TToInputProgress,
            typename TToOutputProgress, typename TResultInput, typename TResultPivot, typename TFromState,
            typename TResultIntermediate, typename TResultOutput, typename TToState>
  constexpr auto
    PropagateRecodeDecodeErrorWith(TInput &&input, TToEncoding &&toEncoding,
                                   EncodeResult<TResultInput, TResultPivot, TFromState> &&TEncodeResult,
                                   DecodeResult<TResultIntermediate, TResultOutput, TToState> &&TDecodeResult,
                                   TDecodeErrorHandler &&TDecodeErrorHandler,
                                   TToInputProgress &&toInputProgress,
                                   TToOutputProgress &&toOutputProgress) noexcept
  {
    using TResult = RecodeResult<TResultInput, TResultOutput, TFromState, TToState, TResultPivot>;
    return Krys::PropagateRecodeDecodeErrorWith<TResult>(
      std::forward<TInput>(input), std::forward<TToEncoding>(toEncoding), std::move(TEncodeResult),
      std::move(TDecodeResult), std::forward<TDecodeErrorHandler>(TDecodeErrorHandler),
      std::forward<TToInputProgress>(toInputProgress), std::forward<TToOutputProgress>(toOutputProgress));
  }
}
