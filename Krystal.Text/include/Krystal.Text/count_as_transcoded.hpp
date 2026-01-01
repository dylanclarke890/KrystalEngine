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

#ifndef ZTD_TEXT_count_as_transcoded_HPP
#define ZTD_TEXT_count_as_transcoded_HPP



#include "Krystal.Text/CodePoint.hpp"
#include <ztd/text/default_encoding.hpp>
#include <ztd/text/CountResult.hpp>
#include <ztd/text/error_handler.hpp>
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include <ztd/text/max_units.hpp>
#include <ztd/text/detail/is_lossless.hpp>
#include <ztd/text/detail/encoding_range.hpp>
#include "Krystal.Text/Impl/SpanReconstruct.hpp"

#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Text/CharTraits.hpp"
#include "Krystal.Lib/Utils/Tag.hpp"

#include <string_view>



namespace Krys {
	

	
	/// @addtogroup ztd_text_count_as_transcoded count_as_transcoded
	///
	/// @brief These functions use a variety of means to count the number of code units that will result from the
	/// input code units after a transcoding operation.
	///
	/// @{

	
	/// @brief Counts the number of code units that will result from attempting an transcode operation on the input
	/// code points.
	///
	/// @param[in] input The input range (of code units) to find out how many code units of the transcoded output
	/// there are.
	/// @param[in] __from_encoding The encoding that is going to be used to decode the input into an intermediary
	/// output.
	/// @param[in] __to_encoding The encoding that is going to be used to encode the intermediary output.
	/// @param[in] __from_error_handler The error handler to invoke when an intermediary decoding operation fails.
	/// @param[in] __to_error_handler The error handler to invoke when the final encoding operation fails.
	/// @param[in,out] fromState The state attached to the `__from_encoding` that will be used for the intermediary
	/// decode step.
	/// @param[in,out] toState The state related to the `__to_encoding` that will be used for the final encoding
	/// step.
	/// @param[in, out] pivot A reference to a descriptor of a (potentially usable) pivot range, usually a range of
	/// contiguous data from a span provided by the implementation but customizable by the end-user. If the
	/// intermediate conversion is what failed, then the pivot's `ErrorCode` member will be set to that
	/// error. This only happens if the overall operation also fails, and need not be checked unless to obtain
	/// additional information for when a top-level operation fails.
	///
	/// @returns A CountResult that includes information about how many code units are present,
	/// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler) and a reference
	/// to the provided `fromState` and `toState` .
	///
	/// @remarks This method will not check any ADL extension points. A combination of implementation techniques will
	/// be used to count code units, with a loop over the `.EncodeOne` / `.DecodeOne` call into an intermediate,
	/// unseen buffer being the most basic choice.
	template <typename TInput, typename TFromEncoding, typename TToEncoding, typename _FromErrorHandler,
		typename _ToErrorHandler, typename TFromState, typename TToState, typename TPivot>
	constexpr auto basic_count_as_transcoded(TInput&& input, TFromEncoding&& __from_encoding,
		TToEncoding&& __to_encoding, _FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler,
		TFromState& fromState, TToState& toState, TPivot&& pivot) {
		using _WorkingInput  = Krys::Ranges::subrange_for_t<Impl::span_reconstruct_t<TInput, TInput>>;
		using _UFromEncoding = remove_cvref_t<TFromEncoding>;
		using _UToEncoding   = remove_cvref_t<TToEncoding>;
		using TResult        = CountTranscodeResult<_WorkingInput, TFromState, TToState>;

		_WorkingInput __working_input(Impl::SpanReconstruct<TInput>(std::forward<TInput>(input)));

		std::size_t __code_unit_count = 0;
		std::size_t errorCount     = 0;

#define ZTD_TEXT_BASIC_COUNT_AS_TRANSCODED_LOOP_BODY_CORE_I_()                                                   \
	if (result.ErrorCode != EncodingError::OK) {                                                            \
		return TResult(std::move(result.Input), __code_unit_count, result.FromState, result.ToState, \
		     result.ErrorCode, errorCount);                                                              \
	}                                                                                                           \
	__code_unit_count += result.Count;                                                                        \
	errorCount += result.ErrorCount;                                                                      \
	__working_input = std::move(result.Input);                                                              \
	if (Krys::Ranges::empty(__working_input)) {                                                                \
		if (!::is_state_complete(__from_encoding, result.FromState)) {                           \
			continue;                                                                                         \
		}                                                                                                      \
		if (!::is_state_complete(__to_encoding, result.ToState)) {                               \
			continue;                                                                                         \
		}                                                                                                      \
		break;                                                                                                 \
	}                                                                                                           \
	do {                                                                                                        \
	} while (0)

#define ZTD_TEXT_BASIC_COUNT_AS_TRANSCODED_LOOP_BODY_I_(...)                                                        \
	auto result                                                                                                  \
	     = __VA_ARGS__(Krys::Tag<_UFromEncoding, _UToEncoding> {}, std::move(__working_input), __from_encoding, \
	          __to_encoding, __from_error_handler, __to_error_handler, fromState, toState, pivot);         \
	ZTD_TEXT_BASIC_COUNT_AS_TRANSCODED_LOOP_BODY_CORE_I_()

		if constexpr (is_detected_v<Impl::HasTextCountAsTranscodedOne, _WorkingInput,
			              TFromEncoding, TToEncoding, _FromErrorHandler, _ToErrorHandler, TFromState, TToState,
			              TPivot>) {
			for (;;) {
				ZTD_TEXT_BASIC_COUNT_AS_TRANSCODED_LOOP_BODY_I_(TextCountAsTranscodedOne);
			}
		}
		else if constexpr (is_detected_v<Impl::HasInternalTextCountAsTranscodedOne,
			                   _WorkingInput, TFromEncoding, TToEncoding, _FromErrorHandler, _ToErrorHandler,
			                   TFromState, TToState, TPivot>) {
			for (;;) {
				ZTD_TEXT_BASIC_COUNT_AS_TRANSCODED_LOOP_BODY_I_(InternalTextCountAsTranscodedOne);
			}
		}
		else {
			constexpr std::size_t __output_max = max_transcode_code_units_v<_UFromEncoding, _UToEncoding>;
			using TCodeUnit                      = code_unit_t<_UToEncoding>;

			TCodeUnit __output_storage[__output_max] {};
			Span<TCodeUnit, __output_max> output(__output_storage);

			for (;;) {
				auto result = transcode_one_into_raw(std::move(__working_input), __from_encoding, output,
					__to_encoding, __from_error_handler, __to_error_handler, fromState, toState, pivot);
				if (result.ErrorCode != EncodingError::OK) {
					return TResult(std::move(result.Input), __code_unit_count, result.FromState,
						result.ToState, result.ErrorCode, errorCount);
				}
				std::size_t __written = static_cast<std::size_t>(result.Output.data() - output.data());
				__code_unit_count += __written;
				errorCount += result.ErrorCount;
				__working_input = std::move(result.Input);
				if (Krys::Ranges::empty(__working_input)) {
					if (!::is_state_complete(__from_encoding, fromState)) {
						continue;
					}
					if (!::is_state_complete(__to_encoding, toState)) {
						continue;
					}
					break;
				}
			}
		}
		return TResult(std::move(__working_input), __code_unit_count, fromState, toState, EncodingError::OK,
			errorCount);
	}

	
	/// @brief Counts the number of code units that will result from attempting an transcode operation on the input
	/// code points.
	///
	/// @param[in] input The input range (of code units) to find out how many code units of the transcoded output
	/// there are.
	/// @param[in] __from_encoding The encoding that is going to be used to decode the input into an intermediary
	/// output.
	/// @param[in] __to_encoding The encoding that is going to be used to encode the intermediary output.
	/// @param[in] __from_error_handler The error handler to invoke when an intermediary decoding operation fails.
	/// @param[in] __to_error_handler The error handler to invoke when the final encoding operation fails.
	/// @param[in,out] fromState The state attached to the `__from_encoding` that will be used for the intermediary
	/// decode step.
	/// @param[in,out] toState The state related to the `__to_encoding` that will be used for the final encoding
	/// step.
	/// @param[in, out] pivot A reference to a descriptor of a (potentially usable) pivot range, usually a range of
	/// contiguous data from a span provided by the implementation but customizable by the end-user. If the
	/// intermediate conversion is what failed, then the pivot's `ErrorCode` member will be set to that
	/// error. This only happens if the overall operation also fails, and need not be checked unless to obtain
	/// additional information for when a top-level operation fails.
	///
	/// @returns A CountResult that includes information about how many code units are present,
	/// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler) and a reference
	/// to the provided `fromState` and `toState` .
	///
	/// @remarks This method checks for the ADL extension point `TextCountAsTranscoded` . It will be called if it
	/// is possible. Otherwise, this function will defer to basic_count_as_transcoded.
	template <typename TInput, typename TFromEncoding, typename TToEncoding, typename _FromErrorHandler,
		typename _ToErrorHandler, typename TFromState, typename TToState, typename TPivot>
	constexpr auto count_as_transcoded(TInput&& input, TFromEncoding&& __from_encoding, TToEncoding&& __to_encoding,
		_FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler, TFromState& fromState,
		TToState& toState, TPivot&& pivot) {
		if constexpr (is_detected_v<Impl::HasTextCountAsTranscoded, TInput, TFromEncoding,
			              TToEncoding, _FromErrorHandler, _ToErrorHandler, TFromState, TToState>) {
			return TextCountAsTranscoded(
				Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>> {},
				std::forward<TInput>(input), std::forward<TFromEncoding>(__from_encoding),
				std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
				std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState, pivot);
		}
		else if constexpr (is_detected_v<Impl::HasInternalTextCountAsTranscoded, TInput,
			                   TFromEncoding, TToEncoding, _FromErrorHandler, _ToErrorHandler, TFromState,
			                   TToState>) {
			return InternalTextCountAsTranscoded(
				Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>> {},
				std::forward<TInput>(input), std::forward<TFromEncoding>(__from_encoding),
				std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
				std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState, pivot);
		}
		else {
			return basic_count_as_transcoded(std::forward<TInput>(input),
				std::forward<TFromEncoding>(__from_encoding), std::forward<TToEncoding>(__to_encoding),
				std::forward<_FromErrorHandler>(__from_error_handler),
				std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState, pivot);
		}
	}

	
	/// @brief Counts the number of code units that will result from attempting an transcode operation on the input
	/// code points.
	///
	/// @param[in] input The input range (of code units) to find out how many code units of the transcoded output
	/// there are.
	/// @param[in] __from_encoding The encoding that is going to be used to decode the input into an intermediary
	/// output.
	/// @param[in] __to_encoding The encoding that is going to be used to encode the intermediary output.
	/// @param[in] __from_error_handler The error handler to invoke when an intermediary decoding operation fails.
	/// @param[in] __to_error_handler The error handler to invoke when the final encoding operation fails.
	/// @param[in,out] fromState The state attached to the `__from_encoding` that will be used for the intermediary
	/// decode step.
	/// @param[in,out] toState The state attached to the `__to_encoding` that will be used for the final encode
	/// step.
	///
	/// @returns A StatelessCountResult that includes information about how many code units are present,
	/// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler).
	///
	/// @remarks This method will call count_as_transcoded(input, from_encoding, to_encoding,
	/// from_error_handler, to_error_handler, FromState, ToState) with an `ToState` created by
	/// Krys::CreateEncodeState(to_encoding).
	template <typename TInput, typename TFromEncoding, typename TToEncoding, typename _FromErrorHandler,
		typename _ToErrorHandler, typename TFromState, typename TToState>
	constexpr auto count_as_transcoded(TInput&& input, TFromEncoding&& __from_encoding, TToEncoding&& __to_encoding,
		_FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler, TFromState& fromState,
		TToState& toState) {
		using _UFromEncoding = remove_cvref_t<TFromEncoding>;
		using TCodePoint     = code_point_t<_UFromEncoding>;
		using TPivot         = Span<TCodePoint, MaxCodePoints<_UFromEncoding>>;
		TCodePoint __intermediate_storage[MaxCodePoints<_UFromEncoding>] {};
		TPivot pivot(__intermediate_storage);
		return count_as_transcoded(std::forward<TInput>(input), std::forward<TFromEncoding>(__from_encoding),
			std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
			std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState, pivot);
	}

	
	/// @brief Counts the number of code units that will result from attempting an transcode operation on the input
	/// code points.
	///
	/// @param[in] input The input range (of code units) to find out how many code units of the transcoded output
	/// there are.
	/// @param[in] __from_encoding The encoding that is going to be used to decode the input into an intermediary
	/// output.
	/// @param[in] __to_encoding The encoding that is going to be used to encode the intermediary output.
	/// @param[in] __from_error_handler The error handler to invoke when an intermediary decoding operation fails.
	/// @param[in] __to_error_handler The error handler to invoke when the final encoding operation fails.
	/// @param[in,out] fromState The state attached to the `__from_encoding` that will be used for the intermediary
	/// decode step.
	///
	/// @returns A StatelessCountResult that includes information about how many code units are present,
	/// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler).
	///
	/// @remarks This method will call count_as_transcoded(input, from_encoding, to_encoding,
	/// from_error_handler, to_error_handler, FromState, ToState) with an `ToState` created by
	/// Krys::CreateEncodeState(to_encoding).
	template <typename TInput, typename TFromEncoding, typename TToEncoding, typename _FromErrorHandler,
		typename _ToErrorHandler, typename TFromState>
	constexpr auto count_as_transcoded(TInput&& input, TFromEncoding&& __from_encoding, TToEncoding&& __to_encoding,
		_FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler, TFromState& fromState) {
		using _UToEncoding = remove_cvref_t<TToEncoding>;
		using TState       = EncodeState<_UToEncoding>;

		TState toState = Krys::CreateEncodeState(__to_encoding);

		auto result
			= count_as_transcoded(std::forward<TInput>(input), std::forward<TFromEncoding>(__from_encoding),
			     std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
			     std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState);
		return Impl::SliceToStateless(std::move(result));
	}


	
	/// @brief Counts the number of code units that will result from attempting an transcode operation on the input
	/// code points.
	///
	/// @param[in] input The input range (of code units) to find out how many code units of the transcoded output
	/// there are.
	/// @param[in] __from_encoding The encoding that is going to be used to decode the input into an intermediary
	/// output.
	/// @param[in] __to_encoding The encoding that is going to be used to encode the intermediary output.
	/// @param[in] __from_error_handler The error handler to invoke when the decode portion of the transcode operation
	/// fails.
	/// @param[in] __to_error_handler The error handler to invoke when the encode portion of the transcode operation
	/// fails.
	///
	/// @returns A StatelessCountResult that includes information about how many code units are present,
	/// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler).
	///
	/// @remarks This method will call count_as_transcoded(input, from_encoding, to_encoding,
	/// from_error_handler, to_error_handler, FromState) with an `FromState` created by
	/// CreateDecodeState(from_encoding).
	template <typename TInput, typename TFromEncoding, typename TToEncoding, typename _FromErrorHandler,
		typename _ToErrorHandler>
	constexpr auto count_as_transcoded(TInput&& input, TFromEncoding&& __from_encoding, TToEncoding&& __to_encoding,
		_FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler) {
		using _UFromEncoding = remove_cvref_t<TFromEncoding>;
		using TState         = DecodeState<_UFromEncoding>;

		TState fromState = CreateDecodeState(__from_encoding);

		return count_as_transcoded(std::forward<TInput>(input), std::forward<TFromEncoding>(__from_encoding),
			std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
			std::forward<_ToErrorHandler>(__to_error_handler), fromState);
	}

	
	/// @brief Counts the number of code units that will result from attempting an transcode operation.
	///
	/// @param[in] input The input range (of code units) to find out how many code units of the transcoded output
	/// there are.
	/// @param[in] __from_encoding The encoding that is going to be used to decode the input into an intermediary
	/// output.
	/// @param[in] __to_encoding The encoding that is going to be used to encode the intermediary output.
	/// @param[in] __from_error_handler The error handler to invoke when the decode portion of the transcode operation
	/// fails.
	///
	/// @returns A StatelessCountResult that includes information about how many code units are present,
	/// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler).
	///
	/// @remarks This method will call count_as_transcoded(input, from_encoding, to_encoding,
	/// from_error_handler, to_error_handler) by creating an `to_error_handler` similar to
	/// Handlers::DefaultHandler.
	template <typename TInput, typename TFromEncoding, typename TToEncoding, typename _FromErrorHandler>
	constexpr auto count_as_transcoded(TInput&& input, TFromEncoding&& __from_encoding, TToEncoding&& __to_encoding,
		_FromErrorHandler&& __from_error_handler) {
		auto __handler = Impl::__duplicate_or_be_careless(__from_error_handler);

		return count_as_transcoded(std::forward<TInput>(input), std::forward<TFromEncoding>(__from_encoding),
			std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
			__handler);
	}

	
	/// @brief Counts the number of code units that will result from attempting an transcode operation.
	///
	/// @param[in] input The input range (of code units) to find out how many code units of the transcoded output
	/// there are.
	/// @param[in] __from_encoding The encoding that is going to be used to decode the input into an intermediary
	/// output.
	/// @param[in] __to_encoding The encoding that is going to be used to encode the intermediary output.
	///
	/// @returns A StatelessCountResult that includes information about how many code units are present,
	/// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler).
	///
	/// @remarks This method will call count_as_transcoded(Input, Encoding, ErrorHandler) by creating
	/// an `error_handler` similar to Handlers::DefaultHandler.
	template <typename TInput, typename TFromEncoding, typename TToEncoding>
	constexpr auto count_as_transcoded(
		TInput&& input, TFromEncoding&& __from_encoding, TToEncoding&& __to_encoding) {
		Handlers::DefaultHandler __from_error_handler {};
		return count_as_transcoded(std::forward<TInput>(input), std::forward<TFromEncoding>(__from_encoding),
			std::forward<TToEncoding>(__to_encoding), __from_error_handler);
	}

	
	/// @brief Counts the number of code units that will result from attempting an transcode operation on the input
	/// code points.
	///
	/// @param[in] input The input range (of code units) to find out how many code units of the transcoded output
	/// there are.
	/// @param[in] __to_encoding The encoding that is going to be used to encode the input into an intermediary
	/// output.
	///
	/// @returns A StatelessCountResult that includes information about how many code units are present,
	/// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler).
	///
	/// @remarks Calls count_as_transcoded(Input, Encoding) with an `encoding` that is derived from
	/// default_code_unit_encoding.
	template <typename TInput, typename TToEncoding>
	constexpr auto count_as_transcoded(TInput&& input, TToEncoding&& __to_encoding) {
		using TUInput   = remove_cvref_t<TInput>;
		using TCodeUnit = remove_cvref_t<ranges::rangeTValue_type_t<TUInput>>;
#if ZTD_IS_ON(ZTD_STD_LIBRARY_IS_CONSTANT_EVALUATED)
		if (std::is_constant_evaluated()) {
			// Use literal encoding instead, if we meet the right criteria
			using TFromEncoding = default_consteval_code_unit_encoding_t<TCodeUnit>;
			TFromEncoding __from_encoding {};
			return count_as_transcoded(
				std::forward<TInput>(input), __from_encoding, std::forward<TToEncoding>(__to_encoding));
		}
		else
#endif
		{
			using TFromEncoding = default_code_unit_encoding_t<TCodeUnit>;
			TFromEncoding __from_encoding {};
			return count_as_transcoded(
				std::forward<TInput>(input), __from_encoding, std::forward<TToEncoding>(__to_encoding));
		}
	}

	
	/// @}

	
}



#endif
