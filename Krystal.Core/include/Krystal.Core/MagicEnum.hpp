// __  __             _        ______                          _____
// |  \/  |           (_)      |  ____|                        / ____|_     _
// | \  / | __ _  __ _ _  ___  | |__   _ __  _   _ _ __ ___   | |   _| |_ _| |_
// | |\/| |/ _` |/ _` | |/ __| |  __| | '_ \| | | | '_ ` _ \  | |  |_   _|_   _|
// | |  | | (_| | (_| | | (__  | |____| | | | |_| | | | | | | | |____|_|   |_|
// |_|  |_|\__,_|\__, |_|\___| |______|_| |_|\__,_|_| |_| |_|  \_____|
//               __/ | https://github.com/Neargye/magic_enum
//              |___/  version 0.9.8
//
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2019 - 2026 Daniil Goncharov <neargye@gmail.com>.
//
// Permission is hereby  granted, free of charge, to any  person obtaining a copy
// of this software and associated  documentation files (the "Software"), to deal
// in the Software  without restriction, including without  limitation the rights
// to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
// copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
// IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
// FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
// AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
// LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef NEARGYE_MAGIC_ENUM_HPP
#define NEARGYE_MAGIC_ENUM_HPP

#define MAGIC_ENUM_VERSION_MAJOR 0
#define MAGIC_ENUM_VERSION_MINOR 9
#define MAGIC_ENUM_VERSION_PATCH 8

#ifndef MAGIC_ENUM_USE_STD_MODULE
  #include <array>
  #include <cstddef>
  #include <cstdint>
  #include <functional>
  #include <limits>
  #include <type_traits> // NOLINT(portability-restrict-system-includes): third-party
  #include <utility>
#endif

#if defined(MAGIC_ENUM_CONFIG_FILE)
  #include MAGIC_ENUM_CONFIG_FILE
#endif

#ifndef MAGIC_ENUM_USE_STD_MODULE
  #if !defined(MAGIC_ENUM_USING_ALIAS_OPTIONAL)
    #include <optional>
  #endif
  #if !defined(MAGIC_ENUM_USING_ALIAS_STRING)
    #include <string>
  #endif
  #if !defined(MAGIC_ENUM_USING_ALIAS_STRING_VIEW)
    #include <string_view>
  #endif
#endif

#if defined(MAGIC_ENUM_NO_ASSERT)
  #define MAGIC_ENUM_ASSERT(...) static_cast<void>(0)
#elif !defined(MAGIC_ENUM_ASSERT)
  #include "Krystal.Core/Assert.hpp"
  #define MAGIC_ENUM_ASSERT(...) krys_assert((__VA_ARGS__))
#endif

#if defined(__clang__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wunknown-warning-option"
  #pragma clang diagnostic ignored "-Wenum-constexpr-conversion"
  #pragma clang diagnostic ignored                                                                           \
    "-Wuseless-cast" // suppresses 'static_cast<char_type>('\0')' for char_type = char (common on Linux).
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wmaybe-uninitialized" // May be used uninitialized 'return {};'.
  #pragma GCC diagnostic ignored                                                                             \
    "-Wuseless-cast" // suppresses 'static_cast<char_type>('\0')' for char_type = char (common on Linux).
#elif defined(_MSC_VER)
  #pragma warning(push)
  #pragma warning(disable : 26'495) // Variable 'static_str<N>::chars_' is uninitialized.
  #pragma warning(disable : 28'020) // Arithmetic overflow: Using operator '-' on a 4 byte value and then
                                    // casting the result to a 8 byte value.
  #pragma warning(disable                                                                                    \
                  : 26'451) // The expression '0<=_Param_(1)&&_Param_(1)<=1-1' is not true at this call.
  #pragma warning(disable : 4'514) // Unreferenced inline function has been removed.
#endif

// Checks magic_enum compiler compatibility.
#if defined(__clang__) && __clang_major__ >= 5 || defined(__GNUC__) && __GNUC__ >= 9                         \
  || defined(_MSC_VER) && _MSC_VER >= 1'910 || defined(__RESHARPER__)
  #undef MAGIC_ENUM_SUPPORTED
  #define MAGIC_ENUM_SUPPORTED 1
#endif

// Checks magic_enum compiler aliases compatibility.
#if defined(__clang__) && __clang_major__ >= 5 || defined(__GNUC__) && __GNUC__ >= 9                         \
  || defined(_MSC_VER) && _MSC_VER >= 1'920
  #undef MAGIC_ENUM_SUPPORTED_ALIASES
  #define MAGIC_ENUM_SUPPORTED_ALIASES 1
#endif

// Specify the calling convention for compilers that need it in order to get reliable mangled names under
// different compiler flags. In particular, MSVC allows changing the default calling convention on x86.
#if defined(__clang__) || defined(__GNUC__)
  #define MAGIC_ENUM_CALLING_CONVENTION
#elif defined(_MSC_VER)
  #define MAGIC_ENUM_CALLING_CONVENTION __cdecl
#endif

// Enum value must be greater or equals than MAGIC_ENUM_RANGE_MIN. By default MAGIC_ENUM_RANGE_MIN = -128.
// If need another min range for all enum types by default, redefine the macro MAGIC_ENUM_RANGE_MIN.
#if !defined(MAGIC_ENUM_RANGE_MIN)
  #define MAGIC_ENUM_RANGE_MIN -128
#endif

// Enum value must be less or equals than MAGIC_ENUM_RANGE_MAX. By default MAGIC_ENUM_RANGE_MAX = 127.
// If need another max range for all enum types by default, redefine the macro MAGIC_ENUM_RANGE_MAX.
#if !defined(MAGIC_ENUM_RANGE_MAX)
  #define MAGIC_ENUM_RANGE_MAX 127
#endif

// Improve ReSharper C++ intellisense performance with builtins, avoiding unnecessary template instantiations.
#if defined(__RESHARPER__)
  #undef MAGIC_ENUM_GET_ENUM_NAME_BUILTIN
  #undef MAGIC_ENUM_GET_TYPE_NAME_BUILTIN
  #if __RESHARPER__ >= 20'230'100
    #define MAGIC_ENUM_GET_ENUM_NAME_BUILTIN(V) __rscpp_enumerator_name(V)
    #define MAGIC_ENUM_GET_TYPE_NAME_BUILTIN(T) __rscpp_type_name<T>()
  #else
    #define MAGIC_ENUM_GET_ENUM_NAME_BUILTIN(V) nullptr
    #define MAGIC_ENUM_GET_TYPE_NAME_BUILTIN(T) nullptr
  #endif
#endif

namespace magic_enum
{
// If need another optional type, define the macro MAGIC_ENUM_USING_ALIAS_OPTIONAL.
#if defined(MAGIC_ENUM_USING_ALIAS_OPTIONAL)
  MAGIC_ENUM_USING_ALIAS_OPTIONAL
#else
  using std::optional;
#endif

// If need another string_view type, define the macro MAGIC_ENUM_USING_ALIAS_STRING_VIEW.
#if defined(MAGIC_ENUM_USING_ALIAS_STRING_VIEW)
  MAGIC_ENUM_USING_ALIAS_STRING_VIEW
#else
  using std::string_view;
#endif

// If need another string type, define the macro MAGIC_ENUM_USING_ALIAS_STRING.
#if defined(MAGIC_ENUM_USING_ALIAS_STRING)
  MAGIC_ENUM_USING_ALIAS_STRING
#else
  using std::string;
#endif

  using char_type = string_view::value_type;
  static_assert(std::is_same_v<string_view::value_type, string::value_type>,
                "magic_enum::customize requires same string_view::value_type and string::value_type");
  static_assert(
    []
    {
      if constexpr (std::is_same_v<char_type, wchar_t>)
      {
        constexpr const char c[] = "abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789|";
        constexpr const wchar_t wc[] = L"abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789|";
        static_assert(std::size(c) == std::size(wc),
                      "magic_enum::customize identifier characters are multichars in wchar_t.");

        for (std::size_t i = 0; i < std::size(c); ++i)
        {
          if (c[i] != wc[i])
          {
            return false;
          }
        }
      }
      return true;
    }(),
    "magic_enum::customize wchar_t is not compatible with ASCII.");

  namespace customize
  {
    template <typename E, typename = void>
    struct enum_range;
  }

  namespace detail
  {
    template <typename E, typename = void>
    constexpr std::size_t prefix_length_or_zero = 0;

    template <typename E>
    constexpr auto prefix_length_or_zero<E, std::void_t<decltype(customize::enum_range<E>::prefix_length)>> =
      std::size_t {customize::enum_range<E>::prefix_length};
  }

  namespace customize
  {
    template <bool IsFlags = false, int Min = MAGIC_ENUM_RANGE_MIN, int Max = MAGIC_ENUM_RANGE_MAX,
              std::size_t PrefixLength = 0>
    struct adl_info_holder
    {
      constexpr static int max = Max;
      constexpr static int min = Min;
      constexpr static bool is_flags = IsFlags;
      constexpr static std::size_t prefix_length = PrefixLength;

      template <int min, int max>
      constexpr static adl_info_holder<IsFlags, min, max, PrefixLength> minmax()
      {
        return {};
      }

      template <bool is_flag>
      constexpr static adl_info_holder<is_flag, Min, Max, PrefixLength> flag()
      {
        return {};
      }

      template <std::size_t prefix_len>
      constexpr static adl_info_holder<IsFlags, Min, Max, prefix_len> prefix()
      {
        return {};
      }
    };

    constexpr adl_info_holder<> adl_info()
    {
      return {};
    }

    // Enum value must be in range [MAGIC_ENUM_RANGE_MIN, MAGIC_ENUM_RANGE_MAX]. By default
    // MAGIC_ENUM_RANGE_MIN = -128, MAGIC_ENUM_RANGE_MAX = 127. If need another range for all enum types by
    // default, redefine the macro MAGIC_ENUM_RANGE_MIN and MAGIC_ENUM_RANGE_MAX. If need another range for
    // specific enum type, add specialization enum_range for necessary enum type.
    template <typename E, typename>
    struct enum_range
    {
      constexpr static int min = MAGIC_ENUM_RANGE_MIN;
      constexpr static int max = MAGIC_ENUM_RANGE_MAX;
    };

