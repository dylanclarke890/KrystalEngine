#pragma once

#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Detection/Environment.hpp"
#include "Krystal.Lib/Pointers/RefCounted/GetPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/ThreadSafeRefCounted.hpp"
#include "Krystal.Lib/Pointers/RefCounted/TypeTraits.hpp"
#include "Krystal.Lib/Pointers/RefCounted/WeakPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/WeakPtrImpl.hpp"

namespace Krys
{
  // Classes that offer weak pointers should also offer RefPtr or CheckedPtr. Please do not add new
  // exceptions.
  template <typename T>
  struct IsDeprecatedWeakRefSmartPointerException : std::false_type
  {
  };

  enum class EnableWeakPtrThreadingAssertions : bool
  {
    No,
    Yes
  };

  // Similar to a WeakPtr but it is an error for it to become null. It is useful for hardening when replacing
  // things like `Foo& m_foo`. It is similar to CheckedRef but it generates crashes that are more actionable.
  template <typename T, typename WeakPtrImpl>
  class WeakRef
  {
  public:
    WeakRef(const T &object,
            EnableWeakPtrThreadingAssertions shouldEnableAssertions = EnableWeakPtrThreadingAssertions::Yes)
    requires(!IsSmartPtr<T>::value && !std::is_pointer_v<T>)
        : m_impl(object.weakImpl())
#if KRYS_ENV(DEV)
          ,
          m_shouldEnableAssertions(shouldEnableAssertions == EnableWeakPtrThreadingAssertions::Yes)
#endif
    {
      (void)shouldEnableAssertions;
    }

    explicit WeakRef(Ref<WeakPtrImpl> &&impl, EnableWeakPtrThreadingAssertions shouldEnableAssertions =
                                                EnableWeakPtrThreadingAssertions::Yes)
        : m_impl(Krys::Move(impl))
#if KRYS_ENV(DEV)
          ,
          m_shouldEnableAssertions(shouldEnableAssertions == EnableWeakPtrThreadingAssertions::Yes)
#endif
    {
      (void)shouldEnableAssertions;
    }

    WeakRef(HashTableDeletedValueType) : m_impl(HashTableDeletedValue)
    {
    }
    WeakRef(HashTableEmptyValueType) : m_impl(HashTableEmptyValue)
    {
    }

    bool isHashTableDeletedValue() const
    {
      return m_impl.isHashTableDeletedValue();
    }
    bool isHashTableEmptyValue() const
    {
      return m_impl.isHashTableEmptyValue();
    }

    WeakPtrImpl &impl() const
    {
      return m_impl;
    }
    Ref<WeakPtrImpl> releaseImpl()
    {
      return Krys::Move(m_impl);
    }

    T *ptrAllowingHashTableEmptyValue() const
    {
      static_assert(HasRefPtrMemberFunctions<T>::value || HasCheckedPtrMemberFunctions<T>::value
                      || IsDeprecatedWeakRefSmartPointerException<std::remove_cv_t<T>>::value,
                    "Classes that offer weak pointers should also offer RefPtr or CheckedPtr. Please do not "
                    "add new exceptions.");

      return !m_impl.isHashTableEmptyValue() ? static_cast<T *>(m_impl->template get<T>()) : nullptr;
    }

    T *ptr() const
    {
      static_assert(HasRefPtrMemberFunctions<T>::value || HasCheckedPtrMemberFunctions<T>::value
                      || IsDeprecatedWeakRefSmartPointerException<std::remove_cv_t<T>>::value,
                    "Classes that offer weak pointers should also offer RefPtr or CheckedPtr. Please do not "
                    "add new exceptions.");

      auto *ptr = static_cast<T *>(m_impl->template get<T>());
      assert(ptr);
      return ptr;
    }

    T &get() const
    {
      static_assert(HasRefPtrMemberFunctions<T>::value || HasCheckedPtrMemberFunctions<T>::value
                      || IsDeprecatedWeakRefSmartPointerException<std::remove_cv_t<T>>::value,
                    "Classes that offer weak pointers should also offer RefPtr or CheckedPtr. Please do not "
                    "add new exceptions.");

      auto *ptr = static_cast<T *>(m_impl->template get<T>());
      assert(ptr);
      return *ptr;
    }

