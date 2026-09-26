#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Detection/AddressSpaceBitSize.hpp"
#include "Krystal.Core/Detection/CPU.hpp"
#include "Krystal.Core/Detection/OS.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Utils/AllowCompactPointers.hpp"
#include "Krystal.Core/Utils/Tags.hpp"
#include "Krystal.Core/Utils/UnalignedAccess.hpp"
#include <bit>

namespace krys
{
  template <typename T>
  class Packed
  {
  private:
    Array<byte, sizeof(T)> _storage;

  public:
    constexpr static bool IsPackedType = true;

    constexpr Packed(const T &value) noexcept
    {
      UnalignedStore<T>(_storage.data(), value);
    }

    constexpr Packed() noexcept(NoThrowDefaultConstructible<T>) : Packed(T {})
    {
    }

    KRYS_NODISCARD constexpr T get() const noexcept
    {
      return UnalignedLoad<T>(_storage.data());
    }

    constexpr void set(const T &value) noexcept
    {
      UnalignedStore<T>(_storage.data(), value);
    }

    constexpr Packed<T> &operator=(const T &value) noexcept
    {
      set(value);
      return *this;
    }

    template <class U>
    constexpr T exchange(U &&newValue) noexcept
    {
      T oldValue = get();
      set(std::forward<U>(newValue));
      return oldValue;
    }

    constexpr void swap(Packed &other) noexcept
    {
      _storage.swap(other._storage);
    }

    constexpr void swap(T &t2) noexcept
    {
      T t1 = get();
      std::swap(t1, t2);
      set(t1);
    }

    template <typename Other>
    requires Other::IsPackedType
    constexpr void swap(Other &other) noexcept
    {
      T t1 = get();
      T t2 = other.get();

      set(t2);
      other.set(t1);
    }
  };

  // PackedAlignedPtr can take alignment parameter too. PackedAlignedPtr only uses this alignment information
  // if it is profitable: we use alignment information only when we can reduce the size of the storage.
  template <typename T, size_t AlignTo>
  class PackedAlignedPtr
  {
    static_assert(krys::AllowsCompactPointers<T *>());
    static_assert(std::has_single_bit(AlignTo), "Alignment needs to be power-of-two");

    constexpr static size_t EffectiveAddressWidth = KRYS_EFFECTIVE_ADDRESS_WIDTH;

    constexpr static auto CalcStorageSize = [](size_t value) constexpr -> size_t
    {
      size_t roundedToMultipleOf8 = (value + 8 - 1) & ~(8 - 1);
      return roundedToMultipleOf8 / 8uz;
    };

    constexpr static auto AsByteSpan = [](uintptr_t value) noexcept -> Span<const uint8_t>
    {
      return Span {reinterpret_cast<const uint8_t *>(&value), sizeof(value)};
    };

    constexpr static size_t AlignmentShiftSizeIfProfitable = std::countr_zero(AlignTo);

    constexpr static size_t StorageSizeWithoutAlignmentShift = CalcStorageSize(EffectiveAddressWidth);

    constexpr static size_t StorageSizeWithAlignmentShift =
      CalcStorageSize(EffectiveAddressWidth - AlignmentShiftSizeIfProfitable);

    constexpr static bool IsAlignmentShiftProfitable =
      StorageSizeWithoutAlignmentShift > StorageSizeWithAlignmentShift;

    constexpr static size_t AlignmentShiftSize =
      IsAlignmentShiftProfitable ? AlignmentShiftSizeIfProfitable : 0uz;

  public:
    constexpr static bool IsPackedType = true;
    constexpr static size_t Alignment = AlignTo;
    constexpr static size_t StorageSize = StorageSizeWithAlignmentShift;

  private:
    Array<byte, StorageSize> _storage;

    static_assert(StorageSize <= sizeof(uintptr_t));

  public:
    constexpr PackedAlignedPtr() noexcept : _storage()
    {
    }

    constexpr PackedAlignedPtr(std::nullptr_t) noexcept : _storage()
    {
    }

    constexpr PackedAlignedPtr(T *value) noexcept
    {
      set(value);
    }

    KRYS_NODISCARD constexpr T *get() const noexcept
    {
      uintptr_t value = 0;

#if KRYS_CPU(LITTLE_ENDIAN)
      std::ranges::copy(AsByteSpan(value), _storage.begin());
#else
      std::ranges::copy(AsByteSpan(value).last(StorageSize), _storage.begin());
#endif

      if constexpr (IsAlignmentShiftProfitable)
      {
        value <<= AlignmentShiftSize;
      }

#if KRYS_CPU(X86_64) && !(KRYS_OS(DARWIN) || KRYS_OS(LINUX) || KRYS_OS(WINDOWS))
      // The AMD specification requires that the most significant 16
      // bits of any virtual address, bits 48 through 63, must be
      // copies of bit 47 (in a manner akin to sign extension).
      //
      // The above-named OSes will never allocate user space addresses
      // with bit 47 set, thus are already in canonical form.
      //
      // Reference: https://en.wikipedia.org/wiki/X86-64#Virtual_address_space_details
      constexpr size_t shiftBits = (sizeof(uintptr_t) * 8uz) - EffectiveAddressWidth;
      value = (std::bit_cast<intptr_t>(value) << shiftBits) >> shiftBits;
#endif

      return std::bit_cast<T *>(value);
    }