    template <typename E>
    struct enum_range<E, decltype(void(magic_enum_define_range_adl(E {})))>
        : decltype(magic_enum_define_range_adl(E {})) {};

    static_assert(MAGIC_ENUM_RANGE_MAX > MAGIC_ENUM_RANGE_MIN,
                  "MAGIC_ENUM_RANGE_MAX must be greater than MAGIC_ENUM_RANGE_MIN.");

    namespace detail
    {
      enum class customize_tag
      {
        default_tag,
        invalid_tag,
        custom_tag,
      };
    } // namespace magic_enum::customize::detail

    class customize_t : public std::pair<detail::customize_tag, string_view>
    {
    public:
      constexpr customize_t(string_view srt)
          : std::pair<detail::customize_tag, string_view> {detail::customize_tag::custom_tag, srt}
      {
      }

      constexpr customize_t(const char_type *srt) : customize_t {string_view {srt}}
      {
      }

      constexpr customize_t(detail::customize_tag tag)
          : std::pair<detail::customize_tag, string_view> {tag, string_view {}}
      {
        MAGIC_ENUM_ASSERT(tag != detail::customize_tag::custom_tag);
      }
    };

    // Default customize.
    constexpr auto default_tag = customize_t {detail::customize_tag::default_tag};
    // Invalid customize.
    constexpr auto invalid_tag = customize_t {detail::customize_tag::invalid_tag};

    // If need custom names for enum, add specialization enum_name for necessary enum type.
    template <typename E>
    constexpr customize_t enum_name(E) noexcept
    {
      return default_tag;
    }

    // If need custom type name for enum, add specialization enum_type_name for necessary enum type.
    template <typename E>
    constexpr customize_t enum_type_name() noexcept
    {
      return default_tag;
    }
  } // namespace magic_enum::customize

  namespace detail
  {
    template <typename T>
    struct supported
#if defined(MAGIC_ENUM_SUPPORTED) || defined(MAGIC_ENUM_NO_CHECK_SUPPORT)
        : std::true_type
    {
    };
#else
        : std::false_type
    {
    };
#endif

    template <auto V, typename E = std::decay_t<decltype(V)>, std::enable_if_t<std::is_enum_v<E>, int> = 0>
    using enum_constant = std::integral_constant<E, V>;

    template <typename... T>
    constexpr bool always_false_v = false;

    template <typename T, typename = void>
    struct has_is_flags : std::false_type
    {
    };

    template <typename T>
    struct has_is_flags<T, std::void_t<decltype(customize::enum_range<T>::is_flags)>>
        : std::bool_constant<std::is_same_v<bool, std::decay_t<decltype(customize::enum_range<T>::is_flags)>>>
    {
    };

    template <typename T, typename = void>
    struct range_min : std::integral_constant<int, MAGIC_ENUM_RANGE_MIN>
    {
    };

    template <typename T>
    struct range_min<T, std::void_t<decltype(customize::enum_range<T>::min)>>
        : std::integral_constant<decltype(customize::enum_range<T>::min), customize::enum_range<T>::min>
    {
    };

    template <typename T, typename = void>
    struct range_max : std::integral_constant<int, MAGIC_ENUM_RANGE_MAX>
    {
    };

    template <typename T>
    struct range_max<T, std::void_t<decltype(customize::enum_range<T>::max)>>
        : std::integral_constant<decltype(customize::enum_range<T>::max), customize::enum_range<T>::max>
    {
    };

    struct str_view
    {
      const char *str_ = nullptr;
      std::size_t size_ = 0;
    };

    template <std::uint16_t N>
    class static_str
    {
    public:
      constexpr explicit static_str(str_view str) noexcept
          : static_str {str.str_, std::make_integer_sequence<std::uint16_t, N> {}}
      {
        MAGIC_ENUM_ASSERT(str.size_ == N);
      }

      constexpr explicit static_str(const char *const str) noexcept
          : static_str {str, std::make_integer_sequence<std::uint16_t, N> {}}
      {
      }

      constexpr explicit static_str(string_view str) noexcept
          : static_str {str.data(), std::make_integer_sequence<std::uint16_t, N> {}}
      {
        MAGIC_ENUM_ASSERT(str.size() == N);
      }

      constexpr const char_type *data() const noexcept
      {
        return chars_;
      }

      constexpr std::uint16_t size() const noexcept
      {
        return N;
      }

      constexpr string_view str() const noexcept
      {
        return string_view(data(), size());
      }

      char_type chars_[static_cast<std::size_t>(N) + 1];

    private:
      template <std::uint16_t... I>
      constexpr static_str(const char *str, std::integer_sequence<std::uint16_t, I...>) noexcept
          : chars_ {static_cast<char_type>(str[I])..., static_cast<char_type>('\0')}
      {
      }

      template <std::uint16_t... I>
      constexpr static_str(string_view str, std::integer_sequence<std::uint16_t, I...>) noexcept
          : chars_ {str[I]..., static_cast<char_type>('\0')}
      {
      }
    };

    template <>
    class static_str<0>
    {
    public:
      constexpr static_str() noexcept = default;

      constexpr static_str(str_view) noexcept
      {
      }

      constexpr static_str(string_view) noexcept
      {
      }

      constexpr const char_type *data() const noexcept
      {
        return chars_;
      }

      constexpr std::uint16_t size() const noexcept
      {
        return 0;
      }

      constexpr string_view str() const noexcept
      {
        return string_view(data(), size());
      }

      constexpr static char_type chars_[1] = {};
    };

    template <typename Op = std::equal_to<>>
    class case_insensitive
    {
      constexpr static char_type to_lower(char_type c) noexcept
      {
        return (c >= static_cast<char_type>('A') && c <= static_cast<char_type>('Z'))
                 ? static_cast<char_type>(c + (static_cast<char_type>('a') - static_cast<char_type>('A')))
                 : c;
      }

    public:
      template <typename L, typename R>
      constexpr auto operator()(L lhs, R rhs) const noexcept -> std::enable_if_t<
        std::is_same_v<std::decay_t<L>, char_type> && std::is_same_v<std::decay_t<R>, char_type>, bool>
      {
        return Op {}(to_lower(lhs), to_lower(rhs));
      }
    };

    constexpr std::size_t find(string_view str, char_type c) noexcept
    {
#if defined(__clang__) && __clang_major__ < 9 && defined(__GLIBCXX__)                                        \
  || defined(_MSC_VER) && _MSC_VER < 1'920 && !defined(__clang__)
      // https://stackoverflow.com/questions/56484834/constexpr-stdstring-viewfind-last-of-doesnt-work-on-clang-8-with-libstdc
      // https://developercommunity.visualstudio.com/content/problem/360432/vs20178-regression-c-failed-in-test.html
      constexpr bool workaround = true;
#else
      constexpr bool workaround = false;
#endif

      if constexpr (workaround)
      {
        for (std::size_t i = 0; i < str.size(); ++i)
        {
          if (str[i] == c)
          {
            return i;
          }
        }

        return string_view::npos;
      }
      else
      {
        return str.find(c);
      }
    }

    template <typename BinaryPredicate>
    constexpr bool is_default_predicate_v =
      std::is_same_v<std::decay_t<BinaryPredicate>, std::equal_to<string_view::value_type>>
      || std::is_same_v<std::decay_t<BinaryPredicate>, std::equal_to<>>;

    template <typename BinaryPredicate>
    constexpr bool is_nothrow_invocable_v =
      is_default_predicate_v<BinaryPredicate>
      || std::is_nothrow_invocable_r_v<bool, BinaryPredicate, char_type, char_type>;

    template <typename BinaryPredicate>
    constexpr bool
      cmp_equal(string_view lhs, string_view rhs,
                [[maybe_unused]] BinaryPredicate &&p) noexcept(is_nothrow_invocable_v<BinaryPredicate>)
    {
#if defined(_MSC_VER) && _MSC_VER < 1'920 && !defined(__clang__)
      // https://developercommunity.visualstudio.com/content/problem/360432/vs20178-regression-c-failed-in-test.html
      // https://developercommunity.visualstudio.com/content/problem/232218/c-constexpr-string-view.html
      constexpr bool workaround = true;
#else
      constexpr bool workaround = false;
#endif

      if constexpr (!is_default_predicate_v<BinaryPredicate> || workaround)
      {
        if (lhs.size() != rhs.size())
        {
          return false;
        }

        const auto size = lhs.size();
        for (std::size_t i = 0; i < size; ++i)
        {
          if (!p(lhs[i], rhs[i]))
          {
            return false;
          }
        }

        return true;
      }
      else
      {
        return lhs == rhs;
      }
    }

    template <typename L, typename R>
    constexpr bool cmp_less(L lhs, R rhs) noexcept
    {
      static_assert(std::is_integral_v<L> && std::is_integral_v<R>,
                    "magic_enum::detail::cmp_less requires integral type.");

      if constexpr (std::is_signed_v<L> == std::is_signed_v<R>)
      {
        // If same signedness (both signed or both unsigned).
        return lhs < rhs;
      }
      else if constexpr (std::is_same_v<L, bool>)
      { // bool special case
        return static_cast<R>(lhs) < rhs;
      }
      else if constexpr (std::is_same_v<R, bool>)
      { // bool special case
        return lhs < static_cast<L>(rhs);
      }
      else if constexpr (std::is_signed_v<R>)
      {
        // If 'right' is negative, then result is 'false', otherwise cast & compare.
        return rhs > 0 && lhs < static_cast<std::make_unsigned_t<R>>(rhs);
      }
      else
      {
        // If 'left' is negative, then result is 'true', otherwise cast & compare.
        return lhs < 0 || static_cast<std::make_unsigned_t<L>>(lhs) < rhs;
      }
    }

