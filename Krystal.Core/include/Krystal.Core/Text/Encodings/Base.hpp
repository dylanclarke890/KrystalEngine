#pragma once

#include "Krystal.Core/Base.hpp"
#include "Krystal.Core/Text/Encodings/EncodingId.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/StronglyTypedValue.hpp"
#include <ranges>

namespace krys::text
{
#pragma region Concepts

  /// @brief A range whose elements are convertible to the target encoding's code_unit.
  template <typename T, typename Encoding>
  concept CodeUnitInputRange =
    std::ranges::input_range<T>
    && ConvertibleTo<std::ranges::range_reference_t<T>, typename Encoding::code_unit>;

  /// @brief A range whose elements are convertible to char32.
  template <typename T>
  concept CodePointInputRange =
    std::ranges::input_range<T> && ConvertibleTo<std::ranges::range_reference_t<T>, char32>;

  /// @brief An iterator whose value type is convertible to the target encoding's code_unit.
  template <typename T, typename Encoding>
  concept CodeUnitInputIterator =
    std::input_iterator<T> && ConvertibleTo<std::iter_value_t<T>, typename Encoding::code_unit>;

  /// @brief An iterator whose value type is convertible to char32.
  template <typename T>
  concept CodePointInputIterator = std::input_iterator<T> && ConvertibleTo<std::iter_value_t<T>, char32>;

  /// @brief A type which defines a text encoding.
  template <typename TEncoding>
  concept Encoding = requires {
    /// @brief The unique identifier for the encoding.
    requires SameType<decltype(TEncoding::Id), const EncodingId>;

    /// @brief The canonical name of the encoding.
    requires SameType<decltype(TEncoding::Name), const utf8_stringview>;

    /// @brief Any aliases for the encoding, lowercased. Includes the lowercased canonical name as the first
    /// alias.
    requires ConvertibleTo<decltype(TEncoding::Aliases), Span<const utf8_stringview>>;

    /// @brief This is char for most encodings but uses char8 for UTF-8, char16 for UTF-16, etc.
    requires IsChar<typename TEncoding::code_unit>;

    /// @brief Whether encoding a code point to code units is lossless. For most encodings this is false, but
    /// for encodings such as UTF-8 this is true.
    requires SameType<decltype(TEncoding::IsEncodeLossless), const bool>;

    /// @brief The maximum number of code units that can be produced by encoding a single code point. All
    /// 8-bit encodings such as ASCII and ISO-8859-1 will have a maximum of 1 code unit but others such as
    /// UTF-8 can produce up to 4 code units for a single code point.
    requires SameType<decltype(TEncoding::MaxEncodeCodeUnits), const size_t>;

    /// @brief The maximum number of code points that can be produced by decoding a single code unit sequence.
    /// Most encodings will produce a single code point but others such as Big5-HKSCS can produce 2 code
    /// points for a single code unit sequence (in rare cases).
    requires SameType<decltype(TEncoding::MaxDecodeCodePoints), const size_t>;

    /// @brief The replacement code units for the encoding. For example, ASCII and ISO-8859-1 will have a
    /// replacement of 1 code unit (the ASCII question mark), while UTF-8 will have a replacement of 3 code
    /// units (U+FFFD encoded as UTF-8).
    requires ConvertibleTo<decltype(TEncoding::ReplacementCodeUnits),
                           Span<const typename TEncoding::code_unit>>;
  };

  template <typename TEncoding>
  concept HasByteOrderMark = Encoding<TEncoding> && requires {
    { TEncoding::BOM } -> ConvertibleTo<Span<const typename TEncoding::code_unit>>;
  };

  /// @brief Common traits for most encodings.
  struct DefaultEncodingTraits
  {
    using code_unit = char;

    constexpr static bool IsEncodeLossless = false;

    constexpr static size_t MaxEncodeCodeUnits = 1uz;

    constexpr static size_t MaxDecodeCodePoints = 1uz;

    constexpr static Array<code_unit, 1uz> ReplacementCodeUnits = {ASCIIReplacementCodeUnit<code_unit>};
  };

#pragma endregion

#pragma region WriteToOutput

  template <typename TOutput, IsChar TChar>
  constexpr void WriteToOutput(TOutput &output, TChar value) noexcept
  {
    output.push_back(value);
  }

