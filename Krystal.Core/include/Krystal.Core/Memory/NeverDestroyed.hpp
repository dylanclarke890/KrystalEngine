/*
 * Copyright (C) 2013-2019 Apple Inc. All rights reserved.
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
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Memory/AlignedStorage.hpp"
#include "Krystal.Core/Memory/ForbidHeapAllocation.hpp"
#include "Krystal.Core/Move.hpp"
#include "Krystal.Core/Threading/MainThread.hpp"

namespace krys
{
  struct AnyThreadsAccessTraits
  {
    static void AssertAccess() noexcept
    {
    }
  };

  struct MainThreadAccessTraits
  {
    static void AssertAccess() noexcept
    {
      krys_debug_assert(IsMainThread());
    }
  };

  template <typename T, typename AccessTraits = AnyThreadsAccessTraits>
  class NeverDestroyed
  {
    KRYS_FORBID_HEAP_ALLOCATION;

  private:
    AlignedStorage<T> _storage;

    using PointerType = remove_const_t<T> *;

    PointerType StoragePointer() const noexcept
    {
      AccessTraits::AssertAccess();
      return const_cast<PointerType>(_storage.get());
    }

  public:
    template <typename... Args>
    NeverDestroyed(Args &&...args) noexcept
    {
      AccessTraits::AssertAccess();
      new (StoragePointer()) T(std::forward<Args>(args)...);
    }

    NeverDestroyed(NeverDestroyed &&other) noexcept
    {
      AccessTraits::AssertAccess();
      new (StoragePointer()) T(krys::move(*other.StoragePointer()));
    }

    KRYS_NODISCARD operator T &() noexcept
    {
      return *StoragePointer();
    }

    KRYS_NODISCARD T &operator*() noexcept
    {
      return *StoragePointer();
    }

    KRYS_NODISCARD T *get() noexcept
    {
      return StoragePointer();
    }

    KRYS_NODISCARD T *operator->() noexcept
    {
      return StoragePointer();
    }

    KRYS_NODISCARD operator const T &() const noexcept
    {
      return *StoragePointer();
    }

    KRYS_NODISCARD const T &operator*() const noexcept
    {
      return *StoragePointer();
    }

    KRYS_NODISCARD const T *get() const noexcept
    {
      return StoragePointer();
    }

    KRYS_NODISCARD const T *operator->() const noexcept
    {
      return StoragePointer();
    }
  };

  template <typename T, typename AccessTraits = AnyThreadsAccessTraits>
  class LazyNeverDestroyed
  {
    KRYS_FORBID_HEAP_ALLOCATION;
    KRYS_NON_COPYABLE(LazyNeverDestroyed);

  private:
#if KRYS_DEBUG
    // LazyNeverDestroyed objects are always static, so this is zero-initialised to false. It must not be
    // initialised dynamically: a dynamic initialiser could run after another translation unit's
    // initialiser has already called Construct().
    bool _isConstructed;
#endif
    AlignedStorage<T> _storage;

    using PointerType = remove_const_t<T> *;

    PointerType StoragePointerWithoutAccessCheck() const noexcept
    {
      krys_debug_assert(_isConstructed);
      return const_cast<PointerType>(_storage.get());
    }

    PointerType StoragePointer() const noexcept
    {
      AccessTraits::AssertAccess();
      return StoragePointerWithoutAccessCheck();
    }

  public:
    LazyNeverDestroyed() noexcept = default;

    template <typename... Args>
    void Construct(Args &&...args) noexcept
    {
      AccessTraits::AssertAccess();
      ConstructWithoutAccessCheck(std::forward<Args>(args)...);
    }

    template <typename... Args>
    void ConstructWithoutAccessCheck(Args &&...args) noexcept
    {
#if KRYS_DEBUG
      krys_assert(!_isConstructed);
      _isConstructed = true;
#endif
      new (StoragePointerWithoutAccessCheck()) T(std::forward<Args>(args)...);
    }

#if KRYS_DEBUG
    KRYS_NODISCARD bool IsConstructed() const noexcept
    {
      return _isConstructed;
    }
#endif

    KRYS_NODISCARD operator T &() noexcept
    {
      return *StoragePointer();
    }

    KRYS_NODISCARD T &operator*() noexcept
    {
      return *StoragePointer();
    }

    KRYS_NODISCARD T *get() noexcept
    {
      return StoragePointer();
    }

    KRYS_NODISCARD T *operator->() noexcept
    {
      return StoragePointer();
    }

    KRYS_NODISCARD operator const T &() const noexcept
    {
      return *StoragePointer();
    }

    KRYS_NODISCARD const T &operator*() const noexcept
    {
      return *StoragePointer();
    }

    KRYS_NODISCARD const T *get() const noexcept
    {
      return StoragePointer();
    }

    KRYS_NODISCARD const T *operator->() const noexcept
    {
      return StoragePointer();
    }
  };

  template <typename T>
  using MainThreadNeverDestroyed = NeverDestroyed<T, MainThreadAccessTraits>;

  template <typename T>
  using MainThreadLazyNeverDestroyed = LazyNeverDestroyed<T, MainThreadAccessTraits>;
}
