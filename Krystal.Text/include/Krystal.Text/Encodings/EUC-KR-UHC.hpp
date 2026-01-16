#pragma once

#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Concepts.hpp"
#include "Krystal.Text/Decode/DecodeResult.hpp"
#include "Krystal.Text/Encode/EncodeResult.hpp"
#include "Krystal.Text/Encodings/EncodingTables/EUC-KR-UHC.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/Unicode.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The EUC-KR (Unified Hangul Code) encoding.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_euc_kr_uhc
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = {"EUC-KR"_s};
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 10> Aliases = {
      "cseuckr"_s,        "csksc56011987"_s,  "euc-kr"_s,  "iso-ir-149"_s, "korean"_s,
      "ks_c_5601-1987"_s, "ks_c_5601-1989"_s, "ksc5601"_s, "ksc_5601"_s,   "windows-949"_s};

    using code_unit = TCodeUnit;

    using code_point = TCodePoint;

    using state = EmptyState;

    constexpr static inline std::size_t MaxCodePoints = 1;

    constexpr static inline std::size_t MaxCodeUnits = 2;

    constexpr Span<const code_unit, 1> ReplacementCodeUnits() const noexcept
    {
      return Span<const code_unit, 1>(::Krys::Text::Unicode::ASCIIReplacement<code_unit>.data(),
                                      ::Krys::Text::Unicode::ASCIIReplacement<code_unit>.size());
    }

    using is_decode_injective = std::true_type;

    template <typename TInput, typename TOutput, typename TErrorHandler>
    constexpr static auto DecodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler,
                                    state &s) noexcept
    {
      using TSubInput = ::Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
      using TSubOutput = ::Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
      using TResult = ::Krys::Text::DecodeResult<TSubInput, TSubOutput, state>;

      auto inIt = ::Krys::Ranges::cbegin(input);
      auto inLast = ::Krys::Ranges::cend(input);

      if (inIt == inLast)
      {
        // we don't need more, so we can just say the emptiness is a-okay.
        return TResult(std::move(input), std::move(output), s, EncodingError::OK);
      }

      code_unit units[MaxCodeUnits] = {static_cast<code_unit>(*inIt)};
      uchar unit0 = static_cast<uchar>(units[0]);
      auto outIt = ::Krys::Ranges::begin(output);
      auto outLast = ::Krys::Ranges::end(output);

      constexpr bool CallErrorHandler = !IsIgnorableErrorHandler<TErrorHandler>;
      if (unit0 <= 0x7F)
      {
        // Top-Level case 0: it's an ASCII byte
        const code_point codePoint = static_cast<code_point>(unit0);
        if constexpr (CallErrorHandler)
        {
          if (outIt == outLast)
          {
            basic_euc_kr_uhc self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self,
              TResult(TSubInput(std::move(inIt), std::move(inLast)),
                      TSubOutput(std::move(outIt), std::move(outLast)), s,
                      EncodingError::InsufficientOutputSpace),
              Span<const code_unit, 0>(), Span<const code_point, 0>());
          }
        }
        *outIt = codePoint;
        ++inIt;
        ++outIt;
        return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                       TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
      }
      else if ((unit0 <= 0xFE && unit0 >= 0x81))
      {
        // Top-Level case 1: this is a double-byte sequence!
        if constexpr (CallErrorHandler)
        {
          if (inIt == inLast)
          {
            basic_euc_kr_uhc self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self, TResult(std::move(input), std::move(output), s, EncodingError::IncompleteSequence),
              Span<const code_unit, 1>(std::addressof(units[0]), 1), Span<const code_point, 0>());
          }
        }
        ++inIt;
        units[1] = static_cast<code_unit>(*inIt);
        uchar secondByte = static_cast<uchar>(units[1]);
        if (secondByte <= 0xFE && secondByte >= 0x41)
        {
          const std::size_t lookupIndex = ((unit0 - 0x81) * 190) + (secondByte - 0x41);
          const std::optional<std::uint32_t> maybeCode =
            ::Krys::Text::EncodingTable::euc_kr_uhc_index_to_code_point(lookupIndex);

          if (maybeCode)
          {
            if constexpr (CallErrorHandler)
            {
              if (outIt == outLast)
              {
                basic_euc_kr_uhc self {};
                return std::forward<TErrorHandler>(errorHandler)(
                  self,
                  TResult(TSubInput(std::move(inIt), std::move(inLast)),
                          TSubOutput(std::move(outIt), std::move(outLast)), s,
                          EncodingError::InsufficientOutputSpace),
                  Span<const code_unit, 1>(std::addressof(units[0]), 1), Span<const code_point, 0>());
              }
            }
            const char32 &codePoint = static_cast<code_point>(*maybeCode);
            *outIt = codePoint;
            ++inIt;
            ++outIt;
            return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                           TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
          }
        }
      }

      // Top-Level case 2 (default): unrecognized byte sequence!!
      basic_euc_kr_uhc self {};
      return std::forward<TErrorHandler>(errorHandler)(
        self,
        TResult(TSubInput(std::move(inIt), std::move(inLast)),
                TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
        Span<const code_unit, 1>(std::addressof(units[0]), 1), Span<const code_point, 0>());
    }

    template <typename TInput, typename TOutput, typename TErrorHandler>
    constexpr static auto EncodeOne(TInput &&input, TOutput &&output, TErrorHandler &&errorHandler,
                                    state &s) noexcept
    {
      using TSubInput = ::Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
      using TSubOutput = ::Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
      using TResult = ::Krys::Text::EncodeResult<TSubInput, TSubOutput, state>;

      auto inIt = ::Krys::Ranges::cbegin(input);
      auto inLast = ::Krys::Ranges::cend(input);

      if (inIt == inLast)
      {
        // we don't need more, so we can just say the emptiness is a-okay.
        return TResult(std::move(input), std::move(output), s, EncodingError::OK);
      }

      char32_t codePoint32 = static_cast<char32_t>(*inIt);
      code_point codePoint = static_cast<code_point>(codePoint32);
      auto outIt = ::Krys::Ranges::begin(output);
      auto outLast = ::Krys::Ranges::end(output);

      constexpr bool CallErrorHandler = !IsIgnorableErrorHandler<TErrorHandler>;
      if (codePoint32 <= 0x80)
      {
        if constexpr (CallErrorHandler)
        {
          if (outIt == outLast)
          {
            // output is empty :(
            basic_euc_kr_uhc self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self, TResult(std::move(input), std::move(output), s, EncodingError::InsufficientOutputSpace),
              Span<const code_point, 0>(), Span<const code_unit, 0>());
          }
        }
        ++inIt;
        *outIt = static_cast<code_unit>(codePoint);
        ++outIt;
        return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                       TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
      }

      std::optional<std::size_t> maybeIndex =
        ::Krys::Text::EncodingTable::euc_kr_uhc_code_point_to_index(codePoint32);
      if (maybeIndex)
      {
        const std::size_t i = *maybeIndex;
        std::size_t first = (i / 190) + 0x81;
        std::size_t second = (i % 190) + 0x41;

        if constexpr (CallErrorHandler)
        {
          if (outIt == outLast)
          {
            // output is empty :(
            basic_euc_kr_uhc self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self,
              TResult(TSubInput(std::move(inIt), std::move(inLast)),
                      TSubOutput(std::move(outIt), std::move(outLast)), s,
                      EncodingError::InsufficientOutputSpace),
              Span<const code_point, 0>(), Span<const code_unit, 0>());
          }
        }
        const code_unit unit = static_cast<code_unit>(first);
        *outIt = unit;
        if constexpr (CallErrorHandler)
        {
          if (outIt == outLast)
          {
            // output is empty :(
            basic_euc_kr_uhc self {};
            return std::forward<TErrorHandler>(errorHandler)(
              self,
              TResult(TSubInput(std::move(inIt), std::move(inLast)),
                      TSubOutput(std::move(outIt), std::move(outLast)), s,
                      EncodingError::InsufficientOutputSpace),
              Span<const code_point, 0>(), Span<const code_unit, 1>(std::addressof(unit), 1));
          }
        }
        ++outIt;
        ++inIt;
        *outIt = static_cast<code_unit>(second);
        ++outIt;
        return TResult(TSubInput(std::move(inIt), std::move(inLast)),
                       TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::OK);
      }

      basic_euc_kr_uhc self {};
      return std::forward<TErrorHandler>(errorHandler)(
        self,
        TResult(TSubInput(std::move(inIt), std::move(inLast)),
                TSubOutput(std::move(outIt), std::move(outLast)), s, EncodingError::InvalidSequence),
        Span<const code_point, 0>(), Span<const code_unit, 0>());
    }
  };

  /// @brief The EUC-KR (Unified Hangul Code) encoding.
  constexpr inline basic_euc_kr_uhc<char> euc_kr_uhc = {};
}
