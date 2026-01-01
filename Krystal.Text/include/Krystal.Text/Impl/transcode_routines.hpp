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

#ifndef ZTD_TEXT_DETAIL_TRANSCODE_ROUTINES_HPP
#define ZTD_TEXT_DETAIL_TRANSCODE_ROUTINES_HPP



#include "Krystal.Text/EncodeResult.hpp"
#include "Krystal.Text/DecodeResult.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/IsIgnorableErrorHandler.hpp"
#include <ztd/text/detail/is_lossless.hpp>
#include <ztd/text/detail/encoding_range.hpp>

#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Utils/Tag.hpp"
#include "Krystal.Lib/Ranges/ADL.hpp"
#include <ztd/ranges/unbounded.hpp>
#include <ztd/ranges/subrange.hpp>
#include <ztd/ranges/blackhole_iterator.hpp>
#include "Krystal.Lib/Ranges/Algorithm.hpp"

#include <array>
#include <algorithm>



namespace Krys {
	

	namespace Impl {

		enum class __consume : uchar { __no = 0, __embrace_the_void = 1 };
		enum class __transaction : uchar { Encode = 0, Decode = 1 };

		template <__consume _ConsumeIntoTheNothingness, typename TEncoding, typename TInput, typename TOutput,
			typename TErrorHandler, typename TState>
		constexpr auto __basic_decode_one(TInput&& input, TEncoding&& encoding, TOutput&& output,
			TErrorHandler& errorHandler, TState& state) {
			using _UOutput   = remove_cvref_t<TOutput>;
			using TUEncoding = remove_cvref_t<TEncoding>;

			if constexpr (_ConsumeIntoTheNothingness == __consume::__embrace_the_void
				&& DecodeRangeCategoryOutput<TUEncoding>) {
				(void)output;
				using _Blackhole = ranges::unbounded_view<ranges::blackhole_iterator>;
				_Blackhole __output_range(ranges::blackhole_iterator {});
				return encoding.DecodeOne(
					std::forward<TInput>(input), __output_range, errorHandler, state);
			}
			else if constexpr ((!ranges::is_range_contiguous_range_v<_UOutput>
				                   || ranges::is_range_input_or_output_range_exactly_v<
				                        _UOutput>)&&ContiguousDecodeRangeCategory<TUEncoding>) {
				code_point_t<TUEncoding> __intermediate_output_storage[MaxCodePoints<TUEncoding>];
				Span<code_point_t<TUEncoding>, MaxCodePoints<TUEncoding>> intermediateOutput(
					__intermediate_output_storage);
				auto intermediateResult = encoding.DecodeOne(
					std::forward<TInput>(input), intermediateOutput, errorHandler, state);
				using _ReconstructedResult = DecodeResult<decltype(intermediateResult.Input),
					Krys::Ranges::range_reconstruct_t<TOutput>, TState>;
				if (intermediateResult.ErrorCode != EncodingError::OK) {
					auto __intermediate_last = intermediateResult.Output.data();
					auto outIt            = Krys::Ranges::begin(std::move(output));
					auto outLast          = Krys::Ranges::end(std::move(output));
					for (auto __intermediate_it = intermediateOutput.data();
						__intermediate_it != __intermediate_last;) {
						*outIt = *__intermediate_it;
						++__intermediate_it;
						++outIt;
					}
					return _ReconstructedResult(std::move(intermediateResult.Input),
						Krys::Ranges::Reconstruct(std::in_place_type<_UOutput>, std::forward<TOutput>(output),
						     std::move(outIt), std::move(outLast)),
						state, intermediateResult.ErrorCode, intermediateResult.ErrorCount);
				}
				return _ReconstructedResult(std::move(intermediateResult.Input),
					std::forward<TOutput>(output), state, intermediateResult.ErrorCode,
					intermediateResult.ErrorCount);
			}
			else {
				return encoding.DecodeOne(
					std::forward<TInput>(input), std::forward<TOutput>(output), errorHandler, state);
			}
		}

		template <__consume _ConsumeIntoTheNothingness, typename TEncoding, typename TInput, typename TOutput,
			typename TErrorHandler, typename TState>
		constexpr auto __basic_encode_one(TInput&& input, TEncoding&& encoding, TOutput&& output,
			TErrorHandler& errorHandler, TState& state) {
			using _UOutput   = remove_cvref_t<TOutput>;
			using TUEncoding = remove_cvref_t<TEncoding>;

			if constexpr (_ConsumeIntoTheNothingness == __consume::__embrace_the_void
				&& EncodeRangeCategoryOutput<TUEncoding>) {
				(void)output;
				using _Blackhole = ranges::unbounded_view<ranges::blackhole_iterator>;
				_Blackhole __output_range(ranges::blackhole_iterator {});
				return encoding.EncodeOne(
					std::forward<TInput>(input), __output_range, errorHandler, state);
			}
			else if constexpr ((!ranges::is_range_contiguous_range_v<_UOutput>
				                   || ranges::is_range_input_or_output_range_exactly_v<
				                        _UOutput>)&&ContiguousEncodeRangeCategory<TUEncoding>) {
				constexpr std::size_t _IntermediateMax = MaxCodeUnits<TUEncoding>;
				code_unit_t<TUEncoding> __intermediate_output_storage[_IntermediateMax];
				Span<code_unit_t<TUEncoding>, _IntermediateMax> intermediateOutput(
					__intermediate_output_storage);
				auto intermediateResult = encoding.EncodeOne(
					std::forward<TInput>(input), intermediateOutput, errorHandler, state);
				using _ReconstructedResult = EncodeResult<decltype(intermediateResult.Input),
					Krys::Ranges::range_reconstruct_t<TOutput>, TState>;
				if (intermediateResult.ErrorCode != EncodingError::OK) {
					auto __intermediate_last = intermediateResult.Output.data();
					auto outIt            = Krys::Ranges::begin(std::move(output));
					auto outLast          = Krys::Ranges::end(std::move(output));
					for (auto __intermediate_it = intermediateOutput.data();
						__intermediate_it != __intermediate_last;) {
						*outIt = *__intermediate_it;
						++__intermediate_it;
						++outIt;
					}
					return _ReconstructedResult(std::move(intermediateResult.Input),
						Krys::Ranges::Reconstruct(std::in_place_type<_UOutput>, std::forward<TOutput>(output),
						     std::move(outIt), std::move(outLast)),
						state, intermediateResult.ErrorCode, intermediateResult.ErrorCount);
				}
				return _ReconstructedResult(std::move(intermediateResult.Input),
					std::forward<TOutput>(output), state, intermediateResult.ErrorCode,
					intermediateResult.ErrorCount);
			}
			else {
				return encoding.EncodeOne(
					std::forward<TInput>(input), std::forward<TOutput>(output), errorHandler, state);
			}
		}

		template <__consume _ConsumeIntoTheNothingness, __transaction __encode_or_decode, typename TEncoding,
			typename TInput, typename _OutputContainer, typename TErrorHandler, typename TState>
		constexpr auto __basic_encode_or_decode_one(TInput&& input, TEncoding&& encoding,
			_OutputContainer& output, TErrorHandler& errorHandler, TState& state) {
			if constexpr (__encode_or_decode == __transaction::Decode) {
				return __basic_decode_one<_ConsumeIntoTheNothingness>(std::forward<TInput>(input),
					std::forward<TEncoding>(encoding), output, errorHandler, state);
			}
			else {
				return __basic_encode_one<_ConsumeIntoTheNothingness>(std::forward<TInput>(input),
					std::forward<TEncoding>(encoding), output, errorHandler, state);
			}
		}

	} 

	
}



#endif
