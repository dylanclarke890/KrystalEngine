#pragma once

#include "Krystal.Lib/Pointers/RefCounted/RefPtr.hpp"
#include "Krystal.Lib/Types/Expected.hpp"
#include <cstddef>
#include <type_traits>
#include <utility>

// SFINAE depends on overload resolution. We indicate the overload we'd prefer
// (if it can compile) using a higher priorty type (int), and the overload
// to fall back to using a lower priority type (long). 0 can convert to int
// or long, so we can trigger overload resolution using 0. C++ is awesome!
#define SFINAE_OVERLOAD 0
#define SFINAE_OVERLOAD_DEFAULT long
#define SFINAE_OVERLOAD_PREFERRED int

namespace Krys
{
  namespace detail
  {
    // IsRefcountedSmartPointer implementation.
    template <typename CVRemoved>
    struct IsRefcountedSmartPointerHelper : std::false_type
    {
    };

    template <typename Pointee>
    struct IsRefcountedSmartPointerHelper<RefPtr<Pointee>> : std::true_type
    {
    };

    template <typename Pointee>
    struct IsRefcountedSmartPointerHelper<Ref<Pointee>> : std::true_type
    {
    };

  } // namespace detail

  template <typename T>
  struct IsRefcountedSmartPointer : detail::IsRefcountedSmartPointerHelper<std::remove_cv_t<T>>
  {
  };

  // IsSmartRef implementation
  namespace detail
  {

    template <typename CVRemoved>
    struct IsSmartRefHelper : std::false_type
    {
    };

    template <typename Pointee>
    struct IsSmartRefHelper<Ref<Pointee>> : std::true_type
    {
    };

  } // namespace detail

  template <typename T>
  struct IsSmartRef : detail::IsSmartRefHelper<std::remove_cv_t<T>>
  {
  };

  // RemoveSmartPointer implementation
  namespace detail
  {

    template <typename T, typename CVRemoved>
    struct RemoveSmartPointerHelper
    {
      typedef T type;
    };

    template <typename T, typename Pointee>
    struct RemoveSmartPointerHelper<T, RefPtr<Pointee>>
    {
      typedef Pointee type;
    };

    template <typename T, typename Pointee>
    struct RemoveSmartPointerHelper<T, Ref<Pointee>>
    {
      typedef Pointee type;
    };

  } // namespace detail

  template <typename T>
  struct RemoveSmartPointer : detail::RemoveSmartPointerHelper<T, std::remove_cv_t<T>>
  {
  };

  template <typename T>
  using RemoveCVSmartPointer = std::remove_cvref_t<typename RemoveSmartPointer<T>::type>;

  // HasRefPtrMemberFunctions implementation
  namespace detail
  {

    template <typename>
    struct SFINAE1True : std::true_type
    {
    };

    template <class T>
    static auto HasRefPtrMemberFunctionsTest(SFINAE_OVERLOAD_PREFERRED)
      -> SFINAE1True<decltype(static_cast<std::remove_cv_t<T> *>(nullptr)->AddRef(),
                              static_cast<std::remove_cv_t<T> *>(nullptr)->SubRef())>;
    template <class>
    static auto HasRefPtrMemberFunctionsTest(SFINAE_OVERLOAD_DEFAULT) -> std::false_type;

  } // namespace detail

  template <class T>
  struct HasRefPtrMemberFunctions : decltype(detail::HasRefPtrMemberFunctionsTest<T>(SFINAE_OVERLOAD))
  {
  };

  // HasWeakPtrFunctions implementation
  namespace detail
  {

    template <class T>
    static auto HasWeakPtrFunctionsTest(SFINAE_OVERLOAD_PREFERRED)
      -> SFINAE1True<decltype(static_cast<std::remove_cv_t<T> *>(nullptr)->WeakImpl(),
                              static_cast<std::remove_cv_t<T> *>(nullptr)->weakCount())>;
    template <class>
    static auto HasWeakPtrFunctionsTest(SFINAE_OVERLOAD_DEFAULT) -> std::false_type;

  }

  template <class T>
  struct HasWeakPtrFunctions : decltype(detail::HasWeakPtrFunctionsTest<T>(SFINAE_OVERLOAD))
  {
  };

  // HasThreadSafeWeakPtrFunctions implementation
  namespace detail
  {

    template <class T>
    static auto HasThreadSafeWeakPtrFunctionsTest(SFINAE_OVERLOAD_PREFERRED)
      -> SFINAE1True<decltype(static_cast<std::remove_cv_t<T> *>(nullptr)->weakRefCount())>;
    template <class>
    static auto HasThreadSafeWeakPtrFunctionsTest(SFINAE_OVERLOAD_DEFAULT) -> std::false_type;

  }

  template <class T>
  struct HasThreadSafeWeakPtrFunctions
      : decltype(detail::HasThreadSafeWeakPtrFunctionsTest<T>(SFINAE_OVERLOAD))
  {
  };

  // HasCheckedPtrMemberFunctions implementation
  namespace detail
  {

    template <class T>
    static auto HasCheckedPtrMemberFunctionsTest(SFINAE_OVERLOAD_PREFERRED)
      -> SFINAE1True<decltype(static_cast<std::remove_cv_t<T> *>(nullptr)->incrementCheckedPtrCount(),
                              static_cast<std::remove_cv_t<T> *>(nullptr)->decrementCheckedPtrCount())>;
    template <class>
    static auto HasCheckedPtrMemberFunctionsTest(SFINAE_OVERLOAD_DEFAULT) -> std::false_type;

  } // namespace detail

  template <class T>
  struct HasCheckedPtrMemberFunctions : decltype(detail::HasCheckedPtrMemberFunctionsTest<T>(SFINAE_OVERLOAD))
  {
  };

  // HasIsolatedCopy()
  namespace detail
  {
    // FIXME: This test is incorrectly false for RefCounted objects because
    // substitution for std::declval<T>() fails when the constructor is private.
    template <class T>
    static auto HasIsolatedCopyTest(SFINAE_OVERLOAD_PREFERRED)
      -> SFINAE1True<decltype(std::declval<T>().isolatedCopy())>;
    template <class>
    static auto HasIsolatedCopyTest(SFINAE_OVERLOAD_DEFAULT) -> std::false_type;

  } // namespace detail

  template <class T>
  struct HasIsolatedCopy : decltype(detail::HasIsolatedCopyTest<T>(SFINAE_OVERLOAD))
  {
  };
}
