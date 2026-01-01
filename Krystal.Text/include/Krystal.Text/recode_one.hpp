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

#ifndef ZTD_TEXT_RECODE_ONE_HPP
#define ZTD_TEXT_RECODE_ONE_HPP



#include <ztd/text/EncodeOne.hpp>
#include <ztd/text/DecodeOne.hpp>
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include <ztd/text/max_units.hpp>
#include <ztd/text/error_handler.hpp>
#include "Krystal.Text/State.hpp"
#include <ztd/text/default_encoding.hpp>
#include <ztd/text/RecodeResult.hpp>
#include "Krystal.Text/IsUnicodeCodePoint.hpp"
#include <ztd/text/is_redundant.hpp>
#include "Krystal.Text/IsIgnorableErrorHandler.hpp"
#include <ztd/text/propagate_error.hpp>
#include <ztd/text/detail/forward_if_move_only.hpp>
#include <ztd/text/detail/is_lossless.hpp>
#include <ztd/text/detail/encoding_range.hpp>
#include <ztd/text/detail/recode_extension_points.hpp>
#include "Krystal.Text/Impl/SpanReconstruct.hpp"
#include <ztd/text/detail/progress_handler.hpp>

#include <ztd/ranges/unbounded.hpp>
#include <ztd/ranges/detail/insert_bulk.hpp>
#include "Krystal.Lib/Ranges/Algorithm.hpp"
#include <ztd/ranges/save_range.hpp>
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Text/CharTraits.hpp"
#include "Krystal.Lib/Utils/Tag.hpp"
#include <ztd/inline_containers.hpp>



