#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/Algorithm.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/Impl/Unicode.hpp"
#include "Krystal.Text/TypeTraits.hpp"
#include <cassert>
#include <cstdint>
#include <cuchar>
#include <string>
#include <utility>

namespace Krys
{
  /// @brief A 32-bit value that is within the allowed 21 bits of Unicode. Can be one of the surrogate
  /// values.
  class UnicodeCodePoint
  {
  private:
    char32 TValue;

  public:
    /// @brief Constructs a code point value of indeterminate value (if no parentheses/brackets are
    /// provided) or with the value 0 (if parentheses/brackets are provided for intentional value
    /// initialization).
    UnicodeCodePoint() noexcept = default;

    /// @brief Constructs a code point value with the given code point value.
    constexpr UnicodeCodePoint(char32 codePoint) noexcept : TValue(codePoint)
    {
      assert((Impl::Unicode::IsSurrogate(this->TValue))
             || (this->TValue <= Impl::Unicode::LastUnicodeCodePoint)
                  && "The code point value must be a valid code point.");
    }

    /// @brief An explicit conversion to a typical char32 value, bit-compatible with a normal code point
    /// value.
    constexpr explicit operator char32() const noexcept
    {
      return this->TValue;
    }

    /// @brief Retrieves the underlying value.
    constexpr const char32 &value() const & noexcept
    {
      return this->TValue;
    }

    /// @brief Retrieves the underlying value.
    constexpr char32 &value() & noexcept
    {
      return this->TValue;
    }

    /// @brief Retrieves the underlying value.
    constexpr char32 &&value() && noexcept
    {
      return std::move(this->TValue);
    }
  };

  /// @brief Check if two unicode code points are equal.
  /// @param[in] left Left hand value of equality operator.
  /// @param[in] right Right hand value of equality operator.
  constexpr bool operator==(const UnicodeCodePoint &left, const UnicodeCodePoint &right)
  {
    return left.value() == right.value();
  }

  /// @brief Check if two unicode code points are not equal.
  /// @param[in] left Left hand value of inequality operator.
  /// @param[in] right Right hand value of inequality operator.
  constexpr bool operator!=(const UnicodeCodePoint &left, const UnicodeCodePoint &right)
  {
    return left.value() != right.value();
  }

  /// @brief Check if one unicode code point is less than the other.
  /// @param[in] left Left hand value of less than operator.
  /// @param[in] right Right hand value of less than operator.
  constexpr bool operator<(const UnicodeCodePoint &left, const UnicodeCodePoint &right)
  {
    return left.value() < right.value();
  }
}

namespace std
{
  template <>
  class char_traits<Krys::UnicodeCodePoint>
  {
    using char_type = Krys::UnicodeCodePoint;
    using int_type = std::int_least32_t;
    using pos_type = std::streampos;
    using off_type = std::streamoff;
    using state_type = mbstate_t;

    constexpr static char_type *copy(char_type *destination, const char_type *source,
                                     std::size_t count) noexcept
    {
      (void)Krys::Ranges::Impl::CopyNUnsafe(source, count, destination);
      return destination;
    }

    constexpr static char_type *move(char_type *destination, const char_type *source,
                                     std::size_t count) noexcept
    {
      (void)Krys::Ranges::Impl::CopyNUnsafe(source, count, destination);
      return destination;
    }

    KRYS_NODISCARD constexpr static int compare(const char_type *left, const char_type *right,
                                                std::size_t count) noexcept
    {
      if (count == 0)
      {
        return 0;
      }
      return Krys::Ranges::Impl::LexicographicalCompareThreeWayBasic(left, left + count, right, right + count);
    }

    KRYS_NODISCARD constexpr static size_t length(const char_type *it) noexcept
    {
      size_t count = 0;
      const char_type nullValue {};
      while (*it != nullValue)
      {
        ++count;
        ++it;
      }
      return count;
    }

    KRYS_NODISCARD constexpr static const char_type *find(const char_type *it, size_t count,
                                                          const char_type &c) noexcept
    {
      for (; 0 < count; --count, (void)++it)
      {
        if (*it == c)
        {
          return it;
        }
      }
      return nullptr;
    }

    constexpr static char_type *assign(char_type *first, size_t count, const char_type ch) noexcept
    {
      for (char_type *it = first; count > 0; --count, (void)++it)
      {
        *it = ch;
      }
      return first;
    }

    constexpr static void assign(char_type &left, const char_type &right) noexcept
    {
      left = right;
    }

    KRYS_NODISCARD constexpr static bool eq(const char_type &left, const char_type &right) noexcept
    {
      return left == right;
    }

    KRYS_NODISCARD constexpr static bool lt(const char_type &left, const char_type &right) noexcept
    {
      return left < right;
    }

    KRYS_NODISCARD constexpr static char_type to_char_type(const int_type &chAsInt) noexcept
    {
      return char_type(static_cast<char32_t>(chAsInt));
    }

    KRYS_NODISCARD constexpr static int_type to_int_type(const char_type &c) noexcept
    {
      return static_cast<int_type>(c.value());
    }

    KRYS_NODISCARD constexpr static bool eq_int_type(const int_type &left, const int_type &right) noexcept
    {
      return left == right;
    }

    KRYS_NODISCARD constexpr static int_type not_eof(const int_type &chAsInt) noexcept
    {
      return (chAsInt == eof()) ? int_type {} : chAsInt;
    }

    KRYS_NODISCARD constexpr static int_type eof() noexcept
    {
      return static_cast<int_type>(EOF);
    }
  };
}

namespace Krys
{
  template <>
  class is_character<Krys::UnicodeCodePoint> : public std::true_type
  {
  };

  template <>
  class is_char_traitable<Krys::UnicodeCodePoint> : public std::true_type
  {
  };
}