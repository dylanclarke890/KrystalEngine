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

#ifndef ZTD_TEXT_IS_LOSSLESS_HPP
#define ZTD_TEXT_IS_LOSSLESS_HPP




#include "Krystal.Text/IsFullRangeRepresentable.hpp"

#include "Krystal.Lib/Core/TypeTraits.hpp"



namespace Krys {
	

	namespace Impl {
		template <typename TErrorHandler>
		class __is_careless_error_handler
		: public IntegralConstant<bool, SameType<remove_cvref_t<TErrorHandler>, Handlers::DefaultHandler>> {
		};

		template <typename _Byte, typename _UInputRange, typename _UOutputRange, typename TErrorHandler>
		class __is_careless_error_handler<__scheme_handler<_Byte, _UInputRange, _UOutputRange, TErrorHandler>>
		: public IntegralConstant<bool, __is_careless_error_handler<TErrorHandler>::value> { };

		template <typename TEncoding, typename TErrorHandler>
		class __is_careless_error_handler<__forwarding_handler<TEncoding, TErrorHandler>>
		: public IntegralConstant<bool, __is_careless_error_handler<TErrorHandler>::value> { };

		template <typename TErrorHandler>
		inline constexpr bool __is_careless_error_handler_v = __is_careless_error_handler<TErrorHandler>::value;

		template <typename TEncoding, typename TErrorHandler>
		class __is_encode_lossless_or_deliberate
		: public IntegralConstant<bool,
			  Impl::__is_careless_error_handler_v<remove_cvref_t<TErrorHandler>>
			       ? IsEncodeInjective<remove_cvref_t<TEncoding>>
			       : true> { };

		template <typename TEncoding, typename TErrorHandler>
		inline constexpr bool __is_encode_lossless_or_deliberate_v
			= __is_encode_lossless_or_deliberate<TEncoding, TErrorHandler>::value;

		template <typename TEncoding, typename TErrorHandler>
		class __is_decode_lossless_or_deliberate
		: public IntegralConstant<bool,
			  Impl::__is_careless_error_handler_v<remove_cvref_t<TErrorHandler>>
			       ? IsDecodeInjective<remove_cvref_t<TEncoding>>
			       : true> { };

		template <typename TEncoding, typename TErrorHandler>
		inline constexpr bool __is_decode_lossless_or_deliberate_v
			= __is_decode_lossless_or_deliberate<TEncoding, TErrorHandler>::value;

		template <typename TFromEncoding, typename TToEncoding, typename _FromErrorHandler, typename _ToErrorHandler>
		class __is_transcode_lossless_or_deliberate
		: public IntegralConstant<bool,
			  (Impl::__is_careless_error_handler_v<remove_cvref_t<_FromErrorHandler>>
			            ? IsDecodeInjective<remove_cvref_t<TFromEncoding>>
			            : true)
			       && (Impl::__is_careless_error_handler_v<remove_cvref_t<_ToErrorHandler>>
			                 ? IsEncodeInjective<remove_cvref_t<TToEncoding>>
			                 : true)> { };

		template <typename TFromEncoding, typename TToEncoding, typename _FromErrorHandler, typename _ToErrorHandler>
		inline constexpr bool __is_transcode_lossless_or_deliberate_v
			= __is_transcode_lossless_or_deliberate<TFromEncoding, TToEncoding, _FromErrorHandler,
			     _ToErrorHandler>::value;
	} 

	
}



#endif