  template <typename TOutput, IsChar TChar>
  constexpr void WriteToOutput(TOutput &output, Span<const TChar> values) noexcept
  {
    if constexpr (requires { output.append_range(values); })
    {
      output.append_range(values);
    }
    else if constexpr (requires { output.append(values.data(), values.size()); })
    {
      output.append(values.data(), values.size());
    }
    else
    {
      for (auto v : values)
      {
        output.push_back(v);
      }
    }
  }

#pragma endregion

  template <IsChar TChar, size_t Size>
  using char_or_char_array_t = conditional_t<Size == 1uz, TChar, Array<TChar, Size>>;

  struct CodeUnitsConsumed : public StronglyTypedNumber<CodeUnitsConsumed, uint8>
  {
    using Base = StronglyTypedNumber<CodeUnitsConsumed, uint8>;
    using Base::Base;
  };

  struct CodeUnitsProduced : public StronglyTypedNumber<CodeUnitsProduced, uint8>
  {
    using Base = StronglyTypedNumber<CodeUnitsProduced, uint8>;
    using Base::Base;
  };

  struct MightBeInvalidEntry : public StronglyTypedBool<MightBeInvalidEntry>
  {
    using Base::Base;
  };

  struct CheckForBOM : public StronglyTypedBool<CheckForBOM>
  {
    using Base::Base;
  };

  struct AppendBOM : public StronglyTypedBool<AppendBOM>
  {
    using Base::Base;
  };

  /// @brief A lookup entry for an encoding, mapping a code unit to a Unicode code point.
  struct EncodingLookupEntry
  {
    uint32 Index;
    uint32 CodePoint;
  };

  /// @brief An invalid lookup entry for an encoding, used to pad lookup tables so that indexing can be used
  /// instead of a linear/lower bounded scan.
  KRYS_NODISCARD constexpr EncodingLookupEntry InvalidEntry(uint8 fillsIndex) noexcept
  {
    return {fillsIndex, MaxUnicodeChar + 1u};
  }

  /// @brief The action to take for a particular encoding error when it occurs during an operation.
  enum class EncodingErrorAction : uint8
  {
    /// @brief Replaces the invalid code unit / code point with the appropriate replacement code unit
    /// sequence / code point and continues processing. Consecutive invalid code units / code points each be
    /// replaced with the replacement code unit sequence / code point, instead of replacing the entire
    /// sequence with a single replacement code unit sequence / code point.
    Replace,

    /// @brief Skips the invalid code unit / code point and continues processing.
    Skip,

    /// @brief Stops processing and returns an error.
    Stop,
  };

#pragma region DecodeOne

  /// @brief The error that occurred during a DecodeOne operation, or DecodeOneError::None if the operation
  /// was successful.
  enum class DecodeOneError : uint8
  {
    /// @brief No error occurred.
    None = 0,

    /// @brief Not enough code units were provided to complete a valid sequence.
    IncompleteSequence,

    /// @brief Invalid code unit sequence for the provided encoding.
    InvalidSequence,
  };

  /// @brief The result of a single decode operation.
  template <typename TEncoding>
  struct DecodeOneResult
  {
    /// @brief The decoded output, or U+FFFD if the input code units were invalid. `CodePointsProduced`
    /// indicates how many code
    char_or_char_array_t<char32, TEncoding::MaxDecodeCodePoints> Output;

    /// @brief The number of code units consumed from the input.
    uint8 CodeUnitsConsumed;

    /// @brief The number of code points produced from the input.
    uint8 CodePointsProduced;

    /// @brief The error that occurred, or DecodeOneError::None if the operation was successful.
    DecodeOneError Error;
  };

  template <typename TEncoding>
  KRYS_NODISCARD constexpr DecodeOneResult<TEncoding>
    DecodeOneInvalidSequenceError(CodeUnitsConsumed codeUnitsConsumed = CodeUnitsConsumed(1u)) noexcept
  {
    if constexpr (TEncoding::MaxDecodeCodePoints == 1uz)
    {
      return {.Output = UnicodeReplacementChar,
              .CodeUnitsConsumed = codeUnitsConsumed,
              .CodePointsProduced = 1u,
              .Error = DecodeOneError::InvalidSequence};
    }
    else
    {
      return {.Output = {{UnicodeReplacementChar}},
              .CodeUnitsConsumed = codeUnitsConsumed,
              .CodePointsProduced = 1u,
              .Error = DecodeOneError::InvalidSequence};
    }
  }

