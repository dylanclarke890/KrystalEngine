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

#ifndef ZTD_TEXT_LITERAL_HPP
  #define ZTD_TEXT_LITERAL_HPP

  

  #include "Krystal.Text/UnicodeCodePoint.hpp"

  #include "Krystal.Text/CodePoint.hpp"
  #include "Krystal.Text/CodeUnit.hpp"
  #include <ztd/text/detail/encoding_name.hpp>
  #include <ztd/text/detail/forwarding_handler.hpp>
  #include <ztd/text/is_input_error_skippable.hpp>
  #include "Krystal.Text/IsUnicodeEncoding.hpp"
  #include "Krystal.Text/State.hpp"

  

namespace Krys
{
  namespace text
  {
    

    namespace Impl
    {
      inline constexpr TextEncodingId __literal_id =
        __idk_detail::__to_encoding_id(ZTD_CXX_COMPILE_TIME_ENCODING_NAME_GET_I_());
      using __literal = decltype(__select_compile_time_encoding<char, __literal_id>());
    } 

    
    /// @brief The encoding of string literal_ts ( e.g. @c "👍" ) at compile time.
    class literal_t : public Impl::__literal
    {
    private:
      using TBase = Impl::__literal;

    public:
      
      /// @brief Whether or not this literal_t encoding is a Unicode Transformation Format, such as UTF-8,
      /// UTF-EBCDIC, or GB18030.
      using IsUnicodeEncoding =
        IntegralConstant<bool, is_unicode_encoding_id(Impl::__literal_id)>;
      
      /// @brief The individual units that result from an encode operation or are used as input to a decode
      /// operation.
      using code_unit = code_unit_t<TBase>;
      
      /// @brief The individual units that result from a decode operation or as used as input to an encode
      /// operation. For most encodings, this is going to be a Unicode Code Point or a Unicode Scalar Value.
      using code_point = code_point_t<TBase>;
      
      /// @brief The state that can be used between calls to EncodeOne.
      using EncodeState = EncodeState<TBase>;
      
      /// @brief The state that can be used between calls to DecodeOne.
      using DecodeState = DecodeState<TBase>;
      
      /// @brief Whether or not the decode operation can process all forms of input into code point values.
      ///
      /// @remarks The decode step should always be injective because every encoding used for literal_ts in
      /// C++ needs to be capable of being represented by UCNs. Whether or not a platform is a jerk, who
      /// knows?
      using is_decode_injective = IntegralConstant<bool, IsDecodeInjective<TBase>>;
      
      /// @brief Whether or not the encode operation can process all forms of input into code unit values.
      ///
      /// @remarks This is absolutely not guaranteed to be the case, and as such we must check the provided
      /// encoding name for us to be sure.
      using is_encode_injective = IntegralConstant<bool, IsEncodeInjective<TBase>>;

      
      /// @brief The id representing the decoded text.
      inline static constexpr const Krys::TextEncodingId DecodedId = decoded_id_v<TBase>;
      
      /// @brief The id representing the encoded text.
      inline static constexpr const Krys::TextEncodingId EncodedId = encoded_id_v<TBase>;

      
      /// @brief The maximum number of code points a single complete operation of decoding can produce.
      inline static constexpr std::size_t MaxCodePoints = 16;
      
      /// @brief The maximum code units a single complete operation of encoding can produce.
      inline static constexpr std::size_t MaxCodeUnits = 32;

      
      /// @brief Default constructs a literal.
      constexpr literal_t() noexcept = default;
      
      /// @brief Copy constructs a literal.
      constexpr literal_t(const literal_t &) noexcept = default;
      
      /// @brief Move constructs a literal.
      constexpr literal_t(literal_t &&) noexcept = default;
      
      /// @brief Copy assigns into a literal_t object.
      constexpr literal_t &operator=(const literal_t &) noexcept = default;
      
