#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeTraits.hpp"

namespace krys
{
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
    using UnderlyingType = remove_ptr_t<PtrType>;

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