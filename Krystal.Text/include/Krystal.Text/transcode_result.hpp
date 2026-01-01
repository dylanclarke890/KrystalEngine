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

#ifndef ZTD_TEXT_TRANSCODE_RESULT_HPP
#define ZTD_TEXT_TRANSCODE_RESULT_HPP



#include "Krystal.Text/UnicodeCodePoint.hpp"
#include "Krystal.Text/EncodingError.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/Impl/SpanReconstruct.hpp"
#include "Krystal.Text/Impl/ResultTypeConstraints.hpp"

#include "Krystal.Lib/Utils/ReferenceWrapper.hpp"
#include "Krystal.Lib/Ranges/Reconstruct.hpp"

#include <cstddef>
#include <array>
#include <utility>



namespace Krys {
	

	
	/// @brief The result of transcoding operations (such as transcode) that specifically do not include
	/// a reference to the state.
	template <typename TInput, typename TOutput>
	class stateless_transcode_result {
	public:
		
		/// @brief The reconstructed input_view object, with its .begin() incremented by the number of code units
		/// successfully read (can be identical to .begin() on original range on failure).
		TInput input;
		
		/// @brief The reconstructed output_view object, with its .begin() incremented by the number of code units
		/// successfully written (can be identical to .begin() on original range on failure).
		TOutput output;
		
		/// @brief The kind of error that occured, if any.
		EncodingError ErrorCode;
		
		/// @brief Whether or not the error handler was invoked, regardless of if the ErrorCode is set or not set to
		/// EncodingError::OK.
		std::size_t ErrorCount;

		
		/// @brief Constructs a stateless_transcode_result with the provided parameters and information,
		/// including whether or not an error was handled.
		///
		/// @param[in] other A different but related result type.
		template <typename TArgInput, typename TArgOutput,
			enable_if_t<Impl::ResultTypeCopyConstraint<::stateless_transcode_result,
			     TInput, TArgInput, TOutput, TArgOutput>()>* = nullptr>
		constexpr stateless_transcode_result(const stateless_transcode_result<TArgInput, TArgOutput>&
			     other) noexcept(Impl::ResultTypeCopyNoexcept<::stateless_transcode_result,
			TInput, TArgInput, TOutput, TArgOutput>())
		: input(other.Input)
		, output(other.Output)
		, ErrorCode(other.ErrorCode)
		, ErrorCount(other.ErrorCount) {
		}

		
		/// @brief Constructs a stateless_transcode_result with the provided parameters and information,
		/// including whether or not an error was handled.
		///
		/// @param[in] other A different but related result type.
		template <typename TArgInput, typename TArgOutput,
			enable_if_t<Impl::ResultTypeMoveConstraint<::stateless_transcode_result,
			     TInput, TArgInput, TOutput, TArgOutput>()>* = nullptr>
		constexpr stateless_transcode_result(stateless_transcode_result<TArgInput, TArgOutput>&& other) noexcept(
			Impl::ResultTypeMoveNoexcept<::stateless_transcode_result, TInput, TArgInput,
			     TOutput, TArgOutput>)
		: input(std::move(other.Input))
		, output(std::move(other.Output))
		, ErrorCode(other.ErrorCode)
		, ErrorCount(other.ErrorCount) {
		}


		
		/// @brief Constructs a stateless_transcode_result, defaulting the error code to
		/// EncodingError::OK if not provided.
		///
		/// @param[in] input The input range to store.
		/// @param[in] output The output range to store.
		/// @param[in] errorCode The error code for the encode operation, taken as the first of either the encode
		/// or decode operation that failed.
		template <typename TArgInput, typename TArgOutput>
		constexpr stateless_transcode_result(TArgInput&& input, TArgOutput&& output,
			EncodingError errorCode
			= EncodingError::OK) noexcept(noexcept(stateless_transcode_result(std::forward<TArgInput>(input),
			std::forward<TArgOutput>(output), errorCode, errorCode != EncodingError::OK)))
		: stateless_transcode_result(std::forward<TArgInput>(input), std::forward<TArgOutput>(output),
			  errorCode, errorCode != EncodingError::OK) {
		}

		
		/// @brief Constructs a stateless_transcode_result with the provided parameters and
		/// information, including whether or not an error was handled.
		///
		/// @param[in] input The input range to store.
		/// @param[in] output The output range to store.
		/// @param[in] errorCode The error code for the encode operation, taken as the first of either the encode
		/// or decode operation that failed.
		/// @param[in] errorCount Whether or not an error was handled. Some error handlers are corrective (see
		/// Handlers::ReplacementHandler), and so the error code is not enough to determine if the handler was
		/// invoked. This allows the value to be provided directly when constructing this result type.
		template <typename TArgInput, typename TArgOutput>
		constexpr stateless_transcode_result(TArgInput&& input, TArgOutput&& output, EncodingError errorCode,
			std::size_t errorCount) noexcept(NoThrowConstructible<TInput, TArgInput>
			&& NoThrowConstructible<TOutput, TArgOutput>)
		: input(std::forward<TArgInput>(input))
		, output(std::forward<TArgOutput>(output))
		, ErrorCode(errorCode)
		, ErrorCount(errorCount) {
		}

		
		/// @brief Whether or not any errors were handled.
		///
		/// @returns Simply checks whether `ErrorCount` is greater than 0.
		constexpr bool ErrorsWereHandled() const noexcept {
			return this->ErrorCount > 0;
		}
	};

	
	/// @brief The result of transcoding operations (such as transcode with the state argument provided).
	template <typename TInput, typename TOutput, typename TFromState, typename TToState>
	class pivotless_transcode_result : public stateless_transcode_result<TInput, TOutput> {
	private:
		using TBase = stateless_transcode_result<TInput, TOutput>;

