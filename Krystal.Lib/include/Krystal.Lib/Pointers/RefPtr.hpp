#pragma once

#include "Krystal.Lib/Pointers/IntrusivePtr.hpp"

namespace Krys
{
  template <class T>
  using RefPtr = IntrusivePtr<T, typename T::RefPtrTraits>;

  template <class T>
  constexpr RefPtr<T> refcnt_retain(T *ptr) noexcept
  {
    return RefPtr<T>::ref(ptr);
  }

  template <class T>
  constexpr RefPtr<T> refcnt_attach(T *ptr) noexcept
  {
    return RefPtr<T>::noref(ptr);
  }

  template <class T, class... Args>
  inline RefPtr<T> make_refcnt(Args &&...args)
  {
    return RefPtr<T>::noref(new T(std::forward<Args>(args)...));
  }

  template <class T>
  inline RefPtr<typename T::weak_value_type> weak_cast(const RefPtr<T> &src)
  {
    return src->get_weak_ptr();
  }

  template <class T>
  inline RefPtr<const typename T::weak_value_type> weak_cast(const RefPtr<const T> &src)
  {
    return src->get_weak_ptr();
  }

  template <class T>
  inline RefPtr<typename T::strong_value_type> strong_cast(const RefPtr<T> &src) noexcept
  {
    return src->lock();
  }

  template <class T>
  inline RefPtr<const typename T::strong_value_type> strong_cast(const RefPtr<const T> &src) noexcept
  {
    return src->lock();
  }
}
