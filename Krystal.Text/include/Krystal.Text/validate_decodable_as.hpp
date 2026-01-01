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

#ifndef ZTD_TEXTTValIDATE_DECODABLE_AS_HPP
#define ZTD_TEXTTValIDATE_DECODABLE_AS_HPP



#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include <ztd/text/max_units.hpp>
#include <ztd/text/default_encoding.hpp>
#include <ztd/text/ValidateResult.hpp>
#include <ztd/text/error_handler.hpp>
#include "Krystal.Text/State.hpp"
#include <ztd/text/transcode_one.hpp>
#include <ztd/text/detail/is_lossless.hpp>
#include <ztd/text/detail/encoding_range.hpp>
#include <ztd/text/char_predicates.hpp>

#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Utils/Tag.hpp"

#include <algorithm>
#include <string_view>



namespace Krys {
	

	
	/// @addtogroup ztd_textTValidate_decodable_as ValidateDecodableAs
	/// @brief These functions check if the given input of code points will decode without an error. Note that this
	/// does not mean that an error handler will be invoked that can "smooth over" any possible errors: this checks
	/// solely if it will decode from code units into code points cleanly.
	/// @{

	
	/// @brief Validates the code units of the `input` according to the `encoding` with the given states @p
	/// __decode_state and `__encode_state` to see if it can be turned into code points.
	///
	/// @param[in] input The input range of code units to validate is possible for encoding into code points.
	/// @param[in] encoding The encoding to verify can properly encode the input of code units.
	/// @param[in] __decode_state The state to use for the decoding portion of the validation check.
	/// @param[in] __encode_state The state to use for the encoding portion of the validation check.
	///
	/// @remarks This function explicitly does not call any extension points. It defers to doing a typical loop over
	/// the code points to verify it can be decoded into code points, and then encoded back into code units, with no
	/// errors and with the exact same value sequence as the original.
	template <typename TInput, typename TEncoding, typename TDecodeState, typename TEncodeState>
	constexpr auto basicTValidate_decodable_as(
		TInput&& input, TEncoding&& encoding, TDecodeState& __decode_state, TEncodeState& __encode_state) {
		using TUEncoding    = remove_cvref_t<TEncoding>;
		using _InitialInput = Impl::span_reconstruct_t<TInput, TInput>;
		using _WorkingInput = Krys::Ranges::subrange_for_t<_InitialInput>;
		using TResult       = ValidatePivotlessTranscodeResult<_WorkingInput, TDecodeState, TEncodeState>;

		_WorkingInput __working_input(Impl::SpanReconstruct<TInput>(std::forward<TInput>(input)));

		if constexpr (is_detected_v<Impl::HasTextValidateDecodableAsOne, _WorkingInput,
			              TEncoding, TDecodeState, TEncodeState>) {
			for (;;) {
				auto result = TextValidateDecodableAsOne(Krys::Tag<TUEncoding> {}, encoding,
					std::move(__working_input), __decode_state, __encode_state);
				if (!result.valid) {
					return TResult(std::move(result.Input), false, __decode_state, __encode_state);
				}
				__working_input = std::move(result.Input);
				if (Krys::Ranges::empty(__working_input)) {
					if (!::is_state_complete(encoding, __decode_state)) {
						continue;
					}
					if (!::is_state_complete(encoding, __encode_state)) {
						continue;
					}
					break;
				}
			}
			return TResult(std::move(__working_input), true, __decode_state, __encode_state);
		}
		else if constexpr (is_detected_v<Impl::HasTextValidateDecodableAsOne, _InitialInput,
			                   TEncoding, TDecodeState>) {
			for (;;) {
				auto result = TextValidateDecodableAsOne(
					Krys::Tag<TUEncoding> {}, encoding, std::move(__working_input), __decode_state);
				if (!result.valid) {
					return TResult(std::move(result.Input), false, __decode_state, __encode_state);
				}
				__working_input = std::move(result.Input);
				if (Krys::Ranges::empty(__working_input)) {
					if (!::is_state_complete(encoding, __decode_state)) {
						continue;
					}
					break;
				}
			}
			return TResult(std::move(__working_input), true, __decode_state, __encode_state);
		}
		else if constexpr (is_detected_v<Impl::HasInternalTextValidateDecodableAsOne,
			                   _InitialInput, TEncoding, TDecodeState, TEncodeState>) {
			for (;;) {
				auto result = InternalTextValidateDecodableAsOne(Krys::Tag<TUEncoding> {}, encoding,
					std::move(__working_input), __decode_state, __encode_state);
				if (!result.valid) {
					return TResult(std::move(result.Input), false, __decode_state, __encode_state);
				}
				__working_input = std::move(result.Input);
				if (Krys::Ranges::empty(__working_input)) {
					if (!::is_state_complete(encoding, __decode_state)) {
						continue;
					}
					if (!::is_state_complete(encoding, __encode_state)) {
						continue;
					}
					break;
				}
			}
			return TResult(std::move(__working_input), true, __decode_state, __encode_state);
		}
		else if constexpr (is_detected_v<Impl::HasInternalTextValidateDecodableAsOne,
			                   _InitialInput, TEncoding, TDecodeState>) {

			for (;;) {
				auto result = InternalTextValidateDecodableAsOne(
					Krys::Tag<TUEncoding> {}, encoding, std::move(__working_input), __decode_state);
				if (!result.valid) {
					return TResult(std::move(result.Input), false, __decode_state, __encode_state);
				}
				__working_input = std::move(result.Input);
				if (Krys::Ranges::empty(__working_input)) {
					if (!::is_state_complete(encoding, __decode_state)) {
						continue;
					}
					break;
				}
			}
			return TResult(std::move(__working_input), true, __decode_state, __encode_state);
		}
		else {
			constexpr std::size_t __code_unit_max  = max_transcode_code_units_v<TUEncoding, TUEncoding>;
			constexpr std::size_t __code_point_max = MaxCodePoints<TUEncoding>;
			using TCodeUnit                          = code_unit_t<TUEncoding>;
			using TCodePoint                         = code_point_t<TUEncoding>;
			using TPivot                             = Span<TCodePoint, __code_point_max>;

			TCodePoint __code_point_buf[__code_point_max] {};
			TCodeUnit __code_unit_buf[__code_unit_max] {};
			Span<TCodeUnit, __code_unit_max> __code_unit_view(__code_unit_buf);
			TPivot pivot(__code_point_buf);

			for (;;) {
				auto result = ::transcode_one_into_raw(std::move(__working_input), encoding,
					__code_unit_view, encoding, pass_handler, pass_handler, __decode_state, __encode_state,
					pivot);
				if (result.ErrorCode != EncodingError::OK) {
					return TResult(std::move(result.Input), false, __decode_state, __encode_state);
				}
				const bool __is_transcode_roundtrip_okay
					= Krys::Ranges::equal(Krys::Ranges::cbegin(__working_input),
					     Krys::Ranges::cbegin(result.Input), __code_unit_view.data(),
					     std::to_address(Krys::Ranges::cbegin(result.Output)), ::equal_to_char);
				if (!__is_transcode_roundtrip_okay) {
					return TResult(std::move(result.Input), false, __decode_state, __encode_state);
				}
				__working_input = std::move(result.Input);
				if (Krys::Ranges::empty(__working_input)) {
					if (!::is_state_complete(encoding, __decode_state)) {
						continue;
					}
					if (!::is_state_complete(encoding, __encode_state)) {
						continue;
					}
					break;
				}
			}
			return TResult(std::move(__working_input), true, __decode_state, __encode_state);
		}
	}

	
	/// @brief Validates the code units of the `input` according to the `encoding` with the given states @p
	/// __decode_state and `__encode_state` to see if it can be turned into code points.
	///
	/// @param[in] input The input range of code units to validate is possible for encoding into code points.
	/// @param[in] encoding The encoding to verify can properly encode the input of code units.
	/// @param[in] __decode_state The state to use for the decoding portion of the validation check.
	/// @param[in] __encode_state The state to use for the encoding portion of the validation check.
	///
	/// @remarks This functions checks to see if extension points for `TextValidateDecodableAs` is available taking
	/// the available 4 parameters. If so, it calls this. Otherwise, it defers to ValidateDecodableAs.
	template <typename TInput, typename TEncoding, typename TDecodeState, typename TEncodeState>
	constexpr auto ValidateDecodableAs(
		TInput&& input, TEncoding&& encoding, TDecodeState& __decode_state, TEncodeState& __encode_state) {
		if constexpr (is_detected_v<Impl::HasTextValidateDecodableAs, TInput, TEncoding,
			              TDecodeState>) {
			return TextValidateDecodableAs(Krys::Tag<remove_cvref_t<TEncoding>> {},
				std::forward<TInput>(input), std::forward<TEncoding>(encoding), __decode_state,
				__encode_state);
		}
		else if constexpr (is_detected_v<Impl::HasTextValidateDecodableAs, TInput, TEncoding,
			                   TDecodeState>) {
			(void)__encode_state;
			return TextValidateDecodableAs(Krys::Tag<remove_cvref_t<TEncoding>> {},
				std::forward<TInput>(input), std::forward<TEncoding>(encoding), __decode_state);
		}
		else if constexpr (is_detected_v<Impl::HasInternalTextValidateDecodableAs, TInput,
			                   TEncoding, TDecodeState>) {
			return InternalTextValidateDecodableAs(Krys::Tag<remove_cvref_t<TEncoding>> {},
				std::forward<TInput>(input), std::forward<TEncoding>(encoding), __decode_state,
				__encode_state);
		}
		else if constexpr (is_detected_v<Impl::HasInternalTextValidateDecodableAs, TInput,
			                   TEncoding, TDecodeState>) {
			(void)__encode_state;
			return InternalTextValidateDecodableAs(Krys::Tag<remove_cvref_t<TEncoding>> {},
				std::forward<TInput>(input), std::forward<TEncoding>(encoding), __decode_state);
		}
		else {
			return basicTValidate_decodable_as(std::forward<TInput>(input),
				std::forward<TEncoding>(encoding), __decode_state, __encode_state);
		}
	}

	
	/// @brief Validates the code units of the `input` according to the `encoding` with the given state @p
	/// __decode_state to see if it can be turned into code points.
	///
	/// @param[in] input The input range of code units to validate is possible for encoding into code points.
	/// @param[in] encoding The encoding to verify can properly encode the input of code units.
	/// @param[in] __decode_state The state to use for the decoding portion of the validation check.
	///
	/// @remarks This functions checks to see if extension points for `TextValidateDecodableAs` is available taking
	/// the available 3 parameters. If so, it calls this. Otherwise, it creates an encoding state through
	/// Krys::CreateEncodeState before calling ValidateDecodableAs(input, encoding,
	/// __decode_state,
	/// __encode_state).
	template <typename TInput, typename TEncoding, typename TDecodeState>
	constexpr auto ValidateDecodableAs(TInput&& input, TEncoding&& encoding, TDecodeState& __decode_state) {
		using TUEncoding = remove_cvref_t<TEncoding>;
		if constexpr (is_detected_v<Impl::HasTextValidateDecodableAs, TInput, TEncoding,
			              TDecodeState>) {
			return TextValidateDecodableAs(Krys::Tag<remove_cvref_t<TEncoding>> {},
				std::forward<TInput>(input), std::forward<TEncoding>(encoding), __decode_state);
		}
		else if constexpr (is_detected_v<Impl::HasInternalTextValidateDecodableAs, TInput,
			                   TEncoding, TDecodeState>) {
			return InternalTextValidateDecodableAs(Krys::Tag<remove_cvref_t<TEncoding>> {},
				std::forward<TInput>(input), std::forward<TEncoding>(encoding), __decode_state);
		}
		else {
			using TState = EncodeState<TUEncoding>;

			TState __encode_state  = make_encode_state_with(encoding, __decode_state);
			auto __stateful_result = ValidateDecodableAs(std::forward<TInput>(input),
				std::forward<TEncoding>(encoding), __decode_state, __encode_state);
			if constexpr (IsSpecializationOf<decltype(__stateful_result), ValidateResult>) {
				return __stateful_result;
			}
			else {
				return Impl::DropSingleState(std::move(__stateful_result));
			}
		}
	}

	
	/// @brief Validates the code units of the `input` according to the `encoding` to see if they can be turned
	/// into code points.
	///
	/// @param[in] input The input range of code units to validate is possible for encoding into code points.
	/// @param[in] encoding The encoding to verify can properly encode the input of code units.
	///
	/// @remarks This functions creates an encoding `state` through CreateDecodeState before calling
	/// the next overload of ValidateDecodableAs.
	template <typename TInput, typename TEncoding>
	constexpr auto ValidateDecodableAs(TInput&& input, TEncoding&& encoding) {
		using TUEncoding = remove_cvref_t<TEncoding>;
		using TState     = DecodeState<TUEncoding>;

		TState state = CreateDecodeState(encoding);
		auto __stateful_result
			= ValidateDecodableAs(std::forward<TInput>(input), std::forward<TEncoding>(encoding), state);
		return Impl::SliceToStateless(std::move(__stateful_result));
	}

	
	/// @brief Validates the code units of the `input` to see if it can be turned into code points.
	///
	/// @param[in] input The input range of code units to validate is possible for encoding into code points.
	///
	/// @returns A StatelessValidateResult detailing whether or not the input code points can be turned
	/// into code units of the corresponding encoding.
	///
	/// @remarks This functions creates an `encoding` by passing the `value_type` of the `input` range through
	/// default_code_unit_encoding.
	template <typename TInput>
	constexpr auto ValidateDecodableAs(TInput&& input) {
		using TUInput   = remove_cvref_t<TInput>;
		using TCodeUnit = remove_cvref_t<ranges::rangeTValue_type_t<TUInput>>;
#if ZTD_IS_ON(ZTD_STD_LIBRARY_IS_CONSTANT_EVALUATED)
		if (std::is_constant_evaluated()) {
			// Use literal encoding instead, if we meet the right criteria
			using TEncoding = default_consteval_code_unit_encoding_t<TCodeUnit>;
			TEncoding encoding {};
			return ValidateDecodableAs(std::forward<TInput>(input), encoding);
		}
		else
#endif
		{
			using TEncoding = default_code_unit_encoding_t<TCodeUnit>;
			TEncoding encoding {};
			return ValidateDecodableAs(std::forward<TInput>(input), encoding);
		}
	}

	
	/// @}

	
}



#endif
