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

#ifndef ZTD_TEXT_DETAIL_UPDATE_INPUT_HPP
#define ZTD_TEXT_DETAIL_UPDATE_INPUT_HPP



#include "Krystal.Lib/Ranges/Reconstruct.hpp"



namespace Krys {
	

	namespace Impl {
		template <bool IsMutable, typename _Expected, typename TInput>
		constexpr bool __update_input_noexcept() noexcept {
			using TUInput    = remove_cvref_t<TInput>;
			using _UExpected = remove_cvref_t<_Expected>;
			if constexpr (SameType<TUInput, _UExpected>) {
				return true;
			}
			else {
				if constexpr (IsMutable) {
					return Krys::Ranges::is_nothrow_range_reconstructible_v<_UExpected, TInput>;
				}
				else {
					return Krys::Ranges::is_nothrow_range_const_reconstructible_v<_UExpected, TInput>;
				}
			}
		}

		template <typename _Expected, typename TInput>
		constexpr decltype(auto) __update_input(TInput&& input) noexcept(
			__update_input_noexcept<false, _Expected, TInput>()) {
			using TUInput    = remove_cvref_t<TInput>;
			using _UExpected = remove_cvref_t<_Expected>;
			if constexpr (SameType<TUInput, _UExpected>) {
				return std::forward<TInput>(input);
			}
			else {
				return Krys::Ranges::reconstruct(
					std::in_place_type<_UExpected>, std::forward<TInput>(input));
			}
		}

		template <typename _Expected, typename TInput>
		constexpr decltype(auto) __update_const_input(TInput&& input) noexcept(
			__update_input_noexcept<true, _Expected, TInput>()) {
			using TUInput    = remove_cvref_t<TInput>;
			using _UExpected = remove_cvref_t<_Expected>;
			if constexpr (SameType<TUInput, _UExpected>) {
				return std::forward<TInput>(input);
			}
			else {
				return Krys::Ranges::const_reconstruct(
					std::in_place_type<_UExpected>, std::forward<TInput>(input));
			}
		}
	} 

	
}



#endif
