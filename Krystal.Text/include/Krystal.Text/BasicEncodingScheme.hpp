#pragma once

#include "Krystal.Lib/Core/Endian.hpp"
#include "Krystal.Lib/Ranges/WordIterator.hpp"
#include "Krystal.Lib/Utils/Unwrap.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/Impl/ConstantEncodingTraits.hpp"
#include "Krystal.Text/SkipInputError.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/TextEncodingId.hpp"
#include <cstddef>
#include <optional>

namespace Krys
{
  /// @brief Decomposes the provided Encoding type into a specific endianness (big, little, or native) to
  /// allow for a single encoding type to be viewed in different ways.
  /// @tparam TEncoding The encoding type.
  /// @tparam TEndian The endianess to use. Defaults to Endian::Type::System.
  /// @tparam TByte The byte type to use. Defaults to ``std::byte``.
  /// @remarks For example, this can be used to construct a Big Endian UTF-16 by using
  /// ``EncodingScheme<utf16_t, Endian::Type::big>``. It can be made interopable with ``unsigned
  /// char`` buffers rather than ``std::byte`` buffers by doing:
  /// ``EncodingScheme<utf32_t, Endian::Type::System, uchar>``.
  template <typename TEncoding, Endian::Type TEndian = Endian::System, typename TByte = std::byte>
  class EncodingScheme : public Impl::IsUnicodeEncodingES<EncodingScheme<TEncoding, TEndian, TByte>,
                                                          unwrap_remove_cvref_t<TEncoding>>,
                         private EBCO<TEncoding>
  {
  private:
    using TBase = EBCO<TEncoding>;
    using TUBaseEncoding = unwrap_remove_cvref_t<TEncoding>;
    using TBaseCodeUnit = code_unit_t<TUBaseEncoding>;

  public:
    /// @brief The encoding type that this scheme wraps.
    using encoding_type = TEncoding;

    /// @brief The individual units that result from a decode operation or as used as input to an encode
    /// operation. For most encodings, this is going to be a Unicode Code Point or a Unicode Scalar Value.
    using code_point = code_point_t<TUBaseEncoding>;

    /// @brief The individual units that result from an encode operation or are used as input to a decode
    /// operation.
    /// @remarks Typically, this type is usually always some kind of byte type (uchar or std::byte or
    /// other ``sizeof(obj) == 1`` type)./
    using code_unit = TByte;

    /// @brief The state that can be used between calls to the decode function.
    /// @remarks Even if the underlying encoding only has a single `state` type, we need to separate the two
    /// out in order to generically handle all encodings. Therefore, the EncodingScheme will always have
    /// both `encode_state` and `decode_state.`
    using decode_state = decode_state_t<TUBaseEncoding>;

    /// @brief The state that can be used between calls to the encode function.
    /// @remarks Even if the underlying encoding only has a single `state` type, we need to separate the two
    /// out in order to generically handle all encodings. Therefore, the EncodingScheme will always have
    /// both `encode_state` and `decode_state.`
    using encode_state = encode_state_t<TUBaseEncoding>;

    /// @brief Whether or not the encode operation can process all forms of input into code point values.
    /// @remarks Defers to what the underlying `encoding_type` does.
    using is_encode_injective = IntegralConstant<bool, IsEncodeInjective<TUBaseEncoding>>;

    /// @brief Whether or not the decode operation can process all forms of input into code point values.
    /// @remarks Defers to what the underlying `encoding_type` does.
    using is_decode_injective = IntegralConstant<bool, IsDecodeInjective<TUBaseEncoding>>;

    /// @brief The maximum number of code points a single complete operation of decoding can produce. This is
    /// 1 for all Unicode Transformation Format (UTF) encodings.
    inline static constexpr const std::size_t MaxCodePoints = MaxCodePoints<TUBaseEncoding>;

    /// @brief The maximum code units a single complete operation of encoding can produce.
    inline static constexpr const std::size_t MaxCodeUnits =
      (MaxCodeUnits<TUBaseEncoding> * sizeof(TBaseCodeUnit)) / (sizeof(TByte));

    /// @brief The id representing the decoded text.
    inline static constexpr const Krys::TextEncodingId DecodedId = decoded_id_v<TUBaseEncoding>;

    /// @brief The id representing the encoded text.
    inline static constexpr const Krys::TextEncodingId EncodedId =
      to_byte_text_encoding_id(encoded_id_v<TUBaseEncoding>, TEndian, sizeof(code_unit_t<TUBaseEncoding>));

    /// @brief Default constructs a EncodingScheme.
    EncodingScheme() = default;

    /// @brief Constructs a EncodingScheme with the encoding object and any additional arguments.
    ///
    /// @param[in] arg0 The first argument used to construct the stored encoding.
    /// @param[in] args Any additional arguments used to construct the stored encoding.
    template <typename TArg0, typename... TArgs,
              enable_if_t<!SameType<remove_cvref_t<TArg0>, EncodingScheme> // cf
                          && !SameType<remove_cvref_t<TArg0>, std::in_place_t>> * = nullptr>
    constexpr EncodingScheme(TArg0 &&arg0,
                             TArgs &&...args) noexcept(NoThrowConstructible<TUBaseEncoding, TArg0, TArgs...>)
        : TBase(std::forward<TArg0>(arg0), std::forward<TArgs>(args)...)
    {
    }

    /// @brief Constructs a EncodingScheme with the encoding object and any additional arguments.
    ///
    /// @param[in] args Any additional arguments used to construct the encoding in the erased storage.
    ///
    /// @remarks If the provided encoding does not have a byte CodeUnit type, it is wrapped in an
    /// EncodingScheme first.
    template <typename... TArgs>
    constexpr EncodingScheme(std::in_place_t,
                             TArgs &&...args) noexcept(NoThrowConstructible<TUBaseEncoding, TArgs...>)
        : TBase(std::forward<TArgs>(args)...)
    {
    }

    /// @brief Cannot copy-construct a EncodingScheme object.
    EncodingScheme(const EncodingScheme &) = default;

    /// @brief Cannot copy-assign a EncodingScheme object.
    EncodingScheme &operator=(const EncodingScheme &) = default;

    /// @brief Move-constructs a EncodingScheme from the provided r-value reference.
    ///
    /// @remarks This leaves the passed-in r-value reference without an encoding object. Calling any function
    /// on a moved-fron EncodingScheme, except for destruction, is a violation and invokes
    /// Undefined Behavior (generally, a crash).
    EncodingScheme(EncodingScheme &&) = default;

    /// @brief Move-assigns a EncodingScheme from the provided r-value reference.
    ///
    /// @remarks This leaves the passed-in r-value reference without an encoding object. Calling any function
    /// on a moved-fron EncodingScheme, except for destruction, is a violation and may invoke
    /// Undefined Behavior (generally, a crash).
    EncodingScheme &operator=(EncodingScheme &&) = default;

    /// @brief Retrives the underlying encoding object.
    ///
    /// @returns An l-value reference to the encoding object.
    constexpr encoding_type &base() & noexcept
    {
      return this->TBase::GetValue();
    }

    /// @brief Retrives the underlying encoding object.
    /// @returns An l-value reference to the encoding object.
    constexpr const encoding_type &base() const & noexcept
    {
      return this->TBase::GetValue();
    }

    /// @brief Retrives the underlying encoding object.
    /// @returns An l-value reference to the encoding object.
    constexpr encoding_type &&base() && noexcept
    {
      return this->TBase::GetValue();
    }

    /// @brief Returns, the desired replacement code units to use.
    ///
    /// @remarks This is only callable if the function call exists on the wrapped encoding. It is broken down
    /// into a contiguous view type formulated from bytes if the wrapped code unit types do not match.
    template <typename TUnused = encoding_type, enable_if_t<HasReplacementCodeUnits<TUnused>> * = nullptr>
    constexpr decltype(auto) ReplacementCodeUnits() const noexcept
    {
      using TOriginalCodeUnit = code_unit_t<encoding_type>;

      decltype(auto) original = this->base().ReplacementCodeUnits();
      if constexpr (SameType<TOriginalCodeUnit, code_unit>)
      {
        return original;
      }
      else
      {
        using TOriginalSpan = Span<const TOriginalCodeUnit>;
        using TTransformedSpan = Span<const code_unit>;
        TOriginalSpan guaranteedCodeUnitView(original);
        // transform into proper type...
        auto transformedPtr = reinterpret_cast<const code_unit *>(guaranteedCodeUnitView.data());
        auto transformedSize =
          (guaranteedCodeUnitView.size() * sizeof(TOriginalCodeUnit)) / sizeof(const code_unit);
        return TTransformedSpan(transformedPtr, transformedSize);
      }
    }

    /// @brief Returns the desired replacement code points to use.
    ///
    /// @remarks Is only callable if the function call exists on the wrapped encoding.
    template <typename TUnused = encoding_type, enable_if_t<HasReplacementCodePoints<TUnused>> * = nullptr>
    constexpr decltype(auto) ReplacementCodePoints() const noexcept
    {
      return this->base().ReplacementCodePoints();
    }

    /// @brief Returns the desired replacement code units to use, or an empty optional-like type if there is
    /// nothing present.
    ///
    /// @remarks This is only callable if the function call exists on the wrapped encoding. It is broken down
    /// into a contiguous view type formulated from bytes if the wrapped code unit types do not match.
    template <typename TUnused = encoding_type,
              enable_if_t<HasMaybeReplacementCodeUnits<TUnused>> * = nullptr>
    constexpr decltype(auto) MaybeReplacementCodeUnits() const noexcept
    {
      using TOriginalCodeUnit = code_unit_t<encoding_type>;

      decltype(auto) maybeOriginal = this->base().MaybeReplacementCodeUnits();
      if constexpr (SameType<TOriginalCodeUnit, code_unit>)
      {
        return maybeOriginal;
      }
      else
      {
        using TOriginalSpan = Span<const TOriginalCodeUnit>;
        using TTransformedSpan = Span<const code_unit>;
        if (!maybeOriginal)
        {
          return std::optional<TTransformedSpan>(std::nullopt);
        }
        decltype(auto) original = *maybeOriginal;
        TOriginalSpan guaranteedCodeUnitView(original);
        // transform into proper type...
        auto transformedPtr = reinterpret_cast<const code_unit *>(guaranteedCodeUnitView.data());
        auto transformedSize =
          (guaranteedCodeUnitView.size() * sizeof(TOriginalCodeUnit)) / sizeof(const code_unit);
        return TTransformedSpan(transformedPtr, transformedSize);
      }
    }

    /// @brief Returns the desired replacement code units to use.
    /// @remarks This Is only callable if the function call exists on the wrapped encoding.
    template <typename TUnused = encoding_type>
    requires(HasMaybeReplacementCodePoints<TUnused>)
    constexpr decltype(auto) MaybeReplacementCodePoints() const noexcept
    {
      return this->base().MaybeReplacementCodePoints();
    }

    /// @brief Whether or not this encoding is some form of Unicode encoding.
    constexpr bool ContainsUnicodeEncoding() const noexcept
    {
      return Krys::ContainsUnicodeEncoding(this->base());
    }

    /// @brief Skips any consecutive input errors in the encoded input, where possible.
    /// @remarks This Is only callable if the function call exists on the wrapped encoding.
    template <typename TResult, typename TInputProgress, typename TOutputProgress>
    requires(
      HasSkipInputError<const encoding_type &, TResult, const TInputProgress &, const TOutputProgress &>)
    constexpr decltype(auto) SkipInputError(TResult &&result) const noexcept(
      NoThrowSkipInputError<const encoding_type &, TResult, const TInputProgress &, const TOutputProgress &>)
    {
      return Krys::SkipInputError(this->base(), std::forward<TResult>(result));
    }

    /// @brief Decodes a single complete unit of information as code points and produces a result with the
    /// input and output ranges moved past what was successfully read and written; or, produces an error and
    /// returns the input and output ranges untouched.
    /// @param[in] input The input view to read code uunits from.
    /// @param[in] output The output view to write code points into.
    /// @param[in] errorHandler The error handler to invoke if encoding fails.
    /// @param[in, out] s The necessary state information. For this encoding, the state is empty and means
    /// very little.
    /// @returns A DecodeResult object that contains the input range, output range, error handler, and
    /// a reference to the passed-in state\.
    /// @remarks To the best ability of the implementation, the iterators will be returned untouched (e.g.,
    /// the input models at least a view and a forward_range). If it is not possible, returned ranges may be
    /// incremented even if an error occurs due to the semantics of any view that models an input_range.
    template <typename TInput, typename TOutput, typename TErrorHandler>
    constexpr auto DecodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler,
                             decode_state &s) const
    {
      using TUOutputRange = remove_cvref_t<TOutput>;
      using TCVErrorHandler = remove_ref_t<TErrorHandler>;
      using TSubInput = Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
      using TSubOutput = Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
      using TResult = DecodeResult<TSubInput, TSubOutput, decode_state>;

      using TInByteIt = Krys::Ranges::WordIterator<TBaseCodeUnit, TSubInput, TEndian>;
      using TInByteSentinel = Krys::Ranges::WordSentinel;
      Krys::Ranges::subrange<TInByteIt, TInByteSentinel> inBytes(
        TInByteIt(std::in_place, std::forward<TInput>(input)), TInByteSentinel());
      Impl::SchemeHandler<TByte, TSubInput, TUOutputRange, TCVErrorHandler> intermediateHandler(errorHandler);
      auto result =
        this->base().DecodeOne(std::move(inBytes), std::forward<TOutput>(output), intermediateHandler, s);
      return TResult(Krys::UnwrapIterator(std::move(result.Input).begin()).range(), std::move(result.Output),
                     s, result.ErrorCode, result.ErrorCount);
    }

