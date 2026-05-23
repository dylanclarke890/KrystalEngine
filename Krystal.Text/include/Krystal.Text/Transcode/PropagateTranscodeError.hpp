#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Text/Decode/DecodeResult.hpp"
#include "Krystal.Text/Encode/EncodeResult.hpp"
#include "Krystal.Text/Transcode/TranscodeResult.hpp"

namespace Krys::Text
{
  /// @brief Transcoding helper. Takes the given `fromEncoding`, `toEncoding`,
  /// `decodeErrorHandler`, and `encodeErrorHandler` and launders the DecodeResult through the
  /// `decodeErrorHandler`. The transformed decode result is then transformed to a EncodeResult before
  /// transforming that into the desired TranscodeResult type.
  /// @tparam TResult The exact transcode result type to use.
  /// @param[in] output The output view to be writing into.
  /// @param[in] fromEncoding The desired encoding that performs the encode portion of the transcoding
  /// step.
  /// @param[in] toEncoding The desired encoding that performs the encode portion of the transcoding
  /// step.
  /// @param[in] decodeResult The result value that has an error in it.
  /// @param[in] decodeErrorHandler The error handler to mill the `result` and
  /// other relevant information through.
  /// @param[in] encodeErrorHandler The error handler to mill the `decodeErrorHandler`'s  invoked
  /// result and other relevant information through.
  /// @param[in] toState The current state of the encoding step of the transcode operation.
  /// @param[in] fromInputProgress Any unread input characters in any intermediate between the (failed)
  /// encode and decode operations.
  /// @param[in] fromOutputProgress Any unread intermediate output characters in any intermediates
  /// between the (failed) encode and decode operations.
  /// @param[in] toInputProgress Any unread intermediate input characters in any intermediate between
  /// the (failed) decode and encode operations.
  /// @param[in] toOutputProgress Any unread output characters in any intermediates between the (failed)
  /// decode and encode operations.
  /// @remarks This function is a helper whose sole purpose is to ensure that the other half of error
  /// handling is called by transcode-style functions written by the end user (e.g., writing overriding
  /// hooks for transcode). This function attempts to take care of any unread/unwritten
  /// characters and other minor points in its pursuit of properly making sure the error manifests on the
  /// other side.
  template <typename TResult, typename TOutput, typename TFromEncoding, typename TToEncoding,
            typename TDecodeErrorHandler, typename TEncodeErrorHandler, typename TToState,
            typename TFromInputProgress, typename TFromOutputProgress, typename TToInputProgress,
            typename TToOutputProgress, typename TResultInput, typename TIntermediate, typename TFromState>
  constexpr auto PropagateTranscodeDecodeError(
    TOutput &&output, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
    DecodeResult<TResultInput, TIntermediate, TFromState> &&decodeResult,
    TDecodeErrorHandler &&decodeErrorHandler, TEncodeErrorHandler &&encodeErrorHandler, TToState &toState,
    TFromInputProgress &&fromInputProgress, TFromOutputProgress &&fromOutputProgress,
    TToInputProgress &&toInputProgress, TToOutputProgress &&toOutputProgress) noexcept
  {
    using TEncodeOutput = remove_cvref_t<decltype(std::declval<TResult>().Output)>;
    // first, run the decode error handler
    auto decodeErrorResult = std::forward<TDecodeErrorHandler>(decodeErrorHandler)(
      std::forward<TFromEncoding>(fromEncoding), std::move(decodeResult),
      std::forward<TFromInputProgress>(fromInputProgress),
      std::forward<TFromOutputProgress>(fromOutputProgress));
    // then, run the encode error handler
    EncodeResult<TIntermediate, TEncodeOutput, TToState> encodeResult(
      std::move(decodeErrorResult.Output), std::forward<TOutput>(output), toState,
      decodeErrorResult.ErrorCode, decodeErrorResult.ErrorCount);
    auto encodeErrorResult = std::forward<TEncodeErrorHandler>(encodeErrorHandler)(
      std::forward<TToEncoding>(toEncoding), std::move(encodeResult),
      std::forward<TToInputProgress>(toInputProgress), std::forward<TToOutputProgress>(toOutputProgress));
    // compose into the final result type
    return TResult(std::move(decodeErrorResult.Input), std::move(encodeErrorResult.Output),
                   decodeErrorResult.State, encodeErrorResult.State, encodeErrorResult.ErrorCode,
                   encodeErrorResult.ErrorCount, std::move(decodeErrorResult.Output),
                   decodeErrorResult.ErrorCode, decodeErrorResult.ErrorCount);
  }

