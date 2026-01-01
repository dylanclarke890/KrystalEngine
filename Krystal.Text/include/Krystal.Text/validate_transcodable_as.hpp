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

#ifndef ZTD_TEXTTValIDATE_TRANSCODABLE_AS_HPP
#define ZTD_TEXTTValIDATE_TRANSCODABLE_AS_HPP



#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include <ztd/text/default_encoding.hpp>
#include <ztd/text/ValidateResult.hpp>
#include <ztd/text/error_handler.hpp>
#include "Krystal.Text/State.hpp"
#include <ztd/text/transcode_one.hpp>
#include <ztd/text/detail/is_lossless.hpp>
#include <ztd/text/detail/encoding_range.hpp>

#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Utils/Tag.hpp"

#include <algorithm>
#include <string_view>



namespace Krys {
	

	
	/// @addtogroup ztd_textTValidate_transcodable_as validate_transcodable_as
	/// @brief These functions check if the given input of code points will decode without an error. Note that this
	/// does not mean that an error handler will be invoked that can "smooth over" any possible errors: this checks
	/// solely if it will decode from code units into code points cleanly.
	/// @{

	
	/// @brief Validates the code units of the `input` according to the `__from_encoding` with the given states
	/// `__decode_state` and `__encode_state` to see if it can be turned into code points, and then code units again.
	///
	/// @param[in] input The input range of code units to validate is possible for encoding into code points.
	/// @param[in] __from_encoding The encoding to verify can properly encode the input of code units.
	/// @param[in] __to_encoding The encoding to verify can properly encode the input of code units.
	/// @param[in, out] __decode_state The state to use for the decoding portion of the validation check.
	/// @param[in, out] __encode_state The state to use for the encoding portion of the validation check.
	/// @param[in, out] pivot A reference to a descriptor of a (potentially usable) pivot range, usually a range of
	/// contiguous data from a span provided by the implementation but customizable by the end-user. If the
	/// intermediate conversion is what failed, then the pivot's `ErrorCode` member will be set to that
	/// error. This only happens if the overall operation also fails, and need not be checked unless to obtain
	/// additional information for when a top-level operation fails.
	///
	/// @remarks This function explicitly does not call any extension points. It defers to doing a typical loop over
	/// the code points to verify it can be decoded into code points, and then encoded back into code units, with no
	/// errors and with the exact same value sequence as the original.
	template <typename TInput, typename TFromEncoding, typename TToEncoding, typename TDecodeState,
		typename TEncodeState, typename TPivot>
	constexpr auto basicTValidate_transcodable_as(TInput&& input, TFromEncoding&& __from_encoding,
		TToEncoding&& __to_encoding, TDecodeState& __decode_state, TEncodeState& __encode_state, TPivot&& pivot) {
		using _InitialInput  = Impl::span_reconstruct_t<TInput, TInput>;
		using _WorkingInput  = Krys::Ranges::subrange_for_t<_InitialInput>;
		using _UFromEncoding = remove_cvref_t<TFromEncoding>;
		using _UToEncoding   = remove_cvref_t<TToEncoding>;
		using TResult        = ValidatePivotlessTranscodeResult<_WorkingInput, TDecodeState, TEncodeState>;

		_WorkingInput __working_input = Impl::SpanReconstruct<TInput>(std::forward<TInput>(input));

		if constexpr (is_detected_v<Impl::HasTextValidateTranscodableAsOne, _WorkingInput,
			              TFromEncoding, TToEncoding, TDecodeState, TEncodeState, TPivot>) {
			(void)__encode_state;
			for (;;) {
				auto result = TextValidateTranscodableAsOne(Krys::Tag<_UFromEncoding, _UToEncoding> {},
					__working_input, __from_encoding, __to_encoding, __decode_state, pivot);
				if (!result.valid) {
					return TResult(std::move(result.Input), false, __decode_state);
				}
				__working_input = std::move(result.Input);
				if (Krys::Ranges::empty(__working_input)) {
					if (!::is_state_complete(__from_encoding, __decode_state)) {
						continue;
					}
					if (!::is_state_complete(__to_encoding, __encode_state)) {
						continue;
					}
					break;
				}
			}
			return TResult(Krys::Ranges::Reconstruct(std::in_place_type<_WorkingInput>, std::move(__working_input)),
				true, __decode_state, __encode_state);
		}
		else if constexpr (is_detected_v<Impl::HasInternalTextValidateTranscodableAsOne,
			                   _WorkingInput, TFromEncoding, TToEncoding, TDecodeState, TEncodeState, TPivot>) {
			(void)__encode_state;
			for (;;) {
				auto result = InternalTextValidateTranscodableAsOne(Krys::Tag<_UFromEncoding, _UToEncoding> {},
					__working_input, __from_encoding, __to_encoding, __decode_state, pivot);
				if (!result.valid) {
					return TResult(std::move(result.Input), false, __decode_state);
				}
				__working_input = std::move(result.Input);
				if (Krys::Ranges::empty(__working_input)) {
					if (!::is_state_complete(__from_encoding, __decode_state)) {
						continue;
					}
					if (!::is_state_complete(__to_encoding, __encode_state)) {
						continue;
					}
					break;
				}
			}
			return TResult(Krys::Ranges::Reconstruct(std::in_place_type<_WorkingInput>, std::move(__working_input)),
				true, __decode_state, __encode_state);
		}
		else {
			using TCodeUnit = code_unit_t<_UToEncoding>;

			TCodeUnit __output_storage[MaxCodeUnits<_UToEncoding>] {};
			Span<TCodeUnit, MaxCodeUnits<_UToEncoding>> output(__output_storage);

			pass_handler_t __handler {};

			for (;;) {
				auto __transcode_result
					= ::transcode_one_into_raw(std::move(__working_input), __from_encoding, output,
					     __to_encoding, __handler, __handler, __decode_state, __encode_state, pivot);
				if (__transcode_result.ErrorCode != EncodingError::OK) {
					return TResult(
						Krys::Ranges::Reconstruct(std::in_place_type<_WorkingInput>, std::move(__working_input)),
						false, __decode_state, __encode_state);
				}
				__working_input = std::move(__transcode_result.Input);
				if (Krys::Ranges::empty(__working_input)) {
					if (!::is_state_complete(__from_encoding, __decode_state)) {
						continue;
					}
					if (!::is_state_complete(__to_encoding, __encode_state)) {
						continue;
					}
					break;
				}
			}
			return TResult(Krys::Ranges::Reconstruct(std::in_place_type<_WorkingInput>, std::move(__working_input)),
				true, __decode_state, __encode_state);
		}
	}

	
	/// @brief Validates the code units of the `input` according to the `encoding` with the given states @p
	/// __decode_state and `__encode_state` to see if it can be turned into code points.
	///
	/// @param[in] input The input range of code units to validate is possible for encoding into code points.
	/// @param[in] __from_encoding The encoding to verify can properly encode the input of code units.
	/// @param[in] __to_encoding The encoding to verify can properly encode the input of code units.
	/// @param[in, out] __decode_state The state to use for the decoding portion of the validation check.
	/// @param[in, out] __encode_state The state to use for the encoding portion of the validation check.
	/// @param[in, out] pivot A reference to a descriptor of a (potentially usable) pivot range, usually a range of
	/// contiguous data from a span provided by the implementation but customizable by the end-user. If the
	/// intermediate conversion is what failed, then the pivot's `ErrorCode` member will be set to that
	/// error. This only happens if the overall operation also fails, and need not be checked unless to obtain
	/// additional information for when a top-level operation fails.
	///
	/// @remarks This functions checks to see if extension points for `TextValidateTranscodableAs` is available
	/// taking the available 4 parameters. If so, it calls this. Otherwise, it defers to
	/// validate_transcodable_as.
	template <typename TInput, typename TFromEncoding, typename TToEncoding, typename TDecodeState,
		typename TEncodeState, typename TPivot>
	constexpr auto validate_transcodable_as(TInput&& input, TFromEncoding&& __from_encoding,
		TToEncoding&& __to_encoding, TDecodeState& __decode_state, TEncodeState& __encode_state, TPivot&& pivot) {
		using _UFromEncoding = remove_cvref_t<TToEncoding>;
		using _UToEncoding   = remove_cvref_t<TFromEncoding>;
		if constexpr (is_detected_v<Impl::HasTextValidateTranscodableAs, TInput, TFromEncoding,
			              TToEncoding, TDecodeState, TEncodeState, TPivot>) {
			(void)__encode_state;
			return TextValidateTranscodableAs(Krys::Tag<_UFromEncoding, _UToEncoding> {},
				std::forward<TInput>(input), std::forward<TFromEncoding>(__from_encoding),
				std::forward<TToEncoding>(__to_encoding), __decode_state, __encode_state, pivot);
		}
		else if constexpr (is_detected_v<Impl::HasTextValidateTranscodableAs, TInput,
			                   TFromEncoding, TToEncoding, TDecodeState, TEncodeState, TPivot>) {
			return TextValidateTranscodableAs(Krys::Tag<_UFromEncoding, _UToEncoding> {},
				std::forward<TInput>(input), std::forward<TFromEncoding>(__from_encoding),
				std::forward<TToEncoding>(__to_encoding), __decode_state, __encode_state, pivot);
		}
		else if constexpr (is_detected_v<Impl::HasInternalTextValidateTranscodableAs, TInput,
			                   TFromEncoding, TToEncoding, TDecodeState, TEncodeState, TPivot>) {
			(void)__encode_state;
			return InternalTextValidateTranscodableAs(Krys::Tag<_UFromEncoding, _UToEncoding> {},
				std::forward<TInput>(input), std::forward<TFromEncoding>(__from_encoding),
				std::forward<TToEncoding>(__to_encoding), __decode_state, __encode_state, pivot);
		}
		else {
			return basicTValidate_transcodable_as(std::forward<TInput>(input),
				std::forward<TFromEncoding>(__from_encoding), std::forward<TToEncoding>(__to_encoding),
				__decode_state, __encode_state, pivot);
		}
	}

	
	/// @brief Validates the code units of the `input` according to the `encoding` with the given states @p
	/// __decode_state and `__encode_state` to see if it can be turned into code points.
	///
	/// @param[in] input The input range of code units to validate is possible for encoding into code points.
	/// @param[in] __from_encoding The encoding to verify can properly encode the input of code units.
	/// @param[in] __to_encoding The encoding to verify can properly encode the input of code units.
	/// @param[in, out] __decode_state The state to use for the decoding portion of the validation check.
	/// @param[in, out] __encode_state The state to use for the encoding portion of the validation check.
	///
	/// @remarks This functions checks to see if extension points for `TextValidateTranscodableAs` is available
	/// taking the available 4 parameters. If so, it calls this. Otherwise, it defers to
	/// validate_transcodable_as.
	template <typename TInput, typename TFromEncoding, typename TToEncoding, typename TDecodeState,
		typename TEncodeState>
	constexpr auto validate_transcodable_as(TInput&& input, TFromEncoding&& __from_encoding,
		TToEncoding&& __to_encoding, TDecodeState& __decode_state, TEncodeState& __encode_state) {
		using _UFromEncoding                 = remove_cvref_t<TFromEncoding>;
		using TCodePoint                     = code_point_t<_UFromEncoding>;
		constexpr std::size_t __max_points = MaxCodePoints<_UFromEncoding>;
		TCodePoint intermediate[__max_points] {};
		Span<TCodePoint, __max_points> pivot(intermediate);
		return validate_transcodable_as(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TToEncoding>(__to_encoding),
			__decode_state, __encode_state, pivot);
	}

	
	/// @brief Validates the code units of the `input` according to the `__from_encoding` object with the given
	/// state `__decode_state` to see if it can be turned into code units of the `__to_encoding` object.
	///
	/// @param[in] input The input range of code units to validate is possible for encoding into code points.
	/// @param[in] __from_encoding The encoding to verify can properly encode the input of code units.
	/// @param[in] __to_encoding The encoding to verify can properly encode the input of code units.
	/// @param[in, out] __decode_state The state to use for the decoding portion of the validation check.
	///
	/// @remarks This functions will call Krys::CreateEncodeState with `__to_encoding` to create a default @p
	/// EncodeState.
	template <typename TInput, typename TFromEncoding, typename TToEncoding, typename TDecodeState>
	constexpr auto validate_transcodable_as(TInput&& input, TFromEncoding&& __from_encoding,
		TToEncoding&& __to_encoding, TDecodeState& __decode_state) {
		auto __encode_state = Krys::CreateEncodeState(__to_encoding);
		auto result       = validate_transcodable_as(std::forward<TInput>(input),
			      std::forward<TFromEncoding>(__from_encoding), std::forward<TToEncoding>(__to_encoding),
			      __decode_state, __encode_state);
		return Impl::SliceToStateless(std::move(result));
	}

	
	/// @brief Validates the code units of the `input` according to the `__from_encoding` object to see if it can
	/// be turned into code units of the `__to_encoding` object.
	///
	/// @param[in] input The input range of code units to validate is possible for encoding into code points.
	/// @param[in] __from_encoding The encoding to verify can properly encode the input of code units.
	/// @param[in] __to_encoding The encoding to verify can properly encode the input of code units.
	///
	/// @remarks This functions will call CreateDecodeState with the `__from_encoding` object to create a
	/// default `DecodeState` to use before passing it to the next overload.
	template <typename TInput, typename TFromEncoding, typename TToEncoding>
	constexpr auto validate_transcodable_as(
		TInput&& input, TFromEncoding&& __from_encoding, TToEncoding&& __to_encoding) {
		auto __decode_state = CreateDecodeState(__from_encoding);
		return validate_transcodable_as(std::forward<TInput>(input),
			std::forward<TFromEncoding>(__from_encoding), std::forward<TToEncoding>(__to_encoding),
			__decode_state);
	}

	
	/// @brief Validates the code units of the `input` according to the `__from_encoding` object to see if it can
	/// be turned into code units of the `__to_encoding` object.
	///
	/// @param[in] input The input range of code units to validate is possible for encoding into code points.
	/// @param[in] __to_encoding The encoding to verify can properly encode the input of code units.
	///
	/// @remarks This functions will call Krys::CreateEncodeState with `__to_encoding` to create a default @p
	/// EncodeState.
	template <typename TInput, typename TToEncoding>
	constexpr auto validate_transcodable_as(TInput&& input, TToEncoding&& __to_encoding) {
		using TUInput   = remove_cvref_t<TInput>;
		using TCodeUnit = remove_cvref_t<ranges::rangeTValue_type_t<TUInput>>;
#if ZTD_IS_ON(ZTD_STD_LIBRARY_IS_CONSTANT_EVALUATED)
		if (std::is_constant_evaluated()) {
			// Use literal encoding instead, if we meet the right criteria
			using TFromEncoding = default_consteval_code_unit_encoding_t<TCodeUnit>;
			TFromEncoding __from_encoding {};
			return validate_transcodable_as(
				std::forward<TInput>(input), __from_encoding, std::forward<TToEncoding>(__to_encoding));
		}
		else
#endif
		{
			using TFromEncoding = default_code_unit_encoding_t<TCodeUnit>;
			TFromEncoding __from_encoding {};
			return validate_transcodable_as(
				std::forward<TInput>(input), __from_encoding, std::forward<TToEncoding>(__to_encoding));
		}
	}

	
	/// @}

	
}



#endif
