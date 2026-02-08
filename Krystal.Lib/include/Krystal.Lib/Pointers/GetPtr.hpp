#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include <memory>

namespace Krys::detail
{
  template <typename T, bool IsSmartPtr>
  struct GetPtrHelperBase;

  template <typename T>
  struct GetPtrHelperBase<T, false /* IsSmartPtr */>
  {
    using pointer_type = RawPtr<T>;
    using underlying_type = T;

    KRYS_NODISCARD static pointer_type GetPtr(T &p) noexcept
    {
      return std::addressof(p);
    }
  };

  template <typename T>
  struct GetPtrHelperBase<T, true /* IsSmartPtr */>
  {
    using pointer_type = typename T::pointer_type;
    using underlying_type = typename T::underlying_type;

    KRYS_NODISCARD static pointer_type GetPtr(const T &p) noexcept
    {
      return p.get();
    }
  };
}

namespace Krys
{
  enum HashTableDeletedValueType
  {
    HashTableDeletedValue
  };

  enum HashTableEmptyValueType
  {
    HashTableEmptyValue
  };

  template <typename T>
  KRYS_NODISCARD inline RawPtr<T> GetPtr(RawPtr<T> p) noexcept
  {
    return p;
  }

  template <typename T>
  struct IsSmartPtr
  {
    static constexpr bool value = false;
    static constexpr bool nullable = true;
  };

  template <typename T>
  concept SmartPtr = IsSmartPtr<T>::value;

  template <typename T>
  concept NullableSmartPtr = SmartPtr<T> && IsSmartPtr<T>::nullable;

  template <typename T>
  concept NonNullableSmartPtr = SmartPtr<T> && IsSmartPtr<T>::nullable;

  template <typename T>
  struct GetPtrHelper : ::Krys::detail::GetPtrHelperBase<T, IsSmartPtr<T>::value>
  {
  };

  template <typename T>
  KRYS_NODISCARD inline typename GetPtrHelper<T>::pointer_type GetPtr(T &p) noexcept
  {
    return GetPtrHelper<T>::GetPtr(p);
  }

  template <typename T>
  KRYS_NODISCARD inline typename GetPtrHelper<T>::pointer_type GetPtr(const T &p) noexcept
  {
    return GetPtrHelper<T>::GetPtr(p);
  }

  // Explicit specialization for C++ standard library types.
  template <typename T, typename Deleter>
  struct IsSmartPtr<std::unique_ptr<T, Deleter>>
  {
    static constexpr bool value = true;
    static constexpr bool nullable = true;
  };

  template <typename T, typename Deleter>
  struct GetPtrHelper<std::unique_ptr<T, Deleter>>
  {
    using pointer_type = RawPtr<T>;
    using underlying_type = T;

    KRYS_NODISCARD static pointer_type GetPtr(const std::unique_ptr<T, Deleter> &p) noexcept
    {
      return p.get();
    }
  };
}
