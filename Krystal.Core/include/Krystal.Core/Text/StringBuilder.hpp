#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/StringAtom.hpp"
#include <concepts>
#include <limits>
#include <string_view>
#include <type_traits>
#include <utility>

namespace krys
{
  class StringBuilder;

  /// @brief A type that appends itself to a string builder, for values that aren't strings, characters or
  /// numbers (formatted numbers, separated lists, and so on).
  template <typename T>
  concept StringBuilderWriter = requires(const T &value, StringBuilder &builder) {
    { value.WriteTo(builder) } -> std::same_as<void>;
  };

  /// @brief Builds a UTF-8 string by appending strings, characters and numbers to it.
  ///
  /// `Append` takes any number of values:
  /// - UTF-8 strings: anything that converts to `utf8_stringview` (`utf8_string`, `u8"..."` literals), a
  ///   `StringAtom`, or another builder;
  /// - ASCII strings: anything that converts to `std::string_view` (`"..."` literals);
  /// - characters: `char8` (a UTF-8 code unit), `char32` (a code point, encoded as UTF-8) or an ASCII `char`;
  /// - numbers: integers and enums in decimal, and floating-point numbers in their shortest round-trip form,
  ///   written as ECMAScript's `Number.prototype.toString` writes them (`1.5`, `1e+21`, `1e-7`, `NaN`);
  /// - `StringBuilderWriter`s, which append themselves.
  class StringBuilder
  {
    KRYS_NON_COPYABLE(StringBuilder);

  private:
    utf8_string _buffer;

  public:
    StringBuilder() noexcept = default;
    StringBuilder(StringBuilder &&) noexcept = default;
    StringBuilder &operator=(StringBuilder &&) noexcept = default;

    void Clear() noexcept
    {
      _buffer.clear();
    }

    void Swap(StringBuilder &other) noexcept
    {
      _buffer.swap(other._buffer);
    }

    /// @brief Appends each of @p values in turn.
    template <typename... TValues>
    void Append(const TValues &...values) noexcept
    {
      (AppendOne(values), ...);
    }

    /// @brief Appends at most @p length bytes of @p string from byte @p offset.
    void AppendSubstring(utf8_stringview string, size_t offset,
                         size_t length = std::numeric_limits<size_t>::max()) noexcept
    {
      krys_debug_assert(offset <= string.length());
      _buffer.append(string.substr(offset, length));
    }

    /// @brief Appends @p count copies of @p character.
    void AppendRepeated(char8 character, size_t count) noexcept
    {
      _buffer.append(count, character);
    }

    /// @brief The string built so far.
    KRYS_NODISCARD const utf8_string &ToString() const & noexcept
    {
      return _buffer;
    }

    /// @brief The string built, moved out of the builder.
    KRYS_NODISCARD utf8_string ToString() && noexcept
    {
      return krys::move(_buffer);
    }

    KRYS_NODISCARD StringAtom ToAtomString() const noexcept
    {
      return StringAtom {utf8_stringview {_buffer}};
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _buffer.empty();
    }

    /// @brief The length of the string built so far, in bytes.
    KRYS_NODISCARD size_t Length() const noexcept
    {
      return _buffer.length();
    }

    KRYS_NODISCARD utf8_stringview View() const noexcept
    {
      return _buffer;
    }

    // NOLINTNEXTLINE(google-explicit-constructor): a builder reads as the string built so far.
    operator utf8_stringview() const noexcept
    {
      return _buffer;
    }

    KRYS_NODISCARD char8 operator[](size_t i) const noexcept
    {
      krys_debug_assert(i < _buffer.length());
      return _buffer[i];
    }

    KRYS_NODISCARD size_t Capacity() const noexcept
    {
      return _buffer.capacity();
    }

    void ReserveCapacity(size_t newCapacity) noexcept
    {
      _buffer.reserve(newCapacity);
    }

    /// @brief Truncates the string built so far to @p newLength bytes.
    void Shrink(size_t newLength) noexcept
    {
      krys_debug_assert(newLength <= _buffer.length());
      _buffer.resize(newLength);
    }

    void ShrinkToFit() noexcept
    {
      _buffer.shrink_to_fit();
    }

    KRYS_NODISCARD bool ContainsOnlyASCII() const noexcept;

    KRYS_NODISCARD bool operator==(utf8_stringview other) const noexcept
    {
      return utf8_stringview {_buffer} == other;
    }

  private:
    template <typename T>
    void AppendOne(const T &value) noexcept
    {
      using U = std::remove_cvref_t<T>;
      if constexpr (std::same_as<U, StringBuilder>)
      {
        _buffer.append(value._buffer);
      }
      else if constexpr (std::same_as<U, StringAtom>)
      {
        _buffer.append(value.View());
      }
      else if constexpr (std::convertible_to<const T &, utf8_stringview>)
      {
        _buffer.append(utf8_stringview {value});
      }
      else if constexpr (std::convertible_to<const T &, std::string_view>)
      {
        AppendASCII(std::string_view {value});
      }
      else if constexpr (std::same_as<U, char8>)
      {
        _buffer.push_back(value);
      }
      else if constexpr (std::same_as<U, char32>)
      {
        AppendCodePoint(value);
      }
      else if constexpr (std::same_as<U, char>)
      {
        krys_debug_assert(static_cast<unsigned char>(value) < 0x80);
        _buffer.push_back(static_cast<char8>(value));
      }
      else if constexpr (std::same_as<U, bool> || std::same_as<U, char16_t> || std::same_as<U, wchar_t>)
      {
        static_assert(sizeof(T) == 0, "Append a bool or a UTF-16 character explicitly.");
      }
      else if constexpr (std::is_enum_v<U>)
      {
        AppendInteger(static_cast<std::underlying_type_t<U>>(value));
      }
      else if constexpr (std::integral<U>)
      {
        AppendInteger(value);
      }
      else if constexpr (std::same_as<U, float>)
      {
        AppendNumber(value);
      }
      else if constexpr (std::floating_point<U>)
      {
        AppendNumber(static_cast<double>(value));
      }
      else if constexpr (StringBuilderWriter<U>)
      {
        value.WriteTo(*this);
      }
      else
      {
        static_assert(sizeof(T) == 0, "StringBuilder can't append this type.");
      }
    }

    void AppendASCII(std::string_view string) noexcept;
    void AppendCodePoint(char32 codePoint) noexcept;
    void AppendInteger(int64 number) noexcept;
    void AppendInteger(uint64 number) noexcept;
    void AppendNumber(float number) noexcept;
    void AppendNumber(double number) noexcept;

    template <std::integral T>
    void AppendInteger(T number) noexcept
    {
      if constexpr (std::is_signed_v<T>)
      {
        AppendInteger(static_cast<int64>(number));
      }
      else
      {
        AppendInteger(static_cast<uint64>(number));
      }
    }
  };
}