    /// @brief Encodes a single complete unit of information as code units and produces a result with the
    /// input and output ranges moved past what was successfully read and written; or, produces an error and
    /// returns the input and output ranges untouched.
    /// @param[in] input The input view to read code points from.
    /// @param[in] output The output view to write code units into.
    /// @param[in] errorHandler The error handler to invoke if encoding fails.
    /// @param[in, out] s The necessary state information. For this encoding, the state is empty and means
    /// very little.
    /// @returns A EncodeResult object that contains the input range, output range, error handler, and
    /// a reference to the passed-in state\.
    /// @remarks To the best ability of the implementation, the iterators will be returned untouched (e.g.,
    /// the input models at least a view and a forward_range). If it is not possible, returned ranges may be
    /// incremented even if an error occurs due to the semantics of any view that models an input_range.
    template <typename TInput, typename TOutput, typename TErrorHandler>
    constexpr auto EncodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler,
                             encode_state &s) const
    {
      using TSubInput = Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
      using TSubOutput = Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
      using TOutByteIt = Krys::Ranges::WordIterator<TBaseCodeUnit, TSubOutput, TEndian>;
      using TOutByteSen = Krys::Ranges::WordSentinel;
      using TCVErrorHandler = remove_ref_t<TErrorHandler>;
      using TResult = EncodeResult<TSubInput, TSubOutput, encode_state>;

      TSubOutput outwordsOutput(std::forward<TOutput>(output));
      Krys::Ranges::subrange<TOutByteIt, TOutByteSen> outwords(TOutByteIt(std::move(outwordsOutput)),
                                                               TOutByteSen());
      Impl::SchemeHandler<TByte, TSubInput, TSubOutput, TCVErrorHandler> intermediateHandler(errorHandler);
      auto result = this->base().EncodeOne(std::forward<TInput>(input), outwords, intermediateHandler, s);
      TSubOutput resultOutput(Krys::UnwrapIterator(std::move(result.Output).begin()).range());
      return TResult(std::move(result.Input), std::move(resultOutput), s, result.ErrorCode,
                     result.ErrorCount);
    }
  };
}
