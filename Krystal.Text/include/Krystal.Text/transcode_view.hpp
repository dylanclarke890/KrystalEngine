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

#ifndef ZTD_TEXT_TRANSCODE_VIEW_HPP
#define ZTD_TEXT_TRANSCODE_VIEW_HPP



#include <ztd/text/transcode_iterator.hpp>
#include <ztd/text/error_handler.hpp>
#include <ztd/text/encoding.hpp>
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/Encodings/UTF8.hpp"
#include <ztd/text/execution.hpp>
#include <ztd/text/detail/default_char_view.hpp>

#include <string_view>



namespace Krys {
	

	
	/// @brief A transcoding iterator that takes an input of code units and provides an output over the code units of
	/// the desired `TToEncoding` after converting from the `TFromEncoding` in a fashion that will never produce a
	/// EncodingError::insufficient_output error.
	///
	/// @tparam TFromEncoding The encoding to read the underlying range of code points as.
	/// @tparam TToEncoding The encoding to read the underlying range of code points as.
	/// @tparam TRange The range of input that will be fed into the TFromEncoding's decode operation.
	/// @tparam _FromErrorHandler The error handler for any decode-step failures.
	/// @tparam _ToErrorHandler The error handler for any encode-step failures.
	/// @tparam TFromState The state type to use for the decode operations to intermediate code points.
	/// @tparam TToState The state type to use for the encode operations to intermediate code points.
	///
	/// @remarks This type produces proxies as their reference type, and are only readable, not writable iterators. The
	/// type will also try many different shortcuts for decoding the input and encoding the intermediates,
	/// respectively, including invoking a few customization points for either `DecodeOne.` or @c
	/// EncodeOne . It may also call `transcode_one` to bypass having to do the round-trip through
	/// two encodings, which an encoding pair that a developer is interested in can use to do the conversion more
	/// quickly. The view presents code units one at a time, regardless of how many code units are output by one decode
	/// operation. This means if, for example, one (1) UTF-16 code unit becomes two (2) UTF-8 code units, it will
	/// present each code unit one at a time. If you are looking to explicitly know each collection of characters, you
	/// will have to use lower-level interfaces.
	template <typename TFromEncoding, typename TToEncoding = utf8_t,
		typename TRange            = Impl::default_char_view_t<code_unit_t<TFromEncoding>>,
		typename _FromErrorHandler = Handlers::DefaultHandler, typename _ToErrorHandler = Handlers::DefaultHandler,
		typename TFromState = DecodeState<TFromEncoding>, typename TToState = EncodeState<TToEncoding>>
	class transcode_view : public Krys::Ranges::view_base {
	public:
		
		/// @brief The iterator type for this view.
		using iterator = transcode_iterator<TFromEncoding, TToEncoding, TRange, _FromErrorHandler, _ToErrorHandler,
			TFromState, TToState>;
		
		/// @brief The sentinel type for this view.
		using sentinel = transcode_sentinel_t;
		
		/// @brief The underlying range type.
		using range_type = TRange;
		
		/// @brief The encoding type used for decoding to intermediate code point storage.
		using from_encoding_type = TFromEncoding;
		
		/// @brief The encoding type used for encoding to the final code units storage.
		using to_encoding_type = TToEncoding;
		
		/// @brief The error handler when a decode operation fails.
		using from_error_handler_type = _FromErrorHandler;
		
		/// @brief The error handler when an encode operation fails.
		using to_error_handler_type = _ToErrorHandler;
		
		/// @brief The state type used for decode operations.
		using from_state_type = TFromState;
		
