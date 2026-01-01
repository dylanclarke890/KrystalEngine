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

#ifndef ZTD_TEXT_DECODE_ONE_HPP
#define ZTD_TEXT_DECODE_ONE_HPP



#include "Krystal.Text/CodePoint.hpp"
#include <ztd/text/default_encoding.hpp>
#include "Krystal.Text/DecodeResult.hpp"
#include <ztd/text/error_handler.hpp>
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/IsUnicodeCodePoint.hpp"
#include <ztd/text/max_units.hpp>
#include "Krystal.Text/Impl/SpanReconstruct.hpp"
#include <ztd/text/detail/is_lossless.hpp>
#include <ztd/text/detail/encoding_range.hpp>
#include <ztd/text/detail/forward_if_move_only.hpp>

#include <ztd/ranges/unbounded.hpp>
#include <ztd/ranges/detail/insert_bulk.hpp>
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Text/CharTraits.hpp"
#include "Krystal.Lib/Utils/Tag.hpp"
#include <ztd/inline_containers.hpp>



namespace Krys {
	

	
	/// @addtogroup ztd_text_decode_one DecodeOne[_into]
	///
	/// @brief These functions convert from a view of input into a view of output using either the inferred or
	/// specified encodings. If no error handler is provided, the equivalent of the Handlers::DefaultHandler is
	/// used by default. If no associated state is provided for either the "to" or "from" encodings, one will be
	/// created with automatic storage duration (as a "stack" variable) for the provided encoding.
	/// @{

	
	/// @brief Converts one indivisible unit of information from the code units of the given `input` view through the
	/// encoding to code points into the `output` view.
	///
	/// @param[in] input An input_view to read code units from and use in the DecodeOne operation that will
	/// produce code points.
	/// @param[in] encoding The encoding that will be used to DecodeOne the input's code points into
	/// output code units.
	/// @param[in] output An output_view to write code points to as the result of the DecodeOne operation from
	/// the intermediate code units.
	/// @param[in] errorHandler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in,out] state A reference to the associated state for the `encoding` 's DecodeOne step.
	///
	/// @result A decode_one_result object that contains references to `state`.
	///
	/// @remarks This function is simply a small wrapper for calling DecodeOne on the `encoding` object.
	template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler, typename TState>
	constexpr auto decode_one_into_raw(TInput&& input, TEncoding&& encoding, TOutput&& output,
		TErrorHandler&& errorHandler, TState& state) {
		using TUEncoding     = remove_cvref_t<TEncoding>;
		using TUErrorHandler = remove_cvref_t<TErrorHandler>;

		static_assert(Impl::__is_decode_lossless_or_deliberate_v<TUEncoding, TUErrorHandler>,
			ZTD_TEXT_LOSSY_DECODE_MESSAGE_I_);

		return encoding.DecodeOne(
			std::forward<TInput>(input), std::forward<TOutput>(output), errorHandler, state);
	}

	
	/// @brief Converts one indivisible unit of information from the code units of the given `input` view through the
	/// encoding to code points into the `output` view.
	///
	/// @param[in] input An input_view to read code units from and use in the DecodeOne operation that will
	/// produce code points.
	/// @param[in] encoding The encoding that will be used to DecodeOne the input's code points into
	/// output code units.
	/// @param[in] output An output_view to write code points to as the result of the DecodeOne operation from the
	/// intermediate code units.
	/// @param[in] errorHandler The error handlers for the from and to encodings,
	/// respectively.
	///
	/// @result A stateless_decode_one_result object that contains references to `state`.
	///
	/// @remarks Creates a default `state` using CreateDecodeState.
	template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler>
	constexpr auto decode_one_into_raw(
		TInput&& input, TEncoding&& encoding, TOutput&& output, TErrorHandler&& errorHandler) {
		using TUEncoding = remove_cvref_t<TEncoding>;
		using TState     = DecodeState<TUEncoding>;

		TState state = CreateDecodeState(encoding);
		auto __stateful_result
			= decode_one_into_raw(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
			     std::forward<TOutput>(output), std::forward<TErrorHandler>(errorHandler), state);
		return Impl::__slice_to_stateless_decode(std::move(__stateful_result));
	}

	
	/// @brief Converts one indivisible unit of information from the code units of the given `input` view through the
	/// encoding to code points into the `output` view.
	///
	/// @param[in] input An input_view to read code units from and use in the DecodeOne operation that will
	/// produce code points.
	/// @param[in] encoding The encoding that will be used to DecodeOne the input's code points into
	/// output code units.
	/// @param[in] output An output_view to write code points to as the result of the DecodeOne operation from the
	/// intermediate code units.
	///
	/// @result A stateless_decode_one_result object that contains references to `state`.
	///
	/// @remarks Creates a default `error_handler` that is similar to Handlers::DefaultHandler, but marked as
	/// careless.
	template <typename TInput, typename TEncoding, typename TOutput>
	constexpr auto decode_one_into_raw(TInput&& input, TEncoding&& encoding, TOutput&& output) {
		Handlers::DefaultHandler __handler {};
		return ::decode_one_into_raw(std::forward<TInput>(input),
			std::forward<TEncoding>(encoding), std::forward<TOutput>(output), __handler);
	}

	
	/// @brief Converts one indivisible unit of information from the code units of the given `input` view through the
	/// encoding to code points into the `output` view.
	///
	/// @param[in] input An input_view to read code units from and use in the DecodeOne operation that will
	/// produce code points.
	/// @param[in] output An output_view to write code points to as the result of the DecodeOne operation from the
	/// intermediate code units.
	///
	/// @result A stateless_decode_one_result object that contains references to `state`.
	///
	/// @remarks Creates a default `encoding` by figuring out the `value_type` of the `input`, then passing that
	/// type into default_code_point_encoding_t. That encoding is that used to DecodeOne the input code
	/// units, by default.
	template <typename TInput, typename TOutput>
	constexpr auto decode_one_into_raw(TInput&& input, TOutput&& output) {
		using TUInput   = remove_cvref_t<TInput>;
		using TCodeUnit = ranges::rangeTValue_type_t<TUInput>;
#if ZTD_IS_ON(ZTD_STD_LIBRARY_IS_CONSTANT_EVALUATED)
		if (std::is_constant_evaluated()) {
			// Use literal encoding instead, if we meet the right criteria
			using TEncoding = default_consteval_code_unit_encoding_t<TCodeUnit>;
			TEncoding encoding {};
			return ::decode_one_into_raw(
				std::forward<TInput>(input), encoding, std::forward<TOutput>(output));
		}
		else
#endif
		{
			using TEncoding = default_code_unit_encoding_t<TCodeUnit>;
			TEncoding encoding {};
			return ::decode_one_into_raw(
				std::forward<TInput>(input), encoding, std::forward<TOutput>(output));
		}
	}