    constexpr void set(T *passedValue) noexcept
    {
      uintptr_t value = std::bit_cast<uintptr_t>(passedValue);
      if constexpr (IsAlignmentShiftProfitable)
      {
        value >>= AlignmentShiftSize;
      }

#if KRYS_CPU(LITTLE_ENDIAN)
      std::ranges::copy(_storage, AsByteSpan(value).first(StorageSize));
#else
      std::ranges::copy(_storage, AsByteSpan(value).last(StorageSize));
#endif

      if (std::is_constant_evaluated())
      {
        static_assert(std::bit_cast<uintptr_t>(get()) == value);
      }
      else
      {
        krys_assert(std::bit_cast<uintptr_t>(get()) == value);
      }
    }

    constexpr void reset() noexcept
    {
      set(nullptr);
    }

    constexpr T *operator->() const noexcept
    {
      return get();
    }

    template <typename U = T>
    requires(!Void<U>)
    constexpr U &operator*() const noexcept
    {
      return *get();
    }

    KRYS_NODISCARD constexpr bool operator!() const noexcept
    {
      return !get();
    }

    // This conversion operator allows implicit conversion to bool but not to other integer types.
    typedef T *(PackedAlignedPtr::*UnspecifiedBoolType);
    constexpr operator UnspecifiedBoolType() const noexcept
    {
      return get() ? &PackedAlignedPtr::_storage : nullptr;
    }

    explicit constexpr operator bool() const noexcept
    {
      return get();
    }

    constexpr PackedAlignedPtr &operator=(T *value) noexcept
    {
      set(value);
      return *this;
    }

    template <class U>
    KRYS_NODISCARD constexpr T *exchange(U &&newValue) noexcept
    {
      T *oldValue = get();
      set(std::forward<U>(newValue));
      return oldValue;
    }

    constexpr void swap(std::nullptr_t) noexcept
    {
      clear();
    }

    constexpr void swap(PackedAlignedPtr &other) noexcept
    {
      _storage.swap(other._storage);
    }

    template <typename Other>
    requires Other::IsPackedType
    constexpr void swap(Other &other) noexcept
    {
      T *t1 = get();
      T *t2 = other.get();

      set(t2);
      other.set(t1);
    }

    constexpr void swap(T *t2) noexcept
    {
      T *t1 = get();
      std::swap(t1, t2);
      set(t1);
    }
  };

  template <typename T>
  class Packed<T *> : public PackedAlignedPtr<T, 1>
  {
  public:
    static_assert(krys::AllowsCompactPointers<T *>());

    using Base = PackedAlignedPtr<T, 1>;
    using Base::Base;

    // Hash table deleted values, which are only constructed and never copied or destroyed.
    constexpr Packed(HashTableDeletedValueTag)
        : Base(std::bit_cast<T *>(static_cast<uintptr_t>(Base::Alignment))) noexcept
    {
    }

    KRYS_NODISCARD constexpr bool IsHashTableDeletedValue() const noexcept
    {
      return Base::get() == std::bit_cast<T *>(static_cast<uintptr_t>(Base::Alignment));
    }
  };

  template <typename T>
  using PackedPtr = Packed<T *>;

  template <typename T>
  constexpr bool IsSmartPtr<PackedPtr<T>> = true;

  template <typename T>
  constexpr bool IsNullableSmartPtr<PackedPtr<T>> = true;

  template <typename T, typename U>
  KRYS_NODISCARD constexpr bool operator==(const PackedPtr<T> &a, const PackedPtr<U> &b) noexcept
  {
    return a.get() == b.get();
  }

  template <typename T, typename U>
  KRYS_NODISCARD constexpr bool operator==(const PackedPtr<T> &a, U *b) noexcept
  {
    return a.get() == b;
  }

  template <typename T>
  struct PackedPtrTraits
  {
    template <typename U>
    using RebindTraits = PackedPtrTraits<U>;

    using StorageType = PackedPtr<T>;

    template <class U>
    KRYS_ALWAYS_INLINE KRYS_NODISCARD constexpr static T *exchange(StorageType &ptr, U &&newValue) noexcept
    {
      return ptr.exchange(newValue);
    }

    template <typename Other>
    KRYS_ALWAYS_INLINE constexpr static void swap(PackedPtr<T> &a, Other &b) noexcept
    {
      a.swap(b);
    }

    KRYS_ALWAYS_INLINE KRYS_NODISCARD constexpr static T *unwrap(const StorageType &ptr) noexcept
    {
      return ptr.get();
    }

    // We assume that,
    // 1. The alignment is < 4KB. (It is tested by HashTraits).
    // 2. The first page (including nullptr) is never mapped.
    KRYS_ALWAYS_INLINE constexpr static StorageType HashTableDeletedValue() noexcept
    {
      return StorageType {std::bit_cast<T *>(static_cast<uintptr_t>(StorageType::Alignment))};
    }

    KRYS_ALWAYS_INLINE constexpr static bool IsHashTableDeletedValue(const StorageType &ptr) noexcept
    {
      return ptr.get() == std::bit_cast<T *>(static_cast<uintptr_t>(StorageType::Alignment));
    }
  };

  template <typename P>
  struct DefaultHash<PackedPtr<P>> : PtrHash<PackedPtr<P>>
  {
  };
}
