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

#ifndef ZTD_TEXTTValIDATE_ENCODABLE_AS_HPP
#define ZTD_TEXTTValIDATE_ENCODABLE_AS_HPP



#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include <ztd/text/max_units.hpp>
#include <ztd/text/default_encoding.hpp>
#include <ztd/text/ValidateResult.hpp>
#include <ztd/text/error_handler.hpp>
#include "Krystal.Text/State.hpp"
#include <ztd/text/recode_one.hpp>
#include <ztd/text/detail/is_lossless.hpp>
#include <ztd/text/detail/encoding_range.hpp>
#include <ztd/text/char_predicates.hpp>

#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Lib/Ranges/Algorithm.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Utils/Tag.hpp"

#include <algorithm>
#include <string_view>



namespace Krys {
	

	
	/// @addtogroup ztd_textTValidate_encodable_as ValidateEncodableAs
	/// @brief These functions check if the given input of code points will encode without an error. Note that this
	/// does not mean that an error handler will be invoked that can "smooth over" any possible errors: this checks
	/// solely if it will encode from code points into code units cleanly.
	/// @{

	
	/// @brief Validates the code points of the `input` according to the `encoding` with the given states @p
	/// __encode_state and `__decode_state`.
	///
	/// @param[in] input The input range of code points to validate is possible for encoding into code units.
	/// @param[in] encoding The encoding to verify can properly encode the input of code units.
	/// @param[in] __encode_state The state to use for the encoding portion of the validation check.
	/// @param[in] __decode_state The state to use for the decoding portion of the validation check, if needed.
	///
	/// @remarks This function explicitly does not check any of the extension points. It defers to doing a typical loop
	/// over the code points to verify it can be encoded into code units, and then decoded into code points, with no
	/// errors.
	template <typename TInput, typename TEncoding, typename TEncodeState, typename TDecodeState>
	constexpr auto basicTValidate_encodable_as(
		TInput&& input, TEncoding&& encoding, TEncodeState& __encode_state, TDecodeState& __decode_state) {
		using _WorkingInput = Krys::Ranges::subrange_for_t<Impl::span_reconstruct_t<TInput, TInput>>;
		using TUEncoding    = remove_cvref_t<TEncoding>;
		using TResult       = ValidatePivotlessTranscodeResult<_WorkingInput, TEncodeState, TDecodeState>;

		_WorkingInput __working_input = Impl::SpanReconstruct<TInput>(std::forward<TInput>(input));

		if constexpr (is_detected_v<Impl::HasTextValidateEncodableAsOne, _WorkingInput,
			              TEncoding, TEncodeState, TDecodeState>) {
			for (;;) {
				auto result = TextValidateEncodableAsOne(Krys::Tag<TUEncoding> {},
					std::move(__working_input), encoding, __encode_state, __decode_state);
				if (!result.valid) {
					return TResult(std::move(result.Input), false, __encode_state, __decode_state);
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
			return TResult(std::move(__working_input), true, __encode_state, __decode_state);
		}
		else if constexpr (is_detected_v<Impl::HasTextValidateEncodableAsOne, _WorkingInput,
			                   TEncoding, TEncodeState>) {
			for (;;) {
				auto result = TextValidateEncodableAsOne(
					Krys::Tag<TUEncoding> {}, std::move(__working_input), encoding, __encode_state);
				if (!result.valid) {
					return TResult(std::move(result.Input), false, __encode_state, __decode_state);
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
			return TResult(std::move(__working_input), true, __encode_state, __decode_state);
		}
		else if constexpr (is_detected_v<Impl::HasInternalTextValidateEncodableAsOne,
			                   _WorkingInput, TEncoding, TEncodeState, TDecodeState>) {
			for (;;) {
				auto result = TextValidateEncodableAsOne(Krys::Tag<TUEncoding> {},
					std::move(__working_input), encoding, __encode_state, __decode_state);
				if (!result.valid) {
					return TResult(std::move(result.Input), false, __encode_state, __decode_state);
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
			return TResult(std::move(__working_input), true, __encode_state, __decode_state);
		}
		else if constexpr (is_detected_v<Impl::HasInternalTextValidateEncodableAsOne,
			                   _WorkingInput, TEncoding, TEncodeState>) {
			for (;;) {
				auto result = InternalTextValidateEncodableAsOne(
					Krys::Tag<TUEncoding> {}, std::move(__working_input), encoding, __encode_state);
				if (!result.valid) {
					return TResult(std::move(result.Input), false, __encode_state, __decode_state);
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
			return TResult(std::move(__working_input), true, __encode_state, __decode_state);
		}
		else if constexpr (is_detected_v<Impl::HasInternalTextValidateEncodableAsOne,
			                   _WorkingInput, TEncoding, TEncodeState>) {
			for (;;) {
				auto result = InternalTextValidateEncodableAsOne(
					Krys::Tag<TUEncoding> {}, std::move(__working_input), encoding, __encode_state);
				if (!result.valid) {
					return TResult(std::move(result.Input), false, __encode_state, __decode_state);
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
			return TResult(std::move(__working_input), true, __encode_state, __decode_state);
		}
		else {
			constexpr std::size_t __code_point_max = max_recode_code_points_v<TUEncoding, TUEncoding>;
			constexpr std::size_t __code_unit_max  = MaxCodeUnits<TUEncoding>;
			using TCodeUnit                          = code_unit_t<TUEncoding>;
			using TCodePoint                         = code_point_t<TUEncoding>;
			using TPivot                             = Span<TCodeUnit, __code_unit_max>;

			TCodePoint __code_point_buf[__code_point_max] {};
			TCodeUnit __code_unit_buf[__code_unit_max] {};
			Span<TCodePoint, __code_point_max> __code_point_view(__code_point_buf);
			TPivot pivot(__code_unit_buf);

			for (;;) {
				auto result
					= ::recode_one_into_raw(std::move(__working_input), encoding, __code_point_view,
					     encoding, pass_handler, pass_handler, __encode_state, __decode_state, pivot);
				if (result.ErrorCode != EncodingError::OK) {
					return TResult(std::move(result.Input), false, __encode_state, __decode_state);
				}
				const bool __is_recode_roundtrip_okay = Krys::Ranges::equal(Krys::Ranges::cbegin(__working_input),
					Krys::Ranges::cbegin(result.Input), __code_point_view.data(),
					std::to_address(Krys::Ranges::cbegin(result.Output)), ::equal_to_char);
				if (!__is_recode_roundtrip_okay) {
					return TResult(std::move(result.Input), false, __encode_state, __decode_state);
				}
				__working_input = std::move(result.Input);
				if (Krys::Ranges::empty(__working_input)) {
					if (!::is_state_complete(encoding, __encode_state)) {
						continue;
					}
					if (!::is_state_complete(encoding, __decode_state)) {
						continue;
					}
					break;
				}
			}
			return TResult(std::move(__working_input), true, __encode_state, __decode_state);
		}
	}

	
	/// @brief Validates the code points of the `input` according to the `encoding` with the given states @p
	/// __encode_state and `__decode_state`.
	///
	/// @param[in] input The input range of code points to validate is possible for encoding into code units.
	/// @param[in] encoding The encoding to verify can properly encode the input of code units.
	/// @param[in] __encode_state The state to use for the encoding portion of the validation check.
	/// @param[in] __decode_state The state to use for the decoding portion of the validation check, if needed.
	///
	/// @remarks This functions checks to see if extension points for `TextValidateEncodableAs` is available taking
	/// the available 4 parameters. If so, it calls this. Otherwise, it defers to doing a typical loop over the code
	/// points to verify it can be encoded into code units, and then decoded into code points, with no errors.
	template <typename TInput, typename TEncoding, typename TEncodeState, typename TDecodeState>
	constexpr auto ValidateEncodableAs(
		TInput&& input, TEncoding&& encoding, TEncodeState& __encode_state, TDecodeState& __decode_state) {
		if constexpr (is_detected_v<Impl::HasTextValidateEncodableAs, TInput, TEncoding,
			              TEncodeState, TDecodeState>) {
			return TextValidateEncodableAs(Krys::Tag<remove_cvref_t<TEncoding>> {},
				std::forward<TInput>(input), std::forward<TEncoding>(encoding), __encode_state,
				__decode_state);
		}
		else if constexpr (is_detected_v<Impl::HasTextValidateEncodableAs, TInput, TEncoding,
			                   TEncodeState>) {
			(void)__decode_state;
			return TextValidateEncodableAs(Krys::Tag<remove_cvref_t<TEncoding>> {},
				std::forward<TInput>(input), std::forward<TEncoding>(encoding), __encode_state);
		}
		else if constexpr (is_detected_v<Impl::HasInternalTextValidateEncodableAs, TInput,
			                   TEncoding, TEncodeState>) {
			(void)__decode_state;
			return InternalTextValidateEncodableAs(Krys::Tag<remove_cvref_t<TEncoding>> {},
				std::forward<TInput>(input), std::forward<TEncoding>(encoding), __encode_state);
		}
		else if constexpr (is_detected_v<Impl::HasInternalTextValidateEncodableAs, TInput,
			                   TEncoding, TEncodeState, TDecodeState>) {
			return InternalTextValidateEncodableAs(Krys::Tag<remove_cvref_t<TEncoding>> {},
				std::forward<TInput>(input), std::forward<TEncoding>(encoding), __encode_state,
				__decode_state);
		}
		else {
			return basicTValidate_encodable_as(std::forward<TInput>(input),
				std::forward<TEncoding>(encoding), __encode_state, __decode_state);
		}
	}

	
	/// @brief Validates the code points of the `input` according to the `encoding` with the given states @p
	/// "__encode_state".
	///
	/// @param[in] input The input range of code points to validate is possible for encoding into code units.
	/// @param[in] encoding The encoding to verify can properly encode the input of code units.
	/// @param[in] __encode_state The state for encoding to use.
	///
	/// @remarks This functions checks to see if extension points for `TextValidateEncodableAs` is available taking
	/// the available 3 parameters. If so, it calls this. Otherwise, it defers to ValidateEncodableAs.
	template <typename TInput, typename TEncoding, typename TEncodeState>
	constexpr auto ValidateEncodableAs(TInput&& input, TEncoding&& encoding, TEncodeState& __encode_state) {
		using TUEncoding = remove_cvref_t<TEncoding>;
		if constexpr (is_detected_v<Impl::HasTextValidateEncodableAs, TInput, TEncoding,
			              TEncodeState>) {
			return TextValidateEncodableAs(Krys::Tag<remove_cvref_t<TEncoding>> {},
				std::forward<TInput>(input), std::forward<TEncoding>(encoding), __encode_state);
		}
		else if constexpr (is_detected_v<Impl::HasInternalTextValidateEncodableAs, TInput,
			                   TEncoding, TEncodeState>) {
			return InternalTextValidateEncodableAs(Krys::Tag<remove_cvref_t<TEncoding>> {},
				std::forward<TInput>(input), std::forward<TEncoding>(encoding), __encode_state);
		}
		else {
			using TState = DecodeState<TUEncoding>;

			TState __decode_state  = make_decode_state_with(encoding, __encode_state);
			auto __stateful_result = ValidateEncodableAs(std::forward<TInput>(input),
				std::forward<TEncoding>(encoding), __encode_state, __decode_state);
			if constexpr (IsSpecializationOf<decltype(__stateful_result), ValidateResult>) {
				return __stateful_result;
			}
			else {
				return Impl::DropSingleState(std::move(__stateful_result));
			}
		}
	}

	
	/// @brief Validates the code points of the `input` according to the @p "encoding".
	///
	/// @param[in] input The input range of code points to validate is possible for encoding into code units.
	/// @param[in] encoding The encoding to verify can properly encode the input of code units.
	template <typename TInput, typename TEncoding>
	constexpr auto ValidateEncodableAs(TInput&& input, TEncoding&& encoding) {
		using TUEncoding = remove_cvref_t<TEncoding>;
		using TState     = EncodeState<TUEncoding>;

		TState state = Krys::CreateEncodeState(encoding);
		auto __stateful_result
			= ValidateEncodableAs(std::forward<TInput>(input), std::forward<TEncoding>(encoding), state);
		return Impl::SliceToStateless(std::move(__stateful_result));
	}

	
	/// @brief Validates the code points of the input.
	///
	/// @param[in] input The input range of code points to validate is possible for encoding into code units.
	///
	/// @remarks This passes the default encoding as inferred from the discernible `value_type` of the input range
	/// input into the default_code_point_encoding.
	template <typename TInput>
	constexpr auto ValidateEncodableAs(TInput&& input) {
		using TUInput    = remove_cvref_t<TInput>;
		using TCodePoint = remove_cvref_t<ranges::rangeTValue_type_t<TUInput>>;
#if ZTD_IS_ON(ZTD_STD_LIBRARY_IS_CONSTANT_EVALUATED)
		if (std::is_constant_evaluated()) {
			// Use literal encoding instead, if we meet the right criteria
			using TEncoding = default_consteval_code_point_encoding_t<TCodePoint>;
			TEncoding encoding {};
			return ValidateEncodableAs(std::forward<TInput>(input), encoding);
		}
		else
#endif
		{
			using TEncoding = default_code_point_encoding_t<TCodePoint>;
			TEncoding encoding {};
			return ValidateEncodableAs(std::forward<TInput>(input), encoding);
		}
	}

	
	/// @}

	
}



#endif
