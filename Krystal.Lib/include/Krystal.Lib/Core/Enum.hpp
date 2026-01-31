#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/StronglyTypedValue.hpp"
#include "Krystal.Lib/Utils/ToUnderlying.hpp"
#include <bit>
#include <utility>

namespace Krys
{
  template <typename TEnum>
  struct EnumTraits
  {
    /// @brief Total number of distinct values defined in the enum.
    constexpr static underlying_t<TEnum> DistinctValues = 0u;

    /// @brief True if the enum values are contiguous from 0 to the highest distinct value.
    constexpr static bool Contiguous = false;

    /// @brief True if the enum is used as a bitfield (flags), with each distinct value being a power of two.
    constexpr static bool BitwiseFlags = false;
  };

// Helper to define enum traits for enums.
#define KRYS_SPECIALIZE_ENUM_TRAITS_BEGIN(EnumName)                                                          \
  namespace Krys                                                                                             \
  {                                                                                                          \
    template <>                                                                                              \
    struct EnumTraits<EnumName>                                                                              \
    {
#define KRYS_SPECIALIZE_ENUM_TRAITS_END()                                                                    \
  }                                                                                                          \
  ;                                                                                                          \
  }

// Helper to define enum traits for contiguous enums.
#define KRYS_DEFINE_ENUM_TRAITS(EnumName, DistinctValuesCount, IsContiguous, IsBitwiseFlags)                 \
  KRYS_SPECIALIZE_ENUM_TRAITS_BEGIN(EnumName)                                                                \
    constexpr static underlying_t<EnumName> DistinctValues = DistinctValuesCount;                            \
    constexpr static bool Contiguous = IsContiguous;                                                         \
    constexpr static bool BitwiseFlags = IsBitwiseFlags;                                                     \
  KRYS_SPECIALIZE_ENUM_TRAITS_END()

// Helper to define enum traits for enums.
#define KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(EnumName, DistinctValuesCount)                                    \
  KRYS_DEFINE_ENUM_TRAITS(EnumName, DistinctValuesCount, true, false)

// Helper to define enum traits for bitwise flag enums.
#define KRYS_DEFINE_FLAGS_ENUM_TRAITS(EnumName, DistinctValuesCount)                                         \
  KRYS_DEFINE_ENUM_TRAITS(EnumName, DistinctValuesCount, true, true)

  template <typename TEnum>
  concept ContiguousEnum = IsEnum<TEnum> && EnumTraits<TEnum>::Contiguous;

  template <typename TEnum>
  concept BitwiseFlagsEnum = IsEnum<TEnum> && EnumTraits<TEnum>::BitwiseFlags;

  /// @brief Count of bits needed to store every distinct value of the enum.
  template <typename TEnum>
  KRYS_NODISCARD constexpr static uint32 BitCount() noexcept
  {
    using Traits = EnumTraits<TEnum>;
    static_assert(Traits::DistinctValues > 0u, "BitCount(): must have EnumTraits::DistinctValues > 0");
    static_assert(Traits::Contiguous, "BitCount(): must have EnumTraits::Contiguous = true");

    if constexpr (Traits::BitwiseFlags)
    {
      return std::bit_width(static_cast<underlying_t<TEnum>>((1u << (Traits::DistinctValues - 1u))));
    }
    else
    {
      return std::bit_width(static_cast<underlying_t<TEnum>>((Traits::DistinctValues - 1u)));
    }
  }

  template <BitwiseFlagsEnum TEnum>
  constexpr TEnum operator|(TEnum lhs, TEnum rhs) noexcept
  {
    using U = underlying_t<TEnum>;
    return static_cast<TEnum>(static_cast<U>(lhs) | static_cast<U>(rhs));
  }

  template <BitwiseFlagsEnum TEnum>
  constexpr TEnum &operator|=(TEnum &lhs, TEnum rhs) noexcept
  {
    lhs = lhs | rhs;
    return lhs;
  }

  template <BitwiseFlagsEnum TEnum>
  constexpr TEnum &operator|=(TEnum &lhs, TEnum rhs) noexcept
  {
    lhs = lhs | rhs;
    return lhs;
  }

  template <BitwiseFlagsEnum TEnum>
  constexpr TEnum operator&(TEnum lhs, TEnum rhs) noexcept
  {
    using U = underlying_t<TEnum>;
    return static_cast<TEnum>(static_cast<U>(lhs) & static_cast<U>(rhs));
  }

  template <BitwiseFlagsEnum TEnum>
  constexpr TEnum &operator&=(TEnum &lhs, TEnum rhs) noexcept
  {
    lhs = lhs & rhs;
    return lhs;
  }

  template <BitwiseFlagsEnum TEnum>
  constexpr TEnum operator~(TEnum value) noexcept
  {
    using U = underlying_t<TEnum>;
    return static_cast<TEnum>(~static_cast<U>(value));
  }

  template <BitwiseFlagsEnum TEnum>
  constexpr TEnum operator^(TEnum lhs, TEnum rhs) noexcept
  {
    using U = underlying_t<TEnum>;
    return static_cast<TEnum>(static_cast<U>(lhs) ^ static_cast<U>(rhs));
  }

  template <BitwiseFlagsEnum TEnum>
  constexpr TEnum &operator^=(TEnum &lhs, TEnum rhs) noexcept
  {
    lhs = lhs ^ rhs;
    return lhs;
  }

  template <BitwiseFlagsEnum TEnum>
  KRYS_NODISCARD constexpr bool operator!(TEnum value) noexcept
  {
    return ToUnderlying<TEnum>(value) == 0;
  }

  template <BitwiseFlagsEnum TEnum>
  KRYS_NODISCARD constexpr bool HasFlag(TEnum value, TEnum flag) noexcept
  {
    using U = underlying_t<TEnum>;
    return (static_cast<U>(value) & static_cast<U>(flag)) == static_cast<U>(flag);
  }
}