#pragma once

#include "Krystal.Lib/Core/Endian.hpp"
#include "Krystal.Lib/Ranges/WordIterator.hpp"
#include "Krystal.Lib/Utils/Unwrap.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/_detail/ConstantEncodingTraits.hpp"
#include "Krystal.Text/Handlers/SchemeHandler.hpp"
#include "Krystal.Text/SkipInputError.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/EncodingId.hpp"
#include <cstddef>
#include <optional>

namespace Krys::Text
{
  /// @brief Decomposes the provided Encoding type into a specific endianness (big, little, or native) to
  /// allow for a single encoding type to be viewed in different ways.
  template <typename TEncoding, Endian::Type TEndian = Endian::System, typename TByte = std::byte>
  class EncodingScheme
      : public ::Krys::Text::detail::IsUnicodeEncodingES<EncodingScheme<TEncoding, TEndian, TByte>,
                                                         unwrap_remove_cvref_t<TEncoding>>,
        private EBCO<TEncoding>
  {
  private:
    using TBase = EBCO<TEncoding>;
    using TBaseEncoding = unwrap_remove_cvref_t<TEncoding>;
    using TBaseCodeUnit = code_unit_t<TBaseEncoding>;

  public:
    using encoding_type = TEncoding;

    using code_point = code_point_t<TBaseEncoding>;

    using code_unit = TByte;

    using decode_state = decode_state_t<TBaseEncoding>;

    using encode_state = encode_state_t<TBaseEncoding>;

    using is_encode_injective = IntegralConstant<bool, IsEncodeInjective<TBaseEncoding>>;

    using is_decode_injective = IntegralConstant<bool, IsDecodeInjective<TBaseEncoding>>;

    constexpr inline static const std::size_t MaxCodePoints = ::Krys::Text::MaxCodePoints<TBaseEncoding>;

    constexpr inline static const std::size_t MaxCodeUnits =
      (::Krys::Text::MaxCodeUnits<TBaseEncoding> * sizeof(TBaseCodeUnit)) / (sizeof(TByte));

    constexpr inline static const ::Krys::Text::EncodingId DecodedId =
      ::Krys::Text::DecodeId<TBaseEncoding>;

    constexpr inline static const ::Krys::Text::EncodingId EncodedId = ::Krys::Text::ToByteEncoding(
      ::Krys::Text::EncodeId<TBaseEncoding>, TEndian, sizeof(code_unit_t<TBaseEncoding>));

    EncodingScheme() = default;
    EncodingScheme(const EncodingScheme &) = default;
    EncodingScheme &operator=(const EncodingScheme &) = default;
    EncodingScheme(EncodingScheme &&) = default;
    EncodingScheme &operator=(EncodingScheme &&) = default;

    template <typename TArg0, typename... TArgs>
    requires(!SameType<remove_cvref_t<TArg0>, EncodingScheme>
             && !SameType<remove_cvref_t<TArg0>, std::in_place_t>)
    constexpr EncodingScheme(TArg0 &&arg0, TArgs &&...args) // cf
      noexcept(NoThrowConstructible<TBaseEncoding, TArg0, TArgs...>)
        : TBase(std::forward<TArg0>(arg0), std::forward<TArgs>(args)...)
    {
    }

    template <typename... TArgs>
    constexpr EncodingScheme(std::in_place_t, TArgs &&...args) // cf
      noexcept(NoThrowConstructible<TBaseEncoding, TArgs...>)
        : TBase(std::forward<TArgs>(args)...)
    {
    }

    constexpr encoding_type &Base() & noexcept
    {
      return this->TBase::GetValue();
    }

    constexpr const encoding_type &Base() const & noexcept
    {
      return this->TBase::GetValue();
    }

    constexpr encoding_type &&Base() && noexcept
    {
      return this->TBase::GetValue();
    }

    template <typename TUnused = encoding_type, enable_if_t<HasReplacementCodeUnits<TUnused>> * = nullptr>
    constexpr decltype(auto) ReplacementCodeUnits() const noexcept
    {
      using TOriginalCodeUnit = code_unit_t<encoding_type>;

      decltype(auto) original = this->Base().ReplacementCodeUnits();
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

    template <typename TUnused = encoding_type, enable_if_t<HasReplacementCodePoints<TUnused>> * = nullptr>
    constexpr decltype(auto) ReplacementCodePoints() const noexcept
    {
      return this->Base().ReplacementCodePoints();
    }
    template <typename TUnused = encoding_type,
              enable_if_t<HasMaybeReplacementCodeUnits<TUnused>> * = nullptr>
    constexpr decltype(auto) MaybeReplacementCodeUnits() const noexcept
    {
      using TOriginalCodeUnit = code_unit_t<encoding_type>;

      decltype(auto) maybeOriginal = this->Base().MaybeReplacementCodeUnits();
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

    template <typename TUnused = encoding_type>
    requires(HasMaybeReplacementCodePoints<TUnused>)
    constexpr decltype(auto) MaybeReplacementCodePoints() const noexcept
    {
      return this->Base().MaybeReplacementCodePoints();
    }

    template <typename TResult, typename TInputProgress, typename TOutputProgress>
    requires(
      HasSkipInputError<const encoding_type &, TResult, const TInputProgress &, const TOutputProgress &>)
    constexpr decltype(auto) SkipInputError(TResult &&result) const noexcept(
      NoThrowSkipInputError<const encoding_type &, TResult, const TInputProgress &, const TOutputProgress &>)
    {
      return ::Krys::Text::SkipInputError(this->Base(), std::forward<TResult>(result));
    }

    template <typename TInput, typename TOutput, typename TErrorHandler>
    constexpr auto DecodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler,
                             decode_state &s) const
    {
      using TUOutputRange = remove_cvref_t<TOutput>;
      using TCVErrorHandler = remove_ref_t<TErrorHandler>;
      using TSubInput = ::Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
      using TSubOutput = ::Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
      using TResult = ::Krys::Text::DecodeResult<TSubInput, TSubOutput, decode_state>;

      using TInByteIt = ::Krys::Ranges::WordIterator<TBaseCodeUnit, TSubInput, TEndian>;
      using TInByteSentinel = ::Krys::Ranges::WordSentinel;
      ::Krys::Ranges::subrange<TInByteIt, TInByteSentinel> inBytes(
        TInByteIt(std::in_place, std::forward<TInput>(input)), TInByteSentinel());

      ::Krys::Text::Handlers::SchemeHandler<TByte, TSubInput, TUOutputRange, TCVErrorHandler>
        intermediateHandler(errorHandler);
      auto result =
        this->Base().DecodeOne(std::move(inBytes), std::forward<TOutput>(output), intermediateHandler, s);
      return TResult(::Krys::UnwrapIterator(std::move(result.Input).begin()).range(),
                     std::move(result.Output), s, result.ErrorCode, result.ErrorCount);
    }

    template <typename TInput, typename TOutput, typename TErrorHandler>
    constexpr auto EncodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler,
                             encode_state &s) const
    {
      using TSubInput = ::Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
      using TSubOutput = ::Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
      using TOutByteIt = ::Krys::Ranges::WordIterator<TBaseCodeUnit, TSubOutput, TEndian>;
      using TOutByteSen = ::Krys::Ranges::WordSentinel;
      using TResult = ::Krys::Text::EncodeResult<TSubInput, TSubOutput, encode_state>;

      TSubOutput outwordsOutput(std::forward<TOutput>(output));
      ::Krys::Ranges::subrange<TOutByteIt, TOutByteSen> outwords(TOutByteIt(std::move(outwordsOutput)),
                                                                 TOutByteSen());
      ::Krys::Text::Handlers::SchemeHandler<TByte, TSubInput, TSubOutput, remove_ref_t<TErrorHandler>>
        intermediateHandler(errorHandler);
      auto result = this->Base().EncodeOne(std::forward<TInput>(input), outwords, intermediateHandler, s);
      TSubOutput resultOutput(::Krys::UnwrapIterator(std::move(result.Output).begin()).range());
      return TResult(std::move(result.Input), std::move(resultOutput), s, result.ErrorCode,
                     result.ErrorCount);
    }
  };
}