    template <typename I>
    constexpr I log2(I value) noexcept
    {
      static_assert(std::is_integral_v<I>, "magic_enum::detail::log2 requires integral type.");

      if constexpr (std::is_same_v<I, bool>)
      { // bool special case
        return MAGIC_ENUM_ASSERT(false), value;
      }
      else
      {
        auto ret = I {0};
        for (; value > I {1}; value >>= I {1}, ++ret)
        {
        }

        return ret;
      }
    }

#if defined(__cpp_lib_array_constexpr) && __cpp_lib_array_constexpr >= 201'603l
  #define MAGIC_ENUM_ARRAY_CONSTEXPR 1
#else
    template <typename T, std::size_t N, std::size_t... I>
    constexpr std::array<std::remove_cv_t<T>, N> to_array(T (&a)[N], std::index_sequence<I...>) noexcept
    {
      return {{a[I]...}};
    }
#endif

    template <typename T>
    constexpr bool is_enum_v = std::is_enum_v<T> && std::is_same_v<T, std::decay_t<T>>;

    template <typename E>
    constexpr auto MAGIC_ENUM_CALLING_CONVENTION n() noexcept
    {
      static_assert(is_enum_v<E>, "magic_enum::detail::n requires enum type.");

      if constexpr (supported<E>::value)
      {
#if defined(MAGIC_ENUM_GET_TYPE_NAME_BUILTIN)
        constexpr auto name_ptr = MAGIC_ENUM_GET_TYPE_NAME_BUILTIN(E);
        constexpr auto name =
          name_ptr ? str_view {name_ptr, std::char_traits<char>::length(name_ptr)} : str_view {};
#elif defined(__clang__)
        str_view name;
        if constexpr (sizeof(__PRETTY_FUNCTION__) == sizeof(__FUNCTION__))
        {
          static_assert(always_false_v<E>, "magic_enum::detail::n requires __PRETTY_FUNCTION__.");
          return str_view {};
        }
        else
        {
          name.size_ = sizeof(__PRETTY_FUNCTION__) - 36;
          name.str_ = __PRETTY_FUNCTION__ + 34;
        }
#elif defined(__GNUC__)
        auto name = str_view {__PRETTY_FUNCTION__, sizeof(__PRETTY_FUNCTION__) - 1};
        if constexpr (sizeof(__PRETTY_FUNCTION__) == sizeof(__FUNCTION__))
        {
          static_assert(always_false_v<E>, "magic_enum::detail::n requires __PRETTY_FUNCTION__.");
          return str_view {};
        }
        else if (name.str_[name.size_ - 1] == ']')
        {
          name.size_ -= 50;
          name.str_ += 49;
        }
        else
        {
          name.size_ -= 40;
          name.str_ += 37;
        }
#elif defined(_MSC_VER)
        // CLI/C++ workaround (see https://github.com/Neargye/magic_enum/issues/284).
        str_view name;
        name.str_ = __FUNCSIG__;
        name.str_ += 40;
        name.size_ += sizeof(__FUNCSIG__) - 57;
#else
        auto name = str_view {};
#endif
        std::size_t p = 0;
        for (std::size_t i = name.size_; i > 0; --i)
        {
          if (name.str_[i] == ':')
          {
            p = i + 1;
            break;
          }
        }
        if (p > 0)
        {
          name.size_ -= p;
          name.str_ += p;
        }
        return name;
      }
      else
      {
        return str_view {}; // Unsupported compiler or Invalid customize.
      }
    }

    template <typename E>
    constexpr auto type_name() noexcept
    {
      [[maybe_unused]] constexpr auto custom = customize::enum_type_name<E>();
      static_assert(std::is_same_v<std::decay_t<decltype(custom)>, customize::customize_t>,
                    "magic_enum::customize requires customize_t type.");
      if constexpr (custom.first == customize::detail::customize_tag::custom_tag)
      {
        constexpr auto name = custom.second;
        static_assert(!name.empty(), "magic_enum::customize requires not empty string.");
        return static_str<name.size()> {name};
      }
      else if constexpr (custom.first == customize::detail::customize_tag::invalid_tag)
      {
        return static_str<0> {};
      }
      else if constexpr (custom.first == customize::detail::customize_tag::default_tag)
      {
        constexpr auto name = n<E>();
        return static_str<name.size_> {name};
      }
      else
      {
        static_assert(always_false_v<E>, "magic_enum::customize invalid.");
      }
    }

    template <typename E>
    constexpr auto type_name_v = type_name<E>();

    template <auto V>
    constexpr auto MAGIC_ENUM_CALLING_CONVENTION n() noexcept
    {
      static_assert(is_enum_v<decltype(V)>, "magic_enum::detail::n requires enum type.");

      if constexpr (supported<decltype(V)>::value)
      {
#if defined(MAGIC_ENUM_GET_ENUM_NAME_BUILTIN)
        constexpr auto name_ptr = MAGIC_ENUM_GET_ENUM_NAME_BUILTIN(V);
        auto name = name_ptr ? str_view {name_ptr, std::char_traits<char>::length(name_ptr)} : str_view {};
#elif defined(__clang__)
        str_view name;
        if constexpr (sizeof(__PRETTY_FUNCTION__) == sizeof(__FUNCTION__))
        {
          static_assert(always_false_v<decltype(V)>, "magic_enum::detail::n requires __PRETTY_FUNCTION__.");
          return str_view {};
        }
        else
        {
          name.size_ = sizeof(__PRETTY_FUNCTION__) - 36;
          name.str_ = __PRETTY_FUNCTION__ + 34;
        }
        if (name.size_ > 22 && name.str_[0] == '(' && name.str_[1] == 'a' && name.str_[10] == ' '
            && name.str_[22] == ':')
        {
          name.size_ -= 23;
          name.str_ += 23;
        }
        if (name.str_[0] == '(' || name.str_[0] == '-' || (name.str_[0] >= '0' && name.str_[0] <= '9'))
        {
          name = str_view {};
        }
#elif defined(__GNUC__)
        auto name = str_view {__PRETTY_FUNCTION__, sizeof(__PRETTY_FUNCTION__) - 1};
        if constexpr (sizeof(__PRETTY_FUNCTION__) == sizeof(__FUNCTION__))
        {
          static_assert(always_false_v<decltype(V)>, "magic_enum::detail::n requires __PRETTY_FUNCTION__.");
          return str_view {};
        }
        else if (name.str_[name.size_ - 1] == ']')
        {
          name.size_ -= 55;
          name.str_ += 54;
        }
        else
        {
          name.size_ -= 40;
          name.str_ += 37;
        }
        if (name.str_[0] == '(')
        {
          name = str_view {};
        }
#elif defined(_MSC_VER)
        str_view name;
        if ((__FUNCSIG__[5] == '_' && __FUNCSIG__[35] != '(')
            || (__FUNCSIG__[5] == 'c' && __FUNCSIG__[41] != '('))
        {
          // CLI/C++ workaround (see https://github.com/Neargye/magic_enum/issues/284).
          name.str_ = __FUNCSIG__;
          name.str_ += 35;
          name.size_ = sizeof(__FUNCSIG__) - 52;
        }
#else
        auto name = str_view {};
#endif
        std::size_t p = 0;
        for (std::size_t i = name.size_; i > 0; --i)
        {
          if (name.str_[i] == ':')
          {
            p = i + 1;
            break;
          }
        }
        if (p > 0)
        {
          name.size_ -= p;
          name.str_ += p;
        }
        return name;
      }
      else
      {
        return str_view {}; // Unsupported compiler or Invalid customize.
      }
    }

#if defined(_MSC_VER) && !defined(__clang__) && _MSC_VER < 1'920
  #define MAGIC_ENUM_VS_2017_WORKAROUND 1
#endif

#if defined(MAGIC_ENUM_VS_2017_WORKAROUND)
    template <typename E, E V>
    constexpr auto MAGIC_ENUM_CALLING_CONVENTION n() noexcept
    {
      static_assert(is_enum_v<E>, "magic_enum::detail::n requires enum type.");

  #if defined(MAGIC_ENUM_GET_ENUM_NAME_BUILTIN)
      constexpr auto name_ptr = MAGIC_ENUM_GET_ENUM_NAME_BUILTIN(V);
      auto name = name_ptr ? str_view {name_ptr, std::char_traits<char>::length(name_ptr)} : str_view {};
  #else
      // CLI/C++ workaround (see https://github.com/Neargye/magic_enum/issues/284).
      str_view name;
      name.str_ = __FUNCSIG__;
      name.size_ = sizeof(__FUNCSIG__) - 17;
      std::size_t p = 0;
      for (std::size_t i = name.size_; i > 0; --i)
      {
        if (name.str_[i] == ',' || name.str_[i] == ':')
        {
          p = i + 1;
          break;
        }
      }
      if (p > 0)
      {
        name.size_ -= p;
        name.str_ += p;
      }
      if (name.str_[0] == '(' || name.str_[0] == '-' || (name.str_[0] >= '0' && name.str_[0] <= '9'))
      {
        name = str_view {};
      }
      return name;
  #endif
    }
#endif

    template <typename E, E V>
    constexpr auto enum_name() noexcept
    {
      [[maybe_unused]] constexpr auto custom = customize::enum_name<E>(V);
      static_assert(std::is_same_v<std::decay_t<decltype(custom)>, customize::customize_t>,
                    "magic_enum::customize requires customize_t type.");
      if constexpr (custom.first == customize::detail::customize_tag::custom_tag)
      {
        constexpr auto name = custom.second;
        static_assert(!name.empty(), "magic_enum::customize requires not empty string.");
        return static_str<name.size()> {name};
      }
      else if constexpr (custom.first == customize::detail::customize_tag::invalid_tag)
      {
        return static_str<0> {};
      }
      else if constexpr (custom.first == customize::detail::customize_tag::default_tag)
      {
#if defined(MAGIC_ENUM_VS_2017_WORKAROUND)
        constexpr auto name = n<E, V>();
#else
        constexpr auto name = n<V>();
#endif
        return static_str<name.size_ - prefix_length_or_zero<E>> {name.str_ + prefix_length_or_zero<E>};
      }
      else
      {
        static_assert(always_false_v<E>, "magic_enum::customize invalid.");
      }
    }

