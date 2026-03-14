#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <utility>

namespace Krys
{
  /// @brief Gets the underlying integer value of an enumeration type.
  template <typename T>
  KRYS_NODISCARD constexpr auto ToUnderlying(T value) noexcept
  {
    return std::to_underlying<T>(value);
  }

  template <typename TMaybeEnum>
  inline constexpr auto AnyToUnderlying(TMaybeEnum value) noexcept
  {
    if constexpr (IsEnum<TMaybeEnum>)
    {
      return ToUnderlying(value);
    }
    else
    {
      static_assert(Integral<TMaybeEnum>);
      return value;
    }
  }

  template <typename TMaybeEnum>
  inline constexpr auto AnyToUnderlyingUnsigned(TMaybeEnum value) noexcept
  {
    auto val = ToUnderlying(value);
    return static_cast<make_unsigned_t<decltype(val)>>(val);
  }

  template <typename TIntegralish>
  inline constexpr auto AnyEnumOrCharToUnderlying(TIntegralish value) noexcept
  {
    if constexpr (SameType<TIntegralish, char>)
    {
      using TU = conditional_t<Signed<char>, std::int_least8_t, std::uint_least8_t>;
      return static_cast<TU>(value);
    }
    else if constexpr (SameType<TIntegralish, wchar>)
    {
      if constexpr (sizeof(wchar) <= sizeof(std::uint_least8_t))
      {
        using TU = std::conditional_t<Signed<wchar>, std::int_least8_t, std::uint_least8_t>;
        return static_cast<TU>(value);
      }
      else if constexpr (sizeof(wchar) <= sizeof(std::uint_least16_t))
      {
        using TU = std::conditional_t<Signed<wchar>, std::int_least16_t, std::uint_least16_t>;
        return static_cast<TU>(value);
      }
      else if constexpr (sizeof(wchar) <= sizeof(std::uint_least32_t))
      {
        using TU = std::conditional_t<Signed<wchar>, std::int_least32_t, std::uint_least32_t>;
        return static_cast<TU>(value);
      }
      else
      {
        static_assert(sizeof(wchar) <= sizeof(std::uint_least64_t),
                      "[ztd.idk] The size of wchar exceeds what is anticipated for "
                      "ztd::AnyEnumOrCharToUnderlying(...).");
        using TU = std::conditional_t<Signed<wchar>, std::int_least64_t, std::uint_least64_t>;
        return static_cast<TU>(value);
      }
    }
    else if constexpr (SameType<TIntegralish, char8_t>)
    {
      return static_cast<unsigned char>(value);
    }
    else if constexpr (SameType<TIntegralish, char16_t>)
    {
      return static_cast<uint_least16_t>(value);
    }
    else if constexpr (SameType<TIntegralish, char32_t>)
    {
      return static_cast<uint32>(value);
    }
    else
    {
      return AnyToUnderlying(value);
    }
  }

  template <typename TIntegralish>
  inline constexpr auto AnyEnumOrCharToUnderlyingUnsigned(TIntegralish value) noexcept
  {
    auto val = AnyEnumOrCharToUnderlying(value);
    return static_cast<make_unsigned_t<decltype(val)>>(val);
  }

  namespace Impl
  {
    template <typename TEnumish, typename = void>
    struct AnyToUnderlying
    {
      using type = TEnumish;
    };

    template <typename TEnumish>
    struct AnyToUnderlying<TEnumish, enable_if_t<IsEnum<TEnumish>>>
    {
      using type = std::underlying_type_t<TEnumish>;
    };
  }

  template <typename TEnumish>
  using any_to_underlying_t = typename Impl::AnyToUnderlying<TEnumish>::type;

  template <typename TEnumish>
  using any_enum_or_char_to_underlying_t = decltype(AnyEnumOrCharToUnderlying(std::declval<TEnumish>()));
}