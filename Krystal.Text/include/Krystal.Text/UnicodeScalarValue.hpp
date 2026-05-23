#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/Algorithm.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/TypeTraits.hpp"
#include "Krystal.Text/Unicode.hpp"
#include <cassert>
#include <cstdint>
#include <cuchar>
#include <string>
#include <utility>

namespace Krys::Text
{
  /// @brief A non-surrogate Unicode code point value.
  class alignas(alignof(char32)) UnicodeScalarValue
  {
  private:
    char32 _scalar {};

  public:
    UnicodeScalarValue() noexcept = default;

    constexpr UnicodeScalarValue(char32 codePoint) noexcept : _scalar(codePoint)
    {
      assert(!Unicode::IsSurrogate(this->_scalar) && (this->_scalar <= Unicode::LastUnicodeCodePoint)
             && "The code point value must be a valid code point and must not be a surrogate value.");
    }

    constexpr explicit operator char32() const noexcept
    {
      return this->_scalar;
    }

    constexpr const char32 &value() const & noexcept
    {
      return this->_scalar;
    }

    constexpr char32 &value() & noexcept
    {
      return this->_scalar;
    }

    constexpr char32 &&value() && noexcept
    {
      return std::move(this->_scalar);
    }
  };

  constexpr bool operator==(const UnicodeScalarValue &left, const UnicodeScalarValue &right)
  {
    return left.value() == right.value();
  }

  constexpr bool operator!=(const UnicodeScalarValue &left, const UnicodeScalarValue &right)
  {
    return left.value() != right.value();
  }

  constexpr bool operator<(const UnicodeScalarValue &left, const UnicodeScalarValue &right)
  {
    return left.value() < right.value();
  }
}

namespace std
{
  template <>
  class char_traits<Krys::Text::UnicodeScalarValue>
  {
    using char_type = Krys::Text::UnicodeScalarValue;
    using int_type = std::int_least32_t;
    using pos_type = std::streampos;
    using off_type = std::streamoff;
    using state_type = mbstate_t;

    constexpr static char_type *copy(char_type *destination, const char_type *source,
                                     std::size_t count) noexcept
    {
      std::ranges::copy_n(source, count, destination);
      return destination;
    }

    constexpr static char_type *move(char_type *destination, const char_type *source,
                                     std::size_t count) noexcept
    {
      std::ranges::copy_n(source, count, destination);
      return destination;
    }

    KRYS_NODISCARD constexpr static int compare(const char_type *left, const char_type *right,
                                                std::size_t count) noexcept
    {
      if (count == 0)
      {
        return 0;
      }
      return Krys::Ranges::LexicographicalCompareThreeWayBasic(left, left + count, right, right + count);
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
                                                          const char_type &ch) noexcept
    {
      for (; 0 < count; --count, (void)++it)
      {
        if (*it == ch)
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
      return char_type(static_cast<Krys::char32>(chAsInt));
    }

    KRYS_NODISCARD constexpr static int_type to_int_type(const char_type &ch) noexcept
    {
      return static_cast<int_type>(ch.value());
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
