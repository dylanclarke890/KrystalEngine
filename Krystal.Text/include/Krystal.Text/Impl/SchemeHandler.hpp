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

#ifndef ZTD_TEXT_DETAIL_SCHEME_HANDLER_HPP
#define ZTD_TEXT_DETAIL_SCHEME_HANDLER_HPP



#include "Krystal.Text/DecodeResult.hpp"
#include "Krystal.Text/EncodeResult.hpp"

#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Ranges/Range.hpp"
#include "Krystal.Lib/Utils/ReferenceWrapper.hpp"

#include <type_traits>



namespace Krys {
	

	namespace Impl {

		template <typename _Byte, typename _UInputRange, typename _UOutputRange, typename TErrorHandler>
		class __scheme_handler {
		private:
			ReferenceWrapper<TErrorHandler> _M_handler;

			template <typename TEncoding, typename TInput, typename TOutput, typename TState,
				typename TInputProgress, typename TOutputProgress>
			static constexpr bool __decode_call_operator_noexcept() noexcept {
				if constexpr (std::is_convertible_v<ranges::rangeTValue_type_t<TInputProgress>, _Byte>) {
					if constexpr (std::is_invocable_v<TErrorHandler&, const TEncoding,
						              DecodeResult<TInput, TOutput, TState>, const TInputProgress&,
						              const TOutputProgress&>) {
						return std::is_nothrow_invocable_v<TErrorHandler&, const TEncoding&,
							DecodeResult<TInput, TOutput, TState>, const TInputProgress&,
							const TOutputProgress&>;
					}
					else {
						return std::is_nothrow_invocable_v<TErrorHandler&, const TEncoding&,
							DecodeResult<TInput, TOutput, TState>, Span<_Byte>, const TOutputProgress&>;
					}
				}
				else {
					return std::is_nothrow_invocable_v<TErrorHandler&, const TEncoding&,
						DecodeResult<TInput, TOutput, TState>, Span<_Byte>, const TOutputProgress&>;
				}
			}

			template <typename TEncoding, typename TInput, typename TOutput, typename TState,
				typename TInputProgress, typename TOutputProgress>
			static constexpr bool __encode_call_operator_noexcept() noexcept {
				if constexpr (std::is_convertible_v<ranges::rangeTValue_type_t<TOutputProgress>, _Byte>) {
					if constexpr (std::is_invocable_v<TErrorHandler&, const TEncoding,
						              EncodeResult<TInput, TOutput, TState>, const TInputProgress&,
						              const TOutputProgress&>) {
						return std::is_nothrow_invocable_v<TErrorHandler&, const TEncoding&,
							EncodeResult<TInput, TOutput, TState>, const TInputProgress&,
							const TOutputProgress&>;
					}
					else {
						return std::is_nothrow_invocable_v<TErrorHandler&, const TEncoding&,
							EncodeResult<TInput, TOutput, TState>, const TInputProgress&, Span<_Byte>>;
					}
				}
				else {
					return std::is_nothrow_invocable_v<TErrorHandler&, const TEncoding&,
						EncodeResult<TInput, TOutput, TState>, const TInputProgress&, Span<_Byte>>;
				}
			}

			template <typename TEncoding, typename TInput, typename TOutput, typename TState,
				typename TInputProgress, typename TOutputProgress>
			constexpr auto _M_basic_encode(const TEncoding& encoding,
				EncodeResult<TInput, TOutput, TState>&& result, const TInputProgress& inputProgress,
				const TOutputProgress& outputProgress) const
				noexcept(std::is_nothrow_invocable_v<TErrorHandler&, const TEncoding&,
				     EncodeResult<TInput, TOutput, TState>, const TInputProgress&, Span<_Byte>>) {
				using _ProgressPointer = ranges::range_pointer_t<TOutputProgress>;
				using _ProgressWord    = ranges::rangeTValue_type_t<TOutputProgress>;
				_Byte* __byte_progress_data
					= reinterpret_cast<_Byte*>(const_cast<_ProgressPointer>(outputProgress.data()));
				auto __byte_progress_size
					= (Krys::Ranges::size(outputProgress) * sizeof(_ProgressWord)) / (sizeof(_Byte));
				Span<_Byte> __byte_progress(__byte_progress_data, __byte_progress_size);
				return this->_M_handler.get()(encoding, std::move(result), inputProgress, __byte_progress);
			}

