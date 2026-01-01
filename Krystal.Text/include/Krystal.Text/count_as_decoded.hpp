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

#ifndef ZTD_TEXT_count_as_decoded_HPP
#define ZTD_TEXT_count_as_decoded_HPP



#include "Krystal.Text/CodeUnit.hpp"
#include <ztd/text/default_encoding.hpp>
#include <ztd/text/CountResult.hpp>
#include <ztd/text/error_handler.hpp>
#include "Krystal.Text/State.hpp"
#include <ztd/text/DecodeOne.hpp>
#include <ztd/text/detail/is_lossless.hpp>
#include <ztd/text/detail/encoding_range.hpp>

#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Text/CharTraits.hpp"
#include "Krystal.Lib/Utils/Tag.hpp"

#include <string_view>



namespace Krys {
	

	
	/// @addtogroup ztd_text_count_as_decoded count_as_decoded
	/// @brief These functions use a variety of means to count the number of code units that will result from the input
	/// code points.
	/// @{

	
	/// @brief Counts the number of code units that will result from attempting a decode operation.
	///
	/// @param[in] input The input range (of code units) to find out how many code points there are.
	/// @param[in] encoding The encoding to count the input with.
	/// @param[in] errorHandler The error handler to invoke when an encode operation fails.
	/// @param[in,out] state The state that will be used to count code units.
	///
	/// @returns A CountResult that includes information about how many code units are present,
	/// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler) and a reference
	/// to the provided `state`.
	///
	/// @remarks This method does not call ADL extension points. It attempts a combination of implementation techniques
	/// to count code units, with a loop over the `.decode` call into an intermediate, unseen buffer being the most
	/// basic guaranteed implementation attempt.
	template <typename TInput, typename TEncoding, typename TErrorHandler, typename TState>
	constexpr auto basic_count_as_decoded(
		TInput&& input, TEncoding&& encoding, TErrorHandler&& errorHandler, TState& state) {
		using _WorkingInput = Krys::Ranges::subrange_for_t<TInput>;
		using TUEncoding    = remove_cvref_t<TEncoding>;
		using TResult       = CountResult<_WorkingInput, TState>;

		_WorkingInput __working_input(std::forward<TInput>(input));

		std::size_t __code_point_count = 0;

		if constexpr (is_detected_v<Impl::HasTextCountAsDecodedOne, _WorkingInput, TEncoding,
			              TErrorHandler, TState>) {
			std::size_t errorCount = 0;
			for (;;) {
				auto result = TextCountAsDecodedOne(
					Krys::Tag<TUEncoding> {}, std::move(__working_input), encoding, errorHandler, state);
				errorCount += result.ErrorCount;
				if (result.ErrorCode != EncodingError::OK) {
					return TResult(std::move(result.Input), __code_point_count, state, result.ErrorCode,
						result.ErrorCount);
				}
				__code_point_count += result.Count;
				__working_input = std::move(result.Input);
				if (Krys::Ranges::empty(__working_input)) {
					if (!::is_state_complete(encoding, state)) {
						continue;
					}
					break;
				}
			}
			return TResult(
				std::move(__working_input), __code_point_count, state, EncodingError::OK, errorCount);
		}
		else if constexpr (is_detected_v<Impl::HasInternalTextCountAsDecodedOne, _WorkingInput,
			                   TEncoding, TErrorHandler, TState>) {
			std::size_t errorCount = 0;
			for (;;) {
				auto result = InternalTextCountAsDecodedOne(
					Krys::Tag<TUEncoding> {}, std::move(__working_input), encoding, errorHandler, state);
				errorCount += result.ErrorCount;
				if (result.ErrorCode != EncodingError::OK) {
					return TResult(std::move(result.Input), __code_point_count, state, result.ErrorCode,
						errorCount);
				}
				__code_point_count += result.Count;
				__working_input = std::move(result.Input);
				if (Krys::Ranges::empty(__working_input)) {
					if (!::is_state_complete(encoding, state)) {
						continue;
					}
					break;
				}
			}
			return TResult(
				std::move(__working_input), __code_point_count, state, EncodingError::OK, errorCount);
		}
		else {
			using TCodePoint = code_point_t<TUEncoding>;

			TCodePoint __intermediate_storage[MaxCodePoints<TUEncoding>] {};
			Span<TCodePoint, MaxCodePoints<TUEncoding>> intermediate(__intermediate_storage);
			std::size_t errorCount = 0;
			for (;;) {
				auto result = ::decode_one_into_raw(
					std::move(__working_input), encoding, intermediate, errorHandler, state);
				errorCount += result.ErrorCount;
				if (result.ErrorCode != EncodingError::OK) {
					return TResult(std::move(result.Input), __code_point_count, state, result.ErrorCode,
						errorCount);
				}
				std::size_t usedSize = static_cast<std::size_t>(Krys::Ranges::distance(
					Krys::Ranges::begin(intermediate), Krys::Ranges::begin(result.Output)));
				__code_point_count += usedSize;
				__working_input = std::move(result.Input);
				if (Krys::Ranges::empty(__working_input)) {
					if (!::is_state_complete(encoding, state)) {
						continue;
					}
					break;
				}
			}
			return TResult(
				std::move(__working_input), __code_point_count, state, EncodingError::OK, errorCount);
		}
	}

	
	/// @brief Counts the number of code units that will result from attempting a decode operation.
	///
	/// @param[in] input The input range (of code units) to find out how many code points there are.
	/// @param[in] encoding The encoding to count the input with.
	/// @param[in] errorHandler The error handler to invoke when an encode operation fails.
	/// @param[in,out] state The state that will be used to count code units.
	///
	/// @returns A CountResult that includes information about how many code units are present,
	/// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler) and a reference
	/// to the provided `state`.
	///
	/// @remarks This method will first check if an ADL Extension Point `TextCountAsDecoded` is callable with
	/// the given arguments. If it is, then that method will be used to do the work after forwarding all four arguments
	/// to that function call. Otherwise, it defers to basic_count_as_decoded.
	template <typename TInput, typename TEncoding, typename TErrorHandler, typename TState>
	constexpr auto count_as_decoded(
		TInput&& input, TEncoding&& encoding, TErrorHandler&& errorHandler, TState& state) {
		if constexpr (is_detected_v<Impl::HasTextCountAsDecoded, TInput, TEncoding,
			              TErrorHandler, TState>) {
			return TextCountAsDecoded(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::forward<TInput>(input),
				std::forward<TEncoding>(encoding), std::forward<TErrorHandler>(errorHandler), state);
		}
		else if constexpr (is_detected_v<Impl::HasInternalTextCountAsDecoded, TInput, TEncoding,
			                   TErrorHandler, TState>) {
			return InternalTextCountAsDecoded(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::forward<TInput>(input),
				std::forward<TEncoding>(encoding), std::forward<TErrorHandler>(errorHandler), state);
		}
		else {
			return basic_count_as_decoded(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
				std::forward<TErrorHandler>(errorHandler), state);
		}
	}

	
	/// @brief Counts the number of code units that will result from attempting a decode operation.
	///
	/// @param[in] input The input range (of code units) to find out how many code points there are.
	/// @param[in] encoding The encoding to count the input with.
	/// @param[in] errorHandler The error handler to invoke when an encode operation fails.
	///
	/// @returns A StatelessCountResult that includes information about how many code units are present,
	/// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler).
	///
	/// @remarks Calls count_as_decoded(Input, Encoding, ErrorHandler, State) with an `state` that is
	/// created by CreateDecodeState(Encoding).
	template <typename TInput, typename TEncoding, typename TErrorHandler>
	constexpr auto count_as_decoded(TInput&& input, TEncoding&& encoding, TErrorHandler&& errorHandler) {
		using TUEncoding = remove_cvref_t<TEncoding>;
		using TState     = DecodeState<TUEncoding>;

		TState state         = CreateDecodeState(encoding);
		auto __stateful_result = count_as_decoded(std::forward<TInput>(input),
			std::forward<TEncoding>(encoding), std::forward<TErrorHandler>(errorHandler), state);
		return Impl::SliceToStateless(std::move(__stateful_result));
	}

	
	/// @brief Counts the number of code units that will result from attempting a decode operation.
	///
	/// @param[in] input The input range (of code units) to find out how many code points there are.
	/// @param[in] encoding The encoding to count the input with.
	///
	/// @returns A StatelessCountResult that includes information about how many code units are present,
	/// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler).
	///
	/// @remarks Calls count_as_decoded(Input, Encoding, ErrorHandler) with an `error_handler` that is
	/// similar to Handlers::DefaultHandler.
	template <typename TInput, typename TEncoding>
	constexpr auto count_as_decoded(TInput&& input, TEncoding&& encoding) {
		Handlers::DefaultHandler __handler {};
		return count_as_decoded(std::forward<TInput>(input), std::forward<TEncoding>(encoding), __handler);
	}

	
	/// @brief Counts the number of code units that will result from attempting a decode operation.
	///
	/// @param[in] input The input range (of code units) to find out how many code points there are.
	///
	/// @returns A StatelessCountResult that includes information about how many code units are present,
	/// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler).
	///
	/// @remarks Calls count_as_decoded(Input, Encoding) with an `encoding` that is derived from
	/// default_code_unit_encoding.
	template <typename TInput>
	constexpr auto count_as_decoded(TInput&& input) {
		using TUInput   = remove_cvref_t<TInput>;
		using TCodeUnit = ranges::rangeTValue_type_t<TUInput>;
#if ZTD_IS_ON(ZTD_STD_LIBRARY_IS_CONSTANT_EVALUATED)
		if (std::is_constant_evaluated()) {
			// Use literal encoding instead, if we meet the right criteria
			using TEncoding = default_consteval_code_unit_encoding_t<TCodeUnit>;
			TEncoding encoding {};
			return count_as_decoded(std::forward<TInput>(input), encoding);
		}
		else
#endif
		{
			using TEncoding = default_code_unit_encoding_t<TCodeUnit>;
			TEncoding encoding {};
			return count_as_decoded(std::forward<TInput>(input), encoding);
		}
	}

	
	/// @}

	
}



#endif
