#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Text/Unicode.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::text
{
  /// @brief The result of decoding one UTF-8 sequence.
  struct DecodedCodePoint
  {
    /// @brief The code point, or `UnicodeReplacementChar` if the sequence was invalid.
    char32 CodePoint;
    /// @brief The number of bytes consumed: the sequence's length, or 1 for an invalid byte.
    uint8 Length;
    bool IsValid;
  };

  /// @brief Decodes the UTF-8 sequence at the start of `data`, rejecting overlong forms, surrogates and code
  /// points above U+10FFFF.
  /// @param data Must not be empty.
  KRYS_NODISCARD constexpr DecodedCodePoint DecodeUTF8(Span<const char8> data) noexcept
  {
    krys_debug_assert(!data.empty());

    constexpr DecodedCodePoint invalid {UnicodeReplacementChar, 1, false};

    char8 lead = data[0];
    if (lead < 0x80)
    {
      return {lead, 1, true};
    }

    uint8 length;
    char32 codePoint;
    char32 minimum;
    if (lead >= 0xC2 && lead <= 0xDF)
    {
      length = 2;
      codePoint = lead & 0x1F;
      minimum = 0x80;
    }
    else if (lead >= 0xE0 && lead <= 0xEF)
    {
      length = 3;
      codePoint = lead & 0x0F;
      minimum = 0x800;
    }
    else if (lead >= 0xF0 && lead <= 0xF4)
    {
      length = 4;
      codePoint = lead & 0x07;
      minimum = 0x10000;
    }
    else
    {
      return invalid;
    }

    if (data.size() < length)
    {
      return invalid;
    }

    for (uint8 i = 1; i < length; ++i)
    {
      char8 trail = data[i];
      if ((trail & 0xC0) != 0x80)
      {
        return invalid;
      }
      codePoint = (codePoint << 6) | (trail & 0x3F);
    }

    if (codePoint < minimum || codePoint > MaxUnicodeChar || (codePoint >= 0xD800 && codePoint <= 0xDFFF))
    {
      return invalid;
    }

    return {codePoint, length, true};
  }

  /// @brief Whether `data` is entirely well-formed UTF-8.
  KRYS_NODISCARD constexpr bool IsValidUTF8(utf8_stringview text) noexcept
  {
    Span<const char8> data {text.data(), text.size()};
    while (!data.empty())
    {
      auto decoded = DecodeUTF8(data);
      if (!decoded.IsValid)
      {
        return false;
      }
      data = data.subspan(decoded.Length);
    }
    return true;
  }

  /// @brief Iterates the code points of UTF-8 text. Each invalid byte reads as U+FFFD and advances by one
  /// byte.
  class CodePointIterator
  {
  private:
    Span<const char8> _data;

  public:
    constexpr CodePointIterator() noexcept = default;

    constexpr explicit CodePointIterator(Span<const char8> data) noexcept : _data(data)
    {
    }

    constexpr explicit CodePointIterator(utf8_stringview data) noexcept : _data(data.data(), data.size())
    {
    }

    /// @brief The code units from `begin` up to (not including) `end`.
    constexpr CodePointIterator(const CodePointIterator &begin, const CodePointIterator &end) noexcept
        : _data(begin._data.data(), static_cast<size_t>(end._data.data() - begin._data.data()))
    {
      krys_debug_assert(end._data.data() >= begin._data.data());
    }

    KRYS_NODISCARD constexpr char32 operator*() const noexcept
    {
      krys_debug_assert(!AtEnd());
      return DecodeUTF8(_data).CodePoint;
    }

    constexpr CodePointIterator &operator++() noexcept
    {
      krys_debug_assert(!AtEnd());
      _data = _data.subspan(DecodeUTF8(_data).Length);
      return *this;
    }

    KRYS_NODISCARD friend constexpr bool operator==(const CodePointIterator &a,
                                                    const CodePointIterator &b) noexcept
    {
      return a._data.data() == b._data.data() && a._data.size() == b._data.size();
    }

    KRYS_NODISCARD constexpr bool AtEnd() const noexcept
    {
      return _data.empty();
    }

    KRYS_NODISCARD constexpr size_t CodeUnitsSince(const char8 *reference) const noexcept
    {
      krys_debug_assert(_data.data() >= reference);
      return static_cast<size_t>(_data.data() - reference);
    }

    KRYS_NODISCARD constexpr size_t CodeUnitsSince(const CodePointIterator &other) const noexcept
    {
      return CodeUnitsSince(other._data.data());
    }
  };
}
