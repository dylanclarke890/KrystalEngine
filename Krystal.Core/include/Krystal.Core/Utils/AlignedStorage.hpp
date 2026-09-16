#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/TypeTraits.hpp"

namespace krys
{
  template <typename T, size_t Alignment = AlignmentOf<T>>
  class AlignedStorage
  {
  private:
    struct alignas(Alignment) Storage
    {
      byte data[sizeof(T)];
    } _storage;

  public:
    AlignedStorage() noexcept = default;

    AlignedStorage(AlignedStorage &&) = delete;
    AlignedStorage &operator=(AlignedStorage &&) = delete;

    AlignedStorage(const AlignedStorage &) = delete;
    AlignedStorage &operator=(const AlignedStorage &) = delete;

    KRYS_NODISCARD T *get() noexcept
    {
      return reinterpret_cast<T *>(&_storage);
    }

    KRYS_NODISCARD const T *get() const noexcept
    {
      return reinterpret_cast<const T *>(&_storage);
    }

    KRYS_NODISCARD T &operator*() noexcept
    {
      return *get();
    }

    KRYS_NODISCARD T *operator->() noexcept
    {
      return get();
    }

    KRYS_NODISCARD const T &operator*() const noexcept
    {
      return *get();
    }

    KRYS_NODISCARD const T *operator->() const noexcept
    {
      return get();
    }
  };
}