#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/Algorithm.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"

namespace Krys::Text::detail_traits
{
  template <typename TChar>
  class unsigned_8bit_traits
  {
  public:
    static_assert(Unsigned<TChar>, "the TChar must be unsigned to be used here");

    using char_type = TChar;
    using int_type = std::int_least32_t;
    using pos_type = std::streampos;
    using off_type = std::streamoff;
    using state_type = mbstate_t;

    constexpr static char_type *copy(char_type *destination, const char_type *source,
                                     std::size_t count) noexcept
    {
      (void)::Krys::Ranges::Impl::CopyNUnsafe(source, count, destination);
      return destination;
    }

    constexpr static char_type *move(char_type *destination, const char_type *source,
                                     std::size_t count) noexcept
    {
      (void)::Krys::Ranges::Impl::CopyNUnsafe(source, count, destination);
      return destination;
    }

    KRYS_NODISCARD constexpr static int compare(const char_type *left, const char_type *right,
                                                std::size_t count) noexcept
    {
      if (count == 0)
      {
        return 0;
      }
      return ::Krys::Ranges::Impl::LexicographicalCompareThreeWayBasic(left, left + count, right,
                                                                       right + count);
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
      return char_type(static_cast<char32_t>(chAsInt));
    }

    KRYS_NODISCARD constexpr static int_type to_int_type(const char_type &ch) noexcept
    {
      return static_cast<int_type>(ch);
    }

    KRYS_NODISCARD constexpr static bool eq_int_type(const int_type &left, const int_type &right) noexcept
    {
      return left == right;
    }

    KRYS_NODISCARD constexpr static int_type not_eof(const int_type &chAsInt) noexcept
    {
      return chAsInt == eof() ? eof() : int_type {};
    }

    KRYS_NODISCARD constexpr static int_type eof() noexcept
    {
      return static_cast<int_type>(EOF);
    }
  };

  using unsigned_char_traits = unsigned_8bit_traits<uchar>;

  /// @brief A traits type for char8/uchar8 for whatever it is aliased to.
  using uchar8_traits =
    conditional_t<SameType<uchar8, char8>, std::char_traits<uchar8>, unsigned_8bit_traits<uchar8>>;

  /// @brief Checks if the given type is one of the types that is usable in the standard with the
  /// std::char_traits traits type that's used for std::string_view, std::string and others.
  template <typename T>
  class is_char_traitable
      : public IntegralConstant<bool, SameType<T, char> || SameType<T, wchar> || SameType<T, char8>
                                        || SameType<T, char16> || SameType<T, char32>>
  {
  };

  template <>
  class is_char_traitable<uchar8> : public std::true_type
  {
  };
}

namespace Krys::Text
{
  template <typename TChar>
  using char_traits_for = conditional_t<SameType<TChar, uchar8>, ::Krys::Text::detail_traits::uchar8_traits,
                                        std::char_traits<TChar>>;

  template <typename T>
  concept IsCharTraitable = ::Krys::Text::detail_traits::is_char_traitable<T>::value;

  /// Gets the name of the encoding type.
  template <typename TEncoding>
  constexpr inline ::Krys::Text::ASCIILiteral CodecName = {};

  /// Gets the aliases of the encoding type.
  template <typename TEncoding>
  constexpr inline List<::Krys::Text::ASCIILiteral> CodecAliases = {};
}

namespace std
{
  template <>
  class char_traits<::Krys::uchar> : public ::Krys::Text::detail_traits::unsigned_char_traits
  {
  };
}