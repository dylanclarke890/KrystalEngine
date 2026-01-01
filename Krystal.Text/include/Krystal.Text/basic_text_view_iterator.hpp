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

#ifndef ZTD_TEXT_BASIC_TEXT_VIEW_ITERATOR_HPP
#define ZTD_TEXT_BASIC_TEXT_VIEW_ITERATOR_HPP



#include <ztd/text/error_handler.hpp>
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include <ztd/text/decode_view.hpp>
#include <ztd/text/normalized_iterator.hpp>
#include <ztd/text/error_handler.hpp>
#include <ztd/text/detail/default_char_view.hpp>

#include "Krystal.Lib/Utils/Unwrap.hpp"
#include "Krystal.Lib/Ranges/Range.hpp"
#include <ztd/ranges/default_sentinel.hpp>

#include <string_view>



namespace Krys {
	

	using text_view_sentinel_t = ranges::default_sentinel_t;

	template <typename TEncoding, typename _NormalizationForm = nfkc,
		typename TRange        = Impl::default_char_view_t<code_unit_t<TEncoding>>,
		typename TErrorHandler = Handlers::DefaultHandler, typename TState = DecodeState<TEncoding>,
		typename _NormalizationStorage = Impl::__default_normal_storage_t<TRange, _NormalizationForm>>
	class basic_text_view_iterator
	: public normalized_iterator<_NormalizationForm, decode_view<TEncoding, TRange, TErrorHandler, TState>,
		  _NormalizationStorage> {
	private:
		using __base_normalized_iterator_t = normalized_iterator<_NormalizationForm,
			decode_view<TEncoding, TRange, TErrorHandler, TState>, _NormalizationStorage>;

	public:
		using __base_normalized_iterator_t::__base_normalized_iterator_t;
	};

	
}



#endif
