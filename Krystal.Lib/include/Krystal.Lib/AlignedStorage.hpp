#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys
{
  template <typename T, size_t Alignment = std::alignment_of_v<T>>
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