    template <typename E, E V>
    constexpr auto enum_name_v = enum_name<E, V>();

// CWG1766: Values outside the range of the values of an enumeration
// https://reviews.llvm.org/D130058, https://reviews.llvm.org/D131307
#if defined(__clang__) && __clang_major__ >= 16
    template <typename E, auto V, typename = void>
    constexpr bool is_enum_constexpr_static_cast_valid = false;
    template <typename E, auto V>
    constexpr bool
      is_enum_constexpr_static_cast_valid<E, V, std::void_t<std::integral_constant<E, static_cast<E>(V)>>> =
        true;
#else
    template <typename, auto>
    constexpr bool is_enum_constexpr_static_cast_valid = true;
#endif

    template <typename E, auto V>
    constexpr bool is_valid() noexcept
    {
      if constexpr (is_enum_constexpr_static_cast_valid<E, V>)
      {
        constexpr E v = static_cast<E>(V);
        [[maybe_unused]] constexpr auto custom = customize::enum_name<E>(v);
        static_assert(std::is_same_v<std::decay_t<decltype(custom)>, customize::customize_t>,
                      "magic_enum::customize requires customize_t type.");
        if constexpr (custom.first == customize::detail::customize_tag::custom_tag)
        {
          constexpr auto name = custom.second;
          static_assert(!name.empty(), "magic_enum::customize requires not empty string.");
          return name.size() != 0;
        }
        else if constexpr (custom.first == customize::detail::customize_tag::default_tag)
        {
#if defined(MAGIC_ENUM_VS_2017_WORKAROUND)
          return n<E, v>().size_ != 0;
#else
          return n<v>().size_ != 0;
#endif
        }
        else
        {
          return false;
        }
      }
      else
      {
        return false;
      }
    }

    enum class enum_subtype
    {
      common,
      flags,
    };

    template <typename E, int O, enum_subtype S, typename U = std::underlying_type_t<E>>
    constexpr U ualue(std::size_t i) noexcept
    {
      if constexpr (std::is_same_v<U, bool>)
      { // bool special case
        static_assert(O == 0, "magic_enum::detail::ualue requires valid offset.");

        return static_cast<U>(i);
      }
      else if constexpr (S == enum_subtype::flags)
      {
        return static_cast<U>(U {1} << static_cast<U>(static_cast<int>(i) + O));
      }
      else
      {
        return static_cast<U>(static_cast<int>(i) + O);
      }
    }

    template <typename E, int O, enum_subtype S, typename U = std::underlying_type_t<E>>
    constexpr E value(std::size_t i) noexcept
    {
      return static_cast<E>(ualue<E, O, S>(i));
    }

    template <typename E, enum_subtype S, typename U = std::underlying_type_t<E>>
    constexpr int reflected_min() noexcept
    {
      if constexpr (S == enum_subtype::flags)
      {
        return 0;
      }
      else
      {
        constexpr auto lhs = range_min<E>::value;
        constexpr auto rhs = (std::numeric_limits<U>::min)();

        if constexpr (cmp_less(rhs, lhs))
        {
          return lhs;
        }
        else
        {
          return rhs;
        }
      }
    }

    template <typename E, enum_subtype S, typename U = std::underlying_type_t<E>>
    constexpr int reflected_max() noexcept
    {
      if constexpr (S == enum_subtype::flags)
      {
        return std::numeric_limits<U>::digits - 1;
      }
      else
      {
        constexpr auto lhs = range_max<E>::value;
        constexpr auto rhs = (std::numeric_limits<U>::max)();

        if constexpr (cmp_less(lhs, rhs))
        {
          return lhs;
        }
        else
        {
          return rhs;
        }
      }
    }

    // clang-format off
#define MAGIC_ENUM_FOR_EACH_256(T)                                                                                \
  T(0) T(1) T(2) T(3) T(4) T(5) T(6) T(7) T(8) T(9) T(10) T(11) T(12) T(13) T(14) T(15)                           \
  T(16) T(17) T(18) T(19) T(20) T(21) T(22) T(23) T(24) T(25) T(26) T(27) T(28) T(29) T(30) T(31)                 \
  T(32) T(33) T(34) T(35) T(36) T(37) T(38) T(39) T(40) T(41) T(42) T(43) T(44) T(45) T(46) T(47)                 \
  T(48) T(49) T(50) T(51) T(52) T(53) T(54) T(55) T(56) T(57) T(58) T(59) T(60) T(61) T(62) T(63)                 \
  T(64) T(65) T(66) T(67) T(68) T(69) T(70) T(71) T(72) T(73) T(74) T(75) T(76) T(77) T(78) T(79)                 \
  T(80) T(81) T(82) T(83) T(84) T(85) T(86) T(87) T(88) T(89) T(90) T(91) T(92) T(93) T(94) T(95)                 \
  T(96) T(97) T(98) T(99) T(100) T(101) T(102) T(103) T(104) T(105) T(106) T(107) T(108) T(109) T(110) T(111)     \
  T(112) T(113) T(114) T(115) T(116) T(117) T(118) T(119) T(120) T(121) T(122) T(123) T(124) T(125) T(126) T(127) \
  T(128) T(129) T(130) T(131) T(132) T(133) T(134) T(135) T(136) T(137) T(138) T(139) T(140) T(141) T(142) T(143) \
  T(144) T(145) T(146) T(147) T(148) T(149) T(150) T(151) T(152) T(153) T(154) T(155) T(156) T(157) T(158) T(159) \
  T(160) T(161) T(162) T(163) T(164) T(165) T(166) T(167) T(168) T(169) T(170) T(171) T(172) T(173) T(174) T(175) \
  T(176) T(177) T(178) T(179) T(180) T(181) T(182) T(183) T(184) T(185) T(186) T(187) T(188) T(189) T(190) T(191) \
  T(192) T(193) T(194) T(195) T(196) T(197) T(198) T(199) T(200) T(201) T(202) T(203) T(204) T(205) T(206) T(207) \
  T(208) T(209) T(210) T(211) T(212) T(213) T(214) T(215) T(216) T(217) T(218) T(219) T(220) T(221) T(222) T(223) \
  T(224) T(225) T(226) T(227) T(228) T(229) T(230) T(231) T(232) T(233) T(234) T(235) T(236) T(237) T(238) T(239) \
  T(240) T(241) T(242) T(243) T(244) T(245) T(246) T(247) T(248) T(249) T(250) T(251) T(252) T(253) T(254) T(255)
    // clang-format on

    template <std::size_t N>
    struct valid_count_t
    {
      std::uint16_t count = 0;
      std::uint16_t offsets[N] = {};

      constexpr void set(std::size_t i) noexcept
      {
        offsets[count++] = static_cast<std::uint16_t>(i);
      }
    };

    template <typename E, enum_subtype S, std::size_t Size, int Min, std::size_t I>
    constexpr void valid_count(valid_count_t<Size> &vc) noexcept
    {
#define MAGIC_ENUM_V(O)                                                                                      \
  if constexpr ((I + O) < Size)                                                                              \
  {                                                                                                          \
    if constexpr (is_valid<E, ualue<E, Min, S>(I + O)>())                                                    \
    {                                                                                                        \
      vc.set(I + O);                                                                                         \
    }                                                                                                        \
  }

      MAGIC_ENUM_FOR_EACH_256(MAGIC_ENUM_V)

      if constexpr ((I + 256) < Size)
      {
        valid_count<E, S, Size, Min, I + 256>(vc);
      }
#undef MAGIC_ENUM_V
    }

    template <typename E, enum_subtype S, std::size_t Size, int Min>
    constexpr auto valid_count() noexcept
    {
      valid_count_t<Size> vc;
      valid_count<E, S, Size, Min, 0>(vc);
      return vc;
    }

    template <typename E, enum_subtype S, std::size_t Size, int Min>
    constexpr auto values() noexcept
    {
      constexpr auto vc = valid_count<E, S, Size, Min>();

      if constexpr (vc.count > 0)
      {
#if defined(MAGIC_ENUM_ARRAY_CONSTEXPR)
        std::array<E, vc.count> values = {};
#else
        E values[vc.count] = {};
#endif
        if constexpr (vc.count == Size)
        {
          for (std::size_t i = 0; i < vc.count; ++i)
          {
            values[i] = value<E, Min, S>(i);
          }
        }
        else
        {
          for (std::size_t i = 0; i < vc.count; ++i)
          {
            values[i] = value<E, Min, S>(vc.offsets[i]);
          }
        }
#if defined(MAGIC_ENUM_ARRAY_CONSTEXPR)
        return values;
#else
        return to_array(values, std::make_index_sequence<vc.count> {});
#endif
      }
      else
      {
        return std::array<E, 0> {};
      }
    }

    template <typename E, enum_subtype S, typename U = std::underlying_type_t<E>>
    constexpr auto values() noexcept
    {
      constexpr auto min = reflected_min<E, S>();
      constexpr auto max = reflected_max<E, S>();
      constexpr auto range_size = max - min + 1;
      static_assert(range_size > 0, "magic_enum::enum_range requires valid size.");
      static_assert(range_size <= (std::numeric_limits<std::uint16_t>::max)(),
                    "magic_enum::enum_range requires valid size.");

      return values<E, S, range_size, min>();
    }

    template <typename E, typename U = std::underlying_type_t<E>>
    constexpr enum_subtype subtype(std::true_type) noexcept
    {
      if constexpr (std::is_same_v<U, bool>)
      { // bool special case
        return enum_subtype::common;
      }
      else if constexpr (has_is_flags<E>::value)
      {
        return customize::enum_range<E>::is_flags ? enum_subtype::flags : enum_subtype::common;
      }
      else
      {
        return enum_subtype::common;
      }
    }

    template <typename T>
    constexpr enum_subtype subtype(std::false_type) noexcept
    {
      // For non-enum type return default common subtype.
      return enum_subtype::common;
    }

    template <typename E, typename D = std::decay_t<E>>
    constexpr auto subtype_v = subtype<D>(std::is_enum<D> {});

    template <typename E, enum_subtype S>
    constexpr auto values_v = values<E, S>();

    template <typename E, enum_subtype S, typename D = std::decay_t<E>>
    using values_t = decltype((values_v<D, S>));

    template <typename E, enum_subtype S>
    constexpr auto count_v = values_v<E, S>.size();

    template <typename E, enum_subtype S, typename U = std::underlying_type_t<E>>
    constexpr auto min_v = (count_v<E, S> > 0) ? static_cast<U>(values_v<E, S>.front()) : U {0};

