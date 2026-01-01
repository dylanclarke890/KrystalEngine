// =============================================================================
//
// ztd.text
// Copyright © JeanHeyd "ThePhD" Meneide and Shepherd's Oasis, LLC
// Contact: opensource@soasis.org
//
// Commercial License Usage
// Licensees holding valid commercial ztd.text licenses may use this file in
// accordance with the commercial license agreement provided with the
// Software or, alternatively, in accordance with the terms contained in
// a written agreement between you and Shepherd's Oasis, LLC.
// For licensing terms and conditions see your agreement. For
// further information contact opensource@soasis.org.
//
// Apache License Version 2 Usage
// Alternatively, this file may be used under the terms of Apache License
// Version 2.0 (the "License") for non-commercial use; you may not use this
// file except in compliance with the License. You may obtain a copy of the
// License at
//
// https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// ============================================================================ //

#pragma once

#ifndef ZTD_TEXT_PROPAGATE_RECODE_ERROR_HPP
  #define ZTD_TEXT_PROPAGATE_RECODE_ERROR_HPP

  

  #include "Krystal.Text/DecodeResult.hpp"
  #include "Krystal.Text/EncodeResult.hpp"
  #include <ztd/text/RecodeResult.hpp>

  #include "Krystal.Lib/Core/TypeTraits.hpp"

  

