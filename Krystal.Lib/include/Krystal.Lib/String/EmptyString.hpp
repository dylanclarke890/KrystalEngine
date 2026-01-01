#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/Config.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"

namespace Krys
{
  namespace Impl
  {
    using uchar_one_t = unsigned char[1];
    inline constexpr const uchar_one_t u_shim = {};

    using schar_one_t = signed char[1];
    inline constexpr const schar_one_t s_shim = {};
  }

  /// @brief Returns an array representing an empty c-style string
  /// @tparam TChar The character type.
  /// @return An empty c-string.
  template <typename TChar>
  inline constexpr decltype(auto) EmptyString() noexcept
  {
    static_assert(DependentFalse<TChar>, "unrecognized character type");
    return "";
  }

  /// @brief Returns an array representing an empty c-style string
  /// @return An empty c-string.
  template <>
  inline constexpr decltype(auto) EmptyString<char>() noexcept
  {
    return "";
  }

  /// @brief Returns an array representing an empty c-style string
  /// @return An empty c-string.
  template <>
  inline constexpr decltype(auto) EmptyString<unsigned char>() noexcept
  {
    return (Impl::u_shim);
  }

  /// @brief Returns an array representing an empty c-style string
  /// @return An empty c-string.
  template <>
  inline constexpr decltype(auto) EmptyString<signed char>() noexcept
  {
    return (Impl::s_shim);
  }

  /// @brief Returns an array representing an empty c-style string
  /// @return An empty c-string.
  template <>
  inline constexpr decltype(auto) EmptyString<wchar_t>() noexcept
  {
    return L"";
  }

#if KRYS_CONFIG(NATIVE_CHAR8_T)
  /// @brief Returns an array representing an empty c-style string
  /// @return An empty c-string.
  template <>
  inline constexpr decltype(auto) EmptyString<char8_t>() noexcept
  {
    return u8"";
  }
#endif

  /// @brief Returns an array representing an empty c-style string
  /// @return An empty c-string.
  template <>
  inline constexpr decltype(auto) EmptyString<char16_t>() noexcept
  {
    return u"";
  }

  /// @brief Returns an array representing an empty c-style string
  /// @return An empty c-string.
  template <>
  inline constexpr decltype(auto) EmptyString<char32_t>() noexcept
  {
    return U"";
  }
}