namespace Krys {
	

	
	/// @addtogroup ztd_text_recode_one recode_one[_into/_to]
	///
	/// @brief These functions convert from a view of input code units into a view of output code units
	/// (typically, through an intermediary code point type that is similar between the two) using either the
	/// inferred or specified encodings. If no error handler is provided, the equivalent of the
	/// Handlers::DefaultHandler is used by default, but it is marked as careless. If no associated state is
	/// provided for either the "to" or "from" encodings, one will be created with automatic storage duration (as
	/// a "stack" variable) for the provided encoding.
	/// @{

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding into the output view. nly one.
	///
	/// @param[in]     input An input_view to read code units from and use in the encode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to encode the input's code units into
	/// intermediate code points.
	/// @param[in]     output An output_view to write code units to as the result of the decode operation from
	/// the intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to decode the intermediate code points into
	/// the final code units.
	/// @param[in]     __from_error_handler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in]     __to_error_handler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in,out] fromState A reference to the associated state for the `__from_encoding` 's encode step.
	/// @param[in,out] toState A reference to the associated state for the `__to_encoding` 's decode step.
	/// @param[in, out] pivot A reference to a descriptor of a (potentially usable) range as the intermediate pivot,
	/// usually a range of contiguous data from a span provided by the implementation but can be passed in here by the
	/// user.
	///
	/// @result A RecodeResult object that contains references to `fromState` and @p
	/// toState.
	///
	/// @remark This function detects whether or not the ADL extension point `TextRecode` can be called with
	/// the provided parameters. If so, it will use that ADL extension point over the default implementation.
	/// Otherwise, it will loop over the two encodings and attempt to recode by first decoding the input code
	/// units to code points, then encoding the intermediate code points to the desired, output code units.
	template <typename TInput, typename TOutput, typename TFromEncoding, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler, typename TFromState, typename TToState, typename TPivot>
	constexpr auto basic_recode_one_into_raw(TInput&& input, TFromEncoding&& __from_encoding, TOutput&& output,
		TToEncoding&& __to_encoding, _FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler,
		TFromState& fromState, TToState& toState, TPivot&& pivot) {
		using _CVFromEncoding                  = remove_ref_t<TFromEncoding>;
		using _CVToEncoding                    = remove_ref_t<TToEncoding>;
		using _CVFromErrorHandler              = remove_ref_t<_FromErrorHandler>;
		using _CVToErrorHandler                = remove_ref_t<_ToErrorHandler>;
		using _UFromErrorHandler               = remove_cvref_t<_FromErrorHandler>;
		using _UToErrorHandler                 = remove_cvref_t<_ToErrorHandler>;
		constexpr bool _IsFromProgressHandler  = Impl::IsProgressHandler<_UFromErrorHandler>;
		constexpr bool _IsToProgressHandler    = Impl::IsProgressHandler<_UToErrorHandler>;
		constexpr bool _IsBothProgressHandlers = _IsFromProgressHandler && _IsToProgressHandler;
		constexpr bool _FromAssumeValid        = IsIgnorableErrorHandler<_UFromErrorHandler>;
		constexpr bool _ToAssumeValid          = IsIgnorableErrorHandler<_UToErrorHandler>;
		using _FromProgressHandler             = conditional_t<_IsFromProgressHandler, _CVFromErrorHandler&,
			            Impl::ProgressHandler<_FromAssumeValid, _CVFromEncoding>>;
		using _ToProgressHandler               = conditional_t<_IsToProgressHandler, _CVToErrorHandler&,
			              Impl::ProgressHandler<_ToAssumeValid, _CVToEncoding>>;
		using _FromProgressHandlerRef
			= conditional_t<_IsFromProgressHandler, _CVFromErrorHandler&, _FromProgressHandler&>;
		using _ToProgressHandlerRef
			= conditional_t<_IsToProgressHandler, _CVToErrorHandler&, _ToProgressHandler&>;
		using _WorkingOutput       = Krys::Ranges::subrange_for_t<TOutput>;
		using _IntermediateResult  = decltype(std::forward<TFromEncoding>(__from_encoding)
                    .EncodeOne(std::forward<TInput>(input), std::forward<TPivot>(pivot),
			           std::declval<_FromProgressHandlerRef>(), fromState));
		using _ResultPivot         = decltype(std::declval<_IntermediateResult>().Output);
		using _EndResult           = decltype(std::forward<TToEncoding>(__to_encoding)
                    .DecodeOne(std::declval<_ResultPivot>(), std::declval<_WorkingOutput>(),
			                    std::declval<_ToProgressHandlerRef>(), toState));
		using _WorkingIntermediate = decltype(std::declval<_EndResult>().Input);
		using _ResultInput         = decltype(std::declval<_IntermediateResult>().Input);
		using _ResultOutput        = decltype(std::declval<_EndResult>().Output);
		using TResult              = RecodeResult<_ResultInput, _ResultOutput, TFromState, TToState, _ResultPivot>;

		static_assert(Impl::__is_encode_lossless_or_deliberate_v<remove_cvref_t<TFromEncoding>,
			              remove_cvref_t<_FromErrorHandler>>,
			ZTD_TEXT_LOSSY_RECODE_ENCODE_MESSAGE_I_);
		static_assert(Impl::__is_decode_lossless_or_deliberate_v<remove_cvref_t<TToEncoding>,
			              remove_cvref_t<_ToErrorHandler>>,
			ZTD_TEXT_LOSSY_RECODE_DECODE_MESSAGE_I_);


		auto __saved_input = Krys::Ranges::save_range(input);
		_WorkingOutput __working_output(std::forward<TOutput>(output));
		_FromProgressHandler __from_intermediate_handler(__from_error_handler);
		_ToProgressHandler __to_intermediate_handler(__to_error_handler);
		auto intermediateResult
			= std::forward<TFromEncoding>(__from_encoding)
			       .EncodeOne(std::forward<TInput>(input), std::forward<TPivot>(pivot),
			            __from_intermediate_handler, fromState);
		if (intermediateResult.ErrorCode != EncodingError::OK) {
			if constexpr (_IsBothProgressHandlers) {
				return TResult(std::move(intermediateResult.Input), std::move(__working_output),
					intermediateResult.State, toState, intermediateResult.ErrorCode,
					intermediateResult.ErrorCount, std::move(intermediateResult.Output),
					intermediateResult.ErrorCode, intermediateResult.ErrorCount);
			}
			else {
				return ::propagate_recode_encode_error_with<TResult>(
					Krys::Ranges::restore_range(
					     std::move(__saved_input), std::move(intermediateResult.Input)),
					std::move(__working_output), __from_encoding, __to_encoding,
					std::move(intermediateResult), __from_error_handler, __to_error_handler, toState,
					__from_intermediate_handler.CodePointsProgress(),
					__from_intermediate_handler.CodeUnitsProgress(),
					__to_intermediate_handler.CodeUnitsProgress(),
					__to_intermediate_handler.CodePointsProgress());
			}
		}
		using _SpanTy               = Span<const Krys::Ranges::rangeTValue_type_t<_WorkingIntermediate>>;
		auto __working_intermediate = Krys::Ranges::cascading_reconstruct<_SpanTy, _WorkingIntermediate>(
			Krys::Ranges::cbegin(pivot), Krys::Ranges::cbegin(intermediateResult.Output));
		std::size_t errorCount = intermediateResult.ErrorCount;
		for (;;) {
			auto __end_result = std::forward<TToEncoding>(__to_encoding)
				                    .DecodeOne(std::move(__working_intermediate), std::move(__working_output),
				                         __to_intermediate_handler, toState);
			errorCount += __end_result.ErrorCount;
			if (__end_result.ErrorCode != EncodingError::OK) {
				if constexpr (_IsBothProgressHandlers) {
					return TResult(std::move(intermediateResult.Input), std::move(__end_result.Output),
						intermediateResult.State, __end_result.State, __end_result.ErrorCode, errorCount,
						std::move(intermediateResult.Output), intermediateResult.ErrorCode,
						intermediateResult.ErrorCount);
				}
				else {
					return ::propagate_recode_decode_error_with<TResult>(
						Krys::Ranges::restore_range(
						     std::move(__saved_input), std::move(intermediateResult.Input)),
						__to_encoding, std::move(intermediateResult), std::move(__end_result),
						__to_error_handler, __to_intermediate_handler.CodeUnitsProgress(),
						__to_intermediate_handler.CodePointsProgress());
				}
			}
			if (Krys::Ranges::empty(__end_result.Input)) {
				if constexpr (_IsBothProgressHandlers) {
					return TResult(std::move(intermediateResult.Input), std::move(__end_result.Output),
						intermediateResult.State, __end_result.State, __end_result.ErrorCode, errorCount,
						std::move(intermediateResult.Output), intermediateResult.ErrorCode,
						intermediateResult.ErrorCount);
				}
				else {
					return TResult(std::move(intermediateResult.Input), std::move(__end_result.Output),
						intermediateResult.State, __end_result.State, __end_result.ErrorCode, errorCount,
						std::move(intermediateResult.Output), intermediateResult.ErrorCode,
						intermediateResult.ErrorCount);
				}
			}
			__working_intermediate = std::move(__end_result.Input);
			__working_output       = std::move(__end_result.Output);
		}
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding into the output view. Only performs one distinct unit of encoding.
	///
	/// @param[in]     input An input_view to read code units from and use in the encode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to encode the input's code units into
	/// intermediate code points.
	/// @param[in]     output An output_view to write code units to as the result of the decode operation from
	/// the intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to decode the intermediate code points into
	/// the final code units.
	/// @param[in]     __from_error_handler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in]     __to_error_handler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in,out] fromState A reference to the associated state for the `__from_encoding` 's encode step.
	/// @param[in,out] toState A reference to the associated state for the `__to_encoding` 's decode step.
	/// @param[in, out] pivot A reference to a descriptor of a (potentially usable) range as the intermediate pivot,
	/// usually a range of contiguous data from a span provided by the implementation but can be passed in here by the
	/// user.
	///
	/// @result A RecodeResult object that contains references to `fromState` and @p
	/// toState.
	///
	/// @remark This function detects whether or not the ADL extension point `TextRecodeOne` can be called
	/// with the provided parameters. If so, it will use that ADL extension point over the default implementation.
	/// Otherwise, it will loop over the two encodings and attempt to recode by first decoding the input code
	/// units to code points, then encoding the intermediate code points to the desired, output code units.
	template <typename TInput, typename TOutput, typename TFromEncoding, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler, typename TFromState, typename TToState, typename TPivot>
	constexpr auto recode_one_into_raw(TInput&& input, TFromEncoding&& __from_encoding, TOutput&& output,
		TToEncoding&& __to_encoding, _FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler,
		TFromState& fromState, TToState& toState, TPivot&& pivot) {
		if constexpr (is_detected_v<Impl::HasTextRecodeOne, TInput, TOutput, TFromEncoding,
			              TToEncoding, _FromErrorHandler, _ToErrorHandler, TFromState, TToState, TPivot>) {
			return TextRecodeOne(
				Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>> {},
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
			using TUInput            = remove_cvref_t<TInput>;
			using _UOutput           = remove_cvref_t<TOutput>;
			using _UPivot            = remove_cvref_t<TPivot>;
			if constexpr (is_encode_redundant_v<_UFromEncoding, _UToEncoding> // cf
				&& IsDecodeRedundant<_UFromEncoding, _UToEncoding>       // cf
				&& IsIgnorableErrorHandler<_UFromErrorHandler>          // cf
				&& IsIgnorableErrorHandler<_UToErrorHandler>) {
				// we can simply copy from the input to the output, no questions asked!
				(void)__from_encoding;
				(void)__to_encoding;
				(void)__from_error_handler;
				(void)__to_error_handler;
				(void)fromState;
				(void)toState;
				(void)pivot;
				auto result = Krys::Ranges::__rng_detail::__copy(
					Krys::Ranges::cbegin(std::forward<TInput>(input)), Krys::Ranges::cend(input),
					Krys::Ranges::begin(std::forward<TOutput>(output)), Krys::Ranges::end(output));
				using TResult = Impl::reconstruct_pivot_recode_result_t<TUInput, _UOutput, TFromState,
					TToState, _UPivot>;
				return TResult(Krys::Ranges::Reconstruct(std::in_place_type<TUInput>, std::move(result.in)),
					Krys::Ranges::Reconstruct(std::in_place_type<_UOutput>, std::move(result.out)), fromState,
					toState, EncodingError::OK, 0, std::forward<TPivot>(pivot), EncodingError::OK, 0);
			}
			else if constexpr (is_detected_v<Impl::HasInternalTextRecodeOne, TInput,
				                   TFromEncoding, TOutput, TToEncoding, _FromErrorHandler, _ToErrorHandler,
				                   TFromState, TToState, TPivot>) {
				return InternalTextRecodeOne(
					Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>> {},
					std::forward<TInput>(input), std::forward<TFromEncoding>(__from_encoding),
					std::forward<TOutput>(output), std::forward<TToEncoding>(__to_encoding),
					std::forward<_FromErrorHandler>(__from_error_handler),
					std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState,
					std::forward<TPivot>(pivot));
			}
			else {
				return basic_recode_one_into_raw(std::forward<TInput>(input),
					std::forward<TFromEncoding>(__from_encoding), std::forward<TOutput>(output),
					std::forward<TToEncoding>(__to_encoding),
					std::forward<_FromErrorHandler>(__from_error_handler),
					std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState,
					std::forward<TPivot>(pivot));
			}
		}
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding into the output view. Only performs one distinct unit of encoding.
	///
	/// @param[in]     input An input_view to read code units from and use in the encode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to encode the input's code units into
	/// intermediate code points.
	/// @param[in]     output An output_view to write code units to as the result of the decode operation from
	/// the intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to decode the intermediate code points into
	/// the final code units.
	/// @param[in]     __from_error_handler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in]     __to_error_handler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in,out] fromState A reference to the associated state for the `__from_encoding` 's encode step.
	/// @param[in,out] toState A reference to the associated state for the `__to_encoding` 's decode step.
	///
	/// @result A RecodeResult object that contains references to `fromState` and @p
	/// toState.
	///
	/// @remark This function detects whether or not the ADL extension point `TextRecodeOne` can be called
	/// with the provided parameters. If so, it will use that ADL extension point over the default implementation.
	/// Otherwise, it will loop over the two encodings and attempt to recode by first decoding the input code
	/// units to code points, then encoding the intermediate code points to the desired, output code units.
	template <typename TInput, typename TOutput, typename TFromEncoding, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler, typename TFromState, typename TToState>
	constexpr auto recode_one_into_raw(TInput&& input, TFromEncoding&& __from_encoding, TOutput&& output,
		TToEncoding&& __to_encoding, _FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler,
		TFromState& fromState, TToState& toState) {
		using _UFromEncoding                              = remove_cvref_t<TFromEncoding>;
		using TCodeUnit                                   = code_unit_t<_UFromEncoding>;
		using TPivot                                      = Span<TCodeUnit>;
		constexpr std::size_t __intermediate_buffer_max = MaxCodeUnits<_UFromEncoding> * 2;

		TCodeUnit intermediate[__intermediate_buffer_max] {};
		TPivot pivot(intermediate);
		return ::recode_one_into_raw(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TOutput>(output),
			std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
			std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState, pivot);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding into the output view.
	///
	/// @param[in]     input An input_view to read code units from and use in the encode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to encode the input's code units into
	/// intermediate code points.
	/// @param[in]     output An output_view to write code units to as the result of the decode operation from the
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to decode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handler for the `__from_encoding` 's encode step.
	/// @param[in]     __to_error_handler The error handler for the `__to_encoding` 's decode step.
	/// @param[in,out] fromState A reference to the associated state for the `__from_encoding` 's encode step.
	///
	/// @result A StatelessRecodeResult object.
	///
	/// @remarks This function calls the base reference, the recode_one_into_raw after creating a
	/// `ToState` from CreateDecodeState. The result from this function returns a
	/// StatelessRecodeResult as opposed to a RecodeResult because the state
	/// information is on the stack, and returning the state in those types by reference will result in references to
	/// memory that has already been cleaned up. If you need access to the state parameters, call the lower-level
	/// functionality with your own created states.
	template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler, typename TFromState>
	constexpr auto recode_one_into_raw(TInput&& input, TFromEncoding&& __from_encoding, TOutput&& output,
		TToEncoding&& __to_encoding, _FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler,
		TFromState& fromState) {
		using _UToEncoding = remove_cvref_t<TToEncoding>;
		using TToState     = DecodeState<_UToEncoding>;

		TToState toState = ::CreateDecodeState(__to_encoding);

		auto __stateful_result = ::recode_one_into_raw(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TOutput>(output),
			std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
			std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState);

		return Impl::SliceToStateless(std::move(__stateful_result));
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding into the output view.
	/// @param[in]     input An input_view to read code units from and use in the encode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to encode the input's code units into
	/// intermediate code points.
	/// @param[in]     output An output_view to write code units to as the result of the decode operation from the
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to decode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handler for the `__from_encoding` 's encode step.
	/// @param[in]     __to_error_handler The error handler for the `__to_encoding` 's decode step.
	///
	/// @remarks This function creates a encode state `FromState` by calling Krys::CreateEncodeState. The
	/// result from this function returns a StatelessRecodeResult as opposed to a
	/// RecodeResult because the state information is on the stack, and returning the state in
	/// those types by reference will result in references to memory that has already been cleaned up. If you need
	/// access to the state parameters, call the lower-level functionality with your own created states.
	template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler>
	constexpr auto recode_one_into_raw(TInput&& input, TFromEncoding&& __from_encoding, TOutput&& output,
		TToEncoding&& __to_encoding, _FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler) {
		using _UFromEncoding = remove_cvref_t<TFromEncoding>;
		using TFromState     = EncodeState<_UFromEncoding>;

		TFromState fromState = ::Krys::CreateEncodeState(__from_encoding);

		return ::recode_one_into_raw(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TOutput>(output),
			std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
			std::forward<_ToErrorHandler>(__to_error_handler), fromState);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding into the output view.
	/// @param[in]     input An input_view to read code units from and use in the encode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to encode the input's code units into
	/// intermediate code points.
	/// @param[in]     output An output_view to write code units to as the result of the decode operation from the
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to decode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handler for the `__from_encoding` 's encode step.
	///
	/// @remarks This function creates an `to_error_handler` from a class like Handlers::DefaultHandler, but that
	/// is marked as careless since you did not explicitly provide it. This matters for lossy conversions that are not
	/// injective. The result from this function returns a StatelessRecodeResult as opposed to a
	/// RecodeResult because the state information is on the stack, and returning the state in
	/// those types by reference will result in references to memory that has already been cleaned up. If you need
	/// access to the state parameters, call the lower-level functionality with your own created states.
	template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
		typename _FromErrorHandler>
	constexpr auto recode_one_into_raw(TInput&& input, TFromEncoding&& __from_encoding, TOutput&& output,
		TToEncoding&& __to_encoding, _FromErrorHandler&& __from_error_handler) {
		auto __handler = Impl::__duplicate_or_be_careless(__from_error_handler);

		return ::recode_one_into_raw(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TOutput>(output),
			std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
			__handler);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding into the output view.
	/// @param[in]     input An input_view to read code units from and use in the encode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to encode the input's code units into
	/// intermediate code points.
	/// @param[in]     output An output_view to write code units to as the result of the decode operation from the
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to decode the intermediate code points into the
	/// final code units.
	///
	/// @remarks This function creates an `from_error_handler` from a class like Handlers::DefaultHandler, but
	/// that is marked as careless since you did not explicitly provide it. This matters for lossy conversions that are
	/// not injective. The result from this function returns a StatelessRecodeResult as opposed to a
	/// RecodeResult because the state information is on the stack, and returning the state in
	/// those types by reference will result in references to memory that has already been cleaned up. If you need
	/// access to the state parameters, call the lower-level functionality with your own created states.
	template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding>
	constexpr auto recode_one_into_raw(
		TInput&& input, TFromEncoding&& __from_encoding, TOutput&& output, TToEncoding&& __to_encoding) {
		Handlers::DefaultHandler __handler {};

		return ::recode_one_into_raw(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TOutput>(output),
			std::forward<TToEncoding>(__to_encoding), __handler);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding into the output view.
	/// @param[in]     input An input_view to read code units from and use in the encode operation that will
	/// produce intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to decode the intermediate code points into the
	/// final code units.
	/// @param[in]     output An output_view to write code units to as the result of the decode operation from the
	/// intermediate code points.
	///
	/// @remarks This function creates both: a `from_error_handler` using a Handlers::DefaultHandler that is
	/// marked as careless to pass to the next function overload; and, a `from_encoding` to interpret the `input`
	/// by checking the `input` 's `value_type.` This matters for lossy conversions that are not injective. The
	/// result from this function returns a StatelessRecodeResult as opposed to a
	/// RecodeResult because the state information is on the stack, and returning the state in
	/// those types by reference will result in references to memory that has already been cleaned up. If you need
	/// access to the state parameters, call the lower-level functionality with your own created states.
	template <typename TInput, typename TToEncoding, typename TOutput>
	constexpr auto recode_one_into_raw(TInput&& input, TToEncoding&& __to_encoding, TOutput&& output) {
		using TUInput        = remove_cvref_t<TInput>;
		using _UFromEncoding = default_code_point_encoding_t<ranges::rangeTValue_type_t<TUInput>>;

		_UFromEncoding __from_encoding {};
		Handlers::DefaultHandler __handler {};

		return ::recode_one_into_raw(std::forward<TInput>(input), __from_encoding,
			std::forward<TOutput>(output), std::forward<TToEncoding>(__to_encoding), __handler);
	}

	namespace Impl {
		template <typename _OutputContainer, typename TInput, typename TFromEncoding, typename TToEncoding,
			typename _FromErrorHandler, typename _ToErrorHandler, typename TFromState, typename TToState,
			typename TPivot>
		constexpr auto __intermediate_recode_one_to_storage(TInput&& input, TFromEncoding&& __from_encoding,
			_OutputContainer& output, TToEncoding&& __to_encoding, _FromErrorHandler&& __from_error_handler,
			_ToErrorHandler&& __to_error_handler, TFromState& fromState, TToState& toState, TPivot&& pivot) {
			using _UFromEncoding                              = remove_cvref_t<TFromEncoding>;
			using _UToEncoding                                = remove_cvref_t<TToEncoding>;
			using _UFromErrorHandler                          = remove_cvref_t<_FromErrorHandler>;
			using _UToErrorHandler                            = remove_cvref_t<_ToErrorHandler>;
			constexpr std::size_t __intermediate_buffer_max = MaxCodePoints<_UToEncoding> * 2;
			using _IntermediateValueType                      = code_point_t<_UToEncoding>;
			using TOutput                                     = Span<_IntermediateValueType>;

			static_assert(Impl::__is_encode_lossless_or_deliberate_v<_UFromEncoding, _UFromErrorHandler>,
				ZTD_TEXT_LOSSY_RECODE_ENCODE_MESSAGE_I_);
			static_assert(Impl::__is_decode_lossless_or_deliberate_v<_UToEncoding, _UToErrorHandler>,
				ZTD_TEXT_LOSSY_RECODE_DECODE_MESSAGE_I_);

			_IntermediateValueType __intermediate_translation_buffer[__intermediate_buffer_max] {};
			TOutput __intermediate_initial_output(__intermediate_translation_buffer);
			auto result = ::recode_one_into_raw(std::forward<TInput>(input),
				std::forward<TFromEncoding>(__from_encoding), std::move(__intermediate_initial_output),
				std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
				std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState, pivot);
			TOutput intermediateOutput(__intermediate_initial_output.data(), result.Output.data());
			ranges::__rng_detail::__container_insert_bulk(output, intermediateOutput);
			return result;
		}

		template <bool _OutputOnly, bool _NoState, typename _OutputContainer, typename TInput, typename TFromEncoding,
			typename TToEncoding, typename _FromErrorHandler, typename _ToErrorHandler, typename TFromState,
			typename TToState, typename TPivot>
		constexpr auto __recode_one_dispatch(TInput&& input, TFromEncoding&& __from_encoding,
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
			auto __stateful_result = __intermediate_recode_one_to_storage(std::forward<TInput>(input),
				std::forward<TFromEncoding>(__from_encoding), output,
				std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
				std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState, pivot);
			if constexpr (_OutputOnly) {
				(void)__stateful_result;
				return output;
			}
			else {
				if constexpr (_NoState) {
					return Impl::ReplaceRecodeResultOutputNoState(
						std::move(__stateful_result), std::move(output));
				}
				else {
					return Impl::ReplaceRecodeResultOutput(
						std::move(__stateful_result), std::move(output));
				}
			}
		}
	} 

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding into the output view. Only performs one distinct unit of encoding.
	///
	/// @param[in]     input An input_view to read code units from and use in the encode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to encode the input's code units into
	/// intermediate code points.
	/// @param[in]     output An output_view to write code units to as the result of the decode operation from
	/// the intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to decode the intermediate code points into
	/// the final code units.
	/// @param[in]     __from_error_handler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in]     __to_error_handler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in,out] fromState A reference to the associated state for the `__from_encoding` 's encode step.
	/// @param[in,out] toState A reference to the associated state for the `__to_encoding` 's decode step.
	/// @param[in, out] pivot A reference to a descriptor of a (potentially usable) range as the intermediate pivot,
	/// usually a range of contiguous data from a span provided by the implementation but can be passed in here by the
	/// user.
	///
	/// @result A RecodeResult object that contains references to `fromState` and @p
	/// toState.
	///
	/// @remark This function detects whether or not the ADL extension point `TextRecodeOne` can be called
	/// with the provided parameters. If so, it will use that ADL extension point over the default implementation.
	/// Otherwise, it will loop over the two encodings and attempt to recode by first decoding the input code
	/// units to code points, then encoding the intermediate code points to the desired, output code units.
	template <typename TInput, typename TOutput, typename TFromEncoding, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler, typename TFromState, typename TToState, typename TPivot>
	constexpr auto basic_recode_one_into(TInput&& input, TFromEncoding&& __from_encoding, TOutput&& output,
		TToEncoding&& __to_encoding, _FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler,
		TFromState& fromState, TToState& toState, TPivot&& pivot) {
		auto __reconstructed_input       = Impl::SpanReconstruct<TInput>(std::forward<TInput>(input));
		auto result                    = basic_recode_one_into_raw(std::move(__reconstructed_input),
			                   std::forward<TFromEncoding>(__from_encoding), std::forward<TOutput>(output),
			                   std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
			                   std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState, pivot);
		using _ReconstructedResultInput  = Impl::span_reconstruct_t<TInput, TInput>;
		using _ReconstructedResultOutput = Impl::span_reconstruct_mutable_t<TOutput, TOutput>;
		return PivotlessRecodeResult<_ReconstructedResultInput, _ReconstructedResultOutput, TFromState, TToState>(
			Impl::SpanReconstruct<TInput>(std::move(result.Input)),
			Impl::SpanReconstructMutable<TOutput>(std::move(result.Output)), result.FromState,
			result.ToState, result.ErrorCode, result.ErrorCount);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding into the output view. Only performs one distinct unit of encoding.
	///
	/// @param[in]     input An input_view to read code units from and use in the encode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to encode the input's code units into
	/// intermediate code points.
	/// @param[in]     output An output_view to write code units to as the result of the decode operation from
	/// the intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to decode the intermediate code points into
	/// the final code units.
	/// @param[in]     __from_error_handler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in]     __to_error_handler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in,out] fromState A reference to the associated state for the `__from_encoding` 's encode step.
	/// @param[in,out] toState A reference to the associated state for the `__to_encoding` 's decode step.
	/// @param[in, out] pivot A reference to a descriptor of a (potentially usable) range as the intermediate pivot,
	/// usually a range of contiguous data from a span provided by the implementation but can be passed in here by the
	/// user.
	///
	/// @result A RecodeResult object that contains references to `fromState` and @p
	/// toState.
	///
	/// @remark This function detects whether or not the ADL extension point `TextRecodeOne` can be called
	/// with the provided parameters. If so, it will use that ADL extension point over the default implementation.
	/// Otherwise, it will loop over the two encodings and attempt to recode by first decoding the input code
	/// units to code points, then encoding the intermediate code points to the desired, output code units.
	template <typename TInput, typename TOutput, typename TFromEncoding, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler, typename TFromState, typename TToState, typename TPivot>
	constexpr auto recode_one_into(TInput&& input, TFromEncoding&& __from_encoding, TOutput&& output,
		TToEncoding&& __to_encoding, _FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler,
		TFromState& fromState, TToState& toState, TPivot&& pivot) {
		auto __reconstructed_input      = Impl::SpanReconstruct<TInput>(std::forward<TInput>(input));
		auto result                   = ::recode_one_into_raw(std::move(__reconstructed_input),
			                  std::forward<TFromEncoding>(__from_encoding), std::forward<TOutput>(output),
			                  std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
			                  std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState, pivot);
		using _ReconstructedResultInput = Impl::span_reconstruct_t<TInput, decltype(result.Input)>;
		using _ReconstructedResultOutput
			= Impl::span_reconstruct_mutable_t<TOutput, decltype(result.Output)>;
		return PivotlessRecodeResult<_ReconstructedResultInput, _ReconstructedResultOutput, TFromState, TToState>(
			Impl::SpanReconstruct<TInput>(std::move(result.Input)),
			Impl::SpanReconstructMutable<TOutput>(std::move(result.Output)), result.FromState,
			result.ToState, result.ErrorCode, result.ErrorCount);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding into the output view. Only performs one distinct unit of encoding.
	///
	/// @param[in]     input An input_view to read code units from and use in the encode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to encode the input's code units into
	/// intermediate code points.
	/// @param[in]     output An output_view to write code units to as the result of the decode operation from
	/// the intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to decode the intermediate code points into
	/// the final code units.
	/// @param[in]     __from_error_handler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in]     __to_error_handler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in,out] fromState A reference to the associated state for the `__from_encoding` 's encode step.
	/// @param[in,out] toState A reference to the associated state for the `__to_encoding` 's decode step.
	///
	/// @result A RecodeResult object that contains references to `fromState` and @p
	/// toState.
	///
	/// @remark This function detects whether or not the ADL extension point `TextRecodeOne` can be called
	/// with the provided parameters. If so, it will use that ADL extension point over the default implementation.
	/// Otherwise, it will loop over the two encodings and attempt to recode by first decoding the input code
	/// units to code points, then encoding the intermediate code points to the desired, output code units.
	template <typename TInput, typename TOutput, typename TFromEncoding, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler, typename TFromState, typename TToState>
	constexpr auto recode_one_into(TInput&& input, TFromEncoding&& __from_encoding, TOutput&& output,
		TToEncoding&& __to_encoding, _FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler,
		TFromState& fromState, TToState& toState) {
		using _UFromEncoding                              = remove_cvref_t<TFromEncoding>;
		using TCodeUnit                                   = code_unit_t<_UFromEncoding>;
		using TPivot                                      = Span<TCodeUnit>;
		constexpr std::size_t __intermediate_buffer_max = MaxCodeUnits<_UFromEncoding> * 2;

		TCodeUnit intermediate[__intermediate_buffer_max] {};
		TPivot pivot(intermediate);
		return ::recode_one_into(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TOutput>(output),
			std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
			std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState, pivot);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding into the output view.
	///
	/// @param[in]     input An input_view to read code units from and use in the encode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to encode the input's code units into
	/// intermediate code points.
	/// @param[in]     output An output_view to write code units to as the result of the decode operation from the
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to decode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handler for the `__from_encoding` 's encode step.
	/// @param[in]     __to_error_handler The error handler for the `__to_encoding` 's decode step.
	/// @param[in,out] fromState A reference to the associated state for the `__from_encoding` 's encode step.
	///
	/// @result A StatelessRecodeResult object.
	///
	/// @remarks This function calls the base reference, the recode_one_into after creating a
	/// `ToState` from CreateDecodeState. The result from this function returns a
	/// StatelessRecodeResult as opposed to a RecodeResult because the state
	/// information is on the stack, and returning the state in those types by reference will result in references to
	/// memory that has already been cleaned up. If you need access to the state parameters, call the lower-level
	/// functionality with your own created states.
	template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler, typename TFromState>
	constexpr auto recode_one_into(TInput&& input, TFromEncoding&& __from_encoding, TOutput&& output,
		TToEncoding&& __to_encoding, _FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler,
		TFromState& fromState) {
		using _UToEncoding = remove_cvref_t<TToEncoding>;
		using TToState     = DecodeState<_UToEncoding>;

		TToState toState = ::CreateDecodeState(__to_encoding);

		auto __stateful_result = ::recode_one_into(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TOutput>(output),
			std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
			std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState);

		return Impl::SliceToStateless(std::move(__stateful_result));
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding into the output view.
	/// @param[in]     input An input_view to read code units from and use in the encode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to encode the input's code units into
	/// intermediate code points.
	/// @param[in]     output An output_view to write code units to as the result of the decode operation from the
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to decode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handler for the `__from_encoding` 's encode step.
	/// @param[in]     __to_error_handler The error handler for the `__to_encoding` 's decode step.
	///
	/// @remarks This function creates a encode state `FromState` by calling Krys::CreateEncodeState. The
	/// result from this function returns a StatelessRecodeResult as opposed to a
	/// RecodeResult because the state information is on the stack, and returning the state in
	/// those types by reference will result in references to memory that has already been cleaned up. If you need
	/// access to the state parameters, call the lower-level functionality with your own created states.
	template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler>
	constexpr auto recode_one_into(TInput&& input, TFromEncoding&& __from_encoding, TOutput&& output,
		TToEncoding&& __to_encoding, _FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler) {
		using _UFromEncoding = remove_cvref_t<TFromEncoding>;
		using TFromState     = EncodeState<_UFromEncoding>;

		TFromState fromState = ::Krys::CreateEncodeState(__from_encoding);

		return ::recode_one_into(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TOutput>(output),
			std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
			std::forward<_ToErrorHandler>(__to_error_handler), fromState);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding into the output view.
	/// @param[in]     input An input_view to read code units from and use in the encode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to encode the input's code units into
	/// intermediate code points.
	/// @param[in]     output An output_view to write code units to as the result of the decode operation from the
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to decode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handler for the `__from_encoding` 's encode step.
	///
	/// @remarks This function creates an `to_error_handler` from a class like Handlers::DefaultHandler, but that
	/// is marked as careless since you did not explicitly provide it. This matters for lossy conversions that are not
	/// injective. The result from this function returns a StatelessRecodeResult as opposed to a
	/// RecodeResult because the state information is on the stack, and returning the state in
	/// those types by reference will result in references to memory that has already been cleaned up. If you need
	/// access to the state parameters, call the lower-level functionality with your own created states.
	template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
		typename _FromErrorHandler>
	constexpr auto recode_one_into(TInput&& input, TFromEncoding&& __from_encoding, TOutput&& output,
		TToEncoding&& __to_encoding, _FromErrorHandler&& __from_error_handler) {
		auto __handler = Impl::__duplicate_or_be_careless(__from_error_handler);

		return ::recode_one_into(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TOutput>(output),
			std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
			__handler);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding into the output view.
	/// @param[in]     input An input_view to read code units from and use in the encode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to encode the input's code units into
	/// intermediate code points.
	/// @param[in]     output An output_view to write code units to as the result of the decode operation from the
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to decode the intermediate code points into the
	/// final code units.
	///
	/// @remarks This function creates an `from_error_handler` from a class like Handlers::DefaultHandler, but
	/// that is marked as careless since you did not explicitly provide it. This matters for lossy conversions that are
	/// not injective. The result from this function returns a StatelessRecodeResult as opposed to a
	/// RecodeResult because the state information is on the stack, and returning the state in
	/// those types by reference will result in references to memory that has already been cleaned up. If you need
	/// access to the state parameters, call the lower-level functionality with your own created states.
	template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding>
	constexpr auto recode_one_into(
		TInput&& input, TFromEncoding&& __from_encoding, TOutput&& output, TToEncoding&& __to_encoding) {
		Handlers::DefaultHandler __handler {};

		return ::recode_one_into(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TOutput>(output),
			std::forward<TToEncoding>(__to_encoding), __handler);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding into the output view.
	/// @param[in]     input An input_view to read code units from and use in the encode operation that will
	/// produce intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to decode the intermediate code points into the
	/// final code units.
	/// @param[in]     output An output_view to write code units to as the result of the decode operation from the
	/// intermediate code points.
	///
	/// @remarks This function creates both: a `from_error_handler` using a Handlers::DefaultHandler that is
	/// marked as careless to pass to the next function overload; and, a `from_encoding` to interpret the `input`
	/// by checking the `input` 's `value_type.` This matters for lossy conversions that are not injective. The
	/// result from this function returns a StatelessRecodeResult as opposed to a
	/// RecodeResult because the state information is on the stack, and returning the state in
	/// those types by reference will result in references to memory that has already been cleaned up. If you need
	/// access to the state parameters, call the lower-level functionality with your own created states.
	template <typename TInput, typename TToEncoding, typename TOutput>
	constexpr auto recode_one_into(TInput&& input, TToEncoding&& __to_encoding, TOutput&& output) {
		using TUInput        = remove_cvref_t<TInput>;
		using _UFromEncoding = default_code_point_encoding_t<ranges::rangeTValue_type_t<TUInput>>;

		_UFromEncoding __from_encoding {};
		Handlers::DefaultHandler __handler {};

		return ::recode_one_into(std::forward<TInput>(input), __from_encoding,
			std::forward<TOutput>(output), std::forward<TToEncoding>(__to_encoding), __handler);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding for the output, which is then returned in a result structure with additional information about
	/// success.
	///
	/// @tparam _OutputContainer The container to default-construct and serialize data into. Typically, a fixed-size
	/// container of some sort.
	///
	/// @param[in]     input An input_view to read code units from and use in the encode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to encode the input's code units into
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to decode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handler for the `__from_encoding` 's encode step.
	/// @param[in]     __to_error_handler The error handler for the `__to_encoding` 's decode step.
	/// @param[in,out] fromState A reference to the associated state for the `__from_encoding` 's encode step.
	/// @param[in,out] toState A reference to the associated state for the `__to_encoding` 's decode step.
	/// @param[in, out] pivot A reference to a descriptor of a (potentially usable) range as the intermediate pivot,
	/// usually a range of contiguous data from a span provided by the implementation but can be passed in here by the
	/// user.
	///
	/// @returns A RecodeResult object that contains references to `fromState` and @p
	/// toState and an `output` parameter that contains the `_OutputContainer` specified. If the container has a
	/// `container.reserve` function, it is and some multiple of the input's size is used to pre-size the container,
	/// to aid with `push_back` / `insert` reallocation pains.
	template <typename _OutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler, typename TFromState, typename TToState, typename TPivot>
	constexpr auto recode_one_to(TInput&& input, TFromEncoding&& __from_encoding, TToEncoding&& __to_encoding,
		_FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler, TFromState& fromState,
		TToState& toState, TPivot&& pivot) {
		using _UFromEncoding    = remove_cvref_t<TFromEncoding>;
		using _UToEncoding      = remove_cvref_t<TToEncoding>;
		using _UOutputContainer = remove_cvref_t<_OutputContainer>;
		using _OutputCodePoint  = code_point_t<_UToEncoding>;
		constexpr std::size_t _MinimumIntermediateOutputMax
			= max_recode_code_points_v<_UFromEncoding, _UToEncoding>;
		constexpr bool _IsVoidContainer = Void<_UOutputContainer>;
		constexpr bool _IsStringable
			= (is_char_traitable_v<_OutputCodePoint> || IsUnicodeCodePoint<_OutputCodePoint>);
		if constexpr (_IsVoidContainer && _IsStringable) {
			using _RealOutputContainer = ::ztd::inline_basic_string<_OutputCodePoint, _MinimumIntermediateOutputMax>;
			return Impl::__recode_one_dispatch<false, false, _RealOutputContainer>(
				std::forward<TInput>(input), std::forward<TFromEncoding>(__from_encoding),
				std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
				std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState, pivot);
		}
		else {
			using _RealOutputContainer = conditional_t<_IsVoidContainer,
				::ztd::inline_vector<_OutputCodePoint, _MinimumIntermediateOutputMax>, _OutputContainer>;
			return Impl::__recode_one_dispatch<false, false, _RealOutputContainer>(
				std::forward<TInput>(input), std::forward<TFromEncoding>(__from_encoding),
				std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
				std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState, pivot);
		}
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding for the output, which is then returned in a result structure with additional information about
	/// success.
	///
	/// @tparam _OutputContainer The container to default-construct and serialize data into. Typically, a fixed-size
	/// container of some sort.
	///
	/// @param[in]     input An input_view to read code units from and use in the encode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to encode the input's code units into
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to decode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handler for the `__from_encoding` 's encode step.
	/// @param[in]     __to_error_handler The error handler for the `__to_encoding` 's decode step.
	/// @param[in,out] fromState A reference to the associated state for the `__from_encoding` 's encode step.
	/// @param[in,out] toState A reference to the associated state for the `__to_encoding` 's decode step.
	///
	/// @returns A RecodeResult object that contains references to `fromState` and @p
	/// toState and an `output` parameter that contains the `_OutputContainer` specified. If the container has a
	/// `container.reserve` function, it is and some multiple of the input's size is used to pre-size the container,
	/// to aid with `push_back` / `insert` reallocation pains.
	template <typename _OutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler, typename TFromState, typename TToState>
	constexpr auto recode_one_to(TInput&& input, TFromEncoding&& __from_encoding, TToEncoding&& __to_encoding,
		_FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler, TFromState& fromState,
		TToState& toState) {
		using _UFromEncoding                              = remove_cvref_t<TFromEncoding>;
		using TCodeUnit                                   = code_unit_t<_UFromEncoding>;
		using TPivot                                      = Span<TCodeUnit>;
		constexpr std::size_t __intermediate_buffer_max = MaxCodeUnits<_UFromEncoding> * 2;

		TCodeUnit intermediate[__intermediate_buffer_max] {};
		TPivot pivot(intermediate);
		return ::recode_one_to<_OutputContainer>(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TToEncoding>(__to_encoding),
			std::forward<_FromErrorHandler>(__from_error_handler),
			std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState, pivot);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding for the output, which is then returned in a result structure with additional information about
	/// success.
	///
	/// @tparam _OutputContainer The container to default-construct and serialize data into. Typically, a fixed-size
	/// container of some sort.
	///
	/// @param[in]     input An input_view to read code units from and use in the encode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to encode the input's code units into
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to decode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handler for the `__from_encoding` 's encode step.
	/// @param[in]     __to_error_handler The error handler for the `__to_encoding` 's decode step.
	/// @param[in,out] fromState A reference to the associated state for the `__from_encoding` 's encode step.
	///
	/// @returns A StatelessRecodeResult object that contains references to an `container.Output`
	/// parameter that contains the `_OutputContainer` specified.
	///
	/// @remarks A default state for the decode step of the operation is create using CreateDecodeState. The
	/// return type is stateless since both states must be passed in. If you want to have access to the states, create
	/// both of them yourself and pass them into a lower-level function that accepts those parameters.
	template <typename _OutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler, typename TFromState>
	constexpr auto recode_one_to(TInput&& input, TFromEncoding&& __from_encoding, TToEncoding&& __to_encoding,
		_FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler, TFromState& fromState) {
		using _UToEncoding = remove_cvref_t<TToEncoding>;
		using TToState     = DecodeState<_UToEncoding>;

		TToState toState = ::CreateDecodeState(__to_encoding);

		return ::recode_one_to<_OutputContainer>(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TToEncoding>(__to_encoding),
			std::forward<_FromErrorHandler>(__from_error_handler),
			std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding for the output, which is then returned in a result structure with additional information about
	/// success.
	///
	/// @tparam _OutputContainer The container to default-construct and serialize data into. Typically, a fixed-size
	/// container of some sort.
	///
	/// @param[in]     input An input_view to read code units from and use in the encode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to encode the input's code units into
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to decode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handler for the `__from_encoding` 's encode step.
	/// @param[in]     __to_error_handler The error handler for the `__to_encoding` 's decode step.
	///
	/// @returns A StatelessRecodeResult object that contains references to an `container.Output`
	/// parameter that contains the `_OutputContainer` specified.
	///
	/// @remarks A default state for the encode step of the operation is create using Krys::CreateEncodeState. The
	/// return type is stateless since both states must be passed in. If you want to have access to the states, create
	/// both of them yourself and pass them into a lower-level function that accepts those parameters.
	template <typename _OutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler>
	constexpr auto recode_one_to(TInput&& input, TFromEncoding&& __from_encoding, TToEncoding&& __to_encoding,
		_FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler) {
		using _UFromEncoding = remove_cvref_t<TFromEncoding>;
		using TFromState     = EncodeState<_UFromEncoding>;

		TFromState fromState = ::Krys::CreateEncodeState(__from_encoding);

		return ::recode_one_to<_OutputContainer>(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TToEncoding>(__to_encoding),
			std::forward<_FromErrorHandler>(__from_error_handler),
			std::forward<_ToErrorHandler>(__to_error_handler), fromState);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding for the output, which is then returned in a result structure with additional information about
	/// success.
	///
	/// @tparam _OutputContainer The container to default-construct and serialize data into. Typically, a fixed-size
	/// container of some sort.
	///
	/// @param[in]     input An input_view to read code units from and use in the encode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to encode the input's code units into
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to decode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handler for the `__from_encoding` 's encode step.
	///
	/// @returns A StatelessRecodeResult object that contains references to an `container.Output`
	/// parameter that contains the `_OutputContainer` specified.
	///
	/// @remarks A `to_error_handler` for the decode step of the operation is created using default construction of a
	/// Handlers::DefaultHandler that is marked as careless. The return type is stateless since both states must be
	/// passed in. If you want to have access to the states, create both of them yourself and pass them into a
	/// lower-level function that accepts those parameters.
	template <typename _OutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
		typename _FromErrorHandler>
	constexpr auto recode_one_to(TInput&& input, TFromEncoding&& __from_encoding, TToEncoding&& __to_encoding,
		_FromErrorHandler&& __from_error_handler) {
		auto __handler = Impl::__duplicate_or_be_careless(__from_error_handler);

		return ::recode_one_to<_OutputContainer>(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TToEncoding>(__to_encoding),
			std::forward<_FromErrorHandler>(__from_error_handler), __handler);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding for the output, which is then returned in a result structure with additional information about
	/// success.
	///
	/// @tparam _OutputContainer The container to default-construct and serialize data into. Typically, a fixed-size
	/// container of some sort.
	///
	/// @param[in]     input An input_view to read code units from and use in the encode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to encode the input's code units into
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to decode the intermediate code points into the
	/// final code units.
	///
	/// @returns A StatelessRecodeResult object that contains references to an `container.Output`
	/// parameter that contains the `_OutputContainer` specified.
	///
	/// @remarks A `from_error_handler` for the decode step of the operation is created using default construction of
	/// a Handlers::DefaultHandler that is marked as careless. The return type is stateless since both states must
	/// be passed in. If you want to have access to the states, create both of them yourself and pass them into a
	/// lower-level function that accepts those parameters.
	template <typename _OutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding>
	constexpr auto recode_one_to(TInput&& input, TFromEncoding&& __from_encoding, TToEncoding&& __to_encoding) {
		Handlers::DefaultHandler __handler {};

		return ::recode_one_to<_OutputContainer>(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TToEncoding>(__to_encoding), __handler);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding for the output, which is then returned in a result structure with additional information about
	/// success.
	///
	/// @tparam _OutputContainer The container to default-construct and serialize data into. Typically, a fixed-size
	/// container of some sort.
	///
	/// @param[in]     input An input_view to read code units from and use in the encode operation that will
	/// produce intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to decode the intermediate code points into the
	/// final code units.
	///
	/// @returns A StatelessRecodeResult object that contains references to an `container.Output`
	/// parameter that contains the `_OutputContainer` specified.
	///
	/// @remarks A `from_error_handler` for the decode step of the operation is created using default construction of
	/// a Handlers::DefaultHandler that is marked as careless. The return type is stateless since both states must
	/// be passed in. If you want to have access to the states, create both of them yourself and pass them into a
	/// lower-level function that accepts those parameters.
	template <typename _OutputContainer, typename TInput, typename TToEncoding>
	constexpr auto recode_one_to(TInput&& input, TToEncoding&& __to_encoding) {
		using TUInput    = remove_cvref_t<TInput>;
		using TCodePoint = ranges::rangeTValue_type_t<TUInput>;
#if ZTD_IS_ON(ZTD_STD_LIBRARY_IS_CONSTANT_EVALUATED)
		if (std::is_constant_evaluated()) {
			using _UFromEncoding = default_consteval_code_point_encoding_t<TCodePoint>;
			Handlers::DefaultHandler __handler {};
			_UFromEncoding __from_encoding {};
			return ::recode_one_to<_OutputContainer>(std::forward<TInput>(input), __from_encoding,
				std::forward<TToEncoding>(__to_encoding), __handler);
		}
		else
#endif
		{
			using _UFromEncoding = default_code_point_encoding_t<TCodePoint>;
			Handlers::DefaultHandler __handler {};
			_UFromEncoding __from_encoding {};
			return ::recode_one_to<_OutputContainer>(std::forward<TInput>(input), __from_encoding,
				std::forward<TToEncoding>(__to_encoding), __handler);
		}
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding for the output, which is then returned in a result structure with additional information about
	/// success.
	///
	/// @tparam _OutputContainer The container to default-construct and serialize data into. Typically, a fixed-size
	/// container of some sort.
	///
	/// @param[in]     input An input_view to read code units from and use in the encode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to encode the input's code units into
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to decode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handler for the `__from_encoding` 's encode step.
	/// @param[in]     __to_error_handler The error handler for the `__to_encoding` 's decode step.
	/// @param[in,out] fromState A reference to the associated state for the `__from_encoding` 's encode step.
	/// @param[in,out] toState A reference to the associated state for the `__to_encoding` 's decode step.
	/// @param[in, out] pivot A reference to a descriptor of a (potentially usable) range as the intermediate pivot,
	/// usually a range of contiguous data from a span provided by the implementation but can be passed in here by the
	/// user.
	///
	/// @returns An `_OutputContainer` with the result, regardless of whether an error occurs or not. If you are
	/// looking for error information and not just a quick one-off conversion function, please use
	/// recode_one_to or recode_one_into.
	template <typename _OutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler, typename TFromState, typename TToState, typename TPivot>
	constexpr auto recode_one(TInput&& input, TFromEncoding&& __from_encoding, TToEncoding&& __to_encoding,
		_FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler, TFromState& fromState,
		TToState& toState, TPivot&& pivot) {
		using _UFromEncoding    = remove_cvref_t<TFromEncoding>;
		using _UToEncoding      = remove_cvref_t<TToEncoding>;
		using _UOutputContainer = remove_cvref_t<_OutputContainer>;
		using _OutputCodePoint  = code_point_t<_UToEncoding>;
		constexpr std::size_t _MinimumIntermediateOutputMax
			= max_recode_code_points_v<_UFromEncoding, _UToEncoding>;
		constexpr bool _IsVoidContainer = Void<_UOutputContainer>;
		constexpr bool _IsStringable
			= (is_char_traitable_v<_OutputCodePoint> || IsUnicodeCodePoint<_OutputCodePoint>);
		if constexpr (_IsVoidContainer && _IsStringable) {
			using _RealOutputContainer = ::ztd::inline_basic_string<_OutputCodePoint, _MinimumIntermediateOutputMax>;
			return Impl::__recode_one_dispatch<true, false, _RealOutputContainer>(
				std::forward<TInput>(input), std::forward<TFromEncoding>(__from_encoding),
				std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
				std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState, pivot);
		}
		else {
			using _RealOutputContainer = conditional_t<_IsVoidContainer,
				::ztd::inline_vector<_OutputCodePoint, _MinimumIntermediateOutputMax>, _OutputContainer>;
			return Impl::__recode_one_dispatch<true, false, _RealOutputContainer>(
				std::forward<TInput>(input), std::forward<TFromEncoding>(__from_encoding),
				std::forward<TToEncoding>(__to_encoding), std::forward<_FromErrorHandler>(__from_error_handler),
				std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState, pivot);
		}
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding for the output, which is then returned in a result structure with additional information about
	/// success.
	///
	/// @tparam _OutputContainer The container to default-construct and serialize data into. Typically, a fixed-size
	/// container of some sort.
	///
	/// @param[in]     input An input_view to read code units from and use in the encode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to encode the input's code units into
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to decode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handler for the `__from_encoding` 's encode step.
	/// @param[in]     __to_error_handler The error handler for the `__to_encoding` 's decode step.
	/// @param[in,out] fromState A reference to the associated state for the `__from_encoding` 's encode step.
	/// @param[in,out] toState A reference to the associated state for the `__to_encoding` 's decode step.
	///
	/// @returns An `_OutputContainer` with the result, regardless of whether an error occurs or not. If you are
	/// looking for error information and not just a quick one-off conversion function, please use
	/// recode_one_to or recode_one_into.
	template <typename _OutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler, typename TFromState, typename TToState>
	constexpr auto recode_one(TInput&& input, TFromEncoding&& __from_encoding, TToEncoding&& __to_encoding,
		_FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler, TFromState& fromState,
		TToState& toState) {
		using _UFromEncoding                              = remove_cvref_t<TFromEncoding>;
		using TCodeUnit                                   = code_unit_t<_UFromEncoding>;
		using TPivot                                      = Span<TCodeUnit>;
		constexpr std::size_t __intermediate_buffer_max = MaxCodeUnits<_UFromEncoding> * 2;

		TCodeUnit intermediate[__intermediate_buffer_max] {};
		TPivot pivot(intermediate);
		return ::recode_one<_OutputContainer>(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TToEncoding>(__to_encoding),
			std::forward<_FromErrorHandler>(__from_error_handler),
			std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState, pivot);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding for the output, which is then returned in a result structure with additional information about
	/// success.
	///
	/// @tparam _OutputContainer The container to default-construct and serialize data into. Typically, a fixed-size
	/// container of some sort.
	///
	/// @param[in]     input An input_view to read code units from and use in the encode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to encode the input's code units into
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to decode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handler for the `__from_encoding` 's encode step.
	/// @param[in]     __to_error_handler The error handler for the `__to_encoding` 's decode step.
	/// @param[in,out] fromState A reference to the associated state for the `__from_encoding` 's encode step.
	///
	/// @returns An `_OutputContainer` with the result, regardless of whether an error occurs or not. If you are
	/// looking for error information and not just a quick one-off conversion function, please use
	/// recode_one_to or recode_one_into.
	///
	/// @remarks This function creates an `ToState` for the encoding step of the operation using
	/// CreateDecodeState.
	template <typename _OutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler, typename TFromState>
	constexpr auto recode_one(TInput&& input, TFromEncoding&& __from_encoding, TToEncoding&& __to_encoding,
		_FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler, TFromState& fromState) {
		using _UToEncoding = remove_cvref_t<TToEncoding>;
		using TToState     = DecodeState<_UToEncoding>;

		TToState toState = ::CreateDecodeState(__to_encoding);

		return ::recode_one<_OutputContainer>(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TToEncoding>(__to_encoding),
			std::forward<_FromErrorHandler>(__from_error_handler),
			std::forward<_ToErrorHandler>(__to_error_handler), fromState, toState);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding for the output, which is then returned in a result structure with additional information about
	/// success.
	///
	/// @tparam _OutputContainer The container to default-construct and serialize data into. Typically, a fixed-size
	/// container of some sort.
	///
	/// @param[in]     input An input_view to read code units from and use in the encode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to encode the input's code units into
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to decode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handler for the `__from_encoding` 's encode step.
	/// @param[in]     __to_error_handler The error handler for the `__to_encoding` 's decode step.
	///
	/// @returns An `_OutputContainer` with the result, regardless of whether an error occurs or not. If you are
	/// looking for error information and not just a quick one-off conversion function, please use
	/// recode_one_to or recode_one_into.
	///
	/// @remarks This function creates an `FromState` for the encoding step of the operation using
	/// Krys::CreateEncodeState.
	template <typename _OutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler>
	constexpr auto recode_one(TInput&& input, TFromEncoding&& __from_encoding, TToEncoding&& __to_encoding,
		_FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler) {
		using _UFromEncoding = remove_cvref_t<TFromEncoding>;
		using TFromState     = EncodeState<_UFromEncoding>;

		TFromState fromState = ::Krys::CreateEncodeState(__from_encoding);

		return ::recode_one<_OutputContainer>(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TToEncoding>(__to_encoding),
			std::forward<_FromErrorHandler>(__from_error_handler),
			std::forward<_ToErrorHandler>(__to_error_handler), fromState);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding for the output, which is then returned in a result structure with additional information about
	/// success.
	///
	/// @tparam _OutputContainer The container to default-construct and serialize data into. Typically, a fixed-size
	/// container of some sort.
	///
	/// @param[in]     input An input_view to read code units from and use in the encode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to encode the input's code units into
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to decode the intermediate code points into the
	/// final code units.
	/// @param[in]     __from_error_handler The error handler for the `__from_encoding` 's encode step.
	///
	/// @returns An `_OutputContainer` with the result, regardless of whether an error occurs or not. If you are
	/// looking for error information and not just a quick one-off conversion function, please use
	/// recode_one_to or recode_one_into.
	///
	/// @remarks This function creates a `to_error_handler` from a class like Handlers::DefaultHandler, but that
	/// is marked as careless since you did not explicitly provide it. This matters for lossy conversions that are not
	/// injective.
	template <typename _OutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
		typename _FromErrorHandler>
	constexpr auto recode_one(TInput&& input, TFromEncoding&& __from_encoding, TToEncoding&& __to_encoding,
		_FromErrorHandler&& __from_error_handler) {
		auto __handler = Impl::__duplicate_or_be_careless(__from_error_handler);

		return ::recode_one<_OutputContainer>(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TToEncoding>(__to_encoding),
			std::forward<_FromErrorHandler>(__from_error_handler), __handler);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding for the output, which is then returned in a result structure with additional information about
	/// success.
	///
	/// @tparam _OutputContainer The container to default-construct and serialize data into. Typically, a fixed-size
	/// container of some sort.
	///
	/// @param[in]     input An input_view to read code units from and use in the encode operation that will
	/// produce intermediate code points.
	/// @param[in]     __from_encoding The encoding that will be used to encode the input's code units into
	/// intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to decode the intermediate code points into the
	/// final code units.
	///
	/// @returns An `_OutputContainer` with the result, regardless of whether an error occurs or not. If you are
	/// looking for error information and not just a quick one-off conversion function, please use
	/// recode_one_to or recode_one_into.
	///
	/// @remarks This function creates a `from_error_handler` from a class like Handlers::DefaultHandler, but that
	/// is marked as careless since you did not explicitly provide it. This matters for lossy conversions that are not
	/// injective.
	template <typename _OutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding>
	constexpr auto recode_one(TInput&& input, TFromEncoding&& __from_encoding, TToEncoding&& __to_encoding) {
		Handlers::DefaultHandler __handler {};

		return ::recode_one<_OutputContainer>(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TToEncoding>(__to_encoding), __handler);
	}

	
	/// @brief Converts the code units of the given input view through the from encoding to code units of the to
	/// encoding for the output, which is then returned in a result structure with additional information about
	/// success.
	///
	/// @tparam _OutputContainer The container to default-construct and serialize data into. Typically, a fixed-size
	/// container of some sort.
	///
	/// @param[in]     input An input_view to read code units from and use in the encode operation that will
	/// produce intermediate code points.
	/// @param[in]     __to_encoding The encoding that will be used to decode the intermediate code points into the
	/// final code units.
	///
	/// @returns An `_OutputContainer` with the result, regardless of whether an error occurs or not. If you are
	/// looking for error information and not just a quick one-off conversion function, please use
	/// recode_one_to or recode_one_into.
	///
	/// @remarks This function creates both: a `from_error_handler` from a class like Handlers::DefaultHandler,
	/// but that is marked as careless since you did not explicitly provide it; and, a `from_encoding` derived from
	/// the @p
	/// "input"'s `value_type.` The careless marking matters for lossy conversions that are not injective.
	template <typename _OutputContainer = void, typename TInput, typename TToEncoding>
	constexpr auto recode_one(TInput&& input, TToEncoding&& __to_encoding) {
		using TUInput    = remove_cvref_t<TInput>;
		using TCodePoint = ranges::rangeTValue_type_t<TUInput>;
#if ZTD_IS_ON(ZTD_STD_LIBRARY_IS_CONSTANT_EVALUATED)
		if (std::is_constant_evaluated()) {
			using _UFromEncoding = default_consteval_code_point_encoding_t<TCodePoint>;
			_UFromEncoding __from_encoding {};
			return ::recode_one<_OutputContainer>(
				std::forward<TInput>(input), __from_encoding, std::forward<TToEncoding>(__to_encoding));
		}
		else
#endif
		{
			using _UFromEncoding = default_code_point_encoding_t<TCodePoint>;

			_UFromEncoding __from_encoding {};
			return ::recode_one<_OutputContainer>(
				std::forward<TInput>(input), __from_encoding, std::forward<TToEncoding>(__to_encoding));
		}
	}

	
	/// @}

	
}



#endif