      /// @brief Move assigns into a literal_t object.
      constexpr literal_t &operator=(literal_t &&) noexcept = default;

      
      /// @brief Decodes a single complete unit of information as code points and produces a result with the
      /// input and output ranges moved past what was successfully read and written; or, produces an error and
      /// returns the input and output ranges untouched.
      ///
      /// @param[in] input The input view to read code uunits from.
      /// @param[in] output The output view to write code points into.
      /// @param[in] errorHandler The error handler to invoke if encoding fails.
      /// @param[in, out] state The necessary state information. For this encoding, the state is empty and
      /// means very little.
      ///
      /// @returns A DecodeResult object that contains the input range, output range, error
      /// handler, and a reference to the passed-in state\.
      ///
      /// @remarks To the best ability of the implementation, the iterators will be returned untouched (e.g.,
      /// the input models at least a view and a forward_range). If it is not possible, returned ranges may be
      /// incremented even if an error occurs due to the semantics of any view that models an input_range.
      template <typename TInput, typename TOutput, typename TErrorHandler>
      constexpr auto DecodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler,
                                DecodeState &state) const
      {
  #if ZTD_IS_OFF(ZTD_CXX_COMPILE_TIME_ENCODING_NAME)                                                         \
    && ZTD_IS_OFF(ZTD_TEXT_YES_PLEASE_DESTROY_MY_LITERALS_UTTERLY_I_MEAN_IT)
        static_assert(always_false_v<TInput>,
                      ZTD_TEXT_UNKNOWN_LITERAL_ENCODING_MESSAGE_I_("string literal", "literal_t", ""));
  #endif
  #if ZTD_IS_OFF(ZTD_TEXT_YES_PLEASE_DESTROY_MY_LITERALS_UTTERLY_I_MEAN_IT)
        static_assert(always_true_v<TInput>
                        && !IsSpecializationOf<TBase, ::basic_no_encoding>,
                      ZTD_TEXT_UNIMPLEMENTED_LITERAL_ENCODING_MESSAGE_I_(
                        ZTD_CXX_COMPILE_TIME_ENCODING_NAME_DESCRIPTION_I_(), ""));
  #endif
        Impl::__forwarding_handler<const literal_t, remove_ref_t<TErrorHandler>>
          __underlying_handler(*this, errorHandler);
        return this->TBase::DecodeOne(std::forward<TInput>(input), std::forward<TOutput>(output),
                                          __underlying_handler, state);
      }

      
      /// @brief Encodes a single complete unit of information as code units and produces a result with the
      /// input and output ranges moved past what was successfully read and written; or, produces an error and
      /// returns the input and output ranges untouched.
      ///
      /// @param[in] input The input view to read code points from.
      /// @param[in] output The output view to write code units into.
      /// @param[in] errorHandler The error handler to invoke if encoding fails.
      /// @param[in, out] state The necessary state information. For this encoding, the state is empty and
      /// means very little.
      ///
      /// @returns A EncodeResult object that contains the input range, output range, error
      /// handler, and a reference to the passed-in state\.
      ///
      /// @remarks To the best ability of the implementation, the iterators will be returned untouched (e.g.,
      /// the input models at least a view and a forward_range). If it is not possible, returned ranges may be
      /// incremented even if an error occurs due to the semantics of any view that models an input_range.
      template <typename TInput, typename TOutput, typename TErrorHandler>
      constexpr auto EncodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler,
                                EncodeState &state) const
      {
  #if ZTD_IS_OFF(ZTD_CXX_COMPILE_TIME_ENCODING_NAME)                                                         \
    && ZTD_IS_OFF(ZTD_TEXT_YES_PLEASE_DESTROY_MY_LITERALS_UTTERLY_I_MEAN_IT)
        static_assert(always_false_v<TInput>,
                      ZTD_TEXT_UNKNOWN_LITERAL_ENCODING_MESSAGE_I_("string literal", "literal_t", ""));
  #endif
  #if ZTD_IS_OFF(ZTD_TEXT_YES_PLEASE_DESTROY_MY_LITERALS_UTTERLY_I_MEAN_IT)
        static_assert(always_true_v<TInput>
                        && !IsSpecializationOf<TBase, ::basic_no_encoding>,
                      ZTD_TEXT_UNIMPLEMENTED_LITERAL_ENCODING_MESSAGE_I_(
                        ZTD_CXX_COMPILE_TIME_ENCODING_NAME_DESCRIPTION_I_(), ""));
  #endif
        Impl::__forwarding_handler<const literal_t, remove_ref_t<TErrorHandler>>
          __underlying_handler(*this, errorHandler);
        return this->TBase::EncodeOne(std::forward<TInput>(input), std::forward<TOutput>(output),
                                          __underlying_handler, state);
      }

      template <typename TResult, typename TInputProgress, typename TOutputProgress,
                enable_if_t<::is_input_error_skippable_v<
                  const TBase &, TResult, const TInputProgress &, const TOutputProgress &>> * = nullptr>
      constexpr auto SkipInputError(TResult &&result, const TInputProgress &inputProgress,
                                      const TOutputProgress &outputProgress) const
        noexcept(::is_nothrow_skip_input_error_v<const TBase &, TResult, const TInputProgress &,
                                                            const TOutputProgress &>)
      {
        return ::SkipInputError(static_cast<const TBase &>(*this),
                                             std::forward<TResult>(result), inputProgress,
                                             outputProgress);
      }
    };

    
    /// @brief An instance of the literal_t type for ease of use.
    inline constexpr literal_t literal = {};

    
  }
} 

  

#endif