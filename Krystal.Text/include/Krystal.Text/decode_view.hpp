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

#ifndef ZTD_TEXT_DECODE_VIEW_HPP
#define ZTD_TEXT_DECODE_VIEW_HPP



#include <ztd/text/decode_iterator.hpp>
#include <ztd/text/error_handler.hpp>
#include <ztd/text/encoding.hpp>
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include <ztd/text/detail/default_char_view.hpp>

#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Lib/Ranges/Reconstruct.hpp"

#include <string_view>



namespace Krys {
	

	
	/// @brief A view over a range of code points, presenting the code points as code units. Uses the `TEncoding`
	/// specified to do so.
	///
	/// @tparam TEncoding The encoding to read the underlying range of code points as.
	/// @tparam TRange The range of input that will be fed into the TFromEncoding's decode operation.
	/// @tparam TErrorHandler The error handler for any encode-step failures.
	/// @tparam TState The state type to use for the decode operations to intermediate code points.
	///
	/// @remarks The view presents code point one at a time, regardless of how many code points are output by one
	/// decode operation. This means if, for example, four (4) UTF-8 code units becomes two (2) UTF-16 code points, it
	/// will present one code point at a time. If you are looking to explicitly know what a single decode operation
	/// maps into as far as number of code points to code units (and vice-versa), you will have to use lower-level
	/// interfaces.
	template <typename TEncoding, typename TRange = Impl::default_char_view_t<code_unit_t<TEncoding>>,
		typename TErrorHandler = Handlers::DefaultHandler, typename TState = DecodeState<TEncoding>>
	class decode_view : public Krys::Ranges::view_base {
	private:
		using _CVRange     = unwrap_remove_reference_t<TRange>;
		using _StoredRange = Krys::Ranges::range_reconstruct_t<const _CVRange>;

	public:
		
		/// @brief The iterator type for this view.
		using iterator = decode_iterator<TEncoding, _StoredRange, TErrorHandler, TState>;
		
		/// @brief The sentinel type for this view.
		using sentinel = decode_sentinel_t;
		
		/// @brief The value type for this view.
		using value_type = ranges::iteratorTValue_type_t<iterator>;
		
		/// @brief The underlying range type.
		using range_type = TRange;
		
		/// @brief The encoding type used for transformations.
		using encoding_type = TEncoding;
		
		/// @brief The error handler when a decode operation fails.
		using error_handler_type = TErrorHandler;
		
		/// @brief The state type used for decode operations.
		using state_type = DecodeState<encoding_type>;

		
		/// @brief Constructs a decode_view from the underlying range.
		///
		/// @param[in] range The input range to wrap and iterate over.
		///
		/// @remarks The stored encoding, error handler, and state type are default-constructed.
		template <typename _ArgRange,
			enable_if_t<!SameType<remove_cvref_t<_ArgRange>, decode_view>
			     && !SameType<remove_cvref_t<_ArgRange>, iterator>>* = nullptr>
		constexpr decode_view(_ArgRange&& range) noexcept(NoThrowConstructible<iterator, _ArgRange>)
		: _M_it(std::forward<_ArgRange>(range)) {
		}

		
		/// @brief Constructs a decode_view from the underlying range.
		///
		/// @param[in] range The input range to wrap and iterate over.
		/// @param[in] encoding The encoding object to call `.decode` or equivalent functionality on.
		constexpr decode_view(range_type range, encoding_type encoding) noexcept(
			NoThrowConstructible<iterator, range_type, encoding_type>)
		: _M_it(std::move(range), std::move(encoding)) {
		}

		
		/// @brief Constructs a decode_view from the underlying range.
		///
		/// @param[in] range The input range to wrap and iterate over.
		/// @param[in] encoding The encoding object to call `.decode` or equivalent functionality on.
		/// @param[in] errorHandler The error handler to store in this view.
		constexpr decode_view(range_type range, encoding_type encoding,
			error_handler_type errorHandler) noexcept(NoThrowConstructible<iterator, range_type,
			encoding_type, error_handler_type>)
		: _M_it(std::move(range), std::move(encoding), std::move(errorHandler)) {
		}

		
		/// @brief Constructs a decode_view from the underlying range.
		///
		/// @param[in] range The input range to wrap and iterate over.
		/// @param[in] encoding The encoding object to call `.decode` or equivalent functionality on.
		/// @param[in] errorHandler The error handler to store in this view.
		/// @param[in] state The state to user for the decode operation.
		constexpr decode_view(range_type range, encoding_type encoding, error_handler_type errorHandler,
			state_type state) noexcept(NoThrowConstructible<iterator, range_type, encoding_type,
			error_handler_type, state_type>)
		: _M_it(std::move(range), std::move(encoding), std::move(errorHandler), std::move(state)) {
		}

		
		/// @brief Constructs an encoding_view from one of its iterators, reconstituting the range.
		///
		/// @param[in] it A previously-made decode_view iterator.
		constexpr decode_view(iterator it) noexcept(NoThrowMoveConstructible<iterator>)
		: _M_it(std::move(it)) {
		}

		
		/// @brief Default constructor. Defaulted.
		constexpr decode_view() = default;

		
		/// @brief Copy constructor. Defaulted.
		constexpr decode_view(const decode_view&) = default;

		
		/// @brief Move constructor. Defaulted.
		constexpr decode_view(decode_view&&) = default;

		
		/// @brief Copy assignment operator. Defaulted.
		constexpr decode_view& operator=(const decode_view&) = default;
		
		/// @brief Move assignment operator. Defaulted.
		constexpr decode_view& operator=(decode_view&&) = default;

		
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

	
	/// @brief The reconstruct extension point for rebuilding an encoding view from its iterator and sentinel
	/// type.
	template <typename TEncoding, typename TRange, typename TErrorHandler, typename TState>
	constexpr decode_view<TEncoding, TRange, TErrorHandler, TState> reconstruct(
		std::in_place_type_t<decode_view<TEncoding, TRange, TErrorHandler, TState>>,
		typename decode_view<TEncoding, TRange, TErrorHandler, TState>::iterator it,
		typename decode_view<TEncoding, TRange, TErrorHandler, TState>::sentinel) noexcept(std::
		     is_nothrow_constructible_v<decode_view<TEncoding, TRange, TErrorHandler, TState>,
		          typename decode_view<TEncoding, TRange, TErrorHandler, TState>::iterator&&>) {
		return decode_view<TEncoding, TRange, TErrorHandler, TState>(std::move(it));
	}


	
}

#if ZTD_IS_ON(ZTD_STD_LIBRARY_BORROWED_RANGE)

namespace std { namespace ranges {

	template <typename TEncoding, typename TRange, typename TErrorHandler, typename TState>
	inline constexpr bool enable_borrowed_range<::decode_view<TEncoding, TRange, TErrorHandler, TState>>
		= std::ranges::enable_borrowed_range<TRange>;

}} ::ranges

#else

namespace Krys { namespace ranges {

	template <typename TEncoding, typename TRange, typename TErrorHandler, typename TState>
	inline constexpr bool enable_borrowed_range<::decode_view<TEncoding, TRange, TErrorHandler, TState>>
		= Krys::Ranges::enable_borrowed_range<TRange>;

}} ::ranges

#endif



#endif