    operator T &() const
    {
      return get();
    }

    T *operator->() const
    {
      assert(canSafelyBeUsed());
      return ptr();
    }

    EnableWeakPtrThreadingAssertions enableWeakPtrThreadingAssertions() const
    {
#if KRYS_ENV(DEV)
      return m_shouldEnableAssertions ? EnableWeakPtrThreadingAssertions::Yes
                                      : EnableWeakPtrThreadingAssertions::No;
#else
      return EnableWeakPtrThreadingAssertions::No;
#endif
    }

  private:
#if KRYS_ENV(DEV)
    inline bool canSafelyBeUsed() const
    {
      // FIXME: Our GC threads currently need to get opaque pointers from WeakPtrs and have to be
      // special-cased.
      return !m_impl || !m_shouldEnableAssertions
             || (m_impl->wasConstructedOnMainThread() && Thread::mayBeGCThread())
             || m_impl->wasConstructedOnMainThread() == isMainThread();
    }
#endif

    Ref<WeakPtrImpl> m_impl;
#if KRYS_ENV(DEV)
    bool m_shouldEnableAssertions {true};
#endif
  };

  template <class T>
  requires(!IsSmartPtr<T>::value && !std::is_pointer_v<T>)
  WeakRef(const T &value, EnableWeakPtrThreadingAssertions = EnableWeakPtrThreadingAssertions::Yes)
    -> WeakRef<T, typename T::WeakPtrImplType>;

  template <typename T, typename WeakPtrImpl>
  struct GetPtrHelper<WeakRef<T, WeakPtrImpl>>
  {
    using PtrType = T *;
    using UnderlyingType = T;
    static T *getPtr(const WeakRef<T, WeakPtrImpl> &p)
    {
      return const_cast<T *>(p.ptr());
    }
  };

  template <typename T, typename WeakPtrImpl>
  struct IsSmartPtr<WeakRef<T, WeakPtrImpl>>
  {
    static constexpr bool value = true;
    static constexpr bool isNullable = false;
  };

  template <typename T>
  using SingleThreadWeakRef = WeakRef<T, SingleThreadWeakPtrImpl>;

  template <typename ExpectedType, typename ArgType, typename WeakPtrImpl>
  inline bool is(WeakRef<ArgType, WeakPtrImpl> &source)
  {
    return is<ExpectedType>(source.get());
  }

  template <typename ExpectedType, typename ArgType, typename WeakPtrImpl>
  inline bool is(const WeakRef<ArgType, WeakPtrImpl> &source)
  {
    return is<ExpectedType>(source.get());
  }

  template <typename Target, typename Source, typename WeakPtrImpl>
  inline WeakRef<match_constness_t<Source, Target>, WeakPtrImpl> downcast(WeakRef<Source, WeakPtrImpl> source)
  {
    static_assert(!std::same_as<Source, Target>, "Unnecessary cast to same type");
    static_assert(std::derived_from<Target, Source>, "Should be a downcast");
    assert(is<Target>(source));
    return WeakRef<match_constness_t<Source, Target>, WeakPtrImpl> {
      unsafeRefDowncast<match_constness_t<Source, Target>>(source.releaseImpl()),
      source.enableWeakPtrThreadingAssertions()};
  }

  template <typename Target, typename Source, typename WeakPtrImpl>
  inline WeakPtr<match_constness_t<Source, Target>, WeakPtrImpl, RawPtrTraits<Target>>
    dynamicDowncast(WeakRef<Source, WeakPtrImpl> source)
  {
    static_assert(!std::same_as<Source, Target>, "Unnecessary cast to same type");
    static_assert(std::derived_from<Target, Source>, "Should be a downcast");
    if (!is<Target>(source))
      return nullptr;
    return WeakPtr<match_constness_t<Source, Target>, WeakPtrImpl, RawPtrTraits<Target>> {
      unsafeRefDowncast<match_constness_t<Source, Target>>(source.releaseImpl()),
      source.enableWeakPtrThreadingAssertions()};
  }

}