  /// @brief Transcoding helper. Takes the given `fromEncoding`, `toEncoding`,
  /// `decodeErrorHandler`, and `encodeErrorHandler` and launders the DecodeResult through the
  /// `decodeErrorHandler`. The transformed decode result is then transformed to a
  /// EncodeResult before transforming that into the desired TranscodeResult type.
  /// @tparam TResult The exact transcode result type to use.
  /// @param[in] output The output view to be writing into.
  /// @param[in] fromEncoding The desired encoding that performs the encode portion of the transcoding
  /// step.
  /// @param[in] toEncoding The desired encoding that performs the encode portion of the transcoding
  /// step.
  /// @param[in] decodeResult The result value that has an error in it.
  /// @param[in] decodeErrorHandler The error handler to mill the `result` and
  /// other relevant information through.
  /// @param[in] encodeErrorHandler The error handler to mill the `decodeErrorHandler`'s  invoked
  /// result and other relevant information through.
  /// @param[in] toState The current state of the encoding step of the transcode operation.
  /// @param[in] fromInputProgress Any unread input characters in any intermediate between the (failed)
  /// encode and decode operations.
  /// @param[in] fromOutputProgress Any unread intermediate output characters in any intermediates
  /// between the (failed) encode and decode operations.
  /// @param[in] toInputProgress Any unread output characters in any intermediate between the (failed)
  /// decode and encode operations.
  /// @param[in] toOutputProgress Any unread output characters in any intermediates between the (failed)
  /// decode and encode operations.
  /// @remarks This function is a helper whose sole purpose is to ensure that the other half of error
  /// handling is called by transcode-style functions written by the end user (e.g., writing overriding
  /// hooks for transcode). This function attempts to take care of any unread/unwritten
  /// characters and other minor points in its pursuit of properly making sure the error manifests on the
  /// other side.
  template <typename TOutput, typename TFromEncoding, typename TToEncoding, typename TDecodeErrorHandler,
            typename TEncodeErrorHandler, typename TToState, typename TFromInputProgress,
            typename TFromOutputProgress, typename TToInputProgress, typename TToOutputProgress,
            typename TResultInput, typename TResultIntermediate, typename TFromState>
  constexpr auto PropagateTranscodeDecodeError(
    TOutput &&output, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
    DecodeResult<TResultInput, TResultIntermediate, TFromState> &&decodeResult,
    TDecodeErrorHandler &&decodeErrorHandler, TEncodeErrorHandler &&encodeErrorHandler, TToState &toState,
    TFromInputProgress &&fromInputProgress, TFromOutputProgress &&fromOutputProgress,
    TToInputProgress &&toInputProgress, TToOutputProgress &&toOutputProgress) noexcept
  {
    using TResult = ::Krys::Text::TranscodeResult<TResultInput, remove_cvref_t<TOutput>, TFromState, TToState,
                                                  TResultIntermediate>;
    return ::Krys::Text::PropagateTranscodeDecodeError<TResult>(
      std::forward<TOutput>(output), std::move(decodeResult), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TDecodeErrorHandler>(decodeErrorHandler),
      std::forward<TEncodeErrorHandler>(encodeErrorHandler), toState,
      std::forward<TToInputProgress>(fromInputProgress), std::forward<TToOutputProgress>(fromOutputProgress),
      std::forward<TToInputProgress>(toInputProgress), std::forward<TToOutputProgress>(toOutputProgress));
  }