	public:
		
		/// @brief A reference to the state of the associated Encoding used for decoding input code units to
		/// intermediate code points.
		ReferenceWrapper<TFromState> FromState;
		
		/// @brief A reference to the state of the associated Encoding used for encoding intermediate code points to
		/// code units.
		ReferenceWrapper<TToState> ToState;

		
		/// @brief Constructs a pivotless_transcode_result from a previous pivotless_transcode_result.
		///
		/// @param[in] other A different but related result type.
		template <typename TArgInput, typename TArgOutput, typename TArgFromState, typename TArgToState,
			enable_if_t<Impl::ResultTypeCopyConstraint<::pivotless_transcode_result,
			     TInput, TArgInput, TOutput, TArgOutput, TFromState, TArgFromState, TToState,
			     TArgToState>()>* = nullptr>
		constexpr pivotless_transcode_result(
			const pivotless_transcode_result<TArgInput, TArgOutput, TArgFromState, TArgToState>&
			     other) noexcept(Impl::ResultTypeCopyNoexcept<::pivotless_transcode_result,
			TInput, TArgInput, TOutput, TArgOutput, TFromState, TArgFromState, TToState, TArgToState>())
		: TBase(other.Input, other.Output, other.ErrorCode, other.ErrorCount)
		, FromState(other.FromState)
		, ToState(other.ToState) {
		}

		
		/// @brief Constructs a pivotless_transcode_result from a previous pivotless_transcode_result.
		///
		/// @param[in] other A different but related result type.
		template <typename TArgInput, typename TArgOutput, typename TArgFromState, typename TArgToState,
			enable_if_t<Impl::ResultTypeMoveConstraint<::pivotless_transcode_result,
			     TInput, TArgInput, TOutput, TArgOutput, TFromState, TArgFromState, TToState,
			     TArgToState>()>* = nullptr>
		constexpr pivotless_transcode_result(
			pivotless_transcode_result<TArgInput, TArgOutput, TArgFromState, TArgToState>&&
			     other) noexcept(Impl::ResultTypeMoveNoexcept<::pivotless_transcode_result,
			TInput, TArgInput, TOutput, TArgOutput, TFromState, TArgFromState, TToState, TArgToState>())
		: TBase(std::move(other.Input), std::move(other.Output), other.ErrorCode, other.ErrorCount)
		, FromState(other.FromState)
		, ToState(other.ToState) {
		}

		
		/// @brief Constructs a pivotless_transcode_result, defaulting the error code to
		/// EncodingError::OK if not provided.
		///
		/// @param[in] input The input range to store.
		/// @param[in] output The output range to store.
		/// @param[in] fromState The state related to the "From Encoding" that performed the decode half of the
		/// operation.
		/// @param[in] toState The state related to the "To Encoding" that performed the encode half of the
		/// operation.
		/// @param[in] errorCode The error code for the transcoding operation, taken as the first of either the
		/// encode or decode operation that failed.
		template <typename TArgInput, typename TArgOutput, typename TArgFromState, typename TArgToState>
		constexpr pivotless_transcode_result(TArgInput&& input, TArgOutput&& output, TArgFromState&& fromState,
			TArgToState&& toState, EncodingError errorCode = EncodingError::OK)
		: pivotless_transcode_result(std::forward<TArgInput>(input), std::forward<TArgOutput>(output),
			  std::forward<TArgFromState>(fromState), std::forward<TArgToState>(toState), errorCode,
			  errorCode != EncodingError::OK ? static_cast<std::size_t>(1) : static_cast<std::size_t>(0)) {
		}

		
		/// @brief Constructs a pivotless_transcode_result with the provided parameters and information,
		/// including whether or not an error was handled.
		///
		/// @param[in] input The input range to store.
		/// @param[in] output The output range to store.
		/// @param[in] fromState The state related to the "From Encoding" that performed the decode half of the
		/// operation.
		/// @param[in] toState The state related to the "To Encoding" that performed the encode half of the
		/// operation.
		/// @param[in] errorCode The error code for the encode operation, taken as the first of either the encode
		/// or decode operation that failed.
		/// @param[in] errorCount Whether or not an error was handled. Some error handlers are corrective (see
		/// Handlers::ReplacementHandler), and so the error code is not enough to determine if the handler was
		/// invoked. This allows the value to be provided directly when constructing this result type.
		template <typename TArgInput, typename TArgOutput, typename TArgFromState, typename TArgToState>
		constexpr pivotless_transcode_result(TArgInput&& input, TArgOutput&& output, TArgFromState&& fromState,
			TArgToState&& toState, EncodingError errorCode, std::size_t errorCount)
		: TBase(
			  std::forward<TArgInput>(input), std::forward<TArgOutput>(output), errorCode, errorCount)
		, FromState(std::forward<TArgFromState>(fromState))
		, ToState(std::forward<TArgToState>(toState)) {
		}
	};

	
	/// @brief The result of low-level transcoding operations (such as transcode_into with the pivot
	/// provided as an argument).
	template <typename TInput, typename TOutput, typename TFromState, typename TToState, typename TPivot>
	class transcode_result : public pivotless_transcode_result<TInput, TOutput, TFromState, TToState> {
	private:
		using TBase = pivotless_transcode_result<TInput, TOutput, TFromState, TToState>;