namespace Krys
{
  namespace text
  {
    

    
    /// @addtogroup ztd_text_propagate_errors propagate_(re|trans)code_errors
    ///
    /// @{

    
    /// @brief Transcoding helper. Takes the given `__from_encoding`, `__to_encoding`,
    /// `TEncodeErrorHandler`, and
    /// `TDecodeErrorHandler` and launders the EncodeResult through the
    /// `TEncodeErrorHandler`. The transformed encode result is then transformed to a
    /// DecodeResult before transforming that into the desired RecodeResult type.
    ///
    /// @tparam TResult The exact recode result type to use.
    ///
    /// @param[in] output The output view to be writing into.
    /// @param[in] __from_encoding The desired encoding that performs the decode portion of the transcoding
    /// step.
    /// @param[in] __to_encoding The desired encoding that performs the decode portion of the transcoding
    /// step.
    /// @param[in] TEncodeResult The result value that has an error in it.
    /// @param[in] TEncodeErrorHandler The error handler to mill the `result` and
    /// other relevant information through.
    /// @param[in] TDecodeErrorHandler The error handler to mill the `TEncodeErrorHandler`'s  invoked
    /// result and other relevant information through.
    /// @param[in] toState The current state of the encoding step of the recode operation.
    /// @param[in] __from_input_progress Any unread input characters in any intermediate between the (failed)
    /// encode and decode operations.
    /// @param[in] __from_output_progress Any unread intermediate output characters in any intermediates
    /// between the (failed) encode and decode operations.
    /// @param[in] __to_input_progress Any unread intermediate input characters in any intermediate between
    /// the (failed) encode and decode operations.
    /// @param[in] __to_output_progress Any unread output characters in any intermediates between the (failed)
    /// encode and decode operations.
    ///
    /// @remarks This function is a helper whose sole purpose is to ensure that the other half of error
    /// handling is called by recode-style functions written by the end user (e.g., writing overriding hooks
    /// for recode). This function attempts to take care of any unread/unwritten characters and
    /// other minor points in its pursuit of properly making sure the error manifests on the other side.
    template <typename TResult, typename TOutput, typename TFromEncoding, typename TToEncoding,
              typename _EncodeErrorHandler, typename _DecodeErrorHandler, typename TToState,
              typename _FromInputProgress, typename _FromOutputProgress, typename _ToInputProgress,
              typename _ToOutputProgress, typename _ResultInput, typename _Intermediate, typename TFromState>
    constexpr auto propagate_recode_encode_error(
      TOutput &&output, TFromEncoding &&__from_encoding, TToEncoding &&__to_encoding,
      EncodeResult<_ResultInput, _Intermediate, TFromState> &&TEncodeResult,
      _EncodeErrorHandler &&TEncodeErrorHandler, _DecodeErrorHandler &&TDecodeErrorHandler,
      TToState &toState, _FromInputProgress &&__from_input_progress,
      _FromOutputProgress &&__from_output_progress, _ToInputProgress &&__to_input_progress,
      _ToOutputProgress &&__to_output_progress) noexcept
    {
      // first, run the encode error handler
      auto __encode_error_result = std::forward<_EncodeErrorHandler>(TEncodeErrorHandler)(
        std::forward<TFromEncoding>(__from_encoding), std::move(TEncodeResult),
        std::forward<_FromInputProgress>(__from_input_progress),
        std::forward<_FromOutputProgress>(__from_output_progress));
      // then, run the decode error handler
      DecodeResult<_Intermediate, remove_cvref_t<TOutput>, TToState> TDecodeResult(
        std::move(__encode_error_result.Output), std::forward<TOutput>(output), toState,
        __encode_error_result.ErrorCode, __encode_error_result.ErrorCount);
      auto __decode_error_result = std::forward<_DecodeErrorHandler>(TDecodeErrorHandler)(
        std::forward<TToEncoding>(__to_encoding), std::move(TDecodeResult),
        std::forward<_ToInputProgress>(__to_input_progress),
        std::forward<_ToOutputProgress>(__to_output_progress));
      // compose into the final result type
      return TResult(std::move(__encode_error_result.Input), std::move(__decode_error_result.Output),
                     __encode_error_result.State, __decode_error_result.State,
                     __decode_error_result.ErrorCode, __decode_error_result.ErrorCount,
                     std::move(__encode_error_result.Output), __encode_error_result.ErrorCode,
                     __encode_error_result.ErrorCount);
    }

    
    /// @brief Transcoding helper. Takes the given `__from_encoding`, `__to_encoding`,
    /// `TEncodeErrorHandler`, and
    /// `TDecodeErrorHandler` and launders the EncodeResult through the
    /// `TEncodeErrorHandler`. The transformed encode result is then transformed to a
    /// DecodeResult before transforming that into the desired RecodeResult type.
    ///
    /// @tparam TResult The exact recode result type to use.
    ///
    /// @param[in] output The output view to be writing into.
    /// @param[in] __from_encoding The desired encoding that performs the decode portion of the transcoding
    /// step.
    /// @param[in] __to_encoding The desired encoding that performs the decode portion of the transcoding
    /// step.
    /// @param[in] TEncodeResult The result value that has an error in it.
    /// @param[in] TEncodeErrorHandler The error handler to mill the `result` and
    /// other relevant information through.
    /// @param[in] TDecodeErrorHandler The error handler to mill the `TEncodeErrorHandler`'s  invoked
    /// result and other relevant information through.
    /// @param[in] toState The current state of the encoding step of the recode operation.
    /// @param[in] __from_input_progress Any unread input characters in any intermediate between the (failed)
    /// encode and decode operations.
    /// @param[in] __from_output_progress Any unread intermediate output characters in any intermediates
    /// between the (failed) encode and decode operations.
    /// @param[in] __to_input_progress Any unread intermediate input characters in any intermediate between
    /// the (failed) encode and decode operations.
    /// @param[in] __to_output_progress Any unread output characters in any intermediates between the (failed)
    /// encode and decode operations.
    ///
    /// @remarks This function is a helper whose sole purpose is to ensure that the other half of error
    /// handling is called by recode-style functions written by the end user (e.g., writing overriding hooks
    /// for recode). This function attempts to take care of any unread/unwritten characters and
    /// other minor points in its pursuit of properly making sure the error manifests on the other side.
    template <typename TOutput, typename TFromEncoding, typename TToEncoding, typename _EncodeErrorHandler,
              typename _DecodeErrorHandler, typename TToState, typename _FromInputProgress,
              typename _FromOutputProgress, typename _ToInputProgress, typename _ToOutputProgress,
              typename _ResultInput, typename _ResultIntermediate, typename TFromState>
    constexpr auto propagate_recode_encode_error(
      TOutput &&output, TFromEncoding &&__from_encoding, TToEncoding &&__to_encoding,
      EncodeResult<_ResultInput, _ResultIntermediate, TFromState> &&TEncodeResult,
      _EncodeErrorHandler &&TEncodeErrorHandler, _DecodeErrorHandler &&TDecodeErrorHandler,
      TToState &toState, _FromInputProgress &&__from_input_progress,
      _FromOutputProgress &&__from_output_progress, _ToInputProgress &&__to_input_progress,
      _ToOutputProgress &&__to_output_progress) noexcept
    {
      using TResult = RecodeResult<_ResultInput, remove_cvref_t<TOutput>, TFromState, TToState,
                                    _ResultIntermediate>;
      return ::propagate_recode_encode_error<TResult>(
        std::forward<TOutput>(output), std::move(TEncodeResult),
        std::forward<TFromEncoding>(__from_encoding), std::forward<TToEncoding>(__to_encoding),
        std::forward<_EncodeErrorHandler>(TEncodeErrorHandler),
        std::forward<_DecodeErrorHandler>(TDecodeErrorHandler), toState,
        std::forward<_ToInputProgress>(__from_input_progress),
        std::forward<_ToOutputProgress>(__from_output_progress),
        std::forward<_ToInputProgress>(__to_input_progress),
        std::forward<_ToOutputProgress>(__to_output_progress));
    }

    
    /// @brief Transcoding helper. Takes the given `__to_encoding` and `TDecodeErrorHandler` and launders
    /// the failed EncodeResult through it, producing a DecodeResult and transforming
    /// that into the desired RecodeResult type.
    ///
    /// @tparam TResult The exact recode result type to use.
    ///
    /// @param[in] __to_encoding The desired encoding that performs the decode portion of the transcoding
    /// step.
    /// @param[in] TEncodeResult The encode result value that has an error in it.
    /// @param[in] TDecodeResult The decode result value that has not yet been processed by the decode error
    /// handler.
    /// @param[in] TDecodeErrorHandler The error handler to mill the `result` and other relevant
    /// information through.
    /// @param[in] __to_input_progress Any unread output characters in any intermediate between the (failed)
    /// encode and decode operations.
    /// @param[in] __to_output_progress Any unread output characters in any intermediates between the (failed)
    /// encode and decode operations.
    ///
    /// @remarks This function is a helper whose sole purpose is to ensure that the other half of error
    /// handling is called by recode-style functions written by the end user (e.g., writing overriding hooks
    /// for recode). This function attempts to take care of any unread/unwritten characters and
    /// other minor points in its pursuit of properly making sure the error manifests on the other side.
    template <typename TResult, typename TToEncoding, typename _DecodeErrorHandler, typename _ToInputProgress,
              typename _ToOutputProgress, typename _ResultInput, typename _ResultPivot, typename TFromState,
              typename _ResultIntermediate, typename _ResultOutput, typename TToState>
    constexpr auto propagate_recode_decode_error(
      TToEncoding &&__to_encoding, EncodeResult<_ResultInput, _ResultPivot, TFromState> &&TEncodeResult,
      DecodeResult<_ResultIntermediate, _ResultOutput, TToState> &&TDecodeResult,
      _DecodeErrorHandler &&TDecodeErrorHandler, _ToInputProgress &&__to_input_progress,
      _ToOutputProgress &&__to_output_progress) noexcept
    {
      // just run the decode error handler only
      auto __decode_error_result = std::forward<_DecodeErrorHandler>(TDecodeErrorHandler)(
        std::forward<TToEncoding>(__to_encoding), std::move(TDecodeResult),
        std::forward<_ToInputProgress>(__to_input_progress),
        std::forward<_ToOutputProgress>(__to_output_progress));
      return TResult(std::move(TEncodeResult.Input), std::move(__decode_error_result.Output),
                     TEncodeResult.State, __decode_error_result.State, __decode_error_result.ErrorCode,
                     __decode_error_result.ErrorCount, std::move(TEncodeResult.Output),
                     TEncodeResult.ErrorCode, TEncodeResult.ErrorCount);
    }

    
    /// @brief Takes the given `__to_encoding` and `TDecodeErrorHandler` and launders the failed
    /// EncodeResult through it, producing a DecodeResult and transforming that into
    /// the desired RecodeResult type.
    ///
    /// @param[in] __to_encoding The desired encoding that performs the decode portion of the transcoding
    /// step.
    /// @param[in] TEncodeResult The result value that has an error on it.
    /// @param[in] TDecodeResult The result value that has an error on it.
    /// @param[in] TDecodeErrorHandler The error handler to mill the `result` and other relevant
    /// information through.
    /// @param[in] __to_input_progress Any unread output characters in any intermediate between the (failed)
    /// encode and decode operations.
    /// @param[in] __to_output_progress Any unread output characters in any intermediates between the (failed)
    /// encode and decode operations.
    ///
    /// @remarks This function is a helper whose sole purpose is to ensure that the other half of error
    /// handling is called by recode-style functions written by the end user (e.g., writing overriding hooks
    /// for recode). This function attempts to take care of any unread/unwritten characters and
    /// other minor points in its pursuit of properly making sure the error manifests on the other side.
    /// Unlike it's counterpart, this function does not take an `TResult` template parameter and instead
    /// deduces the returned recode result type from inputs.
    template <typename TToEncoding, typename _DecodeErrorHandler, typename _ToInputProgress,
              typename _ToOutputProgress, typename _ResultInput, typename _ResultPivot, typename TFromState,
              typename _ResultIntermediate, typename _ResultOutput, typename TToState>
    constexpr auto propagate_recode_decode_error(
      TToEncoding &&__to_encoding, EncodeResult<_ResultInput, _ResultPivot, TFromState> &&TEncodeResult,
      DecodeResult<_ResultIntermediate, _ResultOutput, TToState> &&TDecodeResult,
      _DecodeErrorHandler &&TDecodeErrorHandler, _ToInputProgress &&__to_input_progress,
      _ToOutputProgress &&__to_output_progress) noexcept
    {
      using TResult = RecodeResult<_ResultInput, _ResultOutput, TFromState, TToState, _ResultPivot>;
      return ::propagate_recode_decode_error<TResult>(
        std::forward<TToEncoding>(__to_encoding), std::move(TEncodeResult),
        std::move(TDecodeResult), std::forward<_DecodeErrorHandler>(TDecodeErrorHandler),
        std::forward<_ToInputProgress>(__to_input_progress),
        std::forward<_ToOutputProgress>(__to_output_progress));
    }

    
    /// @brief Transcoding helper. Takes the given `__from_encoding`, `__to_encoding`,
    /// `TEncodeErrorHandler`, and
    /// `TDecodeErrorHandler` and launders the EncodeResult through the
    /// `TEncodeErrorHandler`. The transformed encode result is then transformed to a
    /// DecodeResult before transforming that into the desired RecodeResult type.
    ///
    /// @tparam TResult The exact recode result type to use.
    ///
    /// @param[in] input The input view to be reading from.
    /// @param[in] output The output view to be writing into.
    /// @param[in] __from_encoding The desired encoding that performs the decode portion of the transcoding
    /// step.
    /// @param[in] __to_encoding The desired encoding that performs the decode portion of the transcoding
    /// step.
    /// @param[in] TEncodeResult The result value that has an error in it.
    /// @param[in] TEncodeErrorHandler The error handler to mill the `result` and
    /// other relevant information through.
    /// @param[in] TDecodeErrorHandler The error handler to mill the `TEncodeErrorHandler`'s  invoked
    /// result and other relevant information through.
    /// @param[in] toState The current state of the encoding step of the recode operation.
    /// @param[in] __from_input_progress Any unread input characters in any intermediate between the (failed)
    /// encode and decode operations.
    /// @param[in] __from_output_progress Any unread intermediate output characters in any intermediates
    /// between the (failed) encode and decode operations.
    /// @param[in] __to_input_progress Any unread intermediate input characters in any intermediate between
    /// the (failed) encode and decode operations.
    /// @param[in] __to_output_progress Any unread output characters in any intermediates between the (failed)
    /// encode and decode operations.
    ///
    /// @remarks This function is a helper whose sole purpose is to ensure that the other half of error
    /// handling is called by recode-style functions written by the end user (e.g., writing overriding hooks
    /// for recode). This function attempts to take care of any unread/unwritten characters and
    /// other minor points in its pursuit of properly making sure the error manifests on the other side.
    template <typename TResult, typename TInput, typename TOutput, typename TFromEncoding,
              typename TToEncoding, typename _EncodeErrorHandler, typename _DecodeErrorHandler,
              typename TToState, typename _FromInputProgress, typename _FromOutputProgress,
              typename _ToInputProgress, typename _ToOutputProgress, typename _ResultInput,
              typename _Intermediate, typename TFromState>
    constexpr auto propagate_recode_encode_error_with(
      TInput &&input, TOutput &&output, TFromEncoding &&__from_encoding, TToEncoding &&__to_encoding,
      EncodeResult<_ResultInput, _Intermediate, TFromState> &&TEncodeResult,
      _EncodeErrorHandler &&TEncodeErrorHandler, _DecodeErrorHandler &&TDecodeErrorHandler,
      TToState &toState, _FromInputProgress &&__from_input_progress,
      _FromOutputProgress &&__from_output_progress, _ToInputProgress &&__to_input_progress,
      _ToOutputProgress &&__to_output_progress) noexcept
    {
      // first, run the encode error handler
      auto __encode_error_result = std::forward<_EncodeErrorHandler>(TEncodeErrorHandler)(
        std::forward<TFromEncoding>(__from_encoding), std::move(TEncodeResult),
        std::forward<_FromInputProgress>(__from_input_progress),
        std::forward<_FromOutputProgress>(__from_output_progress));
      // then, run the decode error handler
      DecodeResult<_Intermediate, remove_cvref_t<TOutput>, TToState> TDecodeResult(
        std::move(__encode_error_result.Output), std::forward<TOutput>(output), toState,
        __encode_error_result.ErrorCode, __encode_error_result.ErrorCount);
      auto __decode_error_result = std::forward<_DecodeErrorHandler>(TDecodeErrorHandler)(
        std::forward<TToEncoding>(__to_encoding), std::move(TDecodeResult),
        std::forward<_ToInputProgress>(__to_input_progress),
        std::forward<_ToOutputProgress>(__to_output_progress));
      // compose into the final result type
      return TResult(std::forward<TInput>(input), std::move(__decode_error_result.Output),
                     __encode_error_result.State, __decode_error_result.State,
                     __decode_error_result.ErrorCode, __decode_error_result.ErrorCount,
                     std::move(__encode_error_result.Output), __encode_error_result.ErrorCode,
                     __encode_error_result.ErrorCount);
    }

    
    /// @brief Transcoding helper. Takes the given `__from_encoding`, `__to_encoding`,
    /// `TEncodeErrorHandler`, and
    /// `TDecodeErrorHandler` and launders the EncodeResult through the
    /// `TEncodeErrorHandler`. The transformed encode result is then transformed to a
    /// DecodeResult before transforming that into the desired RecodeResult type.
    ///
    /// @tparam TResult The exact recode result type to use.
    ///
    /// @param[in] input The input view to be reading from.
    /// @param[in] output The output view to be writing into.
    /// @param[in] __from_encoding The desired encoding that performs the decode portion of the transcoding
    /// step.
    /// @param[in] __to_encoding The desired encoding that performs the decode portion of the transcoding
    /// step.
    /// @param[in] TEncodeResult The result value that has an error in it.
    /// @param[in] TEncodeErrorHandler The error handler to mill the `result` and
    /// other relevant information through.
    /// @param[in] TDecodeErrorHandler The error handler to mill the `TEncodeErrorHandler`'s  invoked
    /// result and other relevant information through.
    /// @param[in] toState The current state of the encoding step of the recode operation.
    /// @param[in] __from_input_progress Any unread input characters in any intermediate between the (failed)
    /// encode and decode operations.
    /// @param[in] __from_output_progress Any unread intermediate output characters in any intermediates
    /// between the (failed) encode and decode operations.
    /// @param[in] __to_input_progress Any unread intermediate input characters in any intermediate between
    /// the (failed) encode and decode operations.
    /// @param[in] __to_output_progress Any unread output characters in any intermediates between the (failed)
    /// encode and decode operations.
    ///
    /// @remarks This function is a helper whose sole purpose is to ensure that the other half of error
    /// handling is called by recode-style functions written by the end user (e.g., writing overriding hooks
    /// for recode). This function attempts to take care of any unread/unwritten characters and
    /// other minor points in its pursuit of properly making sure the error manifests on the other side.
    template <typename TInput, typename TOutput, typename TFromEncoding, typename TToEncoding,
              typename _EncodeErrorHandler, typename _DecodeErrorHandler, typename TToState,
              typename _FromInputProgress, typename _FromOutputProgress, typename _ToInputProgress,
              typename _ToOutputProgress, typename _ResultInput, typename _ResultIntermediate,
              typename TFromState>
    constexpr auto propagate_recode_encode_error_with(
      TInput &&input, TOutput &&output, TFromEncoding &&__from_encoding, TToEncoding &&__to_encoding,
      EncodeResult<_ResultInput, _ResultIntermediate, TFromState> &&TEncodeResult,
      _EncodeErrorHandler &&TEncodeErrorHandler, _DecodeErrorHandler &&TDecodeErrorHandler,
      TToState &toState, _FromInputProgress &&__from_input_progress,
      _FromOutputProgress &&__from_output_progress, _ToInputProgress &&__to_input_progress,
      _ToOutputProgress &&__to_output_progress) noexcept
    {
      using TResult = RecodeResult<_ResultInput, remove_cvref_t<TOutput>, TFromState, TToState,
                                    _ResultIntermediate>;
      return ::propagate_recode_encode_error_with<TResult>(
        std::forward<TInput>(input), std::forward<TOutput>(output), std::move(TEncodeResult),
        std::forward<TFromEncoding>(__from_encoding), std::forward<TToEncoding>(__to_encoding),
        std::forward<_EncodeErrorHandler>(TEncodeErrorHandler),
        std::forward<_DecodeErrorHandler>(TDecodeErrorHandler), toState,
        std::forward<_ToInputProgress>(__from_input_progress),
        std::forward<_ToOutputProgress>(__from_output_progress),
        std::forward<_ToInputProgress>(__to_input_progress),
        std::forward<_ToOutputProgress>(__to_output_progress));
    }

    
    /// @brief Transcoding helper. Takes the given `__to_encoding` and `TDecodeErrorHandler` and launders
    /// the failed EncodeResult through it, producing a DecodeResult and transforming
    /// that into the desired RecodeResult type.
    ///
    /// @tparam TResult The exact recode result type to use.
    ///
    /// @param[in] input The input view to be read from.
    /// @param[in] __to_encoding The desired encoding that performs the decode portion of the transcoding
    /// step.
    /// @param[in] TEncodeResult The result value that has an error on it.
    /// @param[in] TDecodeResult The result value that has an error on it.
    /// @param[in] TDecodeErrorHandler The error handler to mill the `result` and other relevant
    /// information through.
    /// @param[in] __to_input_progress Any unread output characters in any intermediate between the (failed)
    /// encode and decode operations.
    /// @param[in] __to_output_progress Any unread output characters in any intermediates between the (failed)
    /// encode and decode operations.
    ///
    /// @remarks This function is a helper whose sole purpose is to ensure that the other half of error
    /// handling is called by recode-style functions written by the end user (e.g., writing overriding hooks
    /// for recode). This function attempts to take care of any unread/unwritten characters and
    /// other minor points in its pursuit of properly making sure the error manifests on the other side.
    template <typename TResult, typename TInput, typename TToEncoding, typename _DecodeErrorHandler,
              typename _ToInputProgress, typename _ToOutputProgress, typename _ResultInput,
              typename _ResultPivot, typename TFromState, typename _ResultIntermediate,
              typename _ResultOutput, typename TToState>
    constexpr auto propagate_recode_decode_error_with(
      TInput &&input, TToEncoding &&__to_encoding,
      EncodeResult<_ResultInput, _ResultPivot, TFromState> &&TEncodeResult,
      DecodeResult<_ResultIntermediate, _ResultOutput, TToState> &&TDecodeResult,
      _DecodeErrorHandler &&TDecodeErrorHandler, _ToInputProgress &&__to_input_progress,
      _ToOutputProgress &&__to_output_progress) noexcept
    {
      // just run the decode error handler only
      auto __decode_error_result = std::forward<_DecodeErrorHandler>(TDecodeErrorHandler)(
        std::forward<TToEncoding>(__to_encoding), std::move(TDecodeResult),
        std::forward<_ToInputProgress>(__to_input_progress),
        std::forward<_ToOutputProgress>(__to_output_progress));
      return TResult(std::forward<TInput>(input), std::move(__decode_error_result.Output),
                     TEncodeResult.State, __decode_error_result.State, __decode_error_result.ErrorCode,
                     __decode_error_result.ErrorCount, std::move(TEncodeResult.Output),
                     TEncodeResult.ErrorCode, TEncodeResult.ErrorCount);
    }

    
    /// @brief Takes the given `__to_encoding` and `TDecodeErrorHandler` and launders the failed
    /// EncodeResult through it, producing a DecodeResult and transforming that into
    /// the desired RecodeResult type.
    ///
    /// @param[in] input The input view to be read from.
    /// @param[in] __to_encoding The desired encoding that performs the decode portion of the transcoding
    /// step.
    /// @param[in] TEncodeResult The result value that has an error on it.
    /// @param[in] TDecodeResult The result value that has an error on it.
    /// @param[in] TDecodeErrorHandler The error handler to mill the `result` and other relevant
    /// information through.
    /// @param[in] __to_input_progress Any unread output characters in any intermediate between the (failed)
    /// encode and decode operations.
    /// @param[in] __to_output_progress Any unread output characters in any intermediates between the (failed)
    /// encode and decode operations.
    ///
    /// @remarks This function is a helper whose sole purpose is to ensure that the other half of error
    /// handling is called by recode-style functions written by the end user (e.g., writing overriding hooks
    /// for recode). This function attempts to take care of any unread/unwritten characters and
    /// other minor points in its pursuit of properly making sure the error manifests on the other side.
    /// Unlike it's counterpart, this function does not take an `TResult` template parameter and instead
    /// deduces the returned recode result type from inputs.
    template <typename TInput, typename TToEncoding, typename _DecodeErrorHandler, typename _ToInputProgress,
              typename _ToOutputProgress, typename _ResultInput, typename _ResultPivot, typename TFromState,
              typename _ResultIntermediate, typename _ResultOutput, typename TToState>
    constexpr auto propagate_recode_decode_error_with(
      TInput &&input, TToEncoding &&__to_encoding,
      EncodeResult<_ResultInput, _ResultPivot, TFromState> &&TEncodeResult,
      DecodeResult<_ResultIntermediate, _ResultOutput, TToState> &&TDecodeResult,
      _DecodeErrorHandler &&TDecodeErrorHandler, _ToInputProgress &&__to_input_progress,
      _ToOutputProgress &&__to_output_progress) noexcept
    {
      using TResult = RecodeResult<_ResultInput, _ResultOutput, TFromState, TToState, _ResultPivot>;
      return ::propagate_recode_decode_error_with<TResult>(
        std::forward<TInput>(input), std::forward<TToEncoding>(__to_encoding),
        std::move(TEncodeResult), std::move(TDecodeResult),
        std::forward<_DecodeErrorHandler>(TDecodeErrorHandler),
        std::forward<_ToInputProgress>(__to_input_progress),
        std::forward<_ToOutputProgress>(__to_output_progress));
    }

    
    /// @}

    
  }
} 

  

#endif