  template <typename TEncoding>
  KRYS_NODISCARD constexpr DecodeOneResult<TEncoding>
    DecodeOneIncompleteSequenceError(CodeUnitsConsumed codeUnitsConsumed = CodeUnitsConsumed(1u)) noexcept
  {
    if constexpr (TEncoding::MaxDecodeCodePoints == 1uz)
    {
      return {.Output = UnicodeReplacementChar,
              .CodeUnitsConsumed = codeUnitsConsumed,
              .CodePointsProduced = 1u,
              .Error = DecodeOneError::IncompleteSequence};
    }
    else
    {
      return {.Output = {{UnicodeReplacementChar}},
              .CodeUnitsConsumed = codeUnitsConsumed,
              .CodePointsProduced = 1u,
              .Error = DecodeOneError::IncompleteSequence};
    }
  }

  template <typename TEncoding>
  KRYS_NODISCARD constexpr DecodeOneResult<TEncoding>
    DecodeOneSuccess(char32 codePoint, CodeUnitsConsumed codeUnitsConsumed = CodeUnitsConsumed(1u)) noexcept
  {
    if constexpr (TEncoding::MaxDecodeCodePoints == 1uz)
    {
      return {.Output = codePoint,
              .CodeUnitsConsumed = codeUnitsConsumed,
              .CodePointsProduced = 1u,
              .Error = DecodeOneError::None};
    }
    else
    {
      return {.Output = {{codePoint}},
              .CodeUnitsConsumed = codeUnitsConsumed,
              .CodePointsProduced = 1u,
              .Error = DecodeOneError::None};
    }
  }

  template <typename TEncoding>
  KRYS_NODISCARD constexpr DecodeOneResult<TEncoding>
    DecodeOneSuccess(char32 first, char32 second,
                     CodeUnitsConsumed codeUnitsConsumed = CodeUnitsConsumed(1u)) noexcept
  {
    return {.Output = {{first, second}},
            .CodeUnitsConsumed = codeUnitsConsumed,
            .CodePointsProduced = 2u,
            .Error = DecodeOneError::None};
  }

  template <Encoding TEncoding>
  KRYS_NODISCARD constexpr DecodeOneResult<TEncoding> DecodeOneWithLookupMap(char32 codeUnit) noexcept
  {
    if (codeUnit >= TEncoding::LookupMap.size())
    {
      return DecodeOneInvalidSequenceError<TEncoding>();
    }

    auto index = static_cast<size_t>(codeUnit);
    krys_debug_assert(!IsNonUnicodeCharacter(TEncoding::LookupMap[index].CodePoint));
    return DecodeOneSuccess<TEncoding>(TEncoding::LookupMap[index].CodePoint);
  }

  template <Encoding TEncoding, MightBeInvalidEntry MightBeInvalid = MightBeInvalidEntry(false)>
  KRYS_NODISCARD constexpr DecodeOneResult<TEncoding>
    DecodeOneWithExtendedASCIILookupMap(char32 codeUnit) noexcept
  {
    if (codeUnit <= MaxASCIIChar)
    {
      return DecodeOneSuccess<TEncoding>(codeUnit);
    }

    if (codeUnit > MaxUnicodeChar)
    {
      return DecodeOneInvalidSequenceError<TEncoding>();
    }

    auto index = static_cast<uint8>(codeUnit - 0x80u);
    if constexpr (!MightBeInvalid)
    {
      krys_debug_assert(!IsNonUnicodeCharacter(TEncoding::LookupMap[index].CodePoint));
      return DecodeOneSuccess<TEncoding>(TEncoding::LookupMap[index].CodePoint);
    }

    auto entry = TEncoding::LookupMap[index];
    if (entry == InvalidEntry(index))
    {
      return DecodeOneInvalidSequenceError<TEncoding>();
    }

    krys_debug_assert(!IsNonUnicodeCharacter(entry.CodePoint));
    return DecodeOneSuccess<TEncoding>(entry.CodePoint);
  }

#pragma endregion

#pragma region EncodeOne

  /// @brief The error that occurred during an EncodeOne operation, or EncodeOneError::None if the operation
  /// was successful.
  enum class EncodeOneError : uint8
  {
    /// @brief No error occurred.
    None = 0,

    /// @brief Valid Unicode code point but not representable in the target encoding.
    UnmappableCodePoint,
  };

  /// @brief The result of a single encode operation.
  template <typename TEncoding>
  struct EncodeOneResult
  {
    /// @brief The encoded code unit, or the replacement code units if the input code point was invalid or
    /// unmappable. For most encodings this will only store a single code unit, but for encodings such as
    /// UTF-8 and UTF-16 this may store multiple code units. `CodeUnitsProduced` indicates how many of the
    /// code units in this array out of the maximum were actually produced.
    char_or_char_array_t<typename TEncoding::code_unit, TEncoding::MaxEncodeCodeUnits> Output;

