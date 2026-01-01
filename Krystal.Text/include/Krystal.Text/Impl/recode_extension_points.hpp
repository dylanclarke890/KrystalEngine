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

#ifndef ZTD_TEXT_DETAIL_RECODE_EXTENSION_POINTS_HPP
#define ZTD_TEXT_DETAIL_RECODE_EXTENSION_POINTS_HPP



#include "Krystal.Text/EncodeResult.hpp"
#include "Krystal.Text/DecodeResult.hpp"
#include <ztd/text/RecodeResult.hpp>
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include <ztd/text/extension.hpp>

#include "Krystal.Lib/Utils/Tag.hpp"



namespace Krys {
	

	// These optimizations can only be applied when the encodings are identical AND both
	// IsIgnorableErrorHandler<FromErrorHandler> and IsIgnorableErrorHandler<ToErrorHandler> are true. In the
	// other case, at the very least a decode needs to be performed. If it's successful, THEN we memcpy the used bits
	// from the input directly to the output. This relies on the operation being symmetric for decode and encode,
	// which is not EXACTLY a requirement, but it's worth doing anyhow.
#if 0
	template <typename TInput, typename TOutput, typename TFromEncoding, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler,
		enable_if_t<SameType<remove_cvref_t<TFromEncoding>,
		     remove_cvref_t<TToEncoding>>>* = nullptr>
	constexpr auto InternalTextRecodeOne(
		Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>>,
		TInput&& input, TFromEncoding&& __from_encoding, TOutput&& output, TToEncoding&& __to_encoding,
		_FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler,
		DecodeState<remove_cvref_t<TFromEncoding>>& fromState,
		EncodeState<remove_cvref_t<TToEncoding>>& toState) {
		using TFromState = DecodeState<remove_cvref_t<TFromEncoding>>;
		using TToState   = EncodeState<remove_cvref_t<TToEncoding>>;
		Impl::__copy(Krys::Ranges::begin(input), Krys::Ranges::end(input),
			Krys::Ranges::begin(output), Krys::Ranges::end(output));
		return RecodeResult<TInput, TOutput, TFromState, TToState>(std::forward<TInput>(input),
			std::forward<TOutput>(output), fromState, toState, EncodingError::OK);
	}

	template <typename TInput, typename TOutput, typename TFromEncoding, typename TToEncoding,
		typename _FromErrorHandler, typename _ToErrorHandler,
		enable_if_t<SameType<remove_cvref_t<TFromEncoding>,
		     remove_cvref_t<TToEncoding>>>* = nullptr>
	constexpr auto InternalTextRecode(
		Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>>,
		TInput&& input, TFromEncoding&& __from_encoding, TOutput&& output, TToEncoding&& __to_encoding,
		_FromErrorHandler&& __from_error_handler, _ToErrorHandler&& __to_error_handler,
		DecodeState<remove_cvref_t<TFromEncoding>>& fromState,
		EncodeState<remove_cvref_t<TToEncoding>>& toState) {
		using TFromState = DecodeState<remove_cvref_t<TFromEncoding>>;
		using TToState   = EncodeState<remove_cvref_t<TToEncoding>>;
		Impl::__copy(Krys::Ranges::begin(input), Krys::Ranges::end(input),
			Krys::Ranges::begin(output), Krys::Ranges::end(output));
		return RecodeResult<TInput, TOutput, TFromState, TToState>(std::forward<TInput>(input),
			std::forward<TOutput>(output), fromState, toState, EncodingError::OK);
	}

#endif

	
}



#endif