  /// @brief Transcoding helper. Takes the given `toEncoding` and `encodeErrorHandler` and launders
  /// the failed DecodeResult through it, producing a EncodeResult and transforming
  /// that into the desired TranscodeResult type.
  /// @tparam TResult The exact transcode result type to use.
  /// @param[in] toEncoding The desired encoding that performs the encode portion of the transcoding
  /// step.
  /// @param[in] decodeResult The result value that has an error on it.
  /// @param[in] encodeResult The result value that has an error on it.
  /// @param[in] encodeErrorHandler The error handler to mill the `result` and other relevant
  /// information through.
  /// @param[in] toInputProgress Any unread output characters in any intermediate between the (failed)
  /// decode and encode operations.
  /// @param[in] toOutputProgress Any unread output characters in any intermediates between the (failed)
  /// decode and encode operations.
  /// @remarks This function is a helper whose sole purpose is to ensure that the other half of error
  /// handling is called by transcode-style functions written by the end user (e.g., writing overriding
  /// hooks for transcode). This function attempts to take care of any unread/unwritten
  /// characters and other minor points in its pursuit of properly making sure the error manifests on the
  /// other side.
  template <typename TResult, typename TToEncoding, typename TEncodeErrorHandler, typename TToInputProgress,
            typename TToOutputProgress, typename TResultInput, typename TResultPivot, typename TFromState,
            typename TResultIntermediate, typename TResultOutput, typename TToState>
  constexpr auto PropagateTranscodeEncodeError(
    TToEncoding &&toEncoding, DecodeResult<TResultInput, TResultPivot, TFromState> &&decodeResult,
    EncodeResult<TResultIntermediate, TResultOutput, TToState> &&encodeResult,
    TEncodeErrorHandler &&encodeErrorHandler, TToInputProgress &&toInputProgress,
    TToOutputProgress &&toOutputProgress) noexcept
  {
    // just run the encode error handler only
    auto encodeErrorResult = std::forward<TEncodeErrorHandler>(encodeErrorHandler)(
      std::forward<TToEncoding>(toEncoding), std::move(encodeResult),
      std::forward<TToInputProgress>(toInputProgress), std::forward<TToOutputProgress>(toOutputProgress));
    return TResult(std::move(decodeResult.Input), std::move(encodeErrorResult.Output), decodeResult.State,
                   encodeErrorResult.State, encodeErrorResult.ErrorCode, encodeErrorResult.ErrorCount,
                   std::move(decodeResult.Output), decodeResult.ErrorCode, decodeResult.ErrorCount);
  }

  /// @brief Takes the given `toEncoding` and `encodeErrorHandler` and launders the failed
  /// DecodeResult through it, producing a EncodeResult and transforming that into
  /// the desired TranscodeResult type.
  /// @param[in] toEncoding The desired encoding that performs the encode portion of the transcoding
  /// step.
  /// @param[in] decodeResult The decode result value that has an error on it.
  /// @param[in] encodeResult The encode result value that represents what the next step would be if the
  /// decode did not work.
  /// @param[in] encodeErrorHandler The error handler to mill the `result` and other relevant
  /// information through.
  /// @param[in] toInputProgress Any unread output characters in any intermediate between the (failed)
  /// decode and encode operations.
  /// @param[in] toOutputProgress Any unread output characters in any intermediates between the (failed)
  /// decode and encode operations.
  /// @remarks This function is a helper whose sole purpose is to ensure that the other half of error
  /// handling is called by transcode-style functions written by the end user (e.g., writing overriding
  /// hooks for transcode). This function attempts to take care of any unread/unwritten
  /// characters and other minor points in its pursuit of properly making sure the error manifests on the
  /// other side. Unlike it's counterpart, this function does not take an `TResult` template parameter and
  /// instead deduces the returned transcode result type from inputs.
  template <typename TToEncoding, typename TEncodeErrorHandler, typename TToInputProgress,
            typename TToOutputProgress, typename TResultInput, typename TResultPivot, typename TFromState,
            typename TResultIntermediate, typename TResultOutput, typename TToState>
  constexpr auto PropagateTranscodeEncodeError(
    TToEncoding &&toEncoding, DecodeResult<TResultInput, TResultPivot, TFromState> &&decodeResult,
    EncodeResult<TResultIntermediate, TResultOutput, TToState> &&encodeResult,
    TEncodeErrorHandler &&encodeErrorHandler, TToInputProgress &&toInputProgress,
    TToOutputProgress &&toOutputProgress) noexcept
  {
    using TResult =
      ::Krys::Text::TranscodeResult<TResultInput, TResultOutput, TFromState, TToState, TResultPivot>;
    return ::Krys::Text::PropagateTranscodeEncodeError<TResult>(
      std::forward<TToEncoding>(toEncoding), std::move(decodeResult), std::move(encodeResult),
      std::forward<TEncodeErrorHandler>(encodeErrorHandler), std::forward<TToInputProgress>(toInputProgress),
      std::forward<TToOutputProgress>(toOutputProgress));
  }

