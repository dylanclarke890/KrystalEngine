#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Lib/Ranges/Subrange.hpp"
#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Concepts.hpp"
#include "Krystal.Text/Decode/DecodeResult.hpp"
#include "Krystal.Text/Encode/EncodeResult.hpp"
#include "Krystal.Text/EncodingError.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/Unicode.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The American Standard Code for Information Exchange (ASCII) Encoding.
  template <typename TCodeUnit, typename TCodePoint = UnicodeCodePoint>
  class basic_ascii
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = {"US-ASCII"_s};

    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 12> Aliases = {
      "ASCII"_s, "US-ASCII"_s, "ANSI_X3.4-1968"_s, "ANSI_X3.4-1986"_s, "ISO_646.irv:1991"_s, "ISO646-US"_s,
      "us"_s,    "iso-ir-6"_s, "IBM367"_s,         "cp367"_s,          "csASCII"_s,          "ascii"_s,
    };

    using code_unit = TCodeUnit;

    using code_point = TCodePoint;

    using state = EmptyState;

    using is_decode_injective = std::true_type;

    using is_encode_injective = std::false_type;

    constexpr static inline const std::size_t MaxCodeUnits = 1;

    constexpr static inline const std::size_t MaxCodePoints = 1;

    constexpr static Span<const code_unit, 1> ReplacementCodeUnits() noexcept
    {
      constexpr auto replacement = ::Krys::Text::Unicode::ASCIIReplacement<code_unit>;
      return Span<const code_unit, 1>(std::addressof(replacement), 1);
    }

    template <typename TInput, typename TOutput, typename TErrorHandler>
    constexpr static auto DecodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler, state &s)
    {
      using TSubInput = ::Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
      using TSubOutput = ::Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
      using TResult = ::Krys::Text::DecodeResult<TSubInput, TSubOutput, state>;

      auto inIt = ::Krys::Ranges::cbegin(input);
      auto inLast = ::Krys::Ranges::cend(input);
      if (inIt == inLast)
      {
        // an exhausted sequence is fine
        return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                       TSubOutput(std::forward<TOutput>(output)), s, EncodingError::OK);
      }

      auto outIt = ::Krys::Ranges::begin(output);
      KRYS_MAYBE_UNUSED auto outLast = ::Krys::Ranges::end(output);

      constexpr bool CallErrorHandler = !IsIgnorableErrorHandler<TErrorHandler>;
      if constexpr (CallErrorHandler)
      {
        if (outIt == outLast)
        {
          basic_ascii self {};
          return std::forward<TErrorHandler>(errorHandler)(
            self,
            TResult(TSubInput(std::move(inIt), std::move(inLast)),
                    TSubOutput(std::move(outIt), std::move(outLast)), s,
                    EncodingError::InsufficientOutputSpace),
            Span<code_unit>(), Span<code_point>());
        }
      }

      code_unit units[1] {};
      units[0] = *inIt;
      const code_unit &unit = units[0];

      if constexpr (CallErrorHandler)
      {
        if (static_cast<schar>(unit) < static_cast<schar>(0))
        {
          basic_ascii self {};
          return std::forward<TErrorHandler>(errorHandler)(
            self,
            TResult(TSubInput(std::move(inIt), std::move(inLast)),
                    TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
            Span<code_unit>(std::addressof(units[0]), 1), Span<code_point>());
        }
      }

      ::Krys::Ranges::iter_advance(inIt);

      *outIt = static_cast<code_point>(unit);
      ::Krys::Ranges::iter_advance(outIt);

      return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                     TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
    }

    template <typename TInput, typename TOutput, typename TErrorHandler>
    constexpr static auto EncodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler, state &s)
    {
      using TSubInput = ::Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
      using TSubOutput = ::Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
      using TResult = ::Krys::Text::EncodeResult<TSubInput, TSubOutput, state>;

      auto inIt = ::Krys::Ranges::cbegin(input);
      auto inLast = ::Krys::Ranges::cend(input);

      if (inIt == inLast)
      {
        // an exhausted sequence is fine
        return TResult(TSubInput(std::move(inIt), std::move(inLast)), std::forward<TOutput>(output), s,
                       EncodingError::OK);
      }

      auto outIt = ::Krys::Ranges::begin(output);
      KRYS_MAYBE_UNUSED auto outLast = ::Krys::Ranges::end(output);

      constexpr bool CallErrorHandler = !IsIgnorableErrorHandler<TErrorHandler>;
      if constexpr (CallErrorHandler)
      {
        if (outIt == outLast)
        {
          basic_ascii self {};
          return TResult(std::forward<TErrorHandler>(errorHandler)(
            self,
            TResult(TSubInput(std::move(inIt), std::move(inLast)),
                    TSubOutput(std::move(outIt), std::move(outLast)), s,
                    EncodingError::InsufficientOutputSpace),
            Span<code_point>(), Span<code_unit>()));
        }
      }

      code_point points[1] {};
      points[0] = *inIt;
      const code_point &point = points[0];

      if constexpr (CallErrorHandler)
      {
        if (point > ::Krys::Text::Unicode::LastASCIIValue)
        {
          basic_ascii self {};
          return TResult(std::forward<TErrorHandler>(errorHandler)(
            self,
            TResult(TSubInput(std::move(inIt), std::move(inLast)),
                    TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
            Span<code_point>(std::addressof(points[0]), 1), Span<code_unit>()));
        }
      }

      ::Krys::Ranges::iter_advance(inIt);

      *outIt = static_cast<code_unit>(point);
      ::Krys::Ranges::iter_advance(outIt);

      return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                     TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
    }
  };

  using ascii_t = basic_ascii<char>;

  /// @brief The American Standard Code for Information Exchange (ASCII) Encoding.
  constexpr inline ascii_t ascii = {};
}
