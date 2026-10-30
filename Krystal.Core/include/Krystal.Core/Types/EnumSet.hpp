#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include <bit>
#include <initializer_list>

namespace krys
{
  /// EnumSet is a class that represents a Set of enumerators in a space-efficient manner when the highest
  /// value is less than 64 (powers of 2 are not required). If the enum has defined traits they will be used
  /// to compute the storage size. Otherwise 64 bits are used.
  template <typename E>
  class EnumSet
  {
    static_assert(IsEnum<E>, "E is not an enum type");

  public:
    KRYS_NODISCARD constexpr static size_t StorageSize() noexcept
    {
      if constexpr (EnumTraits<E>::DistinctValues > 0u)
      {
        static_assert(EnumTraits<E>::DistinctValues <= 64u, "EnumSet supports at most 64 enum values");

        // Bytes needed for the highest value's bit, rounded up to a power of two.
        return std::bit_ceil(((static_cast<size_t>(EnumTraits<E>::DistinctValues) - 1uz) >> 3) + 1uz);
      }

      return 8uz;
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
        return static_cast<E>(std::countr_zero(_value));
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

    KRYS_NODISCARD constexpr bool IsEmpty() const noexcept
    {
      return !_storage;
    }

    KRYS_NODISCARD constexpr size_t Size() const noexcept
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
      return !IsEmpty();
    }

    KRYS_NODISCARD constexpr bool Contains(E e) const noexcept
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
      return HasExactlyOneBitSet() ? Maybe<E>(static_cast<E>(std::countr_zero(_storage))) : null;
    }

    friend constexpr bool operator==(const EnumSet &, const EnumSet &) noexcept = default;

    constexpr friend EnumSet operator|(EnumSet lhs, EnumSet rhs) noexcept
    {
      return FromRaw(lhs._storage | rhs._storage);
    }

    constexpr EnumSet &operator|=(const EnumSet &other) noexcept
    {
      Add(other);
      return *this;
    }

    constexpr friend EnumSet operator&(EnumSet lhs, EnumSet rhs) noexcept
    {
      return FromRaw(lhs._storage & rhs._storage);
    }

    constexpr friend EnumSet operator-(EnumSet lhs, EnumSet rhs) noexcept
    {
      return FromRaw(lhs._storage & ~rhs._storage);
    }

    constexpr friend EnumSet operator^(EnumSet lhs, EnumSet rhs) noexcept
    {
      return FromRaw(lhs._storage ^ rhs._storage);
    }

  private:
    constexpr bool Get(E e) const noexcept
    {
      return _storage & (static_cast<StorageType>(1) << ToUnderlying(e));
    }

    constexpr void Set(E e) noexcept
    {
      _storage |= (static_cast<StorageType>(1) << ToUnderlying(e));
    }

    enum InitializationTag
    {
      FromRawValue
    };

    constexpr EnumSet(StorageType rawValue, InitializationTag) noexcept : _storage(rawValue)
    {
    }

    StorageType _storage {0};
  };
}