  /// @brief Transcoding helper. Takes the given `fromEncoding`, `toEncoding`,
  /// `decodeErrorHandler`, and
  /// `encodeErrorHandler` and launders the DecodeResult through the
  /// `decodeErrorHandler`. The transformed decode result is then transformed to a
  /// EncodeResult before transforming that into the desired TranscodeResult type.
  /// @tparam TResult The exact transcode result type to use.
  /// @param[in] input The input view to be reading from.
  /// @param[in] output The output view to be writing into.
  /// @param[in] fromEncoding The desired encoding that performs the encode portion of the transcoding
  /// step.
  /// @param[in] toEncoding The desired encoding that performs the encode portion of the transcoding
  /// step.
  /// @param[in] decodeResult The decode result value that has an error on it.
  /// @param[in] decodeErrorHandler The error handler to mill the `result` and
  /// other relevant information through.
  /// @param[in] encodeErrorHandler The error handler to mill the `decodeErrorHandler`'s  invoked
  /// result and other relevant information through.
  /// @param[in] toState The current state of the encoding step of the transcode operation.
  /// @param[in] fromInputProgress Any unread output characters in any intermediate between the (failed)
  /// decode and encode operations.
  /// @param[in] fromOutputProgress Any unread output characters in any intermediates between the
  /// (failed) decode and encode operations.
  /// @param[in] toInputProgress Any unread output characters in any intermediate between the (failed)
  /// decode and encode operations.
  /// @param[in] toOutputProgress Any unread output characters in any intermediates between the (failed)
  /// decode and encode operations.
  /// @remarks This function is a helper whose sole purpose is to ensure that the other half of error
  /// handling is called by transcode-style functions written by the end user (e.g., writing overriding
  /// hooks for transcode). This function attempts to take care of any unread/unwritten
  /// characters and other minor points in its pursuit of properly making sure the error manifests on the
  /// other side.
  template <typename TResult, typename TInput, typename TOutput, typename TFromEncoding, typename TToEncoding,
            typename TDecodeErrorHandler, typename TEncodeErrorHandler, typename TToState,
            typename TFromInputProgress, typename TFromOutputProgress, typename TToInputProgress,
            typename TToOutputProgress, typename TResultInput, typename TIntermediate, typename TFromState>
  constexpr auto PropagateTranscodeDecodeErrorWith(
    TInput &&input, TOutput &&output, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
    DecodeResult<TResultInput, TIntermediate, TFromState> &&decodeResult,
    TDecodeErrorHandler &&decodeErrorHandler, TEncodeErrorHandler &&encodeErrorHandler, TToState &toState,
    TFromInputProgress &&fromInputProgress, TFromOutputProgress &&fromOutputProgress,
    TToInputProgress &&toInputProgress, TToOutputProgress &&toOutputProgress) noexcept
  {
    // first, run the decode error handler
    auto decodeErrorResult = std::forward<TDecodeErrorHandler>(decodeErrorHandler)(
      std::forward<TFromEncoding>(fromEncoding), std::move(decodeResult),
      std::forward<TFromInputProgress>(fromInputProgress),
      std::forward<TFromOutputProgress>(fromOutputProgress));
    // then, run the encode error handler
    EncodeResult<TIntermediate, remove_cvref_t<TOutput>, TToState> encodeResult(
      std::move(decodeErrorResult.Output), std::forward<TOutput>(output), toState,
      decodeErrorResult.ErrorCode, decodeErrorResult.ErrorCount);
    auto encodeErrorResult = std::forward<TEncodeErrorHandler>(encodeErrorHandler)(
      std::forward<TToEncoding>(toEncoding), std::move(encodeResult),
      std::forward<TToInputProgress>(toInputProgress), std::forward<TToOutputProgress>(toOutputProgress));
    // compose into the final result type
    return TResult(std::forward<TInput>(input), std::move(encodeErrorResult.Output), decodeErrorResult.State,
                   encodeErrorResult.State, encodeErrorResult.ErrorCode, encodeErrorResult.ErrorCount,
                   std::move(decodeErrorResult.Output), decodeErrorResult.ErrorCode,
                   decodeErrorResult.ErrorCount);
  }