    template <typename E, enum_subtype S, typename U = std::underlying_type_t<E>>
    constexpr auto max_v = (count_v<E, S> > 0) ? static_cast<U>(values_v<E, S>.back()) : U {0};

    template <typename E, enum_subtype S, std::size_t... I>
    constexpr auto names(std::index_sequence<I...>) noexcept
    {
      constexpr auto names =
        std::array<string_view, sizeof...(I)> {{enum_name_v<E, values_v<E, S>[I]>.str()...}};
      return names;
    }

    template <typename E, enum_subtype S>
    constexpr auto names_v = names<E, S>(std::make_index_sequence<count_v<E, S>> {});

    template <typename E, enum_subtype S, typename D = std::decay_t<E>>
    using names_t = decltype((names_v<D, S>));

    template <typename E, enum_subtype S, std::size_t... I>
    constexpr auto entries(std::index_sequence<I...>) noexcept
    {
      constexpr auto entries = std::array<std::pair<E, string_view>, sizeof...(I)> {
        {{values_v<E, S>[I], enum_name_v<E, values_v<E, S>[I]>.str()}...}};
      return entries;
    }

    template <typename E, enum_subtype S>
    constexpr auto entries_v = entries<E, S>(std::make_index_sequence<count_v<E, S>> {});

    template <typename E, enum_subtype S, typename D = std::decay_t<E>>
    using entries_t = decltype((entries_v<D, S>));

    template <typename E, enum_subtype S, typename U = std::underlying_type_t<E>>
    constexpr bool is_sparse() noexcept
    {
      if constexpr (count_v<E, S> == 0)
      {
        return false;
      }
      else if constexpr (std::is_same_v<U, bool>)
      { // bool special case
        return false;
      }
      else
      {
        constexpr auto max = (S == enum_subtype::flags) ? log2(max_v<E, S>) : max_v<E, S>;
        constexpr auto min = (S == enum_subtype::flags) ? log2(min_v<E, S>) : min_v<E, S>;
        constexpr auto range_size = max - min + 1;

        return range_size != count_v<E, S>;
      }
    }

    template <typename E, enum_subtype S = subtype_v<E>>
    constexpr bool is_sparse_v = is_sparse<E, S>();

    template <typename E, enum_subtype S>
    struct is_reflected
#if defined(MAGIC_ENUM_NO_CHECK_REFLECTED_ENUM)
        : std::true_type
    {
    };
#else
        : std::bool_constant<std::is_enum_v<E> && (count_v<E, S> != 0)>
    {
    };
#endif

    template <typename E, enum_subtype S>
    constexpr bool is_reflected_v = is_reflected<std::decay_t<E>, S> {};

    template <bool, typename R>
    struct enable_if_enum
    {
    };

    template <typename R>
    struct enable_if_enum<true, R>
    {
      using type = R;
      static_assert(
        supported<R>::value,
        "magic_enum unsupported compiler (https://github.com/Neargye/magic_enum#compiler-compatibility).");
    };

    template <typename T, typename R, typename BinaryPredicate = std::equal_to<>,
              typename D = std::decay_t<T>>
    using enable_if_t = typename enable_if_enum<
      std::is_enum_v<D> && std::is_invocable_r_v<bool, BinaryPredicate, char_type, char_type>, R>::type;

    template <typename T, std::enable_if_t<std::is_enum_v<std::decay_t<T>>, int> = 0>
    using enum_concept = T;

    template <typename T, bool = std::is_enum_v<T>>
    struct is_scoped_enum : std::false_type
    {
    };

    template <typename T>
    struct is_scoped_enum<T, true> : std::bool_constant<!std::is_convertible_v<T, std::underlying_type_t<T>>>
    {
    };

    template <typename T, bool = std::is_enum_v<T>>
    struct is_unscoped_enum : std::false_type
    {
    };

    template <typename T>
    struct is_unscoped_enum<T, true> : std::bool_constant<std::is_convertible_v<T, std::underlying_type_t<T>>>
    {
    };

    template <typename T, bool = std::is_enum_v<T>>
    struct is_flags_enum : std::false_type
    {
    };

    template <typename T>
    struct is_flags_enum<T, true> : std::bool_constant<subtype_v<T> == enum_subtype::flags>
    {
    };

    template <typename T, bool = std::is_enum_v<std::decay_t<T>>>
    struct underlying_type
    {
    };

    template <typename T>
    struct underlying_type<T, true> : std::underlying_type<std::decay_t<T>>
    {
    };

#if defined(MAGIC_ENUM_ENABLE_HASH) || defined(MAGIC_ENUM_ENABLE_HASH_SWITCH)

    template <typename Value, typename = void>
    struct constexpr_hash_t;

    template <typename Value>
    struct constexpr_hash_t<Value, std::enable_if_t<is_enum_v<Value>>>
    {
      constexpr auto operator()(Value value) const noexcept
      {
        using U = typename underlying_type<Value>::type;
        if constexpr (std::is_same_v<U, bool>)
        { // bool special case
          return static_cast<std::size_t>(value);
        }
        else
        {
          return static_cast<U>(value);
        }
      }

      using secondary_hash = constexpr_hash_t;
    };

    template <typename Value>
    struct constexpr_hash_t<Value, std::enable_if_t<std::is_same_v<Value, string_view>>>
    {
      constexpr static std::uint32_t crc_table[256] {
        0x00'00'00'00l, 0x77'07'30'96l, 0xEE'0E'61'2Cl, 0x99'09'51'BAl, 0x07'6D'C4'19l, 0x70'6A'F4'8Fl,
        0xE9'63'A5'35l, 0x9E'64'95'A3l, 0x0E'DB'88'32l, 0x79'DC'B8'A4l, 0xE0'D5'E9'1El, 0x97'D2'D9'88l,
        0x09'B6'4C'2Bl, 0x7E'B1'7C'BDl, 0xE7'B8'2D'07l, 0x90'BF'1D'91l, 0x1D'B7'10'64l, 0x6A'B0'20'F2l,
        0xF3'B9'71'48l, 0x84'BE'41'DEl, 0x1A'DA'D4'7Dl, 0x6D'DD'E4'EBl, 0xF4'D4'B5'51l, 0x83'D3'85'C7l,
        0x13'6C'98'56l, 0x64'6B'A8'C0l, 0xFD'62'F9'7Al, 0x8A'65'C9'ECl, 0x14'01'5C'4Fl, 0x63'06'6C'D9l,
        0xFA'0F'3D'63l, 0x8D'08'0D'F5l, 0x3B'6E'20'C8l, 0x4C'69'10'5El, 0xD5'60'41'E4l, 0xA2'67'71'72l,
        0x3C'03'E4'D1l, 0x4B'04'D4'47l, 0xD2'0D'85'FDl, 0xA5'0A'B5'6Bl, 0x35'B5'A8'FAl, 0x42'B2'98'6Cl,
        0xDB'BB'C9'D6l, 0xAC'BC'F9'40l, 0x32'D8'6C'E3l, 0x45'DF'5C'75l, 0xDC'D6'0D'CFl, 0xAB'D1'3D'59l,
        0x26'D9'30'ACl, 0x51'DE'00'3Al, 0xC8'D7'51'80l, 0xBF'D0'61'16l, 0x21'B4'F4'B5l, 0x56'B3'C4'23l,
        0xCF'BA'95'99l, 0xB8'BD'A5'0Fl, 0x28'02'B8'9El, 0x5F'05'88'08l, 0xC6'0C'D9'B2l, 0xB1'0B'E9'24l,
        0x2F'6F'7C'87l, 0x58'68'4C'11l, 0xC1'61'1D'ABl, 0xB6'66'2D'3Dl, 0x76'DC'41'90l, 0x01'DB'71'06l,
        0x98'D2'20'BCl, 0xEF'D5'10'2Al, 0x71'B1'85'89l, 0x06'B6'B5'1Fl, 0x9F'BF'E4'A5l, 0xE8'B8'D4'33l,
        0x78'07'C9'A2l, 0x0F'00'F9'34l, 0x96'09'A8'8El, 0xE1'0E'98'18l, 0x7F'6A'0D'BBl, 0x08'6D'3D'2Dl,
        0x91'64'6C'97l, 0xE6'63'5C'01l, 0x6B'6B'51'F4l, 0x1C'6C'61'62l, 0x85'65'30'D8l, 0xF2'62'00'4El,
        0x6C'06'95'EDl, 0x1B'01'A5'7Bl, 0x82'08'F4'C1l, 0xF5'0F'C4'57l, 0x65'B0'D9'C6l, 0x12'B7'E9'50l,
        0x8B'BE'B8'EAl, 0xFC'B9'88'7Cl, 0x62'DD'1D'DFl, 0x15'DA'2D'49l, 0x8C'D3'7C'F3l, 0xFB'D4'4C'65l,
        0x4D'B2'61'58l, 0x3A'B5'51'CEl, 0xA3'BC'00'74l, 0xD4'BB'30'E2l, 0x4A'DF'A5'41l, 0x3D'D8'95'D7l,
        0xA4'D1'C4'6Dl, 0xD3'D6'F4'FBl, 0x43'69'E9'6Al, 0x34'6E'D9'FCl, 0xAD'67'88'46l, 0xDA'60'B8'D0l,
        0x44'04'2D'73l, 0x33'03'1D'E5l, 0xAA'0A'4C'5Fl, 0xDD'0D'7C'C9l, 0x50'05'71'3Cl, 0x27'02'41'AAl,
        0xBE'0B'10'10l, 0xC9'0C'20'86l, 0x57'68'B5'25l, 0x20'6F'85'B3l, 0xB9'66'D4'09l, 0xCE'61'E4'9Fl,
        0x5E'DE'F9'0El, 0x29'D9'C9'98l, 0xB0'D0'98'22l, 0xC7'D7'A8'B4l, 0x59'B3'3D'17l, 0x2E'B4'0D'81l,
        0xB7'BD'5C'3Bl, 0xC0'BA'6C'ADl, 0xED'B8'83'20l, 0x9A'BF'B3'B6l, 0x03'B6'E2'0Cl, 0x74'B1'D2'9Al,
        0xEA'D5'47'39l, 0x9D'D2'77'AFl, 0x04'DB'26'15l, 0x73'DC'16'83l, 0xE3'63'0B'12l, 0x94'64'3B'84l,
        0x0D'6D'6A'3El, 0x7A'6A'5A'A8l, 0xE4'0E'CF'0Bl, 0x93'09'FF'9Dl, 0x0A'00'AE'27l, 0x7D'07'9E'B1l,
        0xF0'0F'93'44l, 0x87'08'A3'D2l, 0x1E'01'F2'68l, 0x69'06'C2'FEl, 0xF7'62'57'5Dl, 0x80'65'67'CBl,
        0x19'6C'36'71l, 0x6E'6B'06'E7l, 0xFE'D4'1B'76l, 0x89'D3'2B'E0l, 0x10'DA'7A'5Al, 0x67'DD'4A'CCl,
        0xF9'B9'DF'6Fl, 0x8E'BE'EF'F9l, 0x17'B7'BE'43l, 0x60'B0'8E'D5l, 0xD6'D6'A3'E8l, 0xA1'D1'93'7El,
        0x38'D8'C2'C4l, 0x4F'DF'F2'52l, 0xD1'BB'67'F1l, 0xA6'BC'57'67l, 0x3F'B5'06'DDl, 0x48'B2'36'4Bl,
        0xD8'0D'2B'DAl, 0xAF'0A'1B'4Cl, 0x36'03'4A'F6l, 0x41'04'7A'60l, 0xDF'60'EF'C3l, 0xA8'67'DF'55l,
        0x31'6E'8E'EFl, 0x46'69'BE'79l, 0xCB'61'B3'8Cl, 0xBC'66'83'1Al, 0x25'6F'D2'A0l, 0x52'68'E2'36l,
        0xCC'0C'77'95l, 0xBB'0B'47'03l, 0x22'02'16'B9l, 0x55'05'26'2Fl, 0xC5'BA'3B'BEl, 0xB2'BD'0B'28l,
        0x2B'B4'5A'92l, 0x5C'B3'6A'04l, 0xC2'D7'FF'A7l, 0xB5'D0'CF'31l, 0x2C'D9'9E'8Bl, 0x5B'DE'AE'1Dl,
        0x9B'64'C2'B0l, 0xEC'63'F2'26l, 0x75'6A'A3'9Cl, 0x02'6D'93'0Al, 0x9C'09'06'A9l, 0xEB'0E'36'3Fl,
        0x72'07'67'85l, 0x05'00'57'13l, 0x95'BF'4A'82l, 0xE2'B8'7A'14l, 0x7B'B1'2B'AEl, 0x0C'B6'1B'38l,
        0x92'D2'8E'9Bl, 0xE5'D5'BE'0Dl, 0x7C'DC'EF'B7l, 0x0B'DB'DF'21l, 0x86'D3'D2'D4l, 0xF1'D4'E2'42l,
        0x68'DD'B3'F8l, 0x1F'DA'83'6El, 0x81'BE'16'CDl, 0xF6'B9'26'5Bl, 0x6F'B0'77'E1l, 0x18'B7'47'77l,
        0x88'08'5A'E6l, 0xFF'0F'6A'70l, 0x66'06'3B'CAl, 0x11'01'0B'5Cl, 0x8F'65'9E'FFl, 0xF8'62'AE'69l,
        0x61'6B'FF'D3l, 0x16'6C'CF'45l, 0xA0'0A'E2'78l, 0xD7'0D'D2'EEl, 0x4E'04'83'54l, 0x39'03'B3'C2l,
        0xA7'67'26'61l, 0xD0'60'16'F7l, 0x49'69'47'4Dl, 0x3E'6E'77'DBl, 0xAE'D1'6A'4Al, 0xD9'D6'5A'DCl,
        0x40'DF'0B'66l, 0x37'D8'3B'F0l, 0xA9'BC'AE'53l, 0xDE'BB'9E'C5l, 0x47'B2'CF'7Fl, 0x30'B5'FF'E9l,
        0xBD'BD'F2'1Cl, 0xCA'BA'C2'8Al, 0x53'B3'93'30l, 0x24'B4'A3'A6l, 0xBA'D0'36'05l, 0xCD'D7'06'93l,
        0x54'DE'57'29l, 0x23'D9'67'BFl, 0xB3'66'7A'2El, 0xC4'61'4A'B8l, 0x5D'68'1B'02l, 0x2A'6F'2B'94l,
        0xB4'0B'BE'37l, 0xC3'0C'8E'A1l, 0x5A'05'DF'1Bl, 0x2D'02'EF'8Dl};

