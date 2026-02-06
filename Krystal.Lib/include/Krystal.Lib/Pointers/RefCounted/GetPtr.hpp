#pragma once

#include <memory>

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
  inline T *getPtr(T *p)
  {
    return p;
  }

  template <typename T>
  struct IsSmartPtr
  {
    static constexpr bool value = false;
    static constexpr bool isNullable = true;
  };

  template <typename T>
  inline constexpr bool IsSmartPtrV = IsSmartPtr<T>::value;

  template <typename T>
  inline constexpr bool IsSmartPtrNullableV = IsSmartPtr<T>::isNullable;

  template <typename T>
  concept SmartPtr = IsSmartPtrV<T>;

  template <typename T>
  concept NullableSmartPtr = SmartPtr<T> && IsSmartPtrNullableV<T>;

  template <typename T>
  concept NonNullableSmartPtr = SmartPtr<T> && !IsSmartPtrNullableV<T>;

  template <typename T, bool isSmartPtr>
  struct GetPtrHelperBase;

  template <typename T>
  struct GetPtrHelperBase<T, false /* isSmartPtr */>
  {
    using PtrType = T *;
    using UnderlyingType = T;
    static T *getPtr(T &p)
    {
      return std::addressof(p);
    }
  };

  template <typename T>
  struct GetPtrHelperBase<T, true /* isSmartPtr */>
  {
    using PtrType = typename T::PtrType;
    using UnderlyingType = typename T::ValueType;
    static PtrType getPtr(const T &p)
    {
      return p.get();
    }
  };

  template <typename T>
  struct GetPtrHelper : GetPtrHelperBase<T, IsSmartPtr<T>::value>
  {
  };

  template <typename T>
  inline typename GetPtrHelper<T>::PtrType getPtr(T &p)
  {
    return GetPtrHelper<T>::getPtr(p);
  }

  template <typename T>
  inline typename GetPtrHelper<T>::PtrType getPtr(const T &p)
  {
    return GetPtrHelper<T>::getPtr(p);
  }

  // Explicit specialization for C++ standard library types.

  template <typename T, typename Deleter>
  struct IsSmartPtr<std::unique_ptr<T, Deleter>>
  {
    static constexpr bool value = true;
    static constexpr bool isNullable = true;
  };

  template <typename T, typename Deleter>
  struct GetPtrHelper<std::unique_ptr<T, Deleter>>
  {
    using PtrType = T *;
    using UnderlyingType = T;
    static T *getPtr(const std::unique_ptr<T, Deleter> &p)
    {
      return p.get();
    }
  };
} 