	namespace Impl {
		template <typename TInput, typename TEncoding, typename _OutputContainer, typename TErrorHandler,
			typename TState>
		constexpr auto __intermediate_decode_one_to_storage(TInput&& input, TEncoding&& encoding,
			_OutputContainer& output, TErrorHandler&& errorHandler, TState& state) {
			using TUEncoding                                  = remove_cvref_t<TEncoding>;
			constexpr std::size_t __max_units               = MaxCodePoints<TUEncoding> * 2;
			constexpr std::size_t __intermediate_buffer_max = __max_units;
			using _IntermediateValueType                      = code_point_t<TUEncoding>;
			using _InitialOutput = Span<_IntermediateValueType, __intermediate_buffer_max>;
			using TOutput        = Span<_IntermediateValueType>;

			static_assert(Impl::__is_decode_lossless_or_deliberate_v<TEncoding, TErrorHandler>,
				ZTD_TEXT_LOSSY_DECODE_MESSAGE_I_);

			_IntermediateValueType __intermediate_translation_buffer[__intermediate_buffer_max] {};
			_InitialOutput __intermediate_initial_output(__intermediate_translation_buffer);
			auto result = decode_one_into_raw(std::forward<TInput>(input), encoding,
				__intermediate_initial_output, errorHandler, state);
			TOutput intermediateOutput(__intermediate_initial_output.data(), result.Output.data());
			ranges::__rng_detail::__container_insert_bulk(output, intermediateOutput);
			return result;
		}

