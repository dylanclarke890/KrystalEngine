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

#ifndef ZTD_TEXT_BASIC_ENCODING_SCHEME_HPP
#define ZTD_TEXT_BASIC_ENCODING_SCHEME_HPP



#include "Krystal.Text/State.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/SkipInputError.hpp"
#include "Krystal.Text/Impl/ConstantEncodingTraits.hpp"
#include <ztd/text/detail/basic_encoding_scheme_includes.hpp>
#include "Krystal.Lib/Utils/Unwrap.hpp"
#include <optional>
#include <cstddef>



namespace Krys {
	

	
	/// @addtogroup ztd_text_encoding_scheme encoding_scheme
	/// @{

	
	/// @brief Decomposes the provided Encoding type into a specific endianness (big, little, or native) to allow
	/// for a single encoding type to be viewed in different ways.
	///
	/// @tparam TEncoding The encoding type.
	/// @tparam _Endian The endianess to use. Defaults to ztd::endian::native.
	/// @tparam _Byte The byte type to use. Defaults to ``std::byte``.
	///
	/// @remarks For example, this can be used to construct a Big Endian UTF-16 by using
	/// ``encoding_scheme<utf16_t, ztd::endian::big>``. It can be made interopable with ``unsigned
	/// char`` buffers rather than ``std::byte`` buffers by doing:
	/// ``encoding_scheme<utf32_t, ztd::endian::native, uchar>``.
	template <typename TEncoding, endian _Endian = endian::native, typename _Byte = std::byte>
	class encoding_scheme : public Impl::IsUnicodeEncodingES<encoding_scheme<TEncoding, _Endian, _Byte>,
		                        unwrap_remove_cvref_t<TEncoding>>,
		                   private EBCO<TEncoding> {
	private:
		using TBase       = EBCO<TEncoding>;
		using _UBaseEncoding = unwrap_remove_cvref_t<TEncoding>;
		using _BaseCodeUnit  = code_unit_t<_UBaseEncoding>;

	public:
		
		/// @brief The encoding type.
		
		/// @brief The encoding type that this scheme wraps.
		using encoding_type = TEncoding;
		
		/// @brief The individual units that result from a decode operation or as used as input to an encode
		/// operation. For most encodings, this is going to be a Unicode Code Point or a Unicode Scalar Value.
		using code_point = code_point_t<_UBaseEncoding>;
		/
		/// @brief The individual units that result from an encode operation or are used as input to a decode
		/// operation.
		///
		/// @remarks Typically, this type is usually always some kind of byte type (uchar or std::byte or
		/// other ``sizeof(obj) == 1`` type)./
		using code_unit = _Byte;
		
		/// @brief The state that can be used between calls to the decode function.
		///
		/// @remarks Even if the underlying encoding only has a single `state` type, we need to separate the two
		/// out in order to generically handle all encodings. Therefore, the encoding_scheme will always have
		/// both `EncodeState` and `DecodeState.`
		using DecodeState = DecodeState<_UBaseEncoding>;
		
		/// @brief The state that can be used between calls to the encode function.
		///
		/// @remarks Even if the underlying encoding only has a single `state` type, we need to separate the two
		/// out in order to generically handle all encodings. Therefore, the encoding_scheme will always have
		/// both `EncodeState` and `DecodeState.`
		using EncodeState = EncodeState<_UBaseEncoding>;
		
		/// @brief Whether or not the encode operation can process all forms of input into code point values.
		///
		/// @remarks Defers to what the underlying `encoding_type` does.
		using is_encode_injective = IntegralConstant<bool, IsEncodeInjective<_UBaseEncoding>>;
		
		/// @brief Whether or not the decode operation can process all forms of input into code point values.
		///
		/// @remarks Defers to what the underlying `encoding_type` does.
		using is_decode_injective = IntegralConstant<bool, IsDecodeInjective<_UBaseEncoding>>;
		
		/// @brief The maximum number of code points a single complete operation of decoding can produce. This is
		/// 1 for all Unicode Transformation Format (UTF) encodings.
		inline static constexpr const std::size_t MaxCodePoints = MaxCodePoints<_UBaseEncoding>;
		
		/// @brief The maximum code units a single complete operation of encoding can produce.
		inline static constexpr const std::size_t MaxCodeUnits
			= (MaxCodeUnits<_UBaseEncoding> * sizeof(_BaseCodeUnit)) / (sizeof(_Byte));
		
		/// @brief The id representing the decoded text.
		inline static constexpr const Krys::TextEncodingId DecodedId = decoded_id_v<_UBaseEncoding>;
		
		/// @brief The id representing the encoded text.
		inline static constexpr const Krys::TextEncodingId EncodedId = ::ztd::to_byte_text_encoding_id(
			encoded_id_v<_UBaseEncoding>, _Endian, sizeof(code_unit_t<_UBaseEncoding>));

		
		/// @brief Default constructs a encoding_scheme.
		encoding_scheme() = default;

		
		/// @brief Constructs a encoding_scheme with the encoding object and any additional arguments.
		///
		/// @param[in] __arg0 The first argument used to construct the stored encoding.
		/// @param[in] args Any additional arguments used to construct the stored encoding.
		template <typename _Arg0, typename... TArgs,
			enable_if_t<!SameType<remove_cvref_t<_Arg0>, encoding_scheme> // cf
			     && !SameType<remove_cvref_t<_Arg0>, std::in_place_t>>* = nullptr>
		constexpr encoding_scheme(_Arg0&& __arg0, TArgs&&... args) noexcept(
			NoThrowConstructible<_UBaseEncoding, _Arg0, TArgs...>)
		: TBase(std::forward<_Arg0>(__arg0), std::forward<TArgs>(args)...) {
		}

		
		/// @brief Constructs a encoding_scheme with the encoding object and any additional arguments.
		///
		/// @param[in] args Any additional arguments used to construct the encoding in the erased storage.
		///
		/// @remarks If the provided encoding does not have a byte code_unit type, it is wrapped in an
		/// encoding_scheme first.
		template <typename... TArgs>
		constexpr encoding_scheme(std::in_place_t, TArgs&&... args) noexcept(
			NoThrowConstructible<_UBaseEncoding, TArgs...>)
		: TBase(std::forward<TArgs>(args)...) {
		}

		
		/// @brief Cannot copy-construct a encoding_scheme object.
		encoding_scheme(const encoding_scheme&) = default;

		
		/// @brief Cannot copy-assign a encoding_scheme object.
		encoding_scheme& operator=(const encoding_scheme&) = default;

		
		/// @brief Move-constructs a encoding_scheme from the provided r-value reference.
		///
		/// @remarks This leaves the passed-in r-value reference without an encoding object. Calling any function
		/// on a moved-fron encoding_scheme, except for destruction, is a violation and invokes
		/// Undefined Behavior (generally, a crash).
		encoding_scheme(encoding_scheme&&) = default;

		
		/// @brief Move-assigns a encoding_scheme from the provided r-value reference.
		///
		/// @remarks This leaves the passed-in r-value reference without an encoding object. Calling any function
		/// on a moved-fron encoding_scheme, except for destruction, is a violation and may invoke
		/// Undefined Behavior (generally, a crash).
		encoding_scheme& operator=(encoding_scheme&&) = default;

		
		/// @brief Retrives the underlying encoding object.
		///
		/// @returns An l-value reference to the encoding object.
		constexpr encoding_type& base() & noexcept {
			return this->TBase::GetValue();
		}

		
		/// @brief Retrives the underlying encoding object.
		///
		/// @returns An l-value reference to the encoding object.
		constexpr const encoding_type& base() const& noexcept {
			return this->TBase::GetValue();
		}

		
		/// @brief Retrives the underlying encoding object.
		///
		/// @returns An l-value reference to the encoding object.
		constexpr encoding_type&& base() && noexcept {
			return this->TBase::GetValue();
		}

		
		/// @brief Returns, the desired replacement code units to use.
		///
		/// @remarks This is only callable if the function call exists on the wrapped encoding. It is broken down
		/// into a contiguous view type formulated from bytes if the wrapped code unit types do not match.
		template <typename _Unused                                     = encoding_type,
			enable_if_t<HasReplacementCodeUnits<_Unused>>* = nullptr>
		constexpr decltype(auto) ReplacementCodeUnits() const noexcept {
			using TOriginalCodeUnit = code_unit_t<encoding_type>;

			decltype(auto) original = this->base().ReplacementCodeUnits();
			if constexpr (SameType<TOriginalCodeUnit, code_unit>) {
				return original;
			}
			else {
				using TOriginalSpan    = Span<const TOriginalCodeUnit>;
				using TTransformedSpan = Span<const code_unit>;
				TOriginalSpan guaranteedCodeUnitView(original);
				// transform into proper type...
				auto transformedPtr = reinterpret_cast<const code_unit*>(guaranteedCodeUnitView.data());
				auto transformedSize
					= (guaranteedCodeUnitView.size() * sizeof(TOriginalCodeUnit)) / sizeof(const code_unit);
				return TTransformedSpan(transformedPtr, transformedSize);
			}
		}

		
		/// @brief Returns the desired replacement code points to use.
		///
		/// @remarks Is only callable if the function call exists on the wrapped encoding.
		template <typename _Unused                                      = encoding_type,
			enable_if_t<HasReplacementCodePoints<_Unused>>* = nullptr>
		constexpr decltype(auto) ReplacementCodePoints() const noexcept {
			return this->base().ReplacementCodePoints();
		}

		
		/// @brief Returns the desired replacement code units to use, or an empty optional-like type if there is
		/// nothing present.
		///
		/// @remarks This is only callable if the function call exists on the wrapped encoding. It is broken down
		/// into a contiguous view type formulated from bytes if the wrapped code unit types do not match.
		template <typename _Unused                                           = encoding_type,
			enable_if_t<HasMaybeReplacementCodeUnits<_Unused>>* = nullptr>
		constexpr decltype(auto) MaybeReplacementCodeUnits() const noexcept {
			using TOriginalCodeUnit = code_unit_t<encoding_type>;

			decltype(auto) maybeOriginal = this->base().MaybeReplacementCodeUnits();
			if constexpr (SameType<TOriginalCodeUnit, code_unit>) {
				return maybeOriginal;
			}
			else {
				using TOriginalSpan    = Span<const TOriginalCodeUnit>;
				using TTransformedSpan = Span<const code_unit>;
				if (!maybeOriginal) {
					return std::optional<TTransformedSpan>(std::nullopt);
				}
				decltype(auto) original = *maybeOriginal;
				TOriginalSpan guaranteedCodeUnitView(original);
				// transform into proper type...
				auto transformedPtr = reinterpret_cast<const code_unit*>(guaranteedCodeUnitView.data());
				auto transformedSize
					= (guaranteedCodeUnitView.size() * sizeof(TOriginalCodeUnit)) / sizeof(const code_unit);
				return TTransformedSpan(transformedPtr, transformedSize);
			}
		}

		
		/// @brief Returns the desired replacement code units to use.
		///
		/// @remarks This Is only callable if the function call exists on the wrapped encoding.
		template <typename _Unused                                            = encoding_type,
			enable_if_t<HasMaybeReplacementCodePoints<_Unused>>* = nullptr>
		constexpr decltype(auto) MaybeReplacementCodePoints() const noexcept {
			return this->base().MaybeReplacementCodePoints();
		}

		
		/// @brief Whether or not this encoding is some form of Unicode encoding.
		constexpr bool ContainsUnicodeEncoding() const noexcept {
			return ::ContainsUnicodeEncoding(this->base());
		}

		
		/// @brief Skips any consecutive input errors in the encoded input, where possible.
		///
		/// @remarks This Is only callable if the function call exists on the wrapped encoding.
		template <typename TResult, typename TInputProgress, typename TOutputProgress,
			enable_if_t<is_input_error_skippable_v<const encoding_type&, TResult, const TInputProgress&,
			     const TOutputProgress&>>* = nullptr>
		constexpr decltype(auto) SkipInputError(TResult&& result) const
			noexcept(::is_nothrow_skip_input_error_v<const encoding_type&, TResult, const TInputProgress&,
			     const TOutputProgress&>) {
			return ::SkipInputError(this->base(), std::forward<TResult>(result));
		}

		
		/// @brief Decodes a single complete unit of information as code points and produces a result with the
		/// input and output ranges moved past what was successfully read and written; or, produces an error and
		/// returns the input and output ranges untouched.
		///
		/// @param[in] input The input view to read code uunits from.
		/// @param[in] output The output view to write code points into.
		/// @param[in] errorHandler The error handler to invoke if encoding fails.
		/// @param[in, out] s The necessary state information. For this encoding, the state is empty and means
		/// very little.
		///
		/// @returns A DecodeResult object that contains the input range, output range, error handler, and
		/// a reference to the passed-in state\.
		///
		/// @remarks To the best ability of the implementation, the iterators will be returned untouched (e.g.,
		/// the input models at least a view and a forward_range). If it is not possible, returned ranges may be
		/// incremented even if an error occurs due to the semantics of any view that models an input_range.
		template <typename TInput, typename TOutput, typename TErrorHandler>
		constexpr auto DecodeOne(
			TInput&& input, TOutput&& output, TErrorHandler&& errorHandler, DecodeState& s) const {
			using _UOutputRange   = remove_cvref_t<TOutput>;
			using _CVErrorHandler = remove_ref_t<TErrorHandler>;
			using TSubInput       = Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
			using TSubOutput      = Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
			using TResult         = DecodeResult<TSubInput, TSubOutput, DecodeState>;

			using _InByteIt  = ranges::word_iterator<_BaseCodeUnit, TSubInput, _Endian>;
			using _InByteSen = ranges::word_sentinel;
			Krys::Ranges::subrange<_InByteIt, _InByteSen> __inbytes(
				_InByteIt(std::in_place, std::forward<TInput>(input)), _InByteSen());
			Impl::__scheme_handler<_Byte, TSubInput, _UOutputRange, _CVErrorHandler> intermediateHandler(
				errorHandler);
			auto result = this->base().DecodeOne(
				std::move(__inbytes), std::forward<TOutput>(output), intermediateHandler, s);
			return TResult(::ztd::unwrap_iterator(std::move(result.Input).begin()).range(),
				std::move(result.Output), s, result.ErrorCode, result.ErrorCount);
		}

		
		/// @brief Encodes a single complete unit of information as code units and produces a result with the
		/// input and output ranges moved past what was successfully read and written; or, produces an error and
		/// returns the input and output ranges untouched.
		///
		/// @param[in] input The input view to read code points from.
		/// @param[in] output The output view to write code units into.
		/// @param[in] errorHandler The error handler to invoke if encoding fails.
		/// @param[in, out] s The necessary state information. For this encoding, the state is empty and means
		/// very little.
		///
		/// @returns A EncodeResult object that contains the input range, output range, error handler, and
		/// a reference to the passed-in state\.
		///
		/// @remarks To the best ability of the implementation, the iterators will be returned untouched (e.g.,
		/// the input models at least a view and a forward_range). If it is not possible, returned ranges may be
		/// incremented even if an error occurs due to the semantics of any view that models an input_range.
		template <typename TInput, typename TOutput, typename TErrorHandler>
		constexpr auto EncodeOne(
			TInput&& input, TOutput&& output, TErrorHandler&& errorHandler, EncodeState& s) const {
			using TSubInput       = Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
			using TSubOutput      = Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
			using _OutByteIt      = ranges::word_iterator<_BaseCodeUnit, TSubOutput, _Endian>;
			using _OutByteSen     = ranges::word_sentinel;
			using _CVErrorHandler = remove_ref_t<TErrorHandler>;
			using TResult         = EncodeResult<TSubInput, TSubOutput, EncodeState>;

			TSubOutput __outwords_output(std::forward<TOutput>(output));
			ranges::subrange<_OutByteIt, _OutByteSen> __outwords(
				_OutByteIt(std::move(__outwords_output)), _OutByteSen());
			Impl::__scheme_handler<_Byte, TSubInput, TSubOutput, _CVErrorHandler> intermediateHandler(
				errorHandler);
			auto result
				= this->base().EncodeOne(std::forward<TInput>(input), __outwords, intermediateHandler, s);
			TSubOutput __result_output(::ztd::unwrap_iterator(std::move(result.Output).begin()).range());
			return TResult(std::move(result.Input), std::move(__result_output), s, result.ErrorCode,
				result.ErrorCount);
		}
	};

	
	/// @}

	
}



#endif