      constexpr std::uint32_t operator()(string_view value) const noexcept
      {
        auto crc = static_cast<std::uint32_t>(0xFF'FF'FF'FFl);
        for (const auto c : value)
        {
          crc = (crc >> 8) ^ crc_table[(crc ^ static_cast<std::uint32_t>(c)) & 0xFF];
        }
        return crc ^ 0xFF'FF'FF'FFl;
      }

      struct secondary_hash
      {
        constexpr std::uint32_t operator()(string_view value) const noexcept
        {
          auto acc = static_cast<std::uint64_t>(2'166'136'261ull);
          for (const auto c : value)
          {
            acc = ((acc ^ static_cast<std::uint64_t>(c)) * static_cast<std::uint64_t>(16'777'619ull))
                  & (std::numeric_limits<std::uint32_t>::max)();
          }
          return static_cast<std::uint32_t>(acc);
        }
      };
    };

    template <typename Hash>
    constexpr Hash hash_v {};

    template <auto *GlobValues, typename Hash>
    constexpr auto calculate_cases(std::size_t Page) noexcept
    {
      constexpr std::array values = *GlobValues;
      constexpr std::size_t size = values.size();

      using switch_t = std::invoke_result_t<Hash, typename decltype(values)::value_type>;
      static_assert(std::is_integral_v<switch_t> && !std::is_same_v<switch_t, bool>);
      const std::size_t values_to = (std::min)(static_cast<std::size_t>(256), size - Page);

      std::array<switch_t, 256> result {};
      auto fill = result.begin();
      {
        auto first = values.begin() + static_cast<std::ptrdiff_t>(Page);
        auto last = values.begin() + static_cast<std::ptrdiff_t>(Page + values_to);
        while (first != last)
        {
          *fill++ = hash_v<Hash>(*first++);
        }
      }

      // dead cases, try to avoid case collisions
      for (switch_t last_value = result[values_to - 1];
           fill != result.end() && last_value != (std::numeric_limits<switch_t>::max)();
           *fill++ = ++last_value)
      {
      }

      {
        auto it = result.begin();
        auto last_value = (std::numeric_limits<switch_t>::min)();
        for (; fill != result.end(); *fill++ = last_value++)
        {
          while (last_value == *it)
          {
            ++last_value, ++it;
          }
        }
      }

      return result;
    }

    template <typename R, typename F, typename... Args>
    constexpr R invoke_r(F &&f, Args &&...args) noexcept(std::is_nothrow_invocable_r_v<R, F, Args...>)
    {
      if constexpr (std::is_void_v<R>)
      {
        std::forward<F>(f)(std::forward<Args>(args)...);
      }
      else
      {
        return static_cast<R>(std::forward<F>(f)(std::forward<Args>(args)...));
      }
    }

    enum class case_call_t
    {
      index,
      value,
    };

    template <typename T = void>
    constexpr auto default_result_type_lambda = []() noexcept(std::is_nothrow_default_constructible_v<T>)
    {
      return T {};
    };

    template <>
constexpr auto default_result_type_lambda<void> = []() noexcept {};

    template <auto *Arr, typename Hash>
    constexpr bool has_duplicate() noexcept
    {
      using value_t = std::decay_t<decltype((*Arr)[0])>;
      using hash_value_t = std::invoke_result_t<Hash, value_t>;
      std::array<hash_value_t, Arr->size()> hashes {};
      std::size_t size = 0;
      for (auto elem : *Arr)
      {
        hashes[size] = hash_v<Hash>(elem);
        for (auto i = size++; i > 0; --i)
        {
          if (hashes[i] < hashes[i - 1])
          {
            auto tmp = hashes[i];
            hashes[i] = hashes[i - 1];
            hashes[i - 1] = tmp;
          }
          else if (hashes[i] == hashes[i - 1])
          {
            return false;
          }
          else
          {
            break;
          }
        }
      }
      return true;
    }

  #define MAGIC_ENUM_CASE(val)                                                                               \
    case cases[val]:                                                                                         \
      if constexpr ((val) + Page < size)                                                                     \
      {                                                                                                      \
        if (!pred(values[val + Page], searched))                                                             \
        {                                                                                                    \
          break;                                                                                             \
        }                                                                                                    \
        if constexpr (CallValue == case_call_t::index)                                                       \
        {                                                                                                    \
          if constexpr (std::is_invocable_r_v<result_t, Lambda,                                              \
                                              std::integral_constant<std::size_t, val + Page>>)              \
          {                                                                                                  \
            return detail::invoke_r<result_t>(std::forward<Lambda>(lambda),                                  \
                                              std::integral_constant<std::size_t, val + Page> {});           \
          }                                                                                                  \
          else if constexpr (std::is_invocable_v<Lambda, std::integral_constant<std::size_t, val + Page>>)   \
          {                                                                                                  \
            MAGIC_ENUM_ASSERT(false && "magic_enum::detail::constexpr_switch wrong result type.");           \
          }                                                                                                  \
        }                                                                                                    \
        else if constexpr (CallValue == case_call_t::value)                                                  \
        {                                                                                                    \
          if constexpr (std::is_invocable_r_v<result_t, Lambda, enum_constant<values[val + Page]>>)          \
          {                                                                                                  \
            return detail::invoke_r<result_t>(std::forward<Lambda>(lambda),                                  \
                                              enum_constant<values[val + Page]> {});                         \
          }                                                                                                  \
          else if constexpr (std::is_invocable_v<Lambda, enum_constant<values[val + Page]>>)                 \
          {                                                                                                  \
            MAGIC_ENUM_ASSERT(false && "magic_enum::detail::constexpr_switch wrong result type.");           \
          }                                                                                                  \
        }                                                                                                    \
        break;                                                                                               \
      }                                                                                                      \
      else                                                                                                   \
        [[fallthrough]];

    template <auto *GlobValues, case_call_t CallValue, std::size_t Page = 0,
              typename Hash = constexpr_hash_t<typename std::decay_t<decltype(*GlobValues)>::value_type>,
              typename BinaryPredicate = std::equal_to<>, typename Lambda, typename ResultGetterType>
    constexpr decltype(auto)
      constexpr_switch(Lambda &&lambda, typename std::decay_t<decltype(*GlobValues)>::value_type searched,
                       ResultGetterType &&def, BinaryPredicate &&pred = {})
    {
      using result_t = std::invoke_result_t<ResultGetterType>;
      using hash_t =
        std::conditional_t<has_duplicate<GlobValues, Hash>(), Hash, typename Hash::secondary_hash>;
      static_assert(has_duplicate<GlobValues, hash_t>(),
                    "magic_enum::detail::constexpr_switch duplicated hash found, please report it: "
                    "https://github.com/Neargye/magic_enum/issues.");
      constexpr std::array values = *GlobValues;
      constexpr std::size_t size = values.size();
      constexpr std::array cases = calculate_cases<GlobValues, hash_t>(Page);

      switch (hash_v<hash_t>(searched))
      {
        MAGIC_ENUM_FOR_EACH_256(MAGIC_ENUM_CASE)
        default:
          if constexpr (size > 256 + Page)
          {
            return constexpr_switch<GlobValues, CallValue, Page + 256, Hash>(
              std::forward<Lambda>(lambda), searched, std::forward<ResultGetterType>(def));
          }
          break;
      }
      return def();
    }

  #undef MAGIC_ENUM_CASE

#endif
  } // namespace magic_enum::detail

