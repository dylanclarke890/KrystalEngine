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

#ifndef ZTD_TEXT_TRANSCODE_HPP
#define ZTD_TEXT_TRANSCODE_HPP



#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include <ztd/text/max_units.hpp>
#include <ztd/text/error_handler.hpp>
#include "Krystal.Text/State.hpp"
#include <ztd/text/default_encoding.hpp>
#include <ztd/text/transcode_result.hpp>
#include "Krystal.Text/IsUnicodeCodePoint.hpp"
#include <ztd/text/transcode_one.hpp>
#include <ztd/text/encode.hpp>
#include <ztd/text/decode.hpp>
#include <ztd/text/detail/is_lossless.hpp>
#include <ztd/text/detail/encoding_range.hpp>
#include <ztd/text/detail/transcode_extension_points.hpp>
#include "Krystal.Text/Impl/SpanReconstruct.hpp"
#include <ztd/text/detail/forward_if_move_only.hpp>

#include "Krystal.Lib/Utils/Tag.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Text/CharTraits.hpp"
#include <ztd/ranges/unbounded.hpp>
#include <ztd/ranges/detail/insert_bulk.hpp>

#include <utility>
#include <string>
#include <vector>



namespace Krys {
	

	
	/// @addtogroup ztd_text_transcode transcode[_into/_to]
	///
	/// @brief These functions convert from a view of input code units into a view of output code units (typically,
	/// through an intermediary code point type that is similar between the two) using either the inferred or specified
	/// encodings. If no error handler is provided, the equivalent of the Handlers::DefaultHandler is used by
	/// default, but it is marked as careless. If no associated state is provided for either the "to" or "from"
	/// encodings, one will be created with automatic storage duration (as a "stack" variable) for the provided
	/// encoding.
	/// @{

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding into the output view.
	///
	/// @param[in]     input An input_view to read code units from and use in the decode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to decode the input's code units into
	/// intermediate code points.
	/// @param[in]     output An output_view to write code units to as the result of the encode operation from the
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to encode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in]     __to_error_handler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in,out] fromState A reference to the associated state for the `__from_encoding` 's decode step.
	/// @param[in,out] toState A reference to the associated state for the `__to_encoding` 's encode step.
	/// @param[in, out] pivot A reference to a descriptor of a (potentially usable) range as the intermediate pivot,
	/// usually a range of contiguous data from a span provided by the implementation but can be passed in here by the
	/// user.
	///
	/// @result A transcode_result object that contains references to `fromState` and @p
	/// toState.
	///
	/// @remark This function detects whether or not the ADL extension point `TextTranscode` can be called with the
	/// provided parameters. If so, it will use that ADL extension point over the default implementation. Otherwise, it
	/// will loop over the two encodings and attempt to transcode by first decoding the input code units to code
	/// points, then encoding the intermediate code points to the desired, output code units.
	template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler, typename TFromState, typename TToState, typename TPivot>
	constexpr auto basic_transcode_into_raw(TInput&& input, TFromEncoding&& __from_encoding, TOutput&& output,
		TToEncoding&& __to_encoding, _FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler,
		TFromState& fromState, TToState& toState, TPivot&& pivot) {
		using _InitialInput      = Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
		using _InitialOutput     = Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
		using _UFromEncoding     = remove_cvref_t<TFromEncoding>;
		using _UToEncoding       = remove_cvref_t<TToEncoding>;
		using _UFromErrorHandler = remove_cvref_t<_FromErrorHandler>;
		using _UToErrorHandler   = remove_cvref_t<_ToErrorHandler>;
		using TResult            = decltype(transcode_one_into_raw(std::declval<_InitialInput>(), __from_encoding,
			           std::declval<_InitialOutput>(), __to_encoding, __from_error_handler, __to_error_handler, fromState,
			           toState, pivot));
		using _WorkingInput      = decltype(std::declval<TResult>().Input);
		using _WorkingOutput     = decltype(std::declval<TResult>().Output);

		static_assert(Impl::__is_decode_lossless_or_deliberate_v<_UFromEncoding, _UFromErrorHandler>,
			ZTD_TEXT_LOSSY_TRANSCODE_DECODE_MESSAGE_I_);
		static_assert(Impl::__is_encode_lossless_or_deliberate_v<_UToEncoding, _UToErrorHandler>,
			ZTD_TEXT_LOSSY_TRANSCODE_ENCODE_MESSAGE_I_);

		_WorkingInput __working_input(std::forward<TInput>(input));
		_WorkingOutput __working_output(std::forward<TOutput>(output));

		std::size_t errorCount       = 0;
		std::size_t pivotErrorCount = 0;
		for (;;) {
			auto __transcode_result = ::transcode_one_into_raw(std::move(__working_input),
				__from_encoding, std::move(__working_output), __to_encoding, __from_error_handler,
				__to_error_handler, fromState, toState, pivot);
			errorCount += __transcode_result.ErrorCount;
			pivotErrorCount += __transcode_result.PivotErrorCount;
			__working_input  = std::move(__transcode_result.Input);
			__working_output = std::move(__transcode_result.Output);
			if (__transcode_result.ErrorCode != EncodingError::OK) {
				return TResult(std::move(__working_input), std::move(__working_output), fromState,
					toState, __transcode_result.ErrorCode, __transcode_result.ErrorCount,
					std::move(__transcode_result.Pivot), __transcode_result.PivotErrorCode,
					__transcode_result.PivotErrorCount);
			}
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
		return TResult(std::move(__working_input), std::move(__working_output), fromState, toState,
			EncodingError::OK, errorCount, std::forward<TPivot>(pivot), EncodingError::OK,
			pivotErrorCount);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding into the output view.
	///
	/// @param[in]     input An input_view to read code units from and use in the decode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to decode the input's code units into
	/// intermediate code points.
	/// @param[in]     output An output_view to write code units to as the result of the encode operation from the
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to encode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in]     __to_error_handler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in,out] fromState A reference to the associated state for the `__from_encoding` 's decode step.
	/// @param[in,out] toState A reference to the associated state for the `__to_encoding` 's encode step.
	/// @param[in, out] pivot A reference to a descriptor of a (potentially usable) range as the intermediate pivot,
	/// usually a range of contiguous data from a span provided by the implementation but can be passed in here by the
	/// user.
	///
	/// @result A transcode_result object that contains references to `fromState` and @p
	/// toState.
	///
	/// @remark This function detects whether or not the ADL extension point `TextTranscode` can be called with the
	/// provided parameters. If so, it will use that ADL extension point over the default implementation. Otherwise, it
	/// will loop over the two encodings and attempt to transcode by first decoding the input code units to code
	/// points, then encoding the intermediate code points to the desired, output code units.
	template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler, typename TFromState, typename TToState, typename TPivot>
	constexpr auto transcode_into_raw(TInput&& input, TFromEncoding&& __from_encoding, TOutput&& output,
		TToEncoding&& __to_encoding, _FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler,
		TFromState& fromState, TToState& toState, TPivot&& pivot) {
		if constexpr (is_detected_v<Impl::HasTextTranscode, TInput, TFromEncoding, TOutput,
			              TToEncoding, _FromErrorHandler, _ToErrorHandler, TFromState, TToState, TPivot>) {
			return TextTranscode(Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>> {},
				std::forward<TInput>(input), std::forward<TFromEncoding>(__from_encoding),
				std::forward<TOutput>(output), std::forward<TToEncoding>(__to_encoding),
				std::forward<_FromErrorHandler>(__from_error_handler),
				std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState, pivot);
		}
		else {
			using _UFromEncoding     = remove_cvref_t<TFromEncoding>;
			using _UToEncoding       = remove_cvref_t<TToEncoding>;
			using _UFromErrorHandler = remove_cvref_t<_FromErrorHandler>;
			using _UToErrorHandler   = remove_cvref_t<_ToErrorHandler>;
			using _UPivot            = remove_cvref_t<TPivot>;
			if constexpr (IsDecodeRedundant<_UFromEncoding, _UToEncoding> // cf
				&& is_encode_redundant_v<_UFromEncoding, _UToEncoding>       // cf
				&& IsIgnorableErrorHandler<_UFromErrorHandler>          // cf
				&& IsIgnorableErrorHandler<_UToErrorHandler>) {
				// we can simply copy from the input to the output, no questions asked!
				(void)__from_encoding;
				(void)__to_encoding;
				(void)__from_error_handler;
				(void)__to_error_handler;
				(void)fromState;
				(void)toState;
				auto result = Krys::Ranges::__rng_detail::__copy(
					Krys::Ranges::cbegin(std::forward<TInput>(input)), Krys::Ranges::cend(input),
					Krys::Ranges::begin(std::forward<TOutput>(output)), Krys::Ranges::end(output));
				using TResult = transcode_result<decltype(result.in), decltype(result.out), TFromState,
					TToState, _UPivot>;
				return TResult(std::move(result.in), std::move(result.out), fromState, toState,
					EncodingError::OK, 0, std::forward<TPivot>(pivot), EncodingError::OK, 0);
			}
			else if constexpr (is_detected_v<Impl::HasInternalTextTranscodeOne, TInput,
				                   TFromEncoding, TOutput, TToEncoding, _FromErrorHandler, _ToErrorHandler,
				                   TFromState, TToState, TPivot>) {
				return InternalTextTranscode(
					Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>> {},
					std::forward<TInput>(input), std::forward<TFromEncoding>(__from_encoding),
					std::forward<TOutput>(output), std::forward<TToEncoding>(__to_encoding),
					std::forward<_FromErrorHandler>(__from_error_handler),
					std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState, pivot);
			}
			else {
				return basic_transcode_into_raw(std::forward<TInput>(input),
					std::forward<TFromEncoding>(__from_encoding), std::forward<TOutput>(output),
					std::forward<TToEncoding>(__to_encoding),
					std::forward<_FromErrorHandler>(__from_error_handler),
					std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState, pivot);
			}
		}
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding into the output view.
	///
	/// @param[in]     input An input_view to read code units from and use in the decode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to decode the input's code units into
	/// intermediate code points.
	/// @param[in]     output An output_view to write code units to as the result of the encode operation from the
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to encode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in]     __to_error_handler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in,out] fromState A reference to the associated state for the `__from_encoding` 's decode step.
	/// @param[in,out] toState A reference to the associated state for the `__to_encoding` 's encode step.
	///
	/// @result A pivotless_transcode_result object that contains references to `fromState` and @p
	/// toState.
	///
	/// @remark This function detects whether or not the ADL extension point `TextTranscode` can be called with the
	/// provided parameters. If so, it will use that ADL extension point over the default implementation. Otherwise, it
	/// will loop over the two encodings and attempt to transcode by first decoding the input code units to code
	/// points, then encoding the intermediate code points to the desired, output code units.
	template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler, typename TFromState, typename TToState>
	constexpr auto transcode_into_raw(TInput&& input, TFromEncoding&& __from_encoding, TOutput&& output,
		TToEncoding&& __to_encoding, _FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler,
		TFromState& fromState, TToState& toState) {
		using _UFromEncoding = remove_cvref_t<TFromEncoding>;
		using TCodePoint     = code_point_t<_UFromEncoding>;
		using _PivotRange    = Krys::Ranges::subrange<TCodePoint*>;

		constexpr std::size_t __pivot_buffer_buffer_max
			= ZTD_TEXT_INTERMEDIATE_TRANSCODE_BUFFER_SIZE_I_(code_point_t<_UFromEncoding>)
			     < MaxCodePoints<_UFromEncoding>
			? MaxCodePoints<_UFromEncoding>
			: ZTD_TEXT_INTERMEDIATE_TRANSCODE_BUFFER_SIZE_I_(code_point_t<_UFromEncoding>);

		TCodePoint __pivot_buffer[__pivot_buffer_buffer_max] {};
		_PivotRange pivot(__pivot_buffer);
		return ::transcode_into_raw(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TOutput>(output),
			std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
			std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState, pivot);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding into the output view.
	///
	/// @param[in]     input An input_view to read code units from and use in the decode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to decode the input's code units into
	/// intermediate code points.
	/// @param[in]     output An output_view to write code units to as the result of the encode operation from the
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to encode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handler for the `__from_encoding` 's decode step.
	/// @param[in]     __to_error_handler The error handler for the `__to_encoding` 's encode step.
	/// @param[in,out] fromState A reference to the associated state for the `__from_encoding` 's decode step.
	///
	/// @result A stateless_transcode_result object.
	///
	/// @remarks This function calls the base reference, the transcode_into_raw after creating a `ToState`
	/// from Krys::CreateEncodeState. The result from this function returns a
	/// stateless_transcode_result as opposed to a transcode_result because the state
	/// information is on the stack, and returning the state in those types by reference will result in references to
	/// memory that has already been cleaned up. If you need access to the state parameters, call the lower-level
	/// functionality with your own created states.
	template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler, typename TFromState>
	constexpr auto transcode_into_raw(TInput&& input, TFromEncoding&& __from_encoding, TOutput&& output,
		TToEncoding&& __to_encoding, _FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler,
		TFromState& fromState) {
		using _UToEncoding = remove_cvref_t<TToEncoding>;
		using TToState     = EncodeState<_UToEncoding>;

		TToState toState = ::Krys::CreateEncodeState(__to_encoding);

		auto __stateful_result = ::transcode_into_raw(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TOutput>(output),
			std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
			std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState);

		return Impl::SliceToStateless(std::move(__stateful_result));
	}

	
	/// @brief Converts the code units of the given input view through the
	///        from encoding to code units of the to encoding into the output
	///        view.
	/// @param[in]     input An input_view to read code units from and use in the decode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to decode the input's code units into
	/// intermediate code points.
	/// @param[in]     output An output_view to write code units to as the result of the encode operation from the
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to encode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handler for the `__from_encoding` 's decode step.
	/// @param[in]     __to_error_handler The error handler for the `__to_encoding` 's encode step.
	///
	/// @remarks This function creates a decode state `FromState` by calling CreateDecodeState. The
	/// result from this function returns a stateless_transcode_result as opposed to a
	/// pivotless_transcode_result because the state information is on the stack, and returning the state in
	/// those types by reference will result in references to memory that has already been cleaned up. If you need
	/// access to the state parameters, call the lower-level functionality with your own created states.
	template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler>
	constexpr auto transcode_into_raw(TInput&& input, TFromEncoding&& __from_encoding, TOutput&& output,
		TToEncoding&& __to_encoding, _FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler) {
		using _UFromEncoding = remove_cvref_t<TFromEncoding>;
		using TFromState     = DecodeState<_UFromEncoding>;

		TFromState fromState = ::CreateDecodeState(__from_encoding);

		return ::transcode_into_raw(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TOutput>(output),
			std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
			std::forward<_ToErrorHandler>(__to_error_handler), fromState);
	}

	
	/// @brief Converts the code units of the given input view through the
	///        from encoding to code units of the to encoding into the output
	///        view.
	/// @param[in]     input An input_view to read code units from and use in the decode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to decode the input's code units into
	/// intermediate code points.
	/// @param[in]     output An output_view to write code units to as the result of the encode operation from the
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to encode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handler for the `__from_encoding` 's decode step.
	///
	/// @remarks This function creates an `to_error_handler` from a class like Handlers::DefaultHandler, but that
	/// is marked as careless since you did not explicitly provide it. This matters for lossy conversions that are not
	/// injective. The result from this function returns a stateless_transcode_result as opposed to a
	/// pivotless_transcode_result because the state information is on the stack, and returning the state in
	/// those types by reference will result in references to memory that has already been cleaned up. If you need
	/// access to the state parameters, call the lower-level functionality with your own created states.
	template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
		typename _FromErrorHandler>
	constexpr auto transcode_into_raw(TInput&& input, TFromEncoding&& __from_encoding, TOutput&& output,
		TToEncoding&& __to_encoding, _FromErrorHandler&& __from_error_handler) {
		auto __handler = Impl::__duplicate_or_be_careless(__from_error_handler);

		return ::transcode_into_raw(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TOutput>(output),
			std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
			__handler);
	}

	
	/// @brief Converts the code units of the given input view through the
	///        from encoding to code units of the to encoding into the output
	///        view.
	/// @param[in]     input An input_view to read code units from and use in the decode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to decode the input's code units into
	/// intermediate code points.
	/// @param[in]     output An output_view to write code units to as the result of the encode operation from the
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to encode the intermediate code points into the
	/// final code units.
	///
	/// @remarks This function creates an `from_error_handler` from a class like Handlers::DefaultHandler, but
	/// that is marked as careless since you did not explicitly provide it. This matters for lossy conversions that are
	/// not injective. The result from this function returns a stateless_transcode_result as opposed to a
	/// pivotless_transcode_result because the state information is on the stack, and returning the state in
	/// those types by reference will result in references to memory that has already been cleaned up. If you need
	/// access to the state parameters, call the lower-level functionality with your own created states.
	template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding>
	constexpr auto transcode_into_raw(
		TInput&& input, TFromEncoding&& __from_encoding, TOutput&& output, TToEncoding&& __to_encoding) {
		Handlers::DefaultHandler __handler {};

		return ::transcode_into_raw(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TOutput>(output),
			std::forward<TToEncoding>(__to_encoding), __handler);
	}

	
	/// @brief Converts the code units of the given input view through the
	///        from encoding to code units of the to encoding into the output
	///        view.
	/// @param[in]     input An input_view to read code units from and use in the decode operation that will
	/// produce intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to encode the intermediate code points into the
	/// final code units.
	/// @param[in]     output An output_view to write code units to as the result of the encode operation from the
	/// intermediate code points.
	///
	/// @remarks This function creates both: a `from_error_handler` using a Handlers::DefaultHandler that is
	/// marked as careless to pass to the next function overload; and, a `from_encoding` to interpret the `input`
	/// by checking the `input` 's `value_type.` This matters for lossy conversions that are not injective. The
	/// result from this function returns a stateless_transcode_result as opposed to a
	/// pivotless_transcode_result because the state information is on the stack, and returning the state in
	/// those types by reference will result in references to memory that has already been cleaned up. If you need
	/// access to the state parameters, call the lower-level functionality with your own created states.
	template <typename TInput, typename TToEncoding, typename TOutput>
	constexpr auto transcode_into_raw(TInput&& input, TToEncoding&& __to_encoding, TOutput&& output) {
		using TUInput        = remove_cvref_t<TInput>;
		using _UFromEncoding = default_code_unit_encoding_t<ranges::rangeTValue_type_t<TUInput>>;

		_UFromEncoding __from_encoding {};
		Handlers::DefaultHandler __handler {};

		return ::transcode_into_raw(std::forward<TInput>(input), __from_encoding,
			std::forward<TOutput>(output), std::forward<TToEncoding>(__to_encoding), __handler);
	}

	namespace Impl {
		template <typename TInput, typename TFromEncoding, typename _OutputContainer, typename TToEncoding,
			typename _FromErrorHandler, typename _ToErrorHandler, typename TFromState, typename TToState,
			typename TPivot>
		constexpr auto __intermediate_transcode_to_storage(TInput&& input, TFromEncoding&& __from_encoding,
			_OutputContainer& output, TToEncoding&& __to_encoding, _FromErrorHandler&& __from_error_handler,
			_ToErrorHandler&& __to_error_handler, TFromState& fromState, TToState& toState, TPivot&& pivot) {
			// … Weeeellll. Here we go …
			using _UFromEncoding     = remove_cvref_t<TFromEncoding>;
			using _UToEncoding       = remove_cvref_t<TToEncoding>;
			using _UFromErrorHandler = remove_cvref_t<_FromErrorHandler>;
			using _UToErrorHandler   = remove_cvref_t<_ToErrorHandler>;
			using _FromProgressHandler
				= Impl::ProgressHandler<IsIgnorableErrorHandler<_UFromErrorHandler>,
				     _UFromEncoding>;
			using _ToProgressHandler
				= Impl::ProgressHandler<IsIgnorableErrorHandler<_UToErrorHandler>, _UToEncoding>;
			using _IntermediateOutputValueType = code_unit_t<_UToEncoding>;
			constexpr std::size_t _MinimumIntermediateOutputMax
				= max_transcode_code_units_v<_UFromEncoding, _UToEncoding>;
			constexpr std::size_t _IntermediateOutputMax
				= ZTD_TEXT_INTERMEDIATE_TRANSCODE_BUFFER_SIZE_I_(_IntermediateOutputValueType)
				     < _MinimumIntermediateOutputMax
				? _MinimumIntermediateOutputMax
				: ZTD_TEXT_INTERMEDIATE_TRANSCODE_BUFFER_SIZE_I_(_IntermediateOutputValueType);
			using _InitialInput       = Krys::Ranges::csubrange_for_t<TInput>;
			using _IntermediateOutput = Krys::Ranges::subrange<_IntermediateOutputValueType*>;
			using _TranscodeResult    = decltype(::transcode_into_raw(std::declval<_InitialInput>(),
				   __from_encoding, std::declval<_IntermediateOutput>(), __to_encoding, __from_error_handler,
				   __to_error_handler, fromState, toState, pivot));
			using _WorkingInput       = decltype(std::declval<_TranscodeResult>().Input);

			static_assert(Impl::__is_decode_lossless_or_deliberate_v<remove_cvref_t<TFromEncoding>,
				              remove_cvref_t<_FromErrorHandler>>,
				ZTD_TEXT_LOSSY_TRANSCODE_DECODE_MESSAGE_I_);
			static_assert(Impl::__is_encode_lossless_or_deliberate_v<remove_cvref_t<TToEncoding>,
				              remove_cvref_t<_ToErrorHandler>>,
				ZTD_TEXT_LOSSY_TRANSCODE_ENCODE_MESSAGE_I_);

			_WorkingInput __working_input(Krys::Ranges::cbegin(input), Krys::Ranges::cend(input));
			_IntermediateOutputValueType __intermediate_output_storage[_IntermediateOutputMax] {};
			_FromProgressHandler __from_progress_handler {};
			_ToProgressHandler __to_progress_handler {};
			std::size_t errorCount       = 0;
			std::size_t pivotErrorCount = 0;
			for (;;) {
				__from_progress_handler.clear();
				__to_progress_handler.clear();
				_IntermediateOutput intermediateOutput(__intermediate_output_storage, _IntermediateOutputMax);
				auto result = ::transcode_into_raw(std::move(__working_input), __from_encoding,
					intermediateOutput, __to_encoding, __from_progress_handler, __to_progress_handler,
					fromState, toState, pivot);
				std::size_t __intermediate_written_count
					= static_cast<std::size_t>(result.Output.data() - intermediateOutput.data());
				_IntermediateOutput __written_intermediate(
					intermediateOutput.begin(), intermediateOutput.begin() + __intermediate_written_count);
				ranges::__rng_detail::__container_insert_bulk(output, __written_intermediate);
				if (result.ErrorCode == EncodingError::InsufficientOutputSpace) {
					if (__to_progress_handler.CodeUnitsProgressSize() != 0) {
						ranges::__rng_detail::__container_insert_bulk(
							output, __to_progress_handler.CodeUnitsProgress());
						errorCount += result.ErrorCount;
						pivotErrorCount += result.PivotErrorCount;
						__working_input
							= Impl::__update_const_input<_WorkingInput>(std::move(result.Input));
						continue;
					}
					else if (result.PivotErrorCode == EncodingError::OK) {
						// If this occured, we need to record the original pivot position, and then try to
						// re-serialize with enough space all over again to avoid issues.
						std::size_t __pivot_remnant_count = static_cast<std::size_t>(
							Krys::Ranges::size(pivot) - Krys::Ranges::size(result.Pivot));
						auto __pivot_remnant = Krys::Ranges::reconstruct(std::in_place_type<TPivot>,
							Krys::Ranges::cbegin(pivot), Krys::Ranges::cbegin(pivot) + __pivot_remnant_count);
						auto __pivot_result  = ::encode_into_raw(__pivot_remnant, __to_encoding,
							 intermediateOutput, __to_error_handler, toState);
						std::size_t __intermediate_written_count = static_cast<std::size_t>(
							__pivot_result.Output.data() - intermediateOutput.data());
						_IntermediateOutput __pivot_intermediate_output(intermediateOutput.begin(),
							intermediateOutput.begin() + __intermediate_written_count);
						ranges::__rng_detail::__container_insert_bulk(output, __pivot_intermediate_output);
						errorCount += __pivot_result.ErrorCount;
						pivotErrorCount += result.PivotErrorCount;
						if (__pivot_result.ErrorCode == EncodingError::OK) {
							__working_input = Impl::__update_const_input<_WorkingInput>(
								std::move(result.Input));
							continue;
						}
					}
					else {
						// it's okay, just loop around, we've got S P A C E for more
						__working_input
							= Impl::__update_const_input<_WorkingInput>(std::move(result.Input));
						continue;
					}
				}
				if (result.ErrorCode != EncodingError::OK) {
					using _ErrorDecodeResult = ::DecodeResult<decltype(result.Input),
						decltype(result.Pivot), TFromState>;
					using _ErrorEncodeResult = ::EncodeResult<decltype(result.Pivot),
						decltype(result.Output), TToState>;
					if (result.PivotErrorCode != EncodingError::OK) {
						// need to call the error handler and then propagate it.
						auto __error_result = ::propagate_transcode_decode_error<_TranscodeResult>(
							intermediateOutput, __from_encoding, __to_encoding,
							_ErrorDecodeResult(std::move(result.Input), std::move(result.Pivot),
							     fromState, result.PivotErrorCode, result.PivotErrorCount),
							__from_error_handler, __to_error_handler, toState,
							__from_progress_handler.CodeUnitsProgress(),
							__from_progress_handler.CodePointsProgress(),
							__to_progress_handler.CodePointsProgress(),
							__to_progress_handler.CodeUnitsProgress());
						std::size_t __error_written_count = static_cast<std::size_t>(
							__error_result.Output.data() - intermediateOutput.data());
						_IntermediateOutput __error_intermediate_output(intermediateOutput.begin(),
							intermediateOutput.begin() + __error_written_count);
						ranges::__rng_detail::__container_insert_bulk(output, __error_intermediate_output);
						errorCount += __error_result.ErrorCount;
						pivotErrorCount += __error_result.PivotErrorCount;
						if (__error_result.ErrorCode != EncodingError::OK) {
							return _TranscodeResult(std::move(__error_result.Input),
								std::move(__error_result.Output), __error_result.FromState,
								__error_result.ToState, __error_result.ErrorCode, errorCount,
								std::move(__error_result.Pivot), __error_result.PivotErrorCode,
								pivotErrorCount);
						}
						__working_input = Impl::__update_const_input<_WorkingInput>(
							std::move(__error_result.Input));
					}
					else {
						// just need to call the second edge of the failure
						auto __error_result
							= ::propagate_transcode_encode_error<_TranscodeResult>(__to_encoding,
							     _ErrorDecodeResult(std::move(result.Input), std::move(result.Pivot),
							          fromState, result.PivotErrorCode, result.PivotErrorCount),
							     _ErrorEncodeResult(std::move(result.Pivot), intermediateOutput,
							          toState, result.ErrorCode, result.ErrorCount),
							     __to_error_handler, __to_progress_handler.CodePointsProgress(),
							     __to_progress_handler.CodeUnitsProgress());
						std::size_t __error_written_count
							= __error_result.Output.data() - intermediateOutput.data();
						_IntermediateOutput __error_intermediate_output(intermediateOutput.begin(),
							intermediateOutput.begin() + __error_written_count);
						ranges::__rng_detail::__container_insert_bulk(output, __error_intermediate_output);
						errorCount += __error_result.ErrorCount;
						pivotErrorCount += __error_result.PivotErrorCount;
						if (__error_result.ErrorCode != EncodingError::OK) {
							return _TranscodeResult(std::move(__error_result.Input),
								std::move(__error_result.Output), __error_result.FromState,
								__error_result.ToState, __error_result.ErrorCode, errorCount,
								std::move(__error_result.Pivot), __error_result.PivotErrorCode,
								pivotErrorCount);
						}
						__working_input = Impl::__update_const_input<_WorkingInput>(
							std::move(__error_result.Input));
					}
					continue;
				}
				if (Krys::Ranges::empty(result.Input)
					&& ::is_state_complete(__from_encoding, fromState)
					&& ::is_state_complete(__to_encoding, toState)) {
					errorCount += result.ErrorCount;
					pivotErrorCount += result.PivotErrorCount;
					return _TranscodeResult(std::move(result.Input), std::move(result.Output),
						result.FromState, result.ToState, result.ErrorCode, errorCount,
						std::move(result.Pivot), result.PivotErrorCode, pivotErrorCount);
				}
				__working_input = Impl::__update_const_input<_WorkingInput>(std::move(result.Input));
			}
		}

		template <bool _OutputOnly, bool _NoState, typename _OutputContainer, typename TInput, typename TFromEncoding,
			typename TToEncoding, typename _FromErrorHandler, typename _ToErrorHandler, typename TFromState,
			typename TToState, typename TPivot>
		constexpr auto __transcode_dispatch(TInput&& input, TFromEncoding&& __from_encoding,
			TToEncoding&& __to_encoding, _FromErrorHandler&& __from_error_handler,
			_ToErrorHandler&& __to_error_handler, TFromState& fromState, TToState& toState, TPivot&& pivot) {

			_OutputContainer output {};
			if constexpr (is_detected_v<ranges::detect_adl_size, TInput>) {
				using TSize = decltype(Krys::Ranges::size(input));
				if constexpr (is_detected_v<ranges::detect_reserve_with_size, _OutputContainer, TSize>) {
					TSize __output_size_hint = static_cast<TSize>(Krys::Ranges::size(input));
					output.reserve(__output_size_hint);
				}
			}
			auto __stateful_result = Impl::__intermediate_transcode_to_storage(
				std::forward<TInput>(input), std::forward<TFromEncoding>(__from_encoding), output,
				std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
				std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState, pivot);
			if constexpr (_OutputOnly) {
				(void)__stateful_result;
				return output;
			}
			else if constexpr (_NoState) {
				return Impl::__replace_transcode_result_output_no_state(
					std::move(__stateful_result), std::move(output));
			}
			else {
				return Impl::__replace_transcode_result_output(
					std::move(__stateful_result), std::move(output));
			}
		}
	} 

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding into the output view.
	///
	/// @param[in]     input An input_view to read code units from and use in the decode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to decode the input's code units into
	/// intermediate code points.
	/// @param[in]     output An output_view to write code units to as the result of the encode operation from the
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to encode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in]     __to_error_handler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in,out] fromState A reference to the associated state for the `__from_encoding` 's decode step.
	/// @param[in,out] toState A reference to the associated state for the `__to_encoding` 's encode step.
	/// @param[in, out] pivot A reference to a descriptor of a (potentially usable) range as the intermediate pivot,
	/// usually a range of contiguous data from a span provided by the implementation but can be passed in here by the
	/// user.
	///
	/// @result A pivotless_transcode_result object that contains references to `fromState` and @p
	/// toState.
	///
	/// @remark This function detects whether or not the ADL extension point `TextTranscode` can be called with the
	/// provided parameters. If so, it will use that ADL extension point over the default implementation. Otherwise, it
	/// will loop over the two encodings and attempt to transcode by first decoding the input code units to code
	/// points, then encoding the intermediate code points to the desired, output code units.
	template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler, typename TFromState, typename TToState, typename TPivot>
	constexpr auto transcode_into(TInput&& input, TFromEncoding&& __from_encoding, TOutput&& output,
		TToEncoding&& __to_encoding, _FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler,
		TFromState& fromState, TToState& toState, TPivot&& pivot) {
		auto __reconstructed_input      = Impl::SpanReconstruct<TInput>(std::forward<TInput>(input));
		auto result                   = ::transcode_into_raw(std::move(__reconstructed_input),
			                  std::forward<TFromEncoding>(__from_encoding), std::forward<TOutput>(output),
			                  std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
			                  std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState, pivot);
		using _ReconstructedResultInput = Impl::span_reconstruct_t<TInput, decltype(result.Input)&&>;
		using _ReconstructedResultOutput
			= Impl::span_reconstruct_mutable_t<TOutput, decltype(result.Output)&&>;
		using TResult = transcode_result<_ReconstructedResultInput, _ReconstructedResultOutput, TFromState, TToState,
			decltype(result.Pivot)>;
		return TResult(Impl::SpanReconstruct<TInput>(std::move(result.Input)),
			Impl::SpanReconstructMutable<TOutput>(std::move(result.Output)), result.FromState,
			result.ToState, result.ErrorCode, result.ErrorCount, std::move(result.Pivot),
			result.PivotErrorCode, result.PivotErrorCount);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding into the output view.
	///
	/// @param[in]     input An input_view to read code units from and use in the decode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to decode the input's code units into
	/// intermediate code points.
	/// @param[in]     output An output_view to write code units to as the result of the encode operation from the
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to encode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in]     __to_error_handler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in,out] fromState A reference to the associated state for the `__from_encoding` 's decode step.
	/// @param[in,out] toState A reference to the associated state for the `__to_encoding` 's encode step.
	///
	/// @result A pivotless_transcode_result object that contains references to `fromState` and @p
	/// toState.
	///
	/// @remark This function detects whether or not the ADL extension point `TextTranscode` can be called with the
	/// provided parameters. If so, it will use that ADL extension point over the default implementation. Otherwise, it
	/// will loop over the two encodings and attempt to transcode by first decoding the input code units to code
	/// points, then encoding the intermediate code points to the desired, output code units.
	template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler, typename TFromState, typename TToState>
	constexpr auto transcode_into(TInput&& input, TFromEncoding&& __from_encoding, TOutput&& output,
		TToEncoding&& __to_encoding, _FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler,
		TFromState& fromState, TToState& toState) {
		using _UFromEncoding = remove_cvref_t<TFromEncoding>;
		using TCodePoint     = code_point_t<_UFromEncoding>;
		using _PivotRange    = Krys::Ranges::subrange<TCodePoint*>;

		constexpr std::size_t __pivot_buffer_buffer_max
			= ZTD_TEXT_INTERMEDIATE_TRANSCODE_BUFFER_SIZE_I_(code_point_t<_UFromEncoding>)
			     < MaxCodePoints<_UFromEncoding>
			? MaxCodePoints<_UFromEncoding>
			: ZTD_TEXT_INTERMEDIATE_TRANSCODE_BUFFER_SIZE_I_(code_point_t<_UFromEncoding>);

		TCodePoint __pivot_buffer[__pivot_buffer_buffer_max] {};
		_PivotRange pivot(__pivot_buffer);
		return ::transcode_into(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TOutput>(output),
			std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
			std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState, pivot);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding into the output view.
	///
	/// @param[in]     input An input_view to read code units from and use in the decode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to decode the input's code units into
	/// intermediate code points.
	/// @param[in]     output An output_view to write code units to as the result of the encode operation from the
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to encode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handler for the `__from_encoding` 's decode step.
	/// @param[in]     __to_error_handler The error handler for the `__to_encoding` 's encode step.
	/// @param[in,out] fromState A reference to the associated state for the `__from_encoding` 's decode step.
	///
	/// @result A stateless_transcode_result object.
	///
	/// @remarks This function calls the base reference, the transcode_into after creating a `ToState`
	/// from Krys::CreateEncodeState. The result from this function returns a
	/// stateless_transcode_result as opposed to a pivotless_transcode_result because the state
	/// information is on the stack, and returning the state in those types by reference will result in references to
	/// memory that has already been cleaned up. If you need access to the state parameters, call the lower-level
	/// functionality with your own created states.
	template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler, typename TFromState>
	constexpr auto transcode_into(TInput&& input, TFromEncoding&& __from_encoding, TOutput&& output,
		TToEncoding&& __to_encoding, _FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler,
		TFromState& fromState) {
		using _UToEncoding = remove_cvref_t<TToEncoding>;
		using TToState     = EncodeState<_UToEncoding>;

		TToState toState = ::Krys::CreateEncodeState(__to_encoding);

		auto __stateful_result = ::transcode_into(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TOutput>(output),
			std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
			std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState);

		return Impl::SliceToStateless(std::move(__stateful_result));
	}

	
	/// @brief Converts the code units of the given input view through the
	///        from encoding to code units of the to encoding into the output
	///        view.
	/// @param[in]     input An input_view to read code units from and use in the decode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to decode the input's code units into
	/// intermediate code points.
	/// @param[in]     output An output_view to write code units to as the result of the encode operation from the
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to encode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handler for the `__from_encoding` 's decode step.
	/// @param[in]     __to_error_handler The error handler for the `__to_encoding` 's encode step.
	///
	/// @remarks This function creates a decode state `FromState` by calling CreateDecodeState. The
	/// result from this function returns a stateless_transcode_result as opposed to a
	/// pivotless_transcode_result because the state information is on the stack, and returning the state in
	/// those types by reference will result in references to memory that has already been cleaned up. If you need
	/// access to the state parameters, call the lower-level functionality with your own created states.
	template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler>
	constexpr auto transcode_into(TInput&& input, TFromEncoding&& __from_encoding, TOutput&& output,
		TToEncoding&& __to_encoding, _FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler) {
		using _UFromEncoding = remove_cvref_t<TFromEncoding>;
		using TFromState     = DecodeState<_UFromEncoding>;

		TFromState fromState = ::CreateDecodeState(__from_encoding);

		return ::transcode_into(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TOutput>(output),
			std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
			std::forward<_ToErrorHandler>(__to_error_handler), fromState);
	}

	
	/// @brief Converts the code units of the given input view through the
	///        from encoding to code units of the to encoding into the output
	///        view.
	/// @param[in]     input An input_view to read code units from and use in the decode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to decode the input's code units into
	/// intermediate code points.
	/// @param[in]     output An output_view to write code units to as the result of the encode operation from the
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to encode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handler for the `__from_encoding` 's decode step.
	///
	/// @remarks This function creates an `to_error_handler` from a class like Handlers::DefaultHandler, but that
	/// is marked as careless since you did not explicitly provide it. This matters for lossy conversions that are not
	/// injective. The result from this function returns a stateless_transcode_result as opposed to a
	/// pivotless_transcode_result because the state information is on the stack, and returning the state in
	/// those types by reference will result in references to memory that has already been cleaned up. If you need
	/// access to the state parameters, call the lower-level functionality with your own created states.
	template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
		typename _FromErrorHandler>
	constexpr auto transcode_into(TInput&& input, TFromEncoding&& __from_encoding, TOutput&& output,
		TToEncoding&& __to_encoding, _FromErrorHandler&& __from_error_handler) {
		auto __handler = Impl::__duplicate_or_be_careless(__from_error_handler);

		return ::transcode_into(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TOutput>(output),
			std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
			__handler);
	}

	
	/// @brief Converts the code units of the given input view through the
	///        from encoding to code units of the to encoding into the output
	///        view.
	/// @param[in]     input An input_view to read code units from and use in the decode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to decode the input's code units into
	/// intermediate code points.
	/// @param[in]     output An output_view to write code units to as the result of the encode operation from the
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to encode the intermediate code points into the
	/// final code units.
	///
	/// @remarks This function creates an `from_error_handler` from a class like Handlers::DefaultHandler, but
	/// that is marked as careless since you did not explicitly provide it. This matters for lossy conversions that are
	/// not injective. The result from this function returns a stateless_transcode_result as opposed to a
	/// pivotless_transcode_result because the state information is on the stack, and returning the state in
	/// those types by reference will result in references to memory that has already been cleaned up. If you need
	/// access to the state parameters, call the lower-level functionality with your own created states.
	template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding>
	constexpr auto transcode_into(
		TInput&& input, TFromEncoding&& __from_encoding, TOutput&& output, TToEncoding&& __to_encoding) {
		Handlers::DefaultHandler __handler {};

		return ::transcode_into(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TOutput>(output),
			std::forward<TToEncoding>(__to_encoding), __handler);
	}

	
	/// @brief Converts the code units of the given input view through the
	///        from encoding to code units of the to encoding into the output
	///        view.
	/// @param[in]     input An input_view to read code units from and use in the decode operation that will
	/// produce intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to encode the intermediate code points into the
	/// final code units.
	/// @param[in]     output An output_view to write code units to as the result of the encode operation from the
	/// intermediate code points.
	///
	/// @remarks This function creates both: a `from_error_handler` using a Handlers::DefaultHandler that is
	/// marked as careless to pass to the next function overload; and, a `from_encoding` to interpret the `input`
	/// by checking the `input` 's `value_type.` This matters for lossy conversions that are not injective. The
	/// result from this function returns a stateless_transcode_result as opposed to a
	/// pivotless_transcode_result because the state information is on the stack, and returning the state in
	/// those types by reference will result in references to memory that has already been cleaned up. If you need
	/// access to the state parameters, call the lower-level functionality with your own created states.
	template <typename TInput, typename TToEncoding, typename TOutput>
	constexpr auto transcode_into(TInput&& input, TToEncoding&& __to_encoding, TOutput&& output) {
		using TUInput        = remove_cvref_t<TInput>;
		using _UFromEncoding = default_code_unit_encoding_t<ranges::rangeTValue_type_t<TUInput>>;

		_UFromEncoding __from_encoding {};
		Handlers::DefaultHandler __handler {};

		return ::transcode_into(std::forward<TInput>(input), __from_encoding,
			std::forward<TOutput>(output), std::forward<TToEncoding>(__to_encoding), __handler);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding for the output, which is then returned in a result structure with additional information about
	/// success.
	///
	/// @tparam _OutputContainer The container to default-construct and serialize data into. Typically, a @c
	/// std::basic_string or a `std::vector` of some sort.
	///
	/// @param[in]     input An input_view to read code units from and use in the decode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to decode the input's code units into
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to encode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handler for the `__from_encoding` 's decode step.
	/// @param[in]     __to_error_handler The error handler for the `__to_encoding` 's encode step.
	/// @param[in,out] fromState A reference to the associated state for the `__from_encoding` 's decode step.
	/// @param[in,out] toState A reference to the associated state for the `__to_encoding` 's encode step.
	/// @param[in, out] pivot A reference to a descriptor of a (potentially usable) range as the intermediate pivot,
	/// usually a range of contiguous data from a span provided by the implementation but can be passed in here by the
	/// user.
	///
	/// @returns A transcode_result object that contains references to `fromState` and @p
	/// toState and an `output` parameter that contains the `_OutputContainer` specified. If the container has a
	/// `container.reserve` function, it is and some multiple of the input's size is used to pre-size the container,
	/// to aid with `push_back` / `insert` reallocation pains.
	template <typename _OutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler, typename TFromState, typename TToState, typename TPivot>
	constexpr auto transcode_to(TInput&& input, TFromEncoding&& __from_encoding, TToEncoding&& __to_encoding,
		_FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler, TFromState& fromState,
		TToState& toState, TPivot&& pivot) {
		using _UToEncoding              = remove_cvref_t<TToEncoding>;
		using _UOutputContainer         = remove_cvref_t<_OutputContainer>;
		using _OutputCodeUnit           = code_unit_t<_UToEncoding>;
		constexpr bool _IsVoidContainer = Void<_UOutputContainer>;
		constexpr bool _IsStringable
			= (is_char_traitable_v<_OutputCodeUnit> || IsUnicodeCodePoint<_OutputCodeUnit>);
		if constexpr (_IsVoidContainer && _IsStringable) {
			// prevent instantiation errors with basic_string by boxing it inside of an "if constexpr"
			using _RealOutputContainer = std::basic_string<_OutputCodeUnit>;
			return Impl::__transcode_dispatch<false, false, _RealOutputContainer>(
				std::forward<TInput>(input), std::forward<TFromEncoding>(__from_encoding),
				std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
				std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState, pivot);
		}
		else {
			using _RealOutputContainer
				= conditional_t<_IsVoidContainer, std::vector<_OutputCodeUnit>, _OutputContainer>;
			return Impl::__transcode_dispatch<false, false, _RealOutputContainer>(
				std::forward<TInput>(input), std::forward<TFromEncoding>(__from_encoding),
				std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
				std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState, pivot);
		}
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding for the output, which is then returned in a result structure with additional information about
	/// success.
	///
	/// @tparam _OutputContainer The container to default-construct and serialize data into. Typically, a @c
	/// std::basic_string or a `std::vector` of some sort.
	///
	/// @param[in]     input An input_view to read code units from and use in the decode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to decode the input's code units into
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to encode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handler for the `__from_encoding` 's decode step.
	/// @param[in]     __to_error_handler The error handler for the `__to_encoding` 's encode step.
	/// @param[in,out] fromState A reference to the associated state for the `__from_encoding` 's decode step.
	/// @param[in,out] toState A reference to the associated state for the `__to_encoding` 's encode step.
	///
	/// @returns A pivotless_transcode_result object that contains references to `fromState` and @p
	/// toState and an `output` parameter that contains the `_OutputContainer` specified. If the container has a
	/// `container.reserve` function, it is and some multiple of the input's size is used to pre-size the container,
	/// to aid with `push_back` / `insert` reallocation pains.
	template <typename _OutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler, typename TFromState, typename TToState>
	constexpr auto transcode_to(TInput&& input, TFromEncoding&& __from_encoding, TToEncoding&& __to_encoding,
		_FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler, TFromState& fromState,
		TToState& toState) {
		using _UFromEncoding = remove_cvref_t<TFromEncoding>;
		using TCodePoint     = code_point_t<_UFromEncoding>;
		using _PivotRange    = Krys::Ranges::subrange<TCodePoint*>;

		constexpr std::size_t __pivot_buffer_buffer_max
			= ZTD_TEXT_INTERMEDIATE_TRANSCODE_BUFFER_SIZE_I_(code_point_t<_UFromEncoding>)
			     < MaxCodePoints<_UFromEncoding>
			? MaxCodePoints<_UFromEncoding>
			: ZTD_TEXT_INTERMEDIATE_TRANSCODE_BUFFER_SIZE_I_(code_point_t<_UFromEncoding>);

		TCodePoint __pivot_buffer[__pivot_buffer_buffer_max] {};
		_PivotRange pivot(__pivot_buffer);
		return ::transcode_to<_OutputContainer>(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TToEncoding>(__to_encoding),
			std::forward<_FromErrorHandler>(__from_error_handler),
			std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState, pivot);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding for the output, which is then returned in a result structure with additional information about
	/// success.
	///
	/// @tparam _OutputContainer The container to default-construct and serialize data into. Typically, a @c
	/// std::basic_string or a `std::vector` of some sort.
	///
	/// @param[in]     input An input_view to read code units from and use in the decode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to decode the input's code units into
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to encode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handler for the `__from_encoding` 's decode step.
	/// @param[in]     __to_error_handler The error handler for the `__to_encoding` 's encode step.
	/// @param[in,out] fromState A reference to the associated state for the `__from_encoding` 's decode step.
	///
	/// @returns A stateless_transcode_result object that contains references to an `container.Output`
	/// parameter that contains the `_OutputContainer` specified.
	///
	/// @remarks A default state for the encode step of the operation is create using Krys::CreateEncodeState. The
	/// return type is stateless since both states must be passed in. If you want to have access to the states, create
	/// both of them yourself and pass them into a lower-level function that accepts those parameters.
	template <typename _OutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler, typename TFromState>
	constexpr auto transcode_to(TInput&& input, TFromEncoding&& __from_encoding, TToEncoding&& __to_encoding,
		_FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler, TFromState& fromState) {
		using _UToEncoding = remove_cvref_t<TToEncoding>;
		using TToState     = EncodeState<_UToEncoding>;

		TToState toState = ::Krys::CreateEncodeState(__to_encoding);

		return ::transcode_to<_OutputContainer>(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TToEncoding>(__to_encoding),
			std::forward<_FromErrorHandler>(__from_error_handler),
			std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding for the output, which is then returned in a result structure with additional information about
	/// success.
	///
	/// @tparam _OutputContainer The container to default-construct and serialize data into. Typically, a @c
	/// std::basic_string or a `std::vector` of some sort.
	///
	/// @param[in]     input An input_view to read code units from and use in the decode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to decode the input's code units into
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to encode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handler for the `__from_encoding` 's decode step.
	/// @param[in]     __to_error_handler The error handler for the `__to_encoding` 's encode step.
	///
	/// @returns A stateless_transcode_result object that contains references to an `container.Output`
	/// parameter that contains the `_OutputContainer` specified.
	///
	/// @remarks A default state for the decode step of the operation is create using CreateDecodeState. The
	/// return type is stateless since both states must be passed in. If you want to have access to the states, create
	/// both of them yourself and pass them into a lower-level function that accepts those parameters.
	template <typename _OutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler>
	constexpr auto transcode_to(TInput&& input, TFromEncoding&& __from_encoding, TToEncoding&& __to_encoding,
		_FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler) {
		using _UFromEncoding = remove_cvref_t<TFromEncoding>;
		using TFromState     = DecodeState<_UFromEncoding>;

		TFromState fromState = ::CreateDecodeState(__from_encoding);

		return ::transcode_to<_OutputContainer>(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TToEncoding>(__to_encoding),
			std::forward<_FromErrorHandler>(__from_error_handler),
			std::forward<_ToErrorHandler>(__to_error_handler), fromState);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding for the output, which is then returned in a result structure with additional information about
	/// success.
	///
	/// @tparam _OutputContainer The container to default-construct and serialize data into. Typically, a @c
	/// std::basic_string or a `std::vector` of some sort.
	///
	/// @param[in]     input An input_view to read code units from and use in the decode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to decode the input's code units into
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to encode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handler for the `__from_encoding` 's decode step.
	///
	/// @returns A stateless_transcode_result object that contains references to an `container.Output`
	/// parameter that contains the `_OutputContainer` specified.
	///
	/// @remarks A `to_error_handler` for the encode step of the operation is created using default construction of a
	/// Handlers::DefaultHandler that is marked as careless. The return type is stateless since both states must be
	/// passed in. If you want to have access to the states, create both of them yourself and pass them into a
	/// lower-level function that accepts those parameters.
	template <typename _OutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
		typename _FromErrorHandler>
	constexpr auto transcode_to(TInput&& input, TFromEncoding&& __from_encoding, TToEncoding&& __to_encoding,
		_FromErrorHandler&& __from_error_handler) {
		auto __handler = Impl::__duplicate_or_be_careless(__from_error_handler);

		return ::transcode_to<_OutputContainer>(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TToEncoding>(__to_encoding),
			std::forward<_FromErrorHandler>(__from_error_handler), __handler);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding for the output, which is then returned in a result structure with additional information about
	/// success.
	///
	/// @tparam _OutputContainer The container to default-construct and serialize data into. Typically, a @c
	/// std::basic_string or a `std::vector` of some sort.
	///
	/// @param[in]     input An input_view to read code units from and use in the decode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to decode the input's code units into
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to encode the intermediate code points into the
	/// final code units.
	///
	/// @returns A stateless_transcode_result object that contains references to an `container.Output`
	/// parameter that contains the `_OutputContainer` specified.
	///
	/// @remarks A `from_error_handler` for the encode step of the operation is created using default construction of
	/// a Handlers::DefaultHandler that is marked as careless. The return type is stateless since both states must
	/// be passed in. If you want to have access to the states, create both of them yourself and pass them into a
	/// lower-level function that accepts those parameters.
	template <typename _OutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding>
	constexpr auto transcode_to(TInput&& input, TFromEncoding&& __from_encoding, TToEncoding&& __to_encoding) {
		Handlers::DefaultHandler __handler {};

		return ::transcode_to<_OutputContainer>(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TToEncoding>(__to_encoding), __handler);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding for the output, which is then returned in a result structure with additional information about
	/// success.
	///
	/// @tparam _OutputContainer The container to default-construct and serialize data into. Typically, a @c
	/// std::basic_string or a `std::vector` of some sort.
	///
	/// @param[in]     input An input_view to read code units from and use in the decode operation that will
	/// produce intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to encode the intermediate code points into the
	/// final code units.
	///
	/// @returns A stateless_transcode_result object that contains references to an `container.Output`
	/// parameter that contains the `_OutputContainer` specified.
	///
	/// @remarks A `from_error_handler` for the encode step of the operation is created using default construction of
	/// a Handlers::DefaultHandler that is marked as careless. The return type is stateless since both states must
	/// be passed in. If you want to have access to the states, create both of them yourself and pass them into a
	/// lower-level function that accepts those parameters.
	template <typename _OutputContainer = void, typename TInput, typename TToEncoding>
	constexpr auto transcode_to(TInput&& input, TToEncoding&& __to_encoding) {
		using TUInput   = remove_cvref_t<TInput>;
		using TCodeUnit = ranges::rangeTValue_type_t<TUInput>;
#if ZTD_IS_ON(ZTD_STD_LIBRARY_IS_CONSTANT_EVALUATED)
		if (std::is_constant_evaluated()) {
			using _UFromEncoding = default_consteval_code_unit_encoding_t<TCodeUnit>;
			Handlers::DefaultHandler __handler {};
			_UFromEncoding __from_encoding {};
			return ::transcode_to<_OutputContainer>(std::forward<TInput>(input), __from_encoding,
				std::forward<TToEncoding>(__to_encoding), __handler);
		}
		else
#endif
		{
			using _UFromEncoding = default_code_unit_encoding_t<TCodeUnit>;
			Handlers::DefaultHandler __handler {};
			_UFromEncoding __from_encoding {};
			return ::transcode_to<_OutputContainer>(std::forward<TInput>(input), __from_encoding,
				std::forward<TToEncoding>(__to_encoding), __handler);
		}
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding for the output, which is then returned in a result structure with additional information about
	/// success.
	///
	/// @tparam _OutputContainer The container to default-construct and serialize data into. Typically, a @c
	/// std::basic_string or a `std::vector` of some sort.
	///
	/// @param[in]     input An input_view to read code units from and use in the decode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to decode the input's code units into
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to encode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handler for the `__from_encoding` 's decode step.
	/// @param[in]     __to_error_handler The error handler for the `__to_encoding` 's encode step.
	/// @param[in,out] fromState A reference to the associated state for the `__from_encoding` 's decode step.
	/// @param[in,out] toState A reference to the associated state for the `__to_encoding` 's encode step.
	/// @param[in, out] pivot A reference to a descriptor of a (potentially usable) range as the intermediate pivot,
	/// usually a range of contiguous data from a span provided by the implementation but can be passed in here by the
	/// user.
	///
	/// @returns An `_OutputContainer` with the result, regardless of whether an error occurs or not. If you are
	/// looking for error information and not just a quick one-off conversion function, please use
	/// transcode_to or transcode_into_raw.
	template <typename _OutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler, typename TFromState, typename TToState, typename TPivot>
	constexpr auto transcode(TInput&& input, TFromEncoding&& __from_encoding, TToEncoding&& __to_encoding,
		_FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler, TFromState& fromState,
		TToState& toState, TPivot&& pivot) {
		using _UToEncoding              = remove_cvref_t<TToEncoding>;
		using _UOutputContainer         = remove_cvref_t<_OutputContainer>;
		using _OutputCodeUnit           = code_unit_t<_UToEncoding>;
		constexpr bool _IsVoidContainer = Void<_UOutputContainer>;
		constexpr bool _IsStringable
			= (is_char_traitable_v<_OutputCodeUnit> || IsUnicodeCodePoint<_OutputCodeUnit>);
		if constexpr (_IsVoidContainer && _IsStringable) {
			// prevent instantiation errors with basic_string by boxing it inside of an "if constexpr"
			using _RealOutputContainer = std::basic_string<_OutputCodeUnit>;
			return Impl::__transcode_dispatch<true, false, _RealOutputContainer>(
				std::forward<TInput>(input), std::forward<TFromEncoding>(__from_encoding),
				std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
				std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState, pivot);
		}
		else {
			using _RealOutputContainer
				= conditional_t<_IsVoidContainer, std::vector<_OutputCodeUnit>, _OutputContainer>;
			return Impl::__transcode_dispatch<true, false, _RealOutputContainer>(
				std::forward<TInput>(input), std::forward<TFromEncoding>(__from_encoding),
				std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
				std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState, pivot);
		}
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding for the output, which is then returned in a result structure with additional information about
	/// success.
	///
	/// @tparam _OutputContainer The container to default-construct and serialize data into. Typically, a @c
	/// std::basic_string or a `std::vector` of some sort.
	///
	/// @param[in]     input An input_view to read code units from and use in the decode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to decode the input's code units into
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to encode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handler for the `__from_encoding` 's decode step.
	/// @param[in]     __to_error_handler The error handler for the `__to_encoding` 's encode step.
	/// @param[in,out] fromState A reference to the associated state for the `__from_encoding` 's decode step.
	/// @param[in,out] toState A reference to the associated state for the `__to_encoding` 's encode step.
	///
	/// @returns An `_OutputContainer` with the result, regardless of whether an error occurs or not. If you are
	/// looking for error information and not just a quick one-off conversion function, please use
	/// transcode_to or transcode_into_raw.
	template <typename _OutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler, typename TFromState, typename TToState>
	constexpr auto transcode(TInput&& input, TFromEncoding&& __from_encoding, TToEncoding&& __to_encoding,
		_FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler, TFromState& fromState,
		TToState& toState) {
		using _UFromEncoding = remove_cvref_t<TFromEncoding>;
		using TCodePoint     = code_point_t<_UFromEncoding>;
		using _PivotRange    = Krys::Ranges::subrange<TCodePoint*>;

		constexpr std::size_t __pivot_buffer_buffer_max
			= ZTD_TEXT_INTERMEDIATE_TRANSCODE_BUFFER_SIZE_I_(code_point_t<_UFromEncoding>)
			     < MaxCodePoints<_UFromEncoding>
			? MaxCodePoints<_UFromEncoding>
			: ZTD_TEXT_INTERMEDIATE_TRANSCODE_BUFFER_SIZE_I_(code_point_t<_UFromEncoding>);

		TCodePoint __pivot_buffer[__pivot_buffer_buffer_max] {};
		_PivotRange pivot(__pivot_buffer);
		return ::transcode<_OutputContainer>(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TToEncoding>(__to_encoding),
			std::forward<_FromErrorHandler>(__from_error_handler),
			std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState, pivot);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding for the output, which is then returned in a result structure with additional information about
	/// success.
	///
	/// @tparam _OutputContainer The container to default-construct and serialize data into. Typically, a @c
	/// std::basic_string or a `std::vector` of some sort.
	///
	/// @param[in]     input An input_view to read code units from and use in the decode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to decode the input's code units into
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to encode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handler for the `__from_encoding` 's decode step.
	/// @param[in]     __to_error_handler The error handler for the `__to_encoding` 's encode step.
	/// @param[in,out] fromState A reference to the associated state for the `__from_encoding` 's decode step.
	///
	/// @returns An `_OutputContainer` with the result, regardless of whether an error occurs or not. If you are
	/// looking for error information and not just a quick one-off conversion function, please use
	/// transcode_to or transcode_into_raw.
	///
	/// @remarks This function creates an `ToState` for the encoding step of the operation using
	/// Krys::CreateEncodeState.
	template <typename _OutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler, typename TFromState>
	constexpr auto transcode(TInput&& input, TFromEncoding&& __from_encoding, TToEncoding&& __to_encoding,
		_FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler, TFromState& fromState) {
		using _UToEncoding = remove_cvref_t<TToEncoding>;
		using TToState     = EncodeState<_UToEncoding>;

		TToState toState = ::Krys::CreateEncodeState(__to_encoding);

		return ::transcode<_OutputContainer>(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TToEncoding>(__to_encoding),
			std::forward<_FromErrorHandler>(__from_error_handler),
			std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding for the output, which is then returned in a result structure with additional information about
	/// success.
	///
	/// @tparam _OutputContainer The container to default-construct and serialize data into. Typically, a @c
	/// std::basic_string or a `std::vector` of some sort.
	///
	/// @param[in]     input An input_view to read code units from and use in the decode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to decode the input's code units into
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to encode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handler for the `__from_encoding` 's decode step.
	/// @param[in]     __to_error_handler The error handler for the `__to_encoding` 's encode step.
	///
	/// @returns An `_OutputContainer` with the result, regardless of whether an error occurs or not. If you are
	/// looking for error information and not just a quick one-off conversion function, please use
	/// transcode_to or transcode_into_raw.
	///
	/// @remarks This function creates an `FromState` for the encoding step of the operation using
	/// CreateDecodeState.
	template <typename _OutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler>
	constexpr auto transcode(TInput&& input, TFromEncoding&& __from_encoding, TToEncoding&& __to_encoding,
		_FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler) {
		using _UFromEncoding = remove_cvref_t<TFromEncoding>;
		using TFromState     = DecodeState<_UFromEncoding>;

		TFromState fromState = ::CreateDecodeState(__from_encoding);

		return ::transcode<_OutputContainer>(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TToEncoding>(__to_encoding),
			std::forward<_FromErrorHandler>(__from_error_handler),
			std::forward<_ToErrorHandler>(__to_error_handler), fromState);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding for the output, which is then returned in a result structure with additional information about
	/// success.
	///
	/// @tparam _OutputContainer The container to default-construct and serialize data into. Typically, a @c
	/// std::basic_string or a `std::vector` of some sort.
	///
	/// @param[in]     input An input_view to read code units from and use in the decode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to decode the input's code units into
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to encode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handler for the `__from_encoding` 's decode step.
	///
	/// @returns An `_OutputContainer` with the result, regardless of whether an error occurs or not. If you are
	/// looking for error information and not just a quick one-off conversion function, please use
	/// transcode_to or transcode_into_raw.
	///
	/// @remarks This function creates a `to_error_handler` from a class like Handlers::DefaultHandler, but that
	/// is marked as careless since you did not explicitly provide it. This matters for lossy conversions that are not
	/// injective.
	template <typename _OutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
		typename _FromErrorHandler>
	constexpr auto transcode(TInput&& input, TFromEncoding&& __from_encoding, TToEncoding&& __to_encoding,
		_FromErrorHandler&& __from_error_handler) {
		auto __handler = Impl::__duplicate_or_be_careless(__from_error_handler);

		return ::transcode<_OutputContainer>(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TToEncoding>(__to_encoding),
			std::forward<_FromErrorHandler>(__from_error_handler), __handler);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding for the output, which is then returned in a result structure with additional information about
	/// success.
	///
	/// @tparam _OutputContainer The container to default-construct and serialize data into. Typically, a @c
	/// std::basic_string or a `std::vector` of some sort.
	///
	/// @param[in]     input An input_view to read code units from and use in the decode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to decode the input's code units into
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to encode the intermediate code points into the
	/// final code units.
	///
	/// @returns An `_OutputContainer` with the result, regardless of whether an error occurs or not. If you are
	/// looking for error information and not just a quick one-off conversion function, please use
	/// transcode_to or transcode_into_raw.
	///
	/// @remarks This function creates a `from_error_handler` from a class like Handlers::DefaultHandler, but that
	/// is marked as careless since you did not explicitly provide it. This matters for lossy conversions that are not
	/// injective.
	template <typename _OutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding>
	constexpr auto transcode(TInput&& input, TFromEncoding&& __from_encoding, TToEncoding&& __to_encoding) {
		Handlers::DefaultHandler __handler {};

		return ::transcode<_OutputContainer>(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TToEncoding>(__to_encoding), __handler);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding for the output, which is then returned in a result structure with additional information about
	/// success.
	///
	/// @tparam _OutputContainer The container to default-construct and serialize data into. Typically, a @c
	/// std::basic_string or a `std::vector` of some sort.
	///
	/// @param[in]     input An input_view to read code units from and use in the decode operation that will
	/// produce intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to encode the intermediate code points into the
	/// final code units.
	///
	/// @returns An `_OutputContainer` with the result, regardless of whether an error occurs or not. If you are
	/// looking for error information and not just a quick one-off conversion function, please use
	/// transcode_to or transcode_into_raw.
	///
	/// @remarks This function creates both: a `from_error_handler` from a class like Handlers::DefaultHandler,
	/// but that is marked as careless since you did not explicitly provide it; and, a `from_encoding` derived from
	/// the `input`'s `value_type.` The careless marking matters for lossy conversions that are not injective.
	template <typename _OutputContainer = void, typename TInput, typename TToEncoding>
	constexpr auto transcode(TInput&& input, TToEncoding&& __to_encoding) {
		using TUInput   = remove_cvref_t<TInput>;
		using TCodeUnit = ranges::rangeTValue_type_t<TUInput>;
#if ZTD_IS_ON(ZTD_STD_LIBRARY_IS_CONSTANT_EVALUATED)
		if (std::is_constant_evaluated()) {
			using _UFromEncoding = default_consteval_code_unit_encoding_t<TCodeUnit>;
			_UFromEncoding __from_encoding {};
			return ::transcode<_OutputContainer>(
				std::forward<TInput>(input), __from_encoding, std::forward<TToEncoding>(__to_encoding));
		}
		else
#endif
		{
			using _UFromEncoding = default_code_unit_encoding_t<TCodeUnit>;

			_UFromEncoding __from_encoding {};
			return ::transcode<_OutputContainer>(
				std::forward<TInput>(input), __from_encoding, std::forward<TToEncoding>(__to_encoding));
		}
	}

	
	/// @}

	
}



#endif