  /// @brief Transcoding helper. Takes the given `fromEncoding`, `toEncoding`,
  /// `decodeErrorHandler`, and
  /// `encodeErrorHandler` and launders the DecodeResult through the
  /// `decodeErrorHandler`. The transformed decode result is then transformed to a
  /// EncodeResult before transforming that into the desired TranscodeResult type.
  /// @tparam TResult The exact transcode result type to use.
  /// @param[in] input The input view to be reading from.
  /// @param[in] output The output view to be writing into.
  /// @param[in] fromEncoding The desired encoding that performs the encode portion of the transcoding
  /// step.
  /// @param[in] toEncoding The desired encoding that performs the encode portion of the transcoding
  /// step.
  /// @param[in] decodeResult The decode result value that has an error on it.
  /// @param[in] decodeErrorHandler The error handler to mill the `result` and
  /// other relevant information through.
  /// @param[in] encodeErrorHandler The error handler to mill the `decodeErrorHandler`'s  invoked
  /// result and other relevant information through.
  /// @param[in] toState The current state of the encoding step of the transcode operation.
  /// @param[in] fromInputProgress Any unread output characters in any intermediate between the (failed)
  /// decode and encode operations.
  /// @param[in] fromOutputProgress Any unread output characters in any intermediates between the
  /// (failed) decode and encode operations.
  /// @param[in] toInputProgress Any unread output characters in any intermediate between the (failed)
  /// decode and encode operations.
  /// @param[in] toOutputProgress Any unread output characters in any intermediates between the (failed)
  /// decode and encode operations.
  /// @remarks This function is a helper whose sole purpose is to ensure that the other half of error
  /// handling is called by transcode-style functions written by the end user (e.g., writing overriding
  /// hooks for transcode). This function attempts to take care of any unread/unwritten
  /// characters and other minor points in its pursuit of properly making sure the error manifests on the
  /// other side.
  template <typename TInput, typename TOutput, typename TFromEncoding, typename TToEncoding,
            typename TDecodeErrorHandler, typename TEncodeErrorHandler, typename TToState,
            typename TFromInputProgress, typename TFromOutputProgress, typename TToInputProgress,
            typename TToOutputProgress, typename TResultInput, typename TResultIntermediate,
            typename TFromState>
  constexpr auto PropagateTranscodeDecodeErrorWith(
    TInput &&input, TOutput &&output, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
    DecodeResult<TResultInput, TResultIntermediate, TFromState> &&decodeResult,
    TDecodeErrorHandler &&decodeErrorHandler, TEncodeErrorHandler &&encodeErrorHandler, TToState &toState,
    TFromInputProgress &&fromInputProgress, TFromOutputProgress &&fromOutputProgress,
    TToInputProgress &&toInputProgress, TToOutputProgress &&toOutputProgress) noexcept
  {
    using TResult = ::Krys::Text::TranscodeResult<TResultInput, remove_cvref_t<TOutput>, TFromState, TToState,
                                                  TResultIntermediate>;
    return ::Krys::Text::PropagateTranscodeDecodeErrorWith<TResult>(
      std::forward<TInput>(input), std::forward<TOutput>(output), std::move(decodeResult),
      std::forward<TFromEncoding>(fromEncoding), std::forward<TToEncoding>(toEncoding),
      std::forward<TDecodeErrorHandler>(decodeErrorHandler),
      std::forward<TEncodeErrorHandler>(encodeErrorHandler), toState,
      std::forward<TToInputProgress>(fromInputProgress), std::forward<TToOutputProgress>(fromOutputProgress),
      std::forward<TToInputProgress>(toInputProgress), std::forward<TToOutputProgress>(toOutputProgress));
  }

