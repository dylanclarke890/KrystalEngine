/*
 * Copyright (C) 2025 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include <bit>
#include <initializer_list>
#include <ranges>

namespace krys
{
  /// @brief A set of values of the enum @p E, one bit per value.
  ///
  /// Each enumerator is a bit index (counted from `MinEnumValue`, if declared), not a mask. The storage fits
  /// the enum's `MaxEnumValue` hook (see `Enum.hpp`), or is 64 bits without one; either way the range spans
  /// at most 64 values.
  template <typename E>
  class EnumSet
  {
    static_assert(IsEnum<E>, "E is not an enum type");

    constexpr static int64 MinValue = detail::MinEnumUnderlying<E>();

  public:
    KRYS_NODISCARD constexpr static size_t StorageSize() noexcept
    {
      if constexpr (HasMaxEnumValue<E>)
      {
        constexpr int64 bits = detail::MaxEnumUnderlying<E>() - MinValue + 1;
        static_assert(bits <= 64, "EnumSet supports a range of at most 64 enum values");

        // Bytes needed for the highest bit, rounded up to a power of two.
        return std::bit_ceil(static_cast<size_t>((bits - 1) / 8 + 1));
      }
      else
      {
        return 8uz;
      }
    }

    template <typename StorageType>
    class Iterator
    {
      friend EnumSet;

    private:
      StorageType _value;

      constexpr Iterator(StorageType value) : _value(value)
      {
      }

    public:
      KRYS_NODISCARD constexpr E operator*() const noexcept
      {
        return static_cast<E>(std::countr_zero(_value) + MinValue);
      }

      // Iterates from smallest to largest enum value by turning off the rightmost Set bit.
      constexpr Iterator &operator++() noexcept
      {
        _value &= _value - 1;
        return *this;
      }

      Iterator &operator++(int) = delete;

      constexpr bool operator==(const Iterator &) const noexcept = default;
    };

    using StorageType = unsigned_of_size_t<StorageSize()>;

    using value_type = E;
    using iterator = Iterator<StorageType>;

    constexpr EnumSet() noexcept = default;

    constexpr EnumSet(E e) noexcept
    {
      Set(e);
    }

    constexpr EnumSet(std::initializer_list<E> initializerList) noexcept
    {
      for (auto &e : initializerList)
      {
        Set(e);
      }
    }

    /// @brief A set of the elements of @p range.
    template <std::ranges::input_range TRange>
    constexpr EnumSet(std::from_range_t, TRange &&range) noexcept
    {
      for (E e : range)
      {
        Set(e);
      }
    }

    constexpr EnumSet(Maybe<E> optional) noexcept
    {
      if (optional)
      {
        Set(*optional);
      }
    }

    KRYS_NODISCARD constexpr static EnumSet FromRaw(StorageType rawValue) noexcept
    {
      return EnumSet(rawValue, FromRawValue);
    }

    KRYS_NODISCARD constexpr StorageType ToRaw() const noexcept
    {
      return _storage;
    }

    /// @brief The set's bits, for `FastHash` (which mixes them).
    KRYS_NODISCARD constexpr size_t Hash() const noexcept
    {
      return static_cast<size_t>(_storage);
    }

    KRYS_NODISCARD constexpr bool empty() const noexcept
    {
      return !_storage;
    }

    KRYS_NODISCARD constexpr size_t size() const noexcept
    {
      return std::popcount(_storage);
    }

    KRYS_NODISCARD constexpr iterator begin() const noexcept
    {
      return _storage;
    }

    KRYS_NODISCARD constexpr iterator end() const noexcept
    {
      return 0;
    }

    constexpr explicit operator bool() const noexcept
    {
      return !empty();
    }

    KRYS_NODISCARD constexpr bool contains(E e) const noexcept
    {
      return Get(e);
    }

    KRYS_NODISCARD constexpr bool ContainsAny(EnumSet other) const noexcept
    {
      return !!(*this & other);
    }

    KRYS_NODISCARD constexpr bool ContainsAll(EnumSet other) const noexcept
    {
      return (*this & other) == other;
    }

    KRYS_NODISCARD constexpr bool ContainsOnly(EnumSet other) const noexcept
    {
      return *this == (*this & other);
    }

    constexpr void Add(EnumSet other) noexcept
    {
      _storage |= other._storage;
    }

    constexpr void Remove(EnumSet optionSet) noexcept
    {
      _storage &= ~optionSet._storage;
    }

    constexpr void Set(EnumSet optionSet, bool value) noexcept
    {
      if (value)
      {
        Add(optionSet);
      }
      else
      {
        Remove(optionSet);
      }
    }

    KRYS_NODISCARD constexpr bool HasExactlyOneBitSet() const noexcept
    {
      auto storage = _storage;
      return storage && !(storage & (storage - 1));
    }

    KRYS_NODISCARD constexpr Maybe<E> ToSingleValue() const noexcept
    {
      return HasExactlyOneBitSet() ? Maybe<E>(static_cast<E>(std::countr_zero(_storage) + MinValue)) : null;
    }

    friend constexpr bool operator==(const EnumSet &, const EnumSet &) noexcept = default;

    friend constexpr EnumSet operator|(EnumSet lhs, EnumSet rhs) noexcept
    {
      return FromRaw(lhs._storage | rhs._storage);
    }

    constexpr EnumSet &operator|=(const EnumSet &other) noexcept
    {
      Add(other);
      return *this;
    }

    friend constexpr EnumSet operator&(EnumSet lhs, EnumSet rhs) noexcept
    {
      return FromRaw(lhs._storage & rhs._storage);
    }

    friend constexpr EnumSet operator-(EnumSet lhs, EnumSet rhs) noexcept
    {
      return FromRaw(lhs._storage & ~rhs._storage);
    }

    friend constexpr EnumSet operator^(EnumSet lhs, EnumSet rhs) noexcept
    {
      return FromRaw(lhs._storage ^ rhs._storage);
    }

  private:
    /// @brief The bit for @p e. Out of range, it's a compile error in constant evaluation (the shift
    /// overflows) and an assertion otherwise.
    KRYS_NODISCARD constexpr static StorageType Bit(E e) noexcept
    {
      const int64 index = static_cast<int64>(ToUnderlying(e)) - MinValue;
      if !consteval
      {
        krys_debug_assert(index >= 0 && index < static_cast<int64>(sizeof(StorageType) * 8));
      }
      return static_cast<StorageType>(static_cast<StorageType>(1) << index);
    }

    constexpr bool Get(E e) const noexcept
    {
      return _storage & Bit(e);
    }

    constexpr void Set(E e) noexcept
    {
      _storage |= Bit(e);
    }

    enum InitializationTag
    {
      FromRawValue,
    };

    constexpr EnumSet(StorageType rawValue, InitializationTag) noexcept : _storage(rawValue)
    {
    }

    StorageType _storage {0};
  };
}