	public:
		
		/// @brief The range used to hold the intermediate pivot transcoding units.
		KRYS_NO_UNIQUE_ADDRESS TPivot pivot;
		
		/// @brief The kind of error that occured, if any, for the intermediate pivot.
		EncodingError PivotErrorCode;
		
		/// @brief Whether or not the error handler for the pivot point was invoked, regardless of if the
		/// `PivotErrorCode` is set or not set to EncodingError::OK.
		std::size_t PivotErrorCount;

		
		/// @brief Constructs a pivotless_transcode_result from a previous pivotless_transcode_result.
		///
		/// @param[in] other A different but related result type.
		template <typename TArgInput, typename TArgOutput, typename TArgFromState, typename TArgToState,
			typename TArgPivot,
			enable_if_t<Impl::ResultTypeCopyConstraint<::transcode_result, TInput,
			     TArgInput, TOutput, TArgOutput, TFromState, TArgFromState, TToState, TArgToState, TPivot,
			     TArgPivot>()>* = nullptr>
		constexpr transcode_result(
			const transcode_result<TArgInput, TArgOutput, TArgFromState, TArgToState, TArgPivot>&
			     other) noexcept(Impl::ResultTypeCopyNoexcept<::transcode_result, TInput,
			TArgInput, TOutput, TArgOutput, TFromState, TArgFromState, TToState, TArgToState, TPivot, TArgPivot>())
		: TBase(other.Input, other.Output, other.ErrorCode, other.ErrorCount, other.FromState,
			  other.ToState)
		, pivot(other.Pivot)
		, PivotErrorCode(other.PivotErrorCode)
		, PivotErrorCount(other.PivotErrorCount) {
		}

		
		/// @brief Constructs a pivotless_transcode_result from a previous pivotless_transcode_result.
		///
		/// @param[in] other A different but related result type.
		template <typename TArgInput, typename TArgOutput, typename TArgFromState, typename TArgToState,
			typename TArgPivot,
			enable_if_t<Impl::ResultTypeMoveConstraint<::transcode_result, TInput,
			     TArgInput, TOutput, TArgOutput, TFromState, TArgFromState, TToState, TArgToState, TPivot,
			     TArgPivot>()>* = nullptr>
		constexpr transcode_result(transcode_result<TArgInput, TArgOutput, TArgFromState, TArgToState, TArgPivot>&&
			     other) noexcept(Impl::ResultTypeMoveNoexcept<::transcode_result, TInput,
			TArgInput, TOutput, TArgOutput, TFromState, TArgFromState, TToState, TArgToState, TPivot, TArgPivot>())
		: TBase(std::move(other.Input), std::move(other.Output), other.ErrorCode, other.ErrorCount,
			  other.FromState, other.ToState)
		, pivot(std::move(other.Pivot))
		, PivotErrorCode(std::move(other.PivotErrorCode))
		, PivotErrorCount(std::move(other.PivotErrorCount)) {
		}

		
		/// @brief Constructs a pivotless_transcode_result with the provided parameters and information,
		/// including whether or not an error was handled.
		///
		/// @param[in] input The input range to store.
		/// @param[in] output The output range to store.
		/// @param[in] fromState The state related to the "From Encoding" that performed the decode half of the
		/// operation.
		/// @param[in] toState The state related to the "To Encoding" that performed the encode half of the
		/// operation.
		/// @param[in] errorCode The error code for the transcode operation, taken as the first of either the
		/// encode or decode operation that failed.
		/// @param[in] errorCount Whether or not an error was handled. Some error handlers are corrective (see
		/// Handlers::ReplacementHandler), and so the error code is not enough to determine if the handler was
		/// invoked. This allows the value to be provided directly when constructing this result type.
		/// @param[in] pivot The pivot for this transcode operation.
		/// @param[in] pivotErrorCode The error code for the decode step of the transcode oepration, if it failed.
		/// @param[in] pivotErrorCount Whether or not an error was handled during the decode step of the transcode
		/// operation. Some error handlers are corrective (see Handlers::ReplacementHandler), and so the error
		/// code is not enough to determine if the handler was invoked. This allows the value to be provided directly
		/// when constructing this result type.
		template <typename TArgInput, typename TArgOutput, typename TArgFromState, typename TArgToState,
			typename TArgPivot>
		constexpr transcode_result(TArgInput&& input, TArgOutput&& output, TArgFromState&& fromState,
			TArgToState&& toState, EncodingError errorCode, std::size_t errorCount, TArgPivot&& pivot,
			EncodingError pivotErrorCode, std::size_t pivotErrorCount)
		: TBase(std::forward<TArgInput>(input), std::forward<TArgOutput>(output), fromState, toState,
			  errorCode, errorCount)
		, pivot(std::forward<TArgPivot>(pivot))
		, PivotErrorCode(std::move(pivotErrorCode))
		, PivotErrorCount(std::move(pivotErrorCount)) {
		}
	};