		template <bool _OutputOnly, bool _NoState, typename _OutputContainer, typename TInput, typename TEncoding,
			typename TErrorHandler, typename TState>
		constexpr auto __decode_one_dispatch(
			TInput&& input, TEncoding&& encoding, TErrorHandler&& errorHandler, TState& state) {
			using TUEncoding                    = remove_cvref_t<TEncoding>;
			constexpr std::size_t __max_units = max_decode_code_points_v<TUEncoding>;

			_OutputContainer output {};
			if constexpr (is_detected_v<ranges::detect_adl_size, TInput>) {
				using TSize = decltype(Krys::Ranges::size(input));
				if constexpr (is_detected_v<ranges::detect_reserve_with_size, _OutputContainer, TSize>) {
					auto __output_size_hint = Krys::Ranges::size(input);
					__output_size_hint *= (__max_units / 2);
					output.reserve(__output_size_hint);
				}
			}
			auto __stateful_result = Impl::__intermediate_decode_one_to_storage(
				std::forward<TInput>(input), std::forward<TEncoding>(encoding), output,
				std::forward<TErrorHandler>(errorHandler), state);
			if constexpr (_OutputOnly) {
				// We are explicitly discarding this information with this function call.
				(void)__stateful_result;
				return output;
			}
			else if constexpr (_NoState) {
				return Impl::__replace_decode_result_output_no_state(
					std::move(__stateful_result), std::move(output));
			}
			else {
				return Impl::__replace_decode_result_output(
					std::move(__stateful_result), std::move(output));
			}
		}

	} 

	
	/// @brief Converts one indivisible unit of information from the code units of the given `input` view through the
	/// encoding to code points into the `output` view.
	///
	/// @param[in] input An input_view to read code units from and use in the DecodeOne operation that will
	/// produce code points.
	/// @param[in] encoding The encoding that will be used to DecodeOne the input's code points into
	/// output code units.
	/// @param[in] output An output_view to write code points to as the result of the DecodeOne operation from
	/// the intermediate code units.
	/// @param[in] errorHandler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in,out] state A reference to the associated state for the `encoding` 's DecodeOne step.
	///
	/// @result A decode_one_result object that contains references to `state`.
	///
	/// @remarks This function is simply a small wrapper for calling DecodeOne on the `encoding` object.
	template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler, typename TState>
	constexpr auto decode_one_into(TInput&& input, TEncoding&& encoding, TOutput&& output,
		TErrorHandler&& errorHandler, TState& state) {
		auto __reconstructed_input = Impl::SpanReconstruct<TInput>(std::forward<TInput>(input));
		auto result = decode_one_into_raw(std::move(__reconstructed_input), std::forward<TEncoding>(encoding),
			std::forward<TOutput>(output), errorHandler, state);
		using _ReconstructedResultInput  = Impl::span_reconstruct_t<TInput, TInput>;
		using _ReconstructedResultOutput = Impl::span_reconstruct_mutable_t<TOutput, TOutput>;
		return DecodeResult<_ReconstructedResultInput, _ReconstructedResultOutput, TState>(
			Impl::SpanReconstruct<TInput>(std::move(result.Input)),
			Impl::SpanReconstructMutable<TOutput>(std::move(result.Output)), result.State);
	}

	
	/// @brief Converts one indivisible unit of information from the code units of the given `input` view through the
	/// encoding to code points into the `output` view.
	///
	/// @param[in] input An input_view to read code units from and use in the DecodeOne operation that will
	/// produce code points.
	/// @param[in] encoding The encoding that will be used to DecodeOne the input's code points into
	/// output code units.
	/// @param[in] output An output_view to write code points to as the result of the DecodeOne operation from the
	/// intermediate code units.
	/// @param[in] errorHandler The error handlers for the from and to encodings,
	/// respectively.
	///
	/// @result A stateless_decode_one_result object that contains references to `state`.
	///
	/// @remarks Creates a default `state` using CreateDecodeState.
	template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler>
	constexpr auto decode_one_into(
		TInput&& input, TEncoding&& encoding, TOutput&& output, TErrorHandler&& errorHandler) {
		using TUEncoding = remove_cvref_t<TEncoding>;
		using TState     = DecodeState<TUEncoding>;

		TState state = CreateDecodeState(encoding);
		auto __stateful_result
			= decode_one_into(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
			     std::forward<TOutput>(output), std::forward<TErrorHandler>(errorHandler), state);
		return Impl::__slice_to_stateless_decode(std::move(__stateful_result));
	}

	
	/// @brief Converts one indivisible unit of information from the code units of the given `input` view through the
	/// encoding to code points into the `output` view.
	///
	/// @param[in] input An input_view to read code units from and use in the DecodeOne operation that will
	/// produce code points.
	/// @param[in] encoding The encoding that will be used to DecodeOne the input's code points into
	/// output code units.
	/// @param[in] output An output_view to write code points to as the result of the DecodeOne operation from the
	/// intermediate code units.
	///
	/// @result A stateless_decode_one_result object that contains references to `state`.
	///
	/// @remarks Creates a default `error_handler` that is similar to Handlers::DefaultHandler, but marked as
	/// careless.
	template <typename TInput, typename TEncoding, typename TOutput>
	constexpr auto decode_one_into(TInput&& input, TEncoding&& encoding, TOutput&& output) {
		Handlers::DefaultHandler __handler {};
		return ::decode_one_into(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
			std::forward<TOutput>(output), __handler);
	}

	
	/// @brief Converts one indivisible unit of information from the code units of the given `input` view through the
	/// encoding to code points into the `output` view.
	///
	/// @param[in] input An input_view to read code units from and use in the DecodeOne operation that will
	/// produce code points.
	/// @param[in] output An output_view to write code points to as the result of the DecodeOne operation from the
	/// intermediate code units.
	///
	/// @result A stateless_decode_one_result object that contains references to `state`.
	///
	/// @remarks Creates a default `encoding` by figuring out the `value_type` of the `input`, then passing that
	/// type into default_code_point_encoding_t. That encoding is that used to DecodeOne the input code
	/// units, by default.
	template <typename TInput, typename TOutput>
	constexpr auto decode_one_into(TInput&& input, TOutput&& output) {
		using TUInput   = remove_cvref_t<TInput>;
		using TCodeUnit = ranges::rangeTValue_type_t<TUInput>;
#if ZTD_IS_ON(ZTD_STD_LIBRARY_IS_CONSTANT_EVALUATED)
		if (std::is_constant_evaluated()) {
			// Use literal encoding instead, if we meet the right criteria
			using TEncoding = default_consteval_code_unit_encoding_t<TCodeUnit>;
			TEncoding encoding {};
			return ::decode_one_into(
				std::forward<TInput>(input), encoding, std::forward<TOutput>(output));
		}
		else
#endif
		{
			using TEncoding = default_code_unit_encoding_t<TCodeUnit>;
			TEncoding encoding {};
			return ::decode_one_into(
				std::forward<TInput>(input), encoding, std::forward<TOutput>(output));
		}
	}

	
	/// @brief Converts one indivisible unit of information from the code units of the given `input` view through the
	/// encoding to code points, stored in an object of `_OutputContainer` type.
	///
	/// @tparam _OutputContainer The container type to serialize data into.
	///
	/// @param[in] input An input_view to read code units from and use in the DecodeOne operation that will
	/// produce code points.
	/// @param[in] encoding The encoding that will be used to DecodeOne the input's code points into
	/// output code units.
	/// @param[in] errorHandler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in,out] state A reference to the associated state for the `encoding` 's DecodeOne step.
	///
	/// @result A decode_one_result object that contains references to `state` and an output of type
	/// `_OutputContainer`.
	///
	/// @remarks This function detects creates a container of type `_OutputContainer` and uses a typical @c
	/// std::back_inserter or `std::push_back_inserter` to fill in elements as it is written to. The result is
	/// then returned, with the `.Output` value put into the container.
	template <typename _OutputContainer = void, typename TInput, typename TEncoding, typename TErrorHandler,
		typename TState>
	constexpr auto decode_one_to(
		TInput&& input, TEncoding&& encoding, TErrorHandler&& errorHandler, TState& state) {
		using TUEncoding                = remove_cvref_t<TEncoding>;
		using _UOutputContainer         = remove_cvref_t<_OutputContainer>;
		using _OutputCodePoint          = code_point_t<TUEncoding>;
		constexpr bool _IsVoidContainer = Void<_UOutputContainer>;
		constexpr bool _IsStringable
			= (is_char_traitable_v<_OutputCodePoint> || IsUnicodeCodePoint<_OutputCodePoint>);
		constexpr std::size_t __max_units = max_decode_code_points_v<TUEncoding>;
		if constexpr (_IsVoidContainer && _IsStringable) {
			// prevent instantiation errors with basic_string by boxing it inside of an "if constexpr"
			using _RealOutputContainer = ::ztd::inline_basic_string<_OutputCodePoint, __max_units>;
			return Impl::__decode_one_dispatch<false, false, _RealOutputContainer>(
				std::forward<TInput>(input), std::forward<TEncoding>(encoding),
				std::forward<TErrorHandler>(errorHandler), state);
		}
		else {
			using _RealOutputContainer = conditional_t<_IsVoidContainer,
				::ztd::inline_vector<_OutputCodePoint, __max_units>, _OutputContainer>;
			return Impl::__decode_one_dispatch<false, false, _RealOutputContainer>(
				std::forward<TInput>(input), std::forward<TEncoding>(encoding),
				std::forward<TErrorHandler>(errorHandler), state);
		}
	}

	
	/// @brief Converts one indivisible unit of information from the code units of the given `input` view through the
	/// encoding to code points, stored in an object of `_OutputContainer` type.
	///
	/// @tparam _OutputContainer The container type to serialize data into.
	///
	/// @param[in] input An input_view to read code units from and use in the DecodeOne operation that will
	/// produce code points.
	/// @param[in] encoding The encoding that will be used to DecodeOne the input's code points into
	/// output code units.
	/// @param[in] errorHandler The error handlers for the from and to encodings,
	/// respectively.
	///
	/// @result A stateless_decode_one_result object whose output is of type `_OutputContainer`.
	///
	/// @remarks This function creates a `state` using CreateDecodeState.
	template <typename _OutputContainer = void, typename TInput, typename TEncoding, typename TErrorHandler>
	constexpr auto decode_one_to(TInput&& input, TEncoding&& encoding, TErrorHandler&& errorHandler) {
		using TUEncoding = remove_cvref_t<TEncoding>;
		using TState     = DecodeState<TUEncoding>;
		TState state   = ::CreateDecodeState(encoding);
		return ::decode_one_to<_OutputContainer>(std::forward<TInput>(input),
			std::forward<TEncoding>(encoding), std::forward<TErrorHandler>(errorHandler), state);
	}

	
	/// @brief Converts one indivisible unit of information from the code units of the given `input` view through the
	/// encoding to code points, stored in an object of `_OutputContainer` type.
	///
	/// @tparam _OutputContainer The container type to serialize data into.
	///
	/// @param[in] input An input_view to read code units from and use in the DecodeOne operation that will
	/// produce code points.
	/// @param[in] encoding The encoding that will be used to DecodeOne the input's code points into
	/// output code units.
	///
	/// @result A stateless_decode_one_result object whose output is of type `_OutputContainer`.
	///
	/// @remarks This function creates a `handler` using Handlers::DefaultHandler, but marks it as careless.
	template <typename _OutputContainer = void, typename TInput, typename TEncoding>
	constexpr auto decode_one_to(TInput&& input, TEncoding&& encoding) {
		Handlers::DefaultHandler __handler {};
		return ::decode_one_to<_OutputContainer>(
			std::forward<TInput>(input), std::forward<TEncoding>(encoding), __handler);
	}

	
	/// @brief Converts one indivisible unit of information from the code units of the given `input` view through the
	/// encoding to code points, stored in an object of `_OutputContainer` type.
	///
	/// @tparam _OutputContainer The container type to serialize data into.
	///
	/// @param[in] input An input_view to read code units from and use in the DecodeOne operation that will produce
	/// code points.
	///
	/// @result A stateless_decode_one_result object whose output is of type `_OutputContainer`.
	///
	/// @remarks This function creates an `encoding` by using the `value_type` of the `input` which is then
	/// passed through the default_code_point_encoding type to get the default desired encoding.
	template <typename _OutputContainer = void, typename TInput>
	constexpr auto decode_one_to(TInput&& input) {
		using TUInput   = remove_cvref_t<TInput>;
		using TCodeUnit = ranges::rangeTValue_type_t<TUInput>;
#if ZTD_IS_ON(ZTD_STD_LIBRARY_IS_CONSTANT_EVALUATED)
		if (std::is_constant_evaluated()) {
			// Use literal encoding instead, if we meet the right criteria
			using TEncoding = default_consteval_code_unit_encoding_t<TCodeUnit>;
			TEncoding encoding {};
			return ::decode_one_to<_OutputContainer>(std::forward<TInput>(input), encoding);
		}
		else
#endif
		{
			using TEncoding = default_code_unit_encoding_t<TCodeUnit>;
			TEncoding encoding {};
			return ::decode_one_to<_OutputContainer>(std::forward<TInput>(input), encoding);
		}
	}

	
	/// @brief Converts one indivisible unit of information from the code units of the given `input` view through the
	/// encoding to code points, stored in an object of `_OutputContainer` type.
	///
	/// @tparam _OutputContainer The container type to serialize data into.
	///
	/// @param[in] input An input_view to read code units from and use in the DecodeOne operation that will
	/// produce code points.
	/// @param[in] encoding The encoding that will be used to DecodeOne the input's code points into
	/// output code units.
	/// @param[in] errorHandler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in,out] state A reference to the associated state for the `encoding` 's DecodeOne step.
	///
	/// @result An object of type `_OutputContainer` .
	///
	/// @remarks This function detects creates a container of type `_OutputContainer` and uses a typical @c
	/// std::back_inserter or `std::push_back_inserter` to fill in elements as it is written to.
	template <typename _OutputContainer = void, typename TInput, typename TEncoding, typename TErrorHandler,
		typename TState>
	constexpr auto DecodeOne(
		TInput&& input, TEncoding&& encoding, TErrorHandler&& errorHandler, TState& state) {
		using TUEncoding                    = remove_cvref_t<TEncoding>;
		using _UOutputContainer             = remove_cvref_t<_OutputContainer>;
		using _OutputCodePoint              = code_point_t<TUEncoding>;
		constexpr std::size_t __max_units = max_decode_code_points_v<TUEncoding>;
		constexpr bool _IsVoidContainer     = Void<_UOutputContainer>;
		constexpr bool _IsStringable
			= (is_char_traitable_v<_OutputCodePoint> || IsUnicodeCodePoint<_OutputCodePoint>);

		if constexpr (_IsVoidContainer && _IsStringable) {
			using _RealOutputContainer = ::ztd::inline_basic_string<_OutputCodePoint, __max_units>;
			return Impl::__decode_one_dispatch<true, false, _RealOutputContainer>(
				std::forward<TInput>(input), std::forward<TEncoding>(encoding),
				std::forward<TErrorHandler>(errorHandler), state);
		}
		else {
			using _RealOutputContainer = conditional_t<_IsVoidContainer,
				::ztd::inline_vector<_OutputCodePoint, MaxCodePoints<TUEncoding>>, _OutputContainer>;
			return Impl::__decode_one_dispatch<true, false, _RealOutputContainer>(
				std::forward<TInput>(input), std::forward<TEncoding>(encoding),
				std::forward<TErrorHandler>(errorHandler), state);
		}
	}

	
	/// @brief Converts one indivisible unit of information from the code units of the given `input` view through the
	/// encoding to code points, stored in an object of `_OutputContainer` type.
	///
	/// @tparam _OutputContainer The container type to serialize data into.
	///
	/// @param[in] input An input_view to read code units from and use in the DecodeOne operation that will
	/// produce code points.
	/// @param[in] encoding The encoding that will be used to DecodeOne the input's code points into
	/// output code units.
	/// @param[in] errorHandler The error handlers for the from and to encodings,
	/// respectively.
	///
	/// @result An object of type `_OutputContainer` .
	///
	/// @remarks This function creates a `state` using CreateDecodeState.
	template <typename _OutputContainer = void, typename TInput, typename TEncoding, typename TErrorHandler>
	constexpr auto DecodeOne(TInput&& input, TEncoding&& encoding, TErrorHandler&& errorHandler) {
		using TUEncoding = remove_cvref_t<TEncoding>;
		using TState     = DecodeState<TUEncoding>;

		TState state = CreateDecodeState(encoding);
		return ::DecodeOne<_OutputContainer>(std::forward<TInput>(input),
			std::forward<TEncoding>(encoding), std::forward<TErrorHandler>(errorHandler), state);
	}

	
	/// @brief Converts one indivisible unit of information from the code units of the given `input` view through the
	/// encoding to code points, stored in an object of `_OutputContainer` type.
	///
	/// @tparam _OutputContainer The container type to serialize data into.
	///
	/// @param[in] input An input_view to read code units from and use in the DecodeOne operation that will
	/// produce code points.
	/// @param[in] encoding The encoding that will be used to DecodeOne the input's code points into
	/// output code units.
	///
	/// @result An object of type `_OutputContainer` .
	///
	/// @remarks This function creates a `handler` using Handlers::DefaultHandler, but marks it as careless.
	template <typename _OutputContainer = void, typename TInput, typename TEncoding>
	constexpr auto DecodeOne(TInput&& input, TEncoding&& encoding) {
		Handlers::DefaultHandler __handler {};
		return ::DecodeOne<_OutputContainer>(
			std::forward<TInput>(input), std::forward<TEncoding>(encoding), __handler);
	}

	
	/// @brief Converts one indivisible unit of information from the code units of the given `input` view through the
	/// encoding to code points, stored in an object of `_OutputContainer` type.
	///
	/// @tparam _OutputContainer The container type to serialize data into.
	///
	/// @param[in] input An input_view to read code units from and use in the DecodeOne operation that will
	/// produce code points.
	///
	/// @result An object of type `_OutputContainer` .
	///
	/// @remarks This function creates an `encoding` by using the `value_type` of the `input` which is then
	/// passed through the default_code_point_encoding type to get the default desired encoding.
	template <typename _OutputContainer = void, typename TInput>
	constexpr auto DecodeOne(TInput&& input) {
		using TUInput   = remove_cvref_t<TInput>;
		using TCodeUnit = ranges::rangeTValue_type_t<TUInput>;
#if ZTD_IS_ON(ZTD_STD_LIBRARY_IS_CONSTANT_EVALUATED)
		if (std::is_constant_evaluated()) {
			// Use literal encoding instead, if we meet the right criteria
			using TEncoding = default_consteval_code_unit_encoding_t<TCodeUnit>;
			TEncoding encoding {};
			return ::DecodeOne<_OutputContainer>(std::forward<TInput>(input), encoding);
		}
		else
#endif
		{
			using TEncoding = default_code_unit_encoding_t<TCodeUnit>;
			TEncoding encoding {};
			return ::DecodeOne<_OutputContainer>(std::forward<TInput>(input), encoding);
		}
	}

	
	/// @}

	
}



#endif
