#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/Algorithm.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/TypeTraits.hpp"
#include <cuchar>
#include <string_view>

namespace Krys
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

    static constexpr char_type *copy(char_type *destination, const char_type *source,
                                     std::size_t count) noexcept
    {
      (void)Krys::Ranges::Impl::CopyNUnsafe(source, count, destination);
      return destination;
    }

    static constexpr char_type *move(char_type *destination, const char_type *source,
                                     std::size_t count) noexcept
    {
      (void)Krys::Ranges::Impl::CopyNUnsafe(source, count, destination);
      return destination;
    }

    KRYS_NODISCARD static constexpr int compare(const char_type *left, const char_type *right,
                                                std::size_t count) noexcept
    {
      if (count == 0)
      {
        return 0;
      }
      return Krys::Ranges::Impl::LexicographicalCompareThreeWayBasic(left, left + count, right,
                                                                     right + count);
    }

    KRYS_NODISCARD static constexpr size_t length(const char_type *it) noexcept
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

    KRYS_NODISCARD static constexpr const char_type *find(const char_type *it, size_t count,
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

    static constexpr char_type *assign(char_type *first, size_t count, const char_type ch) noexcept
    {
      for (char_type *it = first; count > 0; --count, (void)++it)
      {
        *it = ch;
      }
      return first;
    }

    static constexpr void assign(char_type &left, const char_type &right) noexcept
    {
      left = right;
    }

    KRYS_NODISCARD static constexpr bool eq(const char_type &left, const char_type &right) noexcept
    {
      return left == right;
    }

    KRYS_NODISCARD static constexpr bool lt(const char_type &left, const char_type &right) noexcept
    {
      return left < right;
    }

    KRYS_NODISCARD static constexpr char_type to_char_type(const int_type &chAsInt) noexcept
    {
      return char_type(static_cast<char32_t>(chAsInt));
    }

    KRYS_NODISCARD static constexpr int_type to_int_type(const char_type &ch) noexcept
    {
      return static_cast<int_type>(ch);
    }

    KRYS_NODISCARD static constexpr bool eq_int_type(const int_type &left, const int_type &right) noexcept
    {
      return left == right;
    }

    KRYS_NODISCARD static constexpr int_type not_eof(const int_type &chAsInt) noexcept
    {
      return chAsInt == eof() ? eof() : int_type {};
    }

    KRYS_NODISCARD static constexpr int_type eof() noexcept
    {
      return static_cast<int_type>(EOF);
    }
  };

  using unsigned_char_traits = unsigned_8bit_traits<uchar>;
}

namespace std
{
  template <>
  class char_traits<Krys::uchar> : public Krys::unsigned_char_traits
  {
  };
}

namespace Krys
{
  /// @brief A traits type for char8/uchar8 for whatever it is aliased to.
  using uchar8_traits =
    conditional_t<SameType<uchar8, char8_t>, std::char_traits<uchar8>, unsigned_8bit_traits<uchar8>>;

  template <typename TChar>
  using char_traits_for = conditional_t<SameType<TChar, uchar8>, uchar8_traits, std::char_traits<TChar>>;

  template <>
  class is_char_traitable<uchar8> : public std::true_type
  {
  };
}