	/////

	namespace Impl {
		template <typename TInput, typename TOutput, typename TFromState, typename TToState>
		constexpr stateless_transcode_result<TInput, TOutput>
		SliceToStateless(pivotless_transcode_result<TInput, TOutput, TFromState, TToState>&& result) noexcept(
			NoThrowConstructible<stateless_transcode_result<TInput, TOutput>,
			     stateless_transcode_result<TInput, TOutput>>) {
			return std::move(result);
		}

		template <typename TInput, typename TOutput, typename TFromState, typename TToState, typename TPivot>
		constexpr pivotless_transcode_result<TInput, TOutput, TFromState, TToState>
		SliceToPivotless(transcode_result<TInput, TOutput, TFromState, TToState, TPivot>&& result) noexcept(
			NoThrowConstructible<pivotless_transcode_result<TInput, TOutput, TFromState, TToState>,
			     pivotless_transcode_result<TInput, TOutput, TFromState, TToState>>) {
			return std::move(result);
		}

		template <typename TInput, typename TOutput, typename TFromState, typename TToState, typename TPivot>
		constexpr stateless_transcode_result<TInput, TOutput> SliceToStatelessPivotless(
			transcode_result<TInput, TOutput, TFromState, TToState, TPivot>&&
			     result) noexcept(NoThrowConstructible<stateless_transcode_result<TInput, TOutput>,
			stateless_transcode_result<TInput, TOutput>>) {
			return std::move(result);
		}

