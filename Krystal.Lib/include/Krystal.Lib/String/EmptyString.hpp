#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"

namespace Krys::detail
{
  using uchar_one_t = unsigned char[1];
  inline constexpr const uchar_one_t u_shim = {};

  using schar_one_t = signed char[1];
  inline constexpr const schar_one_t s_shim = {};
}

namespace Krys
{
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
    return (::Krys::detail::u_shim);
  }

  /// @brief Returns an array representing an empty c-style string
  /// @return An empty c-string.
  template <>
  inline constexpr decltype(auto) EmptyString<signed char>() noexcept
  {
    return (::Krys::detail::s_shim);
  }

  /// @brief Returns an array representing an empty c-style string
  /// @return An empty c-string.
  template <>
  inline constexpr decltype(auto) EmptyString<wchar_t>() noexcept
  {
    return L"";
  }

  /// @brief Returns an array representing an empty c-style string
  /// @return An empty c-string.
  template <>
  inline constexpr decltype(auto) EmptyString<char8_t>() noexcept
  {
    return u8"";
  }

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
