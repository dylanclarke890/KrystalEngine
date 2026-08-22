#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys
{
  template <typename T>
  concept SupportsCheckedPtr = requires(T &value) {
    { value.AddRefChecked() } -> SameType<void>;
    { value.SubRefChecked() } -> SameType<void>;
    { value.GetRefCountChecked() } -> ConvertibleTo<uint32>;
  };

  template <typename T>
  concept SupportsRefPtr = requires(T &value) {
    { value.AddRef() } -> SameType<void>;
    { value.SubRef() } -> SameType<void>;
    { value.GetRefCount() } -> ConvertibleTo<uint32>;
  };

  template <typename T>
  constexpr bool IsSmartPtr = false;

  template <typename T>
  constexpr bool IsNullableSmartPtr = false;

  template <typename T, bool IsSmartPtr>
  struct GetPtrHelperBase;

  template <typename T>
  struct GetPtrHelperBase<T, false /* IsSmartPtr */>
  {
    using PtrType = T *;
    using UnderlyingType = T;

    KRYS_NODISCARD constexpr static PtrType get(T &value) noexcept
    {
      return std::addressof(value);
    }
  };

  template <typename T>
  struct GetPtrHelperBase<T, true /* IsSmartPtr */>
  {
    using PtrType = decltype(std::declval<T>().get());
    using UnderlyingType = remove_pointer_t<PtrType>;

    KRYS_NODISCARD constexpr static PtrType get(const T &value) noexcept
    {
      return value.get();
    }
  };

  template <typename T>
  struct GetPtrHelper : public GetPtrHelperBase<T, IsSmartPtr<T>>
  {
  };

  template <typename T>
  KRYS_NODISCARD constexpr typename GetPtrHelper<T>::PtrType GetPtr(T &p) noexcept
  {
    return GetPtrHelper<T>::get(p);
  }

  template <typename T>
  KRYS_NODISCARD constexpr typename GetPtrHelper<T>::PtrType GetPtr(const T &p) noexcept
  {
    return GetPtrHelper<T>::get(p);
  }
}