			template <typename TEncoding, typename TInput, typename TOutput, typename TState,
				typename TInputProgress, typename TOutputProgress>
			constexpr auto _M_basic_decode(const TEncoding& encoding,
				DecodeResult<TInput, TOutput, TState>&& result, const TInputProgress& inputProgress,
				const TOutputProgress& outputProgress) const
				noexcept(std::is_nothrow_invocable_v<TErrorHandler&, const TEncoding&,
				     DecodeResult<TInput, TOutput, TState>, Span<_Byte>, const TOutputProgress&>) {
				using _ProgressPointer = ranges::range_pointer_t<TInputProgress>;
				using _ProgressWord    = ranges::rangeTValue_type_t<TInputProgress>;
				_Byte* __byte_progress_data
					= reinterpret_cast<_Byte*>(const_cast<_ProgressPointer>(inputProgress.data()));
				auto __byte_progress_size
					= (Krys::Ranges::size(inputProgress) * sizeof(_ProgressWord)) / (sizeof(_Byte));
				Span<_Byte> __byte_progress(__byte_progress_data, __byte_progress_size);
				return this->_M_handler.get()(
					encoding, std::move(result), __byte_progress, outputProgress);
			}

		public:
			constexpr __scheme_handler(TErrorHandler& __handler) noexcept : _M_handler(__handler) {
			}

			template <typename TEncoding, typename TInput, typename TOutput, typename TState,
				typename TInputProgress, typename TOutputProgress>
			constexpr auto operator()(const TEncoding& encoding, DecodeResult<TInput, TOutput, TState> result,
				const TInputProgress& inputProgress, const TOutputProgress& outputProgress) const
				noexcept(__decode_call_operator_noexcept<TEncoding, TInput, TOutput, TState, TInputProgress,
				     TOutputProgress>()) {
				if constexpr (std::is_convertible_v<ranges::rangeTValue_type_t<TInputProgress>, _Byte>) {
					if constexpr (std::is_invocable_v<TErrorHandler&, const TEncoding,
						              DecodeResult<TInput, TOutput, TState>, const TInputProgress&,
						              const TOutputProgress&>) {
						return this->_M_handler.get()(
							encoding, std::move(result), inputProgress, outputProgress);
					}
					else {
						return this->_M_basic_decode(
							encoding, std::move(result), inputProgress, outputProgress);
					}
				}
				else {
					return this->_M_basic_decode(
						encoding, std::move(result), inputProgress, outputProgress);
				}
			}

			template <typename TEncoding, typename TInput, typename TOutput, typename TState,
				typename TInputProgress, typename TOutputProgress>
			constexpr auto operator()(const TEncoding& encoding, EncodeResult<TInput, TOutput, TState> result,
				const TInputProgress& inputProgress, const TOutputProgress& outputProgress) const
				noexcept(__encode_call_operator_noexcept<TEncoding, TInput, TOutput, TState, TInputProgress,
				     TOutputProgress>()) {
				if constexpr (std::is_convertible_v<ranges::rangeTValue_type_t<TInputProgress>, _Byte>) {
					if constexpr (std::is_invocable_v<TErrorHandler&, const TEncoding,
						              EncodeResult<TInput, TOutput, TState>, const TInputProgress&,
						              const TOutputProgress&>) {
						return this->_M_handler.get()(
							encoding, std::move(result), inputProgress, outputProgress);
					}
					else {
						return this->_M_basic_encode(
							encoding, std::move(result), inputProgress, outputProgress);
					}
				}
				else {
					return this->_M_basic_encode(
						encoding, std::move(result), inputProgress, outputProgress);
				}
			}
		};
	} 

	
}



#endif