		template <typename TInput, typename TOutput, typename TDesiredOutput, typename TFromState, typename TToState>
		constexpr auto __replace_transcode_result_output_no_state(
			pivotless_transcode_result<TInput, TOutput, TFromState, TToState>&& result,
			TDesiredOutput&& desiredOutput) noexcept(std::
			     is_nothrow_constructible_v<stateless_transcode_result<TInput, TOutput>, TInput&&, TDesiredOutput,
			          EncodingError, std::size_t>) {
			using TResult = stateless_transcode_result<TInput, remove_cvref_t<TDesiredOutput>>;
			return TResult(std::move(result.Input), std::forward<TDesiredOutput>(desiredOutput),
				result.ErrorCode, result.ErrorCount);
		}

		template <typename TInput, typename TOutput, typename TFromState, typename TToState, typename TDesiredOutput>
		constexpr auto __replace_transcode_result_output(
			pivotless_transcode_result<TInput, TOutput, TFromState, TToState>&& result,
			TDesiredOutput&& desiredOutput) noexcept(std::
			     is_nothrow_constructible_v<pivotless_transcode_result<TInput, TOutput, TFromState, TToState>,
			          TInput&&, TDesiredOutput, TFromState&, TToState&, EncodingError, std::size_t>) {
			using TResult = pivotless_transcode_result<TInput, remove_cvref_t<TDesiredOutput>, TFromState, TToState>;
			return TResult(std::move(result.Input), std::forward<TDesiredOutput>(desiredOutput),
				result.FromState, result.ToState, result.ErrorCode, result.ErrorCount);
		}

		template <typename TInputRange, typename _OutputRange, typename TFromState, typename TToState>
		using __reconstruct_transcode_result_t = pivotless_transcode_result<Krys::Ranges::range_reconstruct_t<TInputRange>,
			Krys::Ranges::range_reconstruct_t<_OutputRange>, TFromState, TToState>;

		template <typename TInputRange, typename _OutputRange, typename TFromState, typename TToState,
			typename TPivot>
		using __reconstruct_pivot_transcode_result_t = transcode_result<Krys::Ranges::range_reconstruct_t<TInputRange>,
			Krys::Ranges::range_reconstruct_t<_OutputRange>, TFromState, TToState, Krys::Ranges::range_reconstruct_t<TPivot>>;
	} 

	
}



#endif
