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

#ifndef ZTD_TEXT_RECODE_ITERATOR_HPP
  #define ZTD_TEXT_RECODE_ITERATOR_HPP



#include <ztd/text/error_handler.hpp>
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/IsIgnorableErrorHandler.hpp"
#include <ztd/text/error_handler_always_returns_ok.hpp>
#include <ztd/text/recode_one.hpp>
#include "Krystal.Text/Impl/EncodingIterator.hpp"
#include <ztd/text/detail/encoding_iterator_storage.hpp>
#include <ztd/text/detail/encoding_range.hpp>

#include "Krystal.Lib/Utils/Unwrap.hpp"
#include "Krystal.Lib/Utils/EBCO.hpp"
#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Lib/Ranges/Range.hpp"



namespace Krys {
	

	
	/// @brief A sentinel type that can be used to compare with a recode_iterator.
	using recode_sentinel_t = Impl::__encoding_sentinel_t;

	
	/// @brief A recoding iterator that takes an input of code units and provides an output over the code units of
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
	/// respectively, including invoking a few customization points for either `DecodeOne` or `EncodeOne` . It
	/// may also call `recode_one` to bypass having to do the round-trip through two encodings, which an encoding
	/// pair that a developer is interested in can use to do the conversion more quickly. The view presents code units
	/// one at a time, regardless of how many code units are output by one decode operation. This means if, for
	/// example, one (1) UTF-16 code unit becomes two (2) UTF-8 code units, it will present each code unit one at a
	/// time. If you are looking to explicitly know each collection of characters, you will have to use lower-level
	/// interfaces.
	template <typename TFromEncoding, typename TToEncoding, typename TRange, typename _FromErrorHandler,
		typename _ToErrorHandler, typename TFromState, typename TToState>
	class recode_iterator
	: private EBCO<remove_cvref_t<TFromEncoding>, 1>,
	  private EBCO<remove_cvref_t<TToEncoding>, 2>,
	  private EBCO<remove_cvref_t<_FromErrorHandler>, 3>,
	  private EBCO<remove_cvref_t<_ToErrorHandler>, 4>,
	  private Impl::StateStorage<remove_cvref_t<TFromEncoding>, remove_cvref_t<TFromState>, 0>,
	  private Impl::StateStorage<remove_cvref_t<TToEncoding>, remove_cvref_t<TToState>, 1>,
	  private Impl::CursorCache<MaxCodeUnits<unwrap_remove_cvref_t<TToEncoding>>,
		  ranges::is_range_input_or_output_range_exactly_v<unwrap_remove_cvref_t<TRange>>>,
	  private Impl::ErrorCache<
		  DecodeErrorHandlerAlwaysReturnsOK<unwrap_remove_cvref_t<TFromEncoding>,
		       unwrap_remove_cvref_t<_FromErrorHandler>> // cf
		  && EncodeErrorHandlerAlwaysReturnsOK<unwrap_remove_cvref_t<TToEncoding>,
		       unwrap_remove_cvref_t<_ToErrorHandler>>>,
	  private EBCO<Krys::Ranges::range_reconstruct_t<unwrap_remove_cvref_t<TRange>>, 4> {
	private:
		using _UNonRRange                                = unwrap_remove_cvref_t<TRange>;
		using TURange                                    = Krys::Ranges::range_reconstruct_t<_UNonRRange>;
		using _UFromEncoding                             = unwrap_remove_cvref_t<TFromEncoding>;
		using _UToEncoding                               = unwrap_remove_cvref_t<TToEncoding>;
		using _UFromErrorHandler                         = unwrap_remove_cvref_t<_FromErrorHandler>;
		using _UToErrorHandler                           = unwrap_remove_cvref_t<_ToErrorHandler>;
		using _UFromState                                = unwrap_remove_cvref_t<TFromState>;
		using _UToState                                  = unwrap_remove_cvref_t<TToState>;
		using _BaseIterator                              = ranges::range_iterator_t<TURange>;
		using _IntermediateCodePoint                     = code_point_t<_UToEncoding>;
		inline static constexpr std::size_t _MaxValues = MaxCodeUnits<_UToEncoding>;
		inline static constexpr bool _IsSingleValueType  = _MaxValues == 1;
		inline static constexpr bool _IsInputOrOutput    = ranges::is_range_input_or_output_range_exactly_v<TURange>;
		inline static constexpr bool _IsCursorless       = _IsSingleValueType && !_IsInputOrOutput;
		inline static constexpr bool _IsErrorless
			= DecodeErrorHandlerAlwaysReturnsOK<_UFromEncoding, _UFromErrorHandler>
			&& EncodeErrorHandlerAlwaysReturnsOK<_UToEncoding, _UToErrorHandler>;
		using __base_cursor_cache_t       = Impl::CursorCache<_MaxValues, _IsInputOrOutput>;
		using __base_cursor_cache_size_t  = typename __base_cursor_cache_t::TSize;
		using __base_error_cache_t        = Impl::ErrorCache<_IsErrorless>;
		using __base_from_encoding_t      = EBCO<remove_cvref_t<TFromEncoding>, 1>;
		using __base_to_encoding_t        = EBCO<remove_cvref_t<TToEncoding>, 2>;
		using __base_from_error_handler_t = EBCO<remove_cvref_t<_FromErrorHandler>, 3>;
		using __base_to_error_handler_t   = EBCO<remove_cvref_t<_ToErrorHandler>, 4>;
		using __base_from_state_t
			= Impl::StateStorage<remove_cvref_t<TFromEncoding>, remove_cvref_t<TFromState>, 0>;
		using __base_to_state_t
			= Impl::StateStorage<remove_cvref_t<TToEncoding>, remove_cvref_t<TToState>, 1>;
		using TRangeBase = EBCO<TURange, 4>;

		inline static constexpr bool _IsBackwards = is_detected_v<Impl::HasEncodeOneBackwardsMember,
			_UFromEncoding, TURange, _UFromErrorHandler, _UFromState>;

	public:
		
		/// @brief The underlying range type.
		using range_type = TRange;
		
		/// @brief The base iterator type.
		using iterator_type = _BaseIterator;
		
		/// @brief The encoding type used for decoding to intermediate code point storage.
		using from_encoding_type = TFromEncoding;
		
		/// @brief The encoding type used for encoding to the final code units storage.
		using to_encoding_type = TToEncoding;
		
		/// @brief The error handler when a decode operation fails.
		using from_error_handler_type = _FromErrorHandler;
		
		/// @brief The error handler when an encode operation fails.
		using to_error_handler_type = _ToErrorHandler;
		
		/// @brief The state type used for decode operations.
		using from_state_type = remove_cvref_t<TFromState>;
		
		/// @brief The state type used for encode operations.
		using to_state_type = remove_cvref_t<TToState>;
		
		/// @brief The strength of the iterator category, as defined in relation to the base.
		using iterator_category = conditional_t<
			Krys::Ranges::is_iterator_concept_or_better_v<std::bidirectional_iterator_tag, _BaseIterator>,
			conditional_t<_IsBackwards, std::bidirectional_iterator_tag, std::forward_iterator_tag>,
			ranges::iterator_category_t<_BaseIterator>>;
		
		/// @brief The strength of the iterator concept, as defined in relation to the base.
		using iterator_concept = conditional_t<
			Krys::Ranges::is_iterator_concept_or_better_v<std::bidirectional_iterator_tag, _BaseIterator>,
			conditional_t<_IsBackwards, std::bidirectional_iterator_tag, std::forward_iterator_tag>,
			ranges::iterator_concept_t<_BaseIterator>>;
		
		/// @brief The object type that gets output on every dereference.
		using value_type = code_unit_t<TToEncoding>;
		
		/// @brief A pointer type to the value_type.
		using pointer = value_type*;
		
		/// @brief The value returned from derefencing the iterator.
		///
		/// @remarks This is a proxy iterator, so the `reference` is a non-reference `value_type.`
		using reference = value_type;
		
		/// @brief The type returned when two of these pointers are subtracted from one another.
		///
		/// @remarks It's not a very useful type...
		using difference_type = ranges::iterator_difference_type_t<_BaseIterator>;

		
		/// @brief Default constructs a recode_iterator.
		///
		/// @remarks This can only work if the underlying encodings, error handlers, and states can handle default
		/// construction.
		constexpr recode_iterator() noexcept(NoThrowDefaultConstructible<__base_from_encoding_t>
			&& NoThrowDefaultConstructible<__base_to_encoding_t>
			&& NoThrowDefaultConstructible<__base_from_error_handler_t>
			&& NoThrowDefaultConstructible<__base_to_error_handler_t>
			&& NoThrowConstructible<__base_from_state_t, TFromEncoding>
			&& NoThrowConstructible<__base_to_state_t, TToEncoding>
			&& std::is_default_constructible_v<TRangeBase>)
		: __base_from_encoding_t()
		, __base_to_encoding_t()
		, __base_from_error_handler_t()
		, __base_to_error_handler_t()
		, __base_from_state_t(this->from_encoding())
		, __base_to_state_t(this->to_encoding())
		, __base_cursor_cache_t()
		, TRangeBase()
		, _M_cache() {
			this->_M_read_one();
		}

		
		/// @brief Copy constructs a recode_iterator.
		constexpr recode_iterator(const recode_iterator&) = default;
		
		/// @brief Move constructs a recode_iterator.
		constexpr recode_iterator(recode_iterator&&) = default;

		
		/// @brief Constructs a recode_iterator from the underlying range.
		///
		/// @param[in] range The input range to wrap and iterate over.
		constexpr recode_iterator(range_type range) noexcept(
			noexcept(recode_iterator(std::move(range), to_encoding_type {})))
		: recode_iterator(std::move(range), to_encoding_type {}) {
		}

		
		/// @brief Constructs a recode_iterator from the underlying range.
		///
		/// @param[in] range The input range to wrap and iterate over.
		/// @param[in] __to_encoding The encoding object to call `EncodeOne` or equivalent functionality on.
		constexpr recode_iterator(range_type range, to_encoding_type __to_encoding)
		: recode_iterator(std::move(range), from_encoding_type {}, std::move(__to_encoding)) {
		}

		
		/// @brief Constructs a recode_iterator from the underlying range.
		///
		/// @param[in] range The input range to wrap and iterate over.
		/// @param[in] __from_encoding The encoding object to call `DecodeOne` or equivalent functionality on.
		/// @param[in] __to_encoding The encoding object to call `EncodeOne` or equivalent functionality on.
		constexpr recode_iterator(
			range_type range, from_encoding_type __from_encoding, to_encoding_type __to_encoding)
		: recode_iterator(std::move(range), std::move(__from_encoding), std::move(__to_encoding),
			  from_error_handler_type {}, to_error_handler_type {}) {
		}

		
		/// @brief Constructs a recode_iterator from the underlying range.
		///
		/// @param[in] range The input range to wrap and iterate over.
		/// @param[in] __from_encoding The encoding object to call `DecodeOne` or equivalent functionality on.
		/// @param[in] __to_encoding The encoding object to call `EncodeOne` or equivalent functionality on.
		/// @param[in] __from_error_handler The error handler for decode operations to store in this view.
		/// @param[in] __to_error_handler The error handler for encode operations to store in this view.
		constexpr recode_iterator(range_type range, from_encoding_type __from_encoding,
			to_encoding_type __to_encoding, from_error_handler_type __from_error_handler,
			to_error_handler_type __to_error_handler)
		: recode_iterator(std::move(range), std::move(__from_encoding), std::move(__to_encoding),
			  std::move(__from_error_handler), std::move(__to_error_handler), from_state_type {},
			  to_state_type {}) {
		}

		
		/// @brief Constructs a recode_iterator from the underlying range.
		///
		/// @param[in] range The input range to wrap and iterate over.
		/// @param[in] __from_encoding The encoding object to call `DecodeOne` or equivalent functionality on.
		/// @param[in] __to_encoding The encoding object to call `EncodeOne` or equivalent functionality on.
		/// @param[in] __from_error_handler The error handler for decode operations to store in this view.
		/// @param[in] __to_error_handler The error handler for encode operations to store in this view.
		/// @param[in] fromState The state to user for the decode operation.
		/// @param[in] toState The state to user for the decode operation.
		constexpr recode_iterator(range_type range, from_encoding_type __from_encoding,
			to_encoding_type __to_encoding, from_error_handler_type __from_error_handler,
			to_error_handler_type __to_error_handler, from_state_type fromState, to_state_type toState)
		: __base_from_encoding_t(std::move(__from_encoding))
		, __base_to_encoding_t(std::move(__to_encoding))
		, __base_from_error_handler_t(std::move(__from_error_handler))
		, __base_to_error_handler_t(std::move(__to_error_handler))
		, __base_from_state_t(this->from_encoding(), std::move(fromState))
		, __base_to_state_t(this->to_encoding(), std::move(toState))
		, __base_cursor_cache_t()
		, TRangeBase(Krys::Ranges::Reconstruct(std::in_place_type<_UNonRRange>, std::move(range)))
		, _M_cache() {
			this->_M_read_one();
		}

		
		/// @brief Copy assigns- a recode_iterator.
		constexpr recode_iterator& operator=(const recode_iterator&) = default;
		
		/// @brief Move assigns a recode_iterator.
		constexpr recode_iterator& operator=(recode_iterator&&) = default;

		// observers

		
		/// @brief The decoding ("from") encoding object.
		///
		/// @returns A const l-value reference to the encoding object used to construct this iterator.
		constexpr const from_encoding_type& from_encoding() const {
			return this->__base_from_encoding_t::GetValue();
		}

		
		/// @brief The decoding ("from") encoding object.
		///
		/// @returns An l-value reference to the encoding object used to construct this iterator.
		constexpr from_encoding_type& from_encoding() {
			return this->__base_from_encoding_t::GetValue();
		}

		
		/// @brief The encoding ("to") encoding object.
		///
		/// @returns A const l-value reference to the encoding object used to construct this iterator.
		constexpr const to_encoding_type& to_encoding() const {
			return this->__base_to_encoding_t::GetValue();
		}

		
		/// @brief The encoding ("to") encoding object.
		///
		/// @returns An l-value reference to the encoding object used to construct this iterator.
		constexpr to_encoding_type& to_encoding() {
			return this->__base_to_encoding_t::GetValue();
		}

		
		/// @brief The decoding ("from") state object.
		constexpr const from_state_type& FromState() const {
			return this->__base_from_state_t::GetState();
		}

		
		/// @brief The decoding ("from") state object.
		constexpr from_state_type& FromState() {
			return this->__base_from_state_t::GetState();
		}

		
		/// @brief The encoding ("to") state object.
		constexpr const to_state_type& ToState() const {
			return this->__base_to_state_t::GetState();
		}

		
		/// @brief The encoding ("to") state object.
		constexpr to_state_type& ToState() {
			return this->__base_to_state_t::GetState();
		}

		
		/// @brief The error handler object.
		constexpr const from_error_handler_type& from_handler() const {
			return this->__base_from_error_handler_t::GetValue();
		}

		
		/// @brief The error handler object.
		constexpr from_error_handler_type& from_handler() {
			return this->__base_from_error_handler_t::GetValue();
		}

		
		/// @brief The error handler object.
		constexpr const to_error_handler_type& to_handler() const& noexcept {
			return this->__base_to_error_handler_t::GetValue();
		}

		
		/// @brief The error handler object.
		constexpr to_error_handler_type& to_handler() & noexcept {
			return this->__base_to_error_handler_t::GetValue();
		}

		
		/// @brief The error handler object.
		constexpr to_error_handler_type&& to_handler() && noexcept {
			return std::move(this->__base_to_error_handler_t::GetValue());
		}

		
		/// @brief The input range used to construct this object.
		constexpr range_type range() & noexcept(std::is_copy_constructible_v<range_type>
			     ? std::is_nothrow_copy_constructible_v<range_type>
			     : (NoThrowMoveConstructible<range_type>)) {
			if constexpr (std::is_copy_constructible_v<range_type>) {
				return this->TRangeBase::GetValue();
			}
			else {
				return std::move(this->TRangeBase::GetValue());
			}
		}

		
		/// @brief The input range used to construct this object.
		constexpr range_type range() const& noexcept(std::is_nothrow_copy_constructible_v<range_type>) {
			return this->TRangeBase::GetValue();
		}

		
		/// @brief The input range used to construct this object.
		constexpr range_type range() && noexcept(NoThrowMoveConstructible<range_type>) {
			return std::move(this->TRangeBase::GetValue());
		}

		
		/// @brief Returns whether the last read operation had an encoding error or not.
		///
		/// @returns The EncodingError that occurred. This can be EncodingError::OK for
		/// an operation that went just fine.
		///
		/// @remarks If the error handler is identified as an error handler that, if given a suitably sized
		/// buffer, will never return an error. This is the case with specific encoding operations with
		/// Handlers::ReplacementHandler, or Handlers::ThrowHandler.
		constexpr EncodingError PivotErrorCode() const noexcept {
			if constexpr (_IsErrorless) {
				return EncodingError::OK;
			}
			else {
				return this->__base_error_cache_t::FromError();
			}
		}

		
		/// @brief Returns whether the last read operation had an encoding error or not.
		///
		/// @returns The EncodingError that occurred. This can be EncodingError::OK for
		/// an operation that went just fine.
		///
		/// @remarks If the error handler is identified as an error handler that, if given a suitably sized
		/// buffer, will never return an error. This is the case with specific encoding operations with
		/// Handlers::ReplacementHandler, or Handlers::ThrowHandler.
		constexpr EncodingError ErrorCode() const noexcept {
			if constexpr (_IsErrorless) {
				return EncodingError::OK;
			}
			else {
				return this->__base_error_cache_t::ToError();
			}
		}

		// observers and modifiers: iteration

		
		/// @brief Copy then increment the iterator.
		///
		/// @returns A copy of iterator, before incrementing.
		constexpr recode_iterator operator++(int) {
			recode_iterator __copy = *this;
			++(*this);
			return __copy;
		}

		
		/// @brief Increment the iterator.
		///
		/// @returns A reference to *this, after incrementing the iterator.
		constexpr recode_iterator& operator++() {
			if constexpr (_IsSingleValueType) {
				this->_M_read_one();
			}
			else {
				++this->Position;
				if (this->Position == this->__base_cursor_cache_t::Size) {
					this->_M_read_one();
				}
			}
			return *this;
		}

		
		/// @brief Dereference the iterator.
		///
		/// @returns A value_type (NOT a reference) of the iterator.
		///
		/// @remarks This is a proxy iterator, and therefore only returns a value_type object and not a reference
		/// object. Encoding iterators are only readable, not writable.
		constexpr value_type operator*() const {
			if constexpr (_IsSingleValueType) {
				return this->_M_cache[0];
			}
			else {
				return this->_M_cache[this->__base_cursor_cache_t::Position];
			}
		}

		// observers: comparison

		
		/// @brief Compares whether or not this iterator has truly reached the end.
		friend constexpr bool operator==(const recode_iterator& it, const recode_sentinel_t&) {
			if constexpr (it._IsCursorless || (it._IsInputOrOutput && it._IsSingleValueType)) {
				return it._M_base_is_empty()
					&& static_cast<__base_cursor_cache_size_t>(Impl::CursorlessSizeSentinel)
					== it.__base_cursor_cache_t::Size;
			}
			else {
				return it._M_base_is_empty()
					&& it.__base_cursor_cache_t::Position == it.__base_cursor_cache_t::Size;
			}
		}

		
		/// @brief Compares whether or not this iterator has truly reached the end.
		friend constexpr bool operator==(const recode_sentinel_t& __sen, const recode_iterator& it) {
			return it == __sen;
		}

		
		/// @brief Compares whether or not this iterator has truly reached the end.
		friend constexpr bool operator!=(const recode_iterator& it, const recode_sentinel_t&) {
			if constexpr (_IsCursorless || (_IsInputOrOutput && _IsSingleValueType)) {
				return !it._M_base_is_empty()
					|| static_cast<__base_cursor_cache_size_t>(Impl::CursorlessSizeSentinel)
					!= it.__base_cursor_cache_t::Size;
			}
			else {
				return !it._M_base_is_empty()
					|| it.__base_cursor_cache_t::Position != it.__base_cursor_cache_t::Size;
			}
		}

		
		/// @brief Compares whether or not this iterator has truly reached the end.
		friend constexpr bool operator!=(const recode_sentinel_t& __sen, const recode_iterator& it) {
			return !(__sen == it);
		}

	private:
		constexpr bool _M_base_is_empty() const noexcept {
			if constexpr (is_detected_v<ranges::detect_adl_empty, TRange>) {
				return Krys::Ranges::empty(this->TRangeBase::GetValue());
			}
			else {
				return Krys::Ranges::begin(this->TRangeBase::GetValue())
					== Krys::Ranges::end(this->TRangeBase::GetValue());
			}
		}

		constexpr void _M_read_one() noexcept {
			if (this->_M_base_is_empty()) {
				if constexpr (_IsCursorless || (_IsSingleValueType && _IsInputOrOutput)) {
					this->__base_cursor_cache_t::Size
						= static_cast<__base_cursor_cache_size_t>(Impl::CursorlessSizeSentinel);
				}
				else {
					this->__base_cursor_cache_t::Size
						= static_cast<__base_cursor_cache_size_t>(this->_M_cache.size());
					this->__base_cursor_cache_t::Position
						= static_cast<__base_cursor_cache_size_t>(this->_M_cache.size());
				}
				return;
			}

			auto& __this_input_range = this->_M_range();
			auto __this_cache_begin  = this->_M_cache.data();
			KRYS_MAYBE_UNUSED decltype(__this_cache_begin) __this_cache_end {};
			Span<value_type, _MaxValues> __cache_view(this->_M_cache);
			_IntermediateCodePoint __intermediate_storage[MaxCodePoints<_UFromEncoding>] {};
			using _Intermediate = Span<_IntermediateCodePoint, MaxCodePoints<_UFromEncoding>>;
			_Intermediate pivot(__intermediate_storage);
			if constexpr (_IsInputOrOutput) {
				auto result    = recode_one_into_raw(std::move(__this_input_range), this->from_encoding(),
					   __cache_view, this->to_encoding(), this->from_handler(), this->to_handler(),
					   this->FromState(), this->ToState(), pivot);
				__this_cache_end = std::to_address(Krys::Ranges::begin(result.Output));
				if constexpr (!_IsErrorless) {
					this->__base_error_cache_t::SetErrors(result.PivotErrorCode, result.ErrorCode);
				}
				this->TRangeBase::GetValue() = std::move(result.Input);
			}
			else {
				auto result    = recode_one_into_raw(__this_input_range, this->from_encoding(), __cache_view,
					   this->to_encoding(), this->from_handler(), this->to_handler(), this->FromState(),
					   this->ToState(), pivot);
				__this_cache_end = std::to_address(Krys::Ranges::begin(result.Output));
				if constexpr (!_IsErrorless) {
					this->__base_error_cache_t::SetErrors(pivot.ErrorCode, result.ErrorCode);
				}
				this->TRangeBase::GetValue() = std::move(result.Input);
			}
			if constexpr (!_IsSingleValueType) {
				__base_cursor_cache_size_t __data_size
					= static_cast<__base_cursor_cache_size_t>(__this_cache_end - __this_cache_begin);
				ZTD_TEXT_ASSERT_MESSAGE_I_("size of produced value can never be bigger than the cache",
					static_cast<std::size_t>(__data_size) <= this->_M_cache.size());
				this->__base_cursor_cache_t::Position = static_cast<__base_cursor_cache_size_t>(0);
				this->__base_cursor_cache_t::Size     = __data_size;
			}
		}

		constexpr TURange& _M_range() noexcept {
			return this->TRangeBase::GetValue();
		}

		constexpr const TURange& _M_range() const noexcept {
			return this->TRangeBase::GetValue();
		}

		std::array<value_type, _MaxValues> _M_cache;
	};


	
}



#endif
