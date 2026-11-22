/*
 * Copyright (C) 2019 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Containers/Array.hpp"
#include "Krystal.Core/Detection/AddressSpaceBitSize.hpp"
#include "Krystal.Core/Detection/CPU.hpp"
#include "Krystal.Core/Detection/OS.hpp"
#include "Krystal.Core/Hashing/FastHash.hpp"
#include "Krystal.Core/Maths/Extras.hpp"
#include "Krystal.Core/Memory/UnalignedAccess.hpp"
#include "Krystal.Core/Utils/Tags.hpp"
#include <bit>
#include <cstring>

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
    static_assert(std::has_single_bit(AlignTo), "Alignment needs to be power-of-two");

    constexpr static size_t EffectiveAddressWidth = KRYS_EFFECTIVE_ADDRESS_WIDTH;

    constexpr static auto CalcStorageSize = [](size_t value) constexpr -> size_t
    {
      size_t roundedToMultipleOf8 = (value + 8 - 1) & ~(8 - 1);
      return roundedToMultipleOf8 / 8uz;
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

    PackedAlignedPtr(T *value) noexcept
    {
      set(value);
    }

    KRYS_NODISCARD T *get() const noexcept
    {
      uintptr_t value = 0;

#if KRYS_CPU(LITTLE_ENDIAN)
      std::memcpy(&value, _storage.data(), StorageSize);
#else
      std::memcpy(reinterpret_cast<byte *>(&value) + sizeof(value) - StorageSize, _storage.data(),
                  StorageSize);
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

    void set(T *passedValue) noexcept
    {
      uintptr_t value = std::bit_cast<uintptr_t>(passedValue);
      if constexpr (IsAlignmentShiftProfitable)
      {
        value >>= AlignmentShiftSize;
      }

#if KRYS_CPU(LITTLE_ENDIAN)
      std::memcpy(_storage.data(), &value, StorageSize);
#else
      std::memcpy(_storage.data(), reinterpret_cast<const byte *>(&value) + sizeof(value) - StorageSize,
                  StorageSize);
#endif

      krys_debug_assert(std::bit_cast<uintptr_t>(get()) == std::bit_cast<uintptr_t>(passedValue));
    }

    void clear() noexcept
    {
      set(nullptr);
    }

    T *operator->() const noexcept
    {
      return get();
    }

    template <typename U = T>
    requires(!Void<U>)
    U &operator*() const noexcept
    {
      return *get();
    }

    KRYS_NODISCARD bool operator!() const noexcept
    {
      return !get();
    }

    explicit operator bool() const noexcept
    {
      return get();
    }

    PackedAlignedPtr &operator=(T *value) noexcept
    {
      set(value);
      return *this;
    }

    template <class U>
    T *exchange(U &&newValue) noexcept
    {
      T *oldValue = get();
      set(std::forward<U>(newValue));
      return oldValue;
    }

    void swap(std::nullptr_t) noexcept
    {
      clear();
    }

    void swap(PackedAlignedPtr &other) noexcept
    {
      _storage.swap(other._storage);
    }

    template <typename Other>
    requires Other::IsPackedType
    void swap(Other &other) noexcept
    {
      T *t1 = get();
      T *t2 = other.get();

      set(t2);
      other.set(t1);
    }

    void swap(T *t2) noexcept
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
    using Base = PackedAlignedPtr<T, 1>;
    using Base::Base;
  };

  template <typename T>
  using PackedPtr = Packed<T *>;

  template <typename T>
  constexpr bool IsSmartPtr<PackedPtr<T>> = true;

  template <typename T>
  constexpr bool IsNullableSmartPtr<PackedPtr<T>> = true;

  template <typename T, typename U>
  KRYS_NODISCARD bool operator==(const PackedPtr<T> &a, const PackedPtr<U> &b) noexcept
  {
    return a.get() == b.get();
  }

  template <typename T, typename U>
  KRYS_NODISCARD bool operator==(const PackedPtr<T> &a, U *b) noexcept
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
    KRYS_ALWAYS_INLINE constexpr static T *exchange(StorageType &ptr, U &&newValue) noexcept
    {
      return ptr.exchange(std::forward<U>(newValue));
    }

    template <typename Other>
    KRYS_ALWAYS_INLINE constexpr static void swap(PackedPtr<T> &a, Other &b) noexcept
    {
      a.swap(b);
    }

    KRYS_NODISCARD KRYS_ALWAYS_INLINE constexpr static T *unwrap(const StorageType &ptr) noexcept
    {
      return ptr.get();
    }
  };
}