  /// @brief Transcoding helper. Takes the given `toEncoding` and `encodeErrorHandler` and launders
  /// the failed DecodeResult through it, producing a EncodeResult and transforming
  /// that into the desired TranscodeResult type.
  /// @tparam TResult The exact transcode result type to use.
  /// @param[in] input The input view to be reading from.
  /// @param[in] toEncoding The desired encoding that performs the encode portion of the transcoding
  /// step.
  /// @param[in] decodeResult The decode result value that has an error on it.
  /// @param[in] encodeResult The encode result value that represents .
  /// @param[in] encodeErrorHandler The error handler to mill the `result` and other relevant
  /// information through.
  /// @param[in] toInputProgress Any unread output characters in any intermediate between the (failed)
  /// decode and encode operations.
  /// @param[in] toOutputProgress Any unread output characters in any intermediates between the (failed)
  /// decode and encode operations.
  /// @remarks This function is a helper whose sole purpose is to ensure that the other half of error
  /// handling is called by transcode-style functions written by the end user (e.g., writing overriding
  /// hooks for transcode). This function attempts to take care of any unread/unwritten
  /// characters and other minor points in its pursuit of properly making sure the error manifests on the
  /// other side.
  template <typename TResult, typename TInput, typename TToEncoding, typename TEncodeErrorHandler,
            typename TToInputProgress, typename TToOutputProgress, typename TResultInput,
            typename TResultPivot, typename TFromState, typename TResultIntermediate, typename TResultOutput,
            typename TToState>
  constexpr auto PropagateTranscodeEncodeErrorWith(
    TInput &&input, TToEncoding &&toEncoding,
    DecodeResult<TResultInput, TResultPivot, TFromState> &&decodeResult,
    EncodeResult<TResultIntermediate, TResultOutput, TToState> &&encodeResult,
    TEncodeErrorHandler &&encodeErrorHandler, TToInputProgress &&toInputProgress,
    TToOutputProgress &&toOutputProgress) noexcept
  {
    // just run the encode error handler only
    auto encodeErrorResult = std::forward<TEncodeErrorHandler>(encodeErrorHandler)(
      std::forward<TToEncoding>(toEncoding), std::move(encodeResult),
      std::forward<TToInputProgress>(toInputProgress), std::forward<TToOutputProgress>(toOutputProgress));
    return TResult(std::forward<TInput>(input), std::move(encodeErrorResult.Output), decodeResult.State,
                   encodeErrorResult.State, encodeErrorResult.ErrorCode, encodeErrorResult.ErrorCount,
                   std::move(decodeResult.Output), decodeResult.ErrorCode, decodeResult.ErrorCount);
  }

  /// @brief Takes the given `toEncoding` and `encodeErrorHandler` and launders the failed
  /// DecodeResult through it, producing a EncodeResult and transforming that into
  /// the desired TranscodeResult type.
  /// @param[in] input The input view to be reading from.
  /// @param[in] toEncoding The desired encoding that performs the encode portion of the transcoding
  /// step.
  /// @param[in] decodeResult The decode result value that has an error on it.
  /// @param[in] encodeResult The encode result value that represents .
  /// @param[in] encodeErrorHandler The error handler to mill the `result` and other relevant
  /// information through.
  /// @param[in] toInputProgress Any unread output characters in any intermediate between the (failed)
  /// decode and encode operations.
  /// @param[in] toOutputProgress Any unread output characters in any intermediates between the (failed)
  /// decode and encode operations.
  /// @remarks This function is a helper whose sole purpose is to ensure that the other half of error
  /// handling is called by transcode-style functions written by the end user (e.g., writing overriding
  /// hooks for transcode). This function attempts to take care of any unread/unwritten
  /// characters and other minor points in its pursuit of properly making sure the error manifests on the
  /// other side. Unlike it's counterpart, this function does not take an `TResult` template parameter and
  /// instead deduces the returned transcode result type from inputs.
  template <typename TInput, typename TToEncoding, typename TEncodeErrorHandler, typename TToInputProgress,
            typename TToOutputProgress, typename TResultInput, typename TResultPivot, typename TFromState,
            typename TResultIntermediate, typename TResultOutput, typename TToState>
  constexpr auto PropagateTranscodeEncodeErrorWith(
    TInput &&input, TToEncoding &&toEncoding,
    DecodeResult<TResultInput, TResultPivot, TFromState> &&decodeResult,
    EncodeResult<TResultIntermediate, TResultOutput, TToState> &&encodeResult,
    TEncodeErrorHandler &&encodeErrorHandler, TToInputProgress &&toInputProgress,
    TToOutputProgress &&toOutputProgress) noexcept
  {
    using TResult =
      ::Krys::Text::TranscodeResult<TResultInput, TResultOutput, TFromState, TToState, TResultPivot>;
    return ::Krys::Text::PropagateTranscodeEncodeErrorWith<TResult>(
      std::forward<TInput>(input), std::forward<TToEncoding>(toEncoding), std::move(decodeResult),
      std::move(encodeResult), std::forward<TEncodeErrorHandler>(encodeErrorHandler),
      std::forward<TToInputProgress>(toInputProgress), std::forward<TToOutputProgress>(toOutputProgress));
  }
}