		/// @brief The state type used for encode operations.
		using to_state_type = TToState;

		
		/// @brief Constructs a transcode_view from the underlying range.
		///
		/// @param[in] range The input range to wrap and iterate over.
		constexpr transcode_view(range_type range) noexcept
		: transcode_view(std::move(range), to_encoding_type {}) {
		}

		
		/// @brief Constructs a transcode_view from the underlying range.
		///
		/// @param[in] range The input range to wrap and iterate over.
		/// @param[in] __to_encoding The encoding object to call `EncodeOne` or equivalent functionality on.
		constexpr transcode_view(range_type range, to_encoding_type __to_encoding) noexcept
		: transcode_view(std::move(range), from_encoding_type {}, std::move(__to_encoding)) {
		}

		
		/// @brief Constructs a transcode_view from the underlying range.
		///
		/// @param[in] range The input range to wrap and iterate over.
		/// @param[in] __from_encoding The encoding object to call `DecodeOne` or equivalent functionality on.
		/// @param[in] __to_encoding The encoding object to call `EncodeOne` or equivalent functionality on.
		constexpr transcode_view(
			range_type range, from_encoding_type __from_encoding, to_encoding_type __to_encoding) noexcept
		: transcode_view(std::move(range), std::move(__from_encoding), std::move(__to_encoding),
			  from_error_handler_type {}, to_error_handler_type {}) {
		}

		
		/// @brief Constructs a transcode_view from the underlying range.
		///
		/// @param[in] range The input range to wrap and iterate over.
		/// @param[in] __from_encoding The encoding object to call `DecodeOne` or equivalent functionality on.
		/// @param[in] __to_encoding The encoding object to call `EncodeOne` or equivalent functionality on.
		/// @param[in] __from_error_handler The error handler for decode operations to store in this view.
		/// @param[in] __to_error_handler The error handler for encode operations to store in this view.
		constexpr transcode_view(range_type range, from_encoding_type __from_encoding,
			to_encoding_type __to_encoding, from_error_handler_type __from_error_handler,
			to_error_handler_type __to_error_handler) noexcept
		: _M_it(std::move(range), std::move(__from_encoding), std::move(__to_encoding),
			  std::move(__from_error_handler), std::move(__to_error_handler)) {
		}

		
		/// @brief Constructs a transcode_view from the underlying range.
		///
		/// @param[in] range The input range to wrap and iterate over.
		/// @param[in] __from_encoding The encoding object to call `DecodeOne` or equivalent functionality on.
		/// @param[in] __to_encoding The encoding object to call `EncodeOne` or equivalent functionality on.
		/// @param[in] __from_error_handler The error handler for decode operations to store in this view.
		/// @param[in] __to_error_handler The error handler for encode operations to store in this view.
		/// @param[in] fromState The state to user for the decode operation.
		/// @param[in] toState The state to user for the decode operation.
		constexpr transcode_view(range_type range, from_encoding_type __from_encoding,
			to_encoding_type __to_encoding, from_error_handler_type __from_error_handler,
			to_error_handler_type __to_error_handler, from_state_type fromState,
			to_state_type toState) noexcept
		: _M_it(std::move(range), std::move(__from_encoding), std::move(__to_encoding),
			  std::move(__from_error_handler), std::move(__to_error_handler), std::move(fromState),
			  std::move(toState)) {
		}

		
		/// @brief The beginning of the range.
		constexpr iterator begin() & noexcept {
			if constexpr (std::is_copy_constructible_v<iterator>) {
				return this->_M_it;
			}
			else {
				return std::move(this->_M_it);
			}
		}

		
		/// @brief The beginning of the range.
		constexpr iterator begin() const& noexcept {
			return this->_M_it;
		}

		
		/// @brief The beginning of the range.
		constexpr iterator begin() && noexcept {
			return std::move(this->_M_it);
		}

		
		/// @brief The end of the range. Uses a sentinel type and not a special iterator.
		constexpr sentinel end() const noexcept {
			return sentinel();
		}

	private:
		iterator _M_it;
	};


	
}

#if ZTD_IS_ON(ZTD_STD_LIBRARY_BORROWED_RANGE)

namespace std { namespace ranges {

	template <typename TFromEncoding, typename TToEncoding, typename TRange, typename _FromErrorHandler,
		typename _ToErrorHandler, typename TFromState, typename TToState>
	inline constexpr bool enable_borrowed_range<::transcode_view<TFromEncoding, TToEncoding, TRange,
		_FromErrorHandler, _ToErrorHandler, TFromState, TToState>>
		= std::ranges::enable_borrowed_range<TRange>;

}} ::ranges

#else

namespace Krys { namespace ranges {

	template <typename TFromEncoding, typename TToEncoding, typename TRange, typename _FromErrorHandler,
		typename _ToErrorHandler, typename TFromState, typename TToState>
	inline constexpr bool enable_borrowed_range<::transcode_view<TFromEncoding, TToEncoding, TRange,
		_FromErrorHandler, _ToErrorHandler, TFromState, TToState>>
		= Krys::Ranges::enable_borrowed_range<TRange>;

}} ::ranges

#endif



#endif
