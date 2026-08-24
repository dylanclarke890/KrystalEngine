#pragma once

#include "Krystal.Lib/AlignedStorage.hpp"
#include "Krystal.Lib/Core/Move.hpp"
#include "Krystal.Lib/ForbidHeapAllocation.hpp"
#include <cassert>

namespace Krys
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
      // TODO: Implement a proper main thread check for the target platform.
      // assert(isMainThread());
    }
  };

  struct MainRunLoopAccessTraits
  {
    static void AssertAccess() noexcept
    {
      // TODO: Implement a proper main run loop check for the target platform.
      // assert(isMainRunLoop());
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
      new (StoragePointer()) T(Krys::Move(*other.StoragePointer()));
    }

    KRYS_NODISCARD operator T &() noexcept
    {
      return *StoragePointer();
    }

    KRYS_NODISCARD T &get() noexcept
    {
      return *StoragePointer();
    }

    KRYS_NODISCARD T *operator->() noexcept
    {
      return StoragePointer();
    }

    KRYS_NODISCARD operator const T &() const noexcept
    {
      return *StoragePointer();
    }

    KRYS_NODISCARD const T &get() const noexcept
    {
      return *StoragePointer();
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

  private:
    AlignedStorage<T> _storage;

    using PointerType = remove_const_t<T> *;

    PointerType StoragePointerWithoutAccessCheck() const noexcept
    {
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
      new (StoragePointerWithoutAccessCheck()) T(std::forward<Args>(args)...);
    }

    KRYS_NODISCARD operator T &() noexcept
    {
      return *StoragePointer();
    }

    KRYS_NODISCARD T &get() noexcept
    {
      return *StoragePointer();
    }

    KRYS_NODISCARD T *operator->() noexcept
    {
      return StoragePointer();
    }

    KRYS_NODISCARD operator const T &() const noexcept
    {
      return *StoragePointer();
    }

    KRYS_NODISCARD const T &get() const noexcept
    {
      return *StoragePointer();
    }

    KRYS_NODISCARD const T *operator->() const noexcept
    {
      return StoragePointer();
    }
  };

  template <typename T>
  using MainThreadNeverDestroyed = NeverDestroyed<T, MainThreadAccessTraits>;

  template <typename T>
  using MainRunLoopNeverDestroyed = NeverDestroyed<T, MainRunLoopAccessTraits>;

  template <typename T>
  using MainThreadLazyNeverDestroyed = LazyNeverDestroyed<T, MainThreadAccessTraits>;

  template <typename T>
  using MainRunLoopLazyNeverDestroyed = LazyNeverDestroyed<T, MainRunLoopAccessTraits>;
}