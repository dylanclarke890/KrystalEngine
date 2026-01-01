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

#ifndef ZTD_TEXT_DECODE_ITERATOR_HPP
#define ZTD_TEXT_DECODE_ITERATOR_HPP



#include <ztd/text/error_handler.hpp>
#include "Krystal.Text/State.hpp"

#include "Krystal.Text/Impl/EncodingIterator.hpp"



namespace Krys {
	

	
	/// @brief The decode sentinel to use as the `end` value for a decode_iterator.
	using decode_sentinel_t = Impl::__encoding_sentinel_t;

	
	/// @brief An iterator over a range of code points, presented as a range of code units, using the `TEncoding`
	/// specified to do so.
	///
	/// @tparam TEncoding The encoding to read the underlying range of code points as.
	/// @tparam TRange The range of input that will be fed into the TFromEncoding's decode operation.
	/// @tparam TErrorHandler The error handler for any encode-step failures.
	/// @tparam TState The state type to use for the encode operations to intermediate code points.
	///
	/// @remarks This type produces proxies as their reference type, and are only readable, not writable iterators. The
	/// iterator presents code point one at a time, regardless of how many code points are output by one decode
	/// operation. This means if, for example, four (4) UTF-8 code units becomes two (2) UTF-16 code points, it will
	/// present one code point at a time. If you are looking to explicitly know what a single decode operation maps
	/// into as far as number of code points to code units (and vice-versa), you will have to use lower-level
	/// interfaces.
	template <typename TEncoding, typename TRange, typename TErrorHandler = Handlers::DefaultHandler,
		typename TState = DecodeState<TEncoding>>
	class decode_iterator : public Impl::__encoding_iterator<Impl::__transaction::Decode,
		                        decode_iterator<TEncoding, TRange, TErrorHandler, TState>,
		                        Impl::IteratorStorage<TEncoding, TRange, TErrorHandler, TState>> {
	private:
		using __iterator_base_it = Impl::__encoding_iterator<Impl::__transaction::Decode,
			decode_iterator<TEncoding, TRange, TErrorHandler, TState>,
			Impl::IteratorStorage<TEncoding, TRange, TErrorHandler, TState>>;

	public:
		
		/// @brief The underlying range type.
		using range_type = typename __iterator_base_it::range_type;
		
		/// @brief The base iterator type.
		using iterator = typename __iterator_base_it::iterator;
		
		/// @brief The encoding type used for transformations.
		using encoding_type = typename __iterator_base_it::encoding_type;
		
		/// @brief The error handler when an encode operation fails.
		using error_handler_type = typename __iterator_base_it::error_handler_type;
		
		/// @brief The state type used for encode operations.
		using state_type = typename __iterator_base_it::state_type;
		
		/// @brief The strength of the iterator category, as defined in relation to the base.
		using iterator_category = typename __iterator_base_it::iterator_category;
		
		/// @brief The strength of the iterator concept, as defined in relation to the base.
		using iterator_concept = typename __iterator_base_it::iterator_concept;
		
		/// @brief The object type that gets output on every dereference.
		using value_type = typename __iterator_base_it::value_type;
		
		/// @brief A pointer type to the value_type.
		using pointer = typename __iterator_base_it::pointer;
		
		/// @brief The value returned from derefencing the iterator.
		///
		/// @remarks This is a proxy iterator, so the `reference` is a non-reference `value_type.`
		using reference = typename __iterator_base_it::value_type;
		
		/// @brief The type returned when two of these pointers are subtracted from one another.
		///
		/// @remarks It's not a very useful type...
		using difference_type = typename __iterator_base_it::difference_type;

		
		/// @brief Default constructor. Defaulted.
		constexpr decode_iterator() = default;

		
		/// @brief Copy constructor. Defaulted.
		constexpr decode_iterator(const decode_iterator&) = default;

		
		/// @brief Move constructor. Defaulted.
		constexpr decode_iterator(decode_iterator&&) = default;

		
		/// @brief Constructs a decode_iterator from the explicitly given `range`.
		///
		/// @param[in] range The range value that will be read from.
		///
		/// @remarks Each argument is moved/forwarded in.
		template <typename _ArgRange,
			enable_if_t<!SameType<remove_cvref_t<_ArgRange>, decode_iterator>>* = nullptr>
		constexpr decode_iterator(_ArgRange&& range) noexcept(
			NoThrowConstructible<__iterator_base_it, range_type>)
		: __iterator_base_it(std::forward<_ArgRange>(range)) {
		}

		
		/// @brief Constructs a decode_iterator from the explicitly given `range`, and `encoding`.
		///
		/// @param[in] range The range value that will be read from.
		/// @param[in] encoding The encoding object to use.
		///
		/// @remarks Each argument is moved in.
		constexpr decode_iterator(range_type range, encoding_type encoding) noexcept(
			NoThrowConstructible<__iterator_base_it, range_type, encoding_type>)
		: __iterator_base_it(std::move(range), std::move(encoding)) {
		}

		
		/// @brief Constructs a decode_iterator from the explicitly given `range`, and @p
		/// errorHandler.
		///
		/// @param[in] range The range value that will be read from.
		/// @param[in] errorHandler The error handler to use for reporting errors.
		///
		/// @remarks Each argument is moved in.
		constexpr decode_iterator(range_type range, error_handler_type errorHandler) noexcept(
			NoThrowConstructible<__iterator_base_it, range_type, error_handler_type>)
		: __iterator_base_it(std::move(range), std::move(errorHandler)) {
		}

		
		/// @brief Constructs a decode_iterator from the explicitly given `range`, `encoding`, and
		/// `errorHandler`.
		///
		/// @param[in] range The range value that will be read from.
		/// @param[in] encoding The encoding object to use.
		/// @param[in] errorHandler The error handler to use for reporting errors.
		///
		/// @remarks Each argument is moved in.
		constexpr decode_iterator(range_type range, encoding_type encoding,
			error_handler_type errorHandler) noexcept(NoThrowConstructible<__iterator_base_it,
			range_type, encoding_type, error_handler_type>)
		: __iterator_base_it(std::move(range), std::move(encoding), std::move(errorHandler)) {
		}

		
		/// @brief Constructs a decode_iterator from the explicitly given `range`, `encoding`, @p
		/// errorHandler and `state`.
		///
		/// @param[in] range The range value that will be read from.
		/// @param[in] encoding The encoding object to use.
		/// @param[in] errorHandler The error handler to use for reporting errors.
		/// @param[in] state The current state.
		///
		/// @remarks Each argument is moved in.
		constexpr decode_iterator(range_type range, encoding_type encoding, error_handler_type errorHandler,
			state_type state) noexcept(NoThrowConstructible<__iterator_base_it, range_type,
			encoding_type, error_handler_type, state_type>)
		: __iterator_base_it(
			  std::move(range), std::move(encoding), std::move(errorHandler), std::move(state)) {
		}

		
		/// @brief Copy assignment operator. Defaulted.
		constexpr decode_iterator& operator=(const decode_iterator&) = default;
		
		/// @brief Move assignment operator. Defaulted.
		constexpr decode_iterator& operator=(decode_iterator&&) = default;
	};


	
}



#endif
