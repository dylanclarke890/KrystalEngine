#pragma once

#include "Krystal.Lib/Pointers/IntrusivePtr.hpp"

namespace Krys
{
  template <class T>
  using RefPtr = IntrusivePtr<T, typename T::RefPtrTraits>;

  template <class T>
  constexpr RefPtr<T> RefPtrRetain(T *ptr) noexcept
  {
    return RefPtr<T>::Ref(ptr);
  }

  template <class T>
  constexpr RefPtr<T> RefPtrAttach(T *ptr) noexcept
  {
    return RefPtr<T>::NoRef(ptr);
  }

  template <class T, class... Args>
  inline RefPtr<T> CreateRefPtr(Args &&...args)
  {
    return RefPtr<T>::NoRef(new T(std::forward<Args>(args)...));
  }

  template <class T>
  inline RefPtr<typename T::weak_value_type> weak_cast(const RefPtr<T> &src)
  {
    return src->GetWeakPtr();
  }

  template <class T>
  inline RefPtr<const typename T::weak_value_type> weak_cast(const RefPtr<const T> &src)
  {
    return src->GetWeakPtr();
  }

  template <class T>
  inline RefPtr<typename T::strong_value_type> strong_cast(const RefPtr<T> &src) noexcept
  {
    return src->Lock();
  }

  template <class T>
  inline RefPtr<const typename T::strong_value_type> strong_cast(const RefPtr<const T> &src) noexcept
  {
    return src->Lock();
  }
}