    /// @brief The number of code units produced in the output.
    uint8 CodeUnitsProduced;

    /// @brief The error that occurred, or EncodeError::None if the operation was successful.
    EncodeOneError Error;
  };

  template <typename TEncoding>
  KRYS_NODISCARD constexpr EncodeOneResult<TEncoding> EncodeOneUnmappableCodePointError() noexcept
  {
    if constexpr (TEncoding::MaxEncodeCodeUnits == 1uz)
    {
      return {.Output = TEncoding::ReplacementCodeUnits[0],
              .CodeUnitsProduced = 1u,
              .Error = EncodeOneError::UnmappableCodePoint};
    }
    else
    {
      EncodeOneResult<TEncoding> result {.Output = {},
                                         .CodeUnitsProduced =
                                           static_cast<uint8>(TEncoding::ReplacementCodeUnits.size()),
                                         .Error = EncodeOneError::UnmappableCodePoint};

      std::ranges::copy(TEncoding::ReplacementCodeUnits, std::ranges::begin(result.Output));

      return result;
    }
  }

  template <typename TEncoding>
  KRYS_NODISCARD constexpr EncodeOneResult<TEncoding>
    EncodeOneSuccess(typename TEncoding::code_unit codeUnit) noexcept
  {
    if constexpr (TEncoding::MaxEncodeCodeUnits == 1uz)
    {
      return {.Output = codeUnit, .CodeUnitsProduced = 1u, .Error = EncodeOneError::None};
    }
    else
    {
      return {.Output = {{codeUnit}}, .CodeUnitsProduced = 1u, .Error = EncodeOneError::None};
    }
  }

  template <typename TEncoding>
  KRYS_NODISCARD constexpr EncodeOneResult<TEncoding>
    EncodeOneSuccess(typename TEncoding::code_unit first, typename TEncoding::code_unit second) noexcept
  {
    return {.Output = {{first, second}}, .CodeUnitsProduced = 2u, .Error = EncodeOneError::None};
  }

  template <typename TEncoding>
  KRYS_NODISCARD constexpr EncodeOneResult<TEncoding>
    EncodeOneSuccess(typename TEncoding::code_unit first, typename TEncoding::code_unit second,
                     typename TEncoding::code_unit third) noexcept
  {
    return {.Output = {{first, second, third}}, .CodeUnitsProduced = 3u, .Error = EncodeOneError::None};
  }

  template <typename TEncoding>
  KRYS_NODISCARD constexpr EncodeOneResult<TEncoding>
    EncodeOneSuccess(typename TEncoding::code_unit first, typename TEncoding::code_unit second,
                     typename TEncoding::code_unit third, typename TEncoding::code_unit fourth) noexcept
  {
    return {
      .Output = {{first, second, third, fourth}}, .CodeUnitsProduced = 4u, .Error = EncodeOneError::None};
  }

  template <Encoding TEncoding>
  KRYS_NODISCARD constexpr EncodeOneResult<TEncoding> EncodeOneWithLookupMap(char32 codePoint) noexcept
  {
    auto it = std::ranges::find_if(TEncoding::LookupMap,
                                   [codePoint](auto &value) { return codePoint == value.CodePoint; });
    if (it == std::ranges::cend(TEncoding::LookupMap))
    {
      return EncodeOneUnmappableCodePointError<TEncoding>();
    }

    return EncodeOneSuccess<TEncoding>(static_cast<typename TEncoding::code_unit>((*it).Index));
  }

  template <Encoding TEncoding>
  KRYS_NODISCARD constexpr EncodeOneResult<TEncoding>
    EncodeOneWithExtendedASCIILookupMap(char32 codePoint) noexcept
  {
    if (codePoint <= MaxASCIIChar)
    {
      return EncodeOneSuccess<TEncoding>(static_cast<typename TEncoding::code_unit>(codePoint));
    }

    auto it = std::ranges::find_if(TEncoding::LookupMap,
                                   [codePoint](auto &value) { return codePoint == value.CodePoint; });
    if (it == std::ranges::cend(TEncoding::LookupMap))
    {
      return EncodeOneUnmappableCodePointError<TEncoding>();
    }

    return EncodeOneSuccess<TEncoding>(static_cast<typename TEncoding::code_unit>((*it).Index));
  }

#pragma endregion
}