  // Checks is magic_enum supported compiler.
  constexpr bool is_magic_enum_supported = detail::supported<void>::value;

  template <typename T>
  using Enum = detail::enum_concept<T>;

  // Returns true if T is an Unscoped enumeration type.
  // Provides the member constant value which is equal to true, if T is an [Unscoped
  // enumeration](https://en.cppreference.com/w/cpp/language/enum#Unscoped_enumeration) type. Otherwise, value
  // is equal to false.
  template <typename T>
  struct is_unscoped_enum : detail::is_unscoped_enum<T>
  {
  };

  template <typename T>
  constexpr bool is_unscoped_enum_v = is_unscoped_enum<T>::value;

  // Returns true if T is a Scoped enumeration type.
  // Provides the member constant value which is equal to true, if T is an [Scoped
  // enumeration](https://en.cppreference.com/w/cpp/language/enum#Scoped_enumerations) type. Otherwise, value
  // is equal to false.
  template <typename T>
  struct is_scoped_enum : detail::is_scoped_enum<T>
  {
  };

  template <typename T>
  constexpr bool is_scoped_enum_v = is_scoped_enum<T>::value;

  // Returns true if T is a flags enumeration type (i.e., enum_range<T>::is_flags == true).
  template <typename T>
  struct is_flags_enum : detail::is_flags_enum<T>
  {
  };

  template <typename T>
  constexpr bool is_flags_v = is_flags_enum<T>::value;

  // If T is a complete enumeration type, provides a member typedef type that names the underlying type of T.
  // Otherwise, if T is not an enumeration type, there is no member type. Otherwise (T is an incomplete
  // enumeration type), the program is ill-formed.
  template <typename T>
  struct underlying_type : detail::underlying_type<T>
  {
  };

  template <typename T>
  using underlying_type_t = typename underlying_type<T>::type;

  template <auto V>
  using enum_constant = detail::enum_constant<V>;

  // Returns type name of enum.
  template <typename E>
  [[nodiscard]] constexpr auto enum_type_name() noexcept -> detail::enable_if_t<E, string_view>
  {
    constexpr string_view name = detail::type_name_v<std::decay_t<E>>.str();
    static_assert(!name.empty(), "magic_enum::enum_type_name enum type does not have a name.");

    return name;
  }

  // Returns number of enum values.
  template <typename E, detail::enum_subtype S = detail::subtype_v<E>>
  [[nodiscard]] constexpr auto enum_count() noexcept -> detail::enable_if_t<E, std::size_t>
  {
    return detail::count_v<std::decay_t<E>, S>;
  }

  // Returns enum value at specified index.
  // No bounds checking is performed: the behavior is undefined if index >= number of enum values.
  template <typename E, detail::enum_subtype S = detail::subtype_v<E>>
  [[nodiscard]] constexpr auto enum_value(std::size_t index) noexcept
    -> detail::enable_if_t<E, std::decay_t<E>>
  {
    using D = std::decay_t<E>;
    static_assert(detail::is_reflected_v<D, S>,
                  "magic_enum requires enum implementation and valid max and min.");

    if constexpr (detail::is_sparse_v<D, S>)
    {
      return MAGIC_ENUM_ASSERT(index < detail::count_v<D, S>), detail::values_v<D, S>[index];
    }
    else
    {
      constexpr auto min =
        (S == detail::enum_subtype::flags) ? detail::log2(detail::min_v<D, S>) : detail::min_v<D, S>;

      return MAGIC_ENUM_ASSERT(index < detail::count_v<D, S>), detail::value<D, min, S>(index);
    }
  }

  // Returns enum value at specified index.
  template <typename E, std::size_t I, detail::enum_subtype S = detail::subtype_v<E>>
  [[nodiscard]] constexpr auto enum_value() noexcept -> detail::enable_if_t<E, std::decay_t<E>>
  {
    using D = std::decay_t<E>;
    static_assert(detail::is_reflected_v<D, S>,
                  "magic_enum requires enum implementation and valid max and min.");
    static_assert(I < detail::count_v<D, S>, "magic_enum::enum_value out of range.");

    return enum_value<D, S>(I);
  }

  // Returns std::array with enum values, sorted by enum value.
  template <typename E, detail::enum_subtype S = detail::subtype_v<E>>
  [[nodiscard]] constexpr auto enum_values() noexcept -> detail::enable_if_t<E, detail::values_t<E, S>>
  {
    using D = std::decay_t<E>;
    static_assert(detail::is_reflected_v<D, S>,
                  "magic_enum requires enum implementation and valid max and min.");

    return detail::values_v<D, S>;
  }

  // Returns integer value from enum value.
  template <typename E>
  [[nodiscard]] constexpr auto enum_integer(E value) noexcept -> underlying_type_t<E>
  {
    return static_cast<underlying_type_t<E>>(value);
  }

  // Returns underlying value from enum value.
  template <typename E>
  [[nodiscard]] constexpr auto enum_underlying(E value) noexcept -> underlying_type_t<E>
  {
    return static_cast<underlying_type_t<E>>(value);
  }

  // Returns index in enum values from enum value.
  // Returns optional with index.
  template <typename E, detail::enum_subtype S = detail::subtype_v<E>>
  [[nodiscard]] constexpr auto enum_index(E value) noexcept -> detail::enable_if_t<E, optional<std::size_t>>
  {
    using D = std::decay_t<E>;
    using U = underlying_type_t<D>;
    static_assert(detail::is_reflected_v<D, S>,
                  "magic_enum requires enum implementation and valid max and min.");

    if constexpr (detail::is_sparse_v<D, S> || (S == detail::enum_subtype::flags))
    {
#if defined(MAGIC_ENUM_ENABLE_HASH)
      return detail::constexpr_switch<&detail::values_v<D, S>, detail::case_call_t::index>(
        [](std::size_t i) { return optional<std::size_t> {i}; }, value,
        detail::default_result_type_lambda<optional<std::size_t>>);
#else
      for (std::size_t i = 0; i < detail::count_v<D, S>; ++i)
      {
        if (enum_value<D, S>(i) == value)
        {
          return i;
        }
      }
      return {}; // Invalid value or out of range.
#endif
    }
    else
    {
      const auto v = static_cast<U>(value);
      if (v >= detail::min_v<D, S> && v <= detail::max_v<D, S>)
      {
        return static_cast<std::size_t>(v - detail::min_v<D, S>);
      }
      return {}; // Invalid value or out of range.
    }
  }

  // Returns index in enum values from enum value.
  // Returns optional with index.
  template <detail::enum_subtype S, typename E>
  [[nodiscard]] constexpr auto enum_index(E value) noexcept -> detail::enable_if_t<E, optional<std::size_t>>
  {
    using D = std::decay_t<E>;
    static_assert(detail::is_reflected_v<D, S>,
                  "magic_enum requires enum implementation and valid max and min.");

    return enum_index<D, S>(value);
  }

  // Returns index in enum values from static storage enum variable.
  template <auto V, detail::enum_subtype S = detail::subtype_v<std::decay_t<decltype(V)>>>
  [[nodiscard]] constexpr auto enum_index() noexcept -> detail::enable_if_t<decltype(V), std::size_t>
  {
    using D = std::decay_t<decltype(V)>;
    static_assert(detail::is_reflected_v<D, S>,
                  "magic_enum requires enum implementation and valid max and min.");
    constexpr auto index = enum_index<D, S>(V);
    static_assert(index, "magic_enum::enum_index enum value does not have a index.");

    return *index;
  }

  // Returns name from static storage enum variable.
  // This version is much lighter on the compile times and is not restricted to the enum_range limitation.
  template <auto V>
  [[nodiscard]] constexpr auto enum_name() noexcept -> detail::enable_if_t<decltype(V), string_view>
  {
    constexpr string_view name = detail::enum_name_v<std::decay_t<decltype(V)>, V>.str();
    static_assert(!name.empty(), "magic_enum::enum_name enum value does not have a name.");

    return name;
  }

  // Returns name from enum value.
  // If enum value does not have name or value out of range, returns empty string.
  template <typename E, detail::enum_subtype S = detail::subtype_v<E>>
  [[nodiscard]] constexpr auto enum_name(E value) noexcept -> detail::enable_if_t<E, string_view>
  {
    using D = std::decay_t<E>;
    static_assert(detail::is_reflected_v<D, S>,
                  "magic_enum requires enum implementation and valid max and min.");

    if (const auto i = enum_index<D, S>(value))
    {
      return detail::names_v<D, S>[*i];
    }
    return detail::static_str<0> {}.str();
  }

  // Returns name from enum value.
  // If enum value does not have name or value out of range, returns empty string.
  template <detail::enum_subtype S, typename E>
  [[nodiscard]] constexpr auto enum_name(E value) -> detail::enable_if_t<E, string_view>
  {
    using D = std::decay_t<E>;
    static_assert(detail::is_reflected_v<D, S>,
                  "magic_enum requires enum implementation and valid max and min.");

    return enum_name<D, S>(value);
  }

  // Returns std::array with names, sorted by enum value.
  template <typename E, detail::enum_subtype S = detail::subtype_v<E>>
  [[nodiscard]] constexpr auto enum_names() noexcept -> detail::enable_if_t<E, detail::names_t<E, S>>
  {
    using D = std::decay_t<E>;
    static_assert(detail::is_reflected_v<D, S>,
                  "magic_enum requires enum implementation and valid max and min.");

    return detail::names_v<D, S>;
  }

  // Returns std::array with pairs (value, name), sorted by enum value.
  template <typename E, detail::enum_subtype S = detail::subtype_v<E>>
  [[nodiscard]] constexpr auto enum_entries() noexcept -> detail::enable_if_t<E, detail::entries_t<E, S>>
  {
    using D = std::decay_t<E>;
    static_assert(detail::is_reflected_v<D, S>,
                  "magic_enum requires enum implementation and valid max and min.");

    return detail::entries_v<D, S>;
  }

  // Allows you to write magic_enum::enum_cast<foo>("bar", magic_enum::case_insensitive);
  constexpr auto case_insensitive = detail::case_insensitive<> {};

  // Returns enum value from integer value.
  // Returns optional with enum value.
  template <typename E, detail::enum_subtype S = detail::subtype_v<E>>
  [[nodiscard]] constexpr auto enum_cast(underlying_type_t<E> value) noexcept
    -> detail::enable_if_t<E, optional<std::decay_t<E>>>
  {
    using D = std::decay_t<E>;
    static_assert(detail::is_reflected_v<D, S>,
                  "magic_enum requires enum implementation and valid max and min.");

    if constexpr (detail::is_sparse_v<D, S> || (S == detail::enum_subtype::flags))
    {
#if defined(MAGIC_ENUM_ENABLE_HASH)
      return detail::constexpr_switch<&detail::values_v<D, S>, detail::case_call_t::value>(
        [](D v) { return optional<D> {v}; }, static_cast<D>(value),
        detail::default_result_type_lambda<optional<D>>);
#else
      for (std::size_t i = 0; i < detail::count_v<D, S>; ++i)
      {
        if (value == static_cast<underlying_type_t<D>>(enum_value<D, S>(i)))
        {
          return static_cast<D>(value);
        }
      }
      return {}; // Invalid value or out of range.
#endif
    }
    else
    {
      if (value >= detail::min_v<D, S> && value <= detail::max_v<D, S>)
      {
        return static_cast<D>(value);
      }
      return {}; // Invalid value or out of range.
    }
  }

  // Returns enum value from name.
  // Returns optional with enum value.
  template <typename E, detail::enum_subtype S = detail::subtype_v<E>,
            typename BinaryPredicate = std::equal_to<>>
  [[nodiscard]] constexpr auto enum_cast(string_view value, [[maybe_unused]] BinaryPredicate p = {}) noexcept(
    detail::is_nothrow_invocable_v<BinaryPredicate>)
    -> detail::enable_if_t<E, optional<std::decay_t<E>>, BinaryPredicate>
  {
    using D = std::decay_t<E>;
    static_assert(detail::is_reflected_v<D, S>,
                  "magic_enum requires enum implementation and valid max and min.");

#if defined(MAGIC_ENUM_ENABLE_HASH)
    if constexpr (detail::is_default_predicate_v<BinaryPredicate>)
    {
      return detail::constexpr_switch<&detail::names_v<D, S>, detail::case_call_t::index>(
        [](std::size_t i) { return optional<D> {detail::values_v<D, S>[i]}; }, value,
        detail::default_result_type_lambda<optional<D>>,
        [&p](string_view lhs, string_view rhs) { return detail::cmp_equal(lhs, rhs, p); });
    }
#endif
    for (std::size_t i = 0; i < detail::count_v<D, S>; ++i)
    {
      if (detail::cmp_equal(value, detail::names_v<D, S>[i], p))
      {
        return enum_value<D, S>(i);
      }
    }
    return {}; // Invalid value or out of range.
  }

  // Returns true if enum contains value with such value.
  template <typename E, detail::enum_subtype S = detail::subtype_v<E>>
  [[nodiscard]] constexpr auto enum_contains(E value) noexcept -> detail::enable_if_t<E, bool>
  {
    using D = std::decay_t<E>;
    using U = underlying_type_t<D>;

    return static_cast<bool>(enum_cast<D, S>(static_cast<U>(value)));
  }

  // Returns true if enum contains value with such value.
  template <detail::enum_subtype S, typename E>
  [[nodiscard]] constexpr auto enum_contains(E value) noexcept -> detail::enable_if_t<E, bool>
  {
    using D = std::decay_t<E>;
    using U = underlying_type_t<D>;

    return static_cast<bool>(enum_cast<D, S>(static_cast<U>(value)));
  }

  // Returns true if enum contains value with such integer value.
  template <typename E, detail::enum_subtype S = detail::subtype_v<E>>
  [[nodiscard]] constexpr auto enum_contains(underlying_type_t<E> value) noexcept
    -> detail::enable_if_t<E, bool>
  {
    using D = std::decay_t<E>;

    return static_cast<bool>(enum_cast<D, S>(value));
  }

  // Returns true if enum contains enumerator with such name.
  template <typename E, detail::enum_subtype S = detail::subtype_v<E>,
            typename BinaryPredicate = std::equal_to<>>
  [[nodiscard]] constexpr auto enum_contains(string_view value, BinaryPredicate p = {}) noexcept(
    detail::is_nothrow_invocable_v<BinaryPredicate>) -> detail::enable_if_t<E, bool, BinaryPredicate>
  {
    using D = std::decay_t<E>;

    return static_cast<bool>(enum_cast<D, S>(value, std::move(p)));
  }

  // Returns true if the enum integer value is in the range of values that can be reflected.
  template <typename E, detail::enum_subtype S = detail::subtype_v<E>>
  [[nodiscard]] constexpr auto enum_reflected(underlying_type_t<E> value) noexcept
    -> detail::enable_if_t<E, bool>
  {
    using D = std::decay_t<E>;

    if constexpr (detail::is_reflected_v<D, S>)
    {
      constexpr auto min = detail::reflected_min<E, S>();
      constexpr auto max = detail::reflected_max<E, S>();
      return value >= min && value <= max;
    }
    else
    {
      return false;
    }
  }

  // Returns true if the enum value is in the range of values that can be reflected.
  template <typename E, detail::enum_subtype S = detail::subtype_v<E>>
  [[nodiscard]] constexpr auto enum_reflected(E value) noexcept -> detail::enable_if_t<E, bool>
  {
    using D = std::decay_t<E>;

    return enum_reflected<D, S>(static_cast<underlying_type_t<D>>(value));
  }

  // Returns true if the enum value is in the range of values that can be reflected.
  template <detail::enum_subtype S, typename E>
  [[nodiscard]] constexpr auto enum_reflected(E value) noexcept -> detail::enable_if_t<E, bool>
  {
    using D = std::decay_t<E>;

    return enum_reflected<D, S>(value);
  }

  template <bool AsFlags = true>
  constexpr auto as_flags = AsFlags ? detail::enum_subtype::flags : detail::enum_subtype::common;

  template <bool AsCommon = true>
  constexpr auto as_common = AsCommon ? detail::enum_subtype::common : detail::enum_subtype::flags;

  namespace bitwise_operators
  {
    template <typename E, detail::enable_if_t<E, int> = 0>
    constexpr E operator~(E rhs) noexcept
    {
      return static_cast<E>(~static_cast<underlying_type_t<E>>(rhs));
    }

    template <typename E, detail::enable_if_t<E, int> = 0>
    constexpr E operator|(E lhs, E rhs) noexcept
    {
      return static_cast<E>(static_cast<underlying_type_t<E>>(lhs) | static_cast<underlying_type_t<E>>(rhs));
    }

    template <typename E, detail::enable_if_t<E, int> = 0>
    constexpr E operator&(E lhs, E rhs) noexcept
    {
      return static_cast<E>(static_cast<underlying_type_t<E>>(lhs) & static_cast<underlying_type_t<E>>(rhs));
    }

    template <typename E, detail::enable_if_t<E, int> = 0>
    constexpr E operator^(E lhs, E rhs) noexcept
    {
      return static_cast<E>(static_cast<underlying_type_t<E>>(lhs) ^ static_cast<underlying_type_t<E>>(rhs));
    }

    template <typename E, detail::enable_if_t<E, int> = 0>
    constexpr E &operator|=(E &lhs, E rhs) noexcept
    {
      return lhs = (lhs | rhs);
    }

    template <typename E, detail::enable_if_t<E, int> = 0>
    constexpr E &operator&=(E &lhs, E rhs) noexcept
    {
      return lhs = (lhs & rhs);
    }

    template <typename E, detail::enable_if_t<E, int> = 0>
    constexpr E &operator^=(E &lhs, E rhs) noexcept
    {
      return lhs = (lhs ^ rhs);
    }
  } // namespace magic_enum::bitwise_operators
} // namespace magic_enum

#if defined(__clang__)
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#elif defined(_MSC_VER)
  #pragma warning(pop)
#endif

#undef MAGIC_ENUM_GET_ENUM_NAME_BUILTIN
#undef MAGIC_ENUM_GET_TYPE_NAME_BUILTIN
#undef MAGIC_ENUM_VS_2017_WORKAROUND
#undef MAGIC_ENUM_ARRAY_CONSTEXPR
#undef MAGIC_ENUM_FOR_EACH_256

#endif // NEARGYE_MAGIC_ENUM_HPP
