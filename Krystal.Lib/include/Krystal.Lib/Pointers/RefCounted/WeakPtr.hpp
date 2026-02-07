#pragma once

#include "Krystal.Lib/Detection/Environment.hpp"
#include "Krystal.Lib/Pointers/RefCounted/CanMakeWeakPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/CompactRefPtrTuple.hpp"
#include "Krystal.Lib/Pointers/RefCounted/GetPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/TypeTraits.hpp"
#include "Krystal.Lib/Pointers/RefCounted/WeakPtrFactory.hpp"
#include "Krystal.Lib/Pointers/RefCounted/WeakRef.hpp"
#include <type_traits>

namespace Krys
{
  class SingleThreadWeakPtrImpl;

  template <typename T, typename WeakPtrImpl, typename PtrTraits>
  class WeakPtr
  {
  public:
    WeakPtr()
    {
    }
    WeakPtr(std::nullptr_t)
    {
    }
    template <typename U>
    WeakPtr(const WeakPtr<U, WeakPtrImpl, PtrTraits> &);
    template <typename U>
    WeakPtr(WeakPtr<U, WeakPtrImpl, PtrTraits> &&);

    template <typename U>
    WeakPtr(const WeakRef<U, WeakPtrImpl> &);
    template <typename U>
    WeakPtr(WeakRef<U, WeakPtrImpl> &&);

    template <typename = std::enable_if_t<!IsSmartPtr<T>::value>>
    WeakPtr(const T *object,
            EnableWeakPtrThreadingAssertions shouldEnableAssertions = EnableWeakPtrThreadingAssertions::Yes)
        : m_impl(object ? &object->weakImpl() : nullptr)
#if KRYS_ENV(DEV)
          ,
          m_shouldEnableAssertions(shouldEnableAssertions == EnableWeakPtrThreadingAssertions::Yes)
#endif
    {
      (void)shouldEnableAssertions;
      assert(!object || object == m_impl->template get<T>());
    }

    template <typename = std::enable_if_t<!IsSmartPtr<T>::value && !std::is_pointer_v<T>>>
    WeakPtr(const T &object,
            EnableWeakPtrThreadingAssertions shouldEnableAssertions = EnableWeakPtrThreadingAssertions::Yes)
        : m_impl(&object.weakImpl())
#if KRYS_ENV(DEV)
          ,
          m_shouldEnableAssertions(shouldEnableAssertions == EnableWeakPtrThreadingAssertions::Yes)
#endif
    {
      (void)shouldEnableAssertions;
      assert(&object == m_impl->template get<T>());
    }

    template <typename = std::enable_if_t<!IsSmartPtr<T>::value>>
    WeakPtr(const Ref<T> &object,
            EnableWeakPtrThreadingAssertions shouldEnableAssertions = EnableWeakPtrThreadingAssertions::Yes)
        : WeakPtr(object.get(), shouldEnableAssertions)
    {
    }

    template <typename = std::enable_if_t<!IsSmartPtr<T>::value>>
    WeakPtr(const RefPtr<T> &object,
            EnableWeakPtrThreadingAssertions shouldEnableAssertions = EnableWeakPtrThreadingAssertions::Yes)
        : WeakPtr(object.get(), shouldEnableAssertions)
    {
    }

    template <typename OtherPtrTraits>
    explicit WeakPtr(RefPtr<WeakPtrImpl, OtherPtrTraits> impl) : m_impl(Krys::Move(impl))
    {
    }

    WeakPtr(HashTableDeletedValueType) : m_impl(HashTableDeletedValue)
    {
    }
    WeakPtr(HashTableEmptyValueType) : m_impl(HashTableEmptyValue)
    {
    }

    bool isHashTableDeletedValue() const
    {
      return m_impl.isHashTableDeletedValue();
    }
    bool isHashTableEmptyValue() const
    {
      return !m_impl;
    }
    bool isWeakNullValue() const
    {
      return !*m_impl;
    }

    T *ptrAllowingHashTableEmptyValue() const
    {
      static_assert(HasRefPtrMemberFunctions<T>::value || HasCheckedPtrMemberFunctions<T>::value
                      || IsDeprecatedWeakRefSmartPointerException<std::remove_cv_t<T>>::value,
                    "Classes that offer weak pointers should also offer RefPtr or CheckedPtr. Please do not "
                    "add new exceptions.");

      return !m_impl.isHashTableEmptyValue() ? static_cast<T *>(m_impl->template get<T>()) : nullptr;
    }

    RefPtr<WeakPtrImpl, PtrTraits> releaseImpl()
    {
      return Krys::Move(m_impl);
    }

    T *get() const
    {
      static_assert(HasRefPtrMemberFunctions<T>::value || HasCheckedPtrMemberFunctions<T>::value
                      || IsDeprecatedWeakRefSmartPointerException<std::remove_cv_t<T>>::value,
                    "Classes that offer weak pointers should also offer RefPtr or CheckedPtr. Please do not "
                    "add new exceptions.");
      static_assert(!IsDeprecatedWeakRefSmartPointerException<std::remove_cv_t<T>>::value
                      || (!HasRefPtrMemberFunctions<T>::value && !HasCheckedPtrMemberFunctions<T>::value),
                    "IsDeprecatedWeakRefSmartPointerException specialization is no longer needed for this "
                    "class, please remove it.");
      assert(canSafelyBeUsed());
      return m_impl ? static_cast<T *>(m_impl->template get<T>()) : nullptr;
    }

    WeakRef<T> releaseNonNull()
    {
      return WeakRef<T> {m_impl.releaseNonNull(), enableWeakPtrThreadingAssertions()};
    }

    bool operator!() const
    {
      return !m_impl || !*m_impl;
    }
    explicit operator bool() const
    {
      return m_impl && *m_impl;
    }

    WeakPtr &operator=(std::nullptr_t)
    {
      m_impl = nullptr;
      return *this;
    }
    template <typename U>
    WeakPtr &operator=(const WeakPtr<U, WeakPtrImpl, PtrTraits> &);
    template <typename U>
    WeakPtr &operator=(WeakPtr<U, WeakPtrImpl, PtrTraits> &&);
    template <typename U>
    WeakPtr &operator=(const WeakRef<U, WeakPtrImpl> &);
    template <typename U>
    WeakPtr &operator=(WeakRef<U, WeakPtrImpl> &&);

    T *operator->() const
    {
      static_assert(HasRefPtrMemberFunctions<T>::value || HasCheckedPtrMemberFunctions<T>::value
                      || IsDeprecatedWeakRefSmartPointerException<std::remove_cv_t<T>>::value,
                    "Classes that offer weak pointers should also offer RefPtr or CheckedPtr. Please do not "
                    "add new exceptions.");
      static_assert(!IsDeprecatedWeakRefSmartPointerException<std::remove_cv_t<T>>::value
                      || (!HasRefPtrMemberFunctions<T>::value && !HasCheckedPtrMemberFunctions<T>::value),
                    "IsDeprecatedWeakRefSmartPointerException specialization is no longer needed for this "
                    "class, please remove it.");

      assert(canSafelyBeUsed());
      auto *result = get();
      assert(result);
      return result;
    }

    T &operator*() const
    {
      static_assert(HasRefPtrMemberFunctions<T>::value || HasCheckedPtrMemberFunctions<T>::value
                      || IsDeprecatedWeakRefSmartPointerException<std::remove_cv_t<T>>::value,
                    "Classes that offer weak pointers should also offer RefPtr or CheckedPtr. Please do not "
                    "add new exceptions.");
      static_assert(!IsDeprecatedWeakRefSmartPointerException<std::remove_cv_t<T>>::value
                      || (!HasRefPtrMemberFunctions<T>::value && !HasCheckedPtrMemberFunctions<T>::value),
                    "IsDeprecatedWeakRefSmartPointerException specialization is no longer needed for this "
                    "class, please remove it.");

      assert(canSafelyBeUsed());
      auto *result = get();
      assert(result);
      return *result;
    }

    void clear()
    {
      m_impl = nullptr;
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
    template <typename, typename, typename>
    friend class WeakPtr;
    template <typename, typename>
    friend class WeakPtrFactory;
    template <typename, typename>
    friend class WeakPtrFactoryWithBitField;

    explicit WeakPtr(Ref<WeakPtrImpl> &&ref, EnableWeakPtrThreadingAssertions shouldEnableAssertions)
        : m_impl(Krys::Move(ref))
#if KRYS_ENV(DEV)
          ,
          m_shouldEnableAssertions(shouldEnableAssertions == EnableWeakPtrThreadingAssertions::Yes)
#endif
    {
      (void)shouldEnableAssertions;
    }

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

    RefPtr<WeakPtrImpl, PtrTraits> m_impl;
#if KRYS_ENV(DEV)
    bool m_shouldEnableAssertions {true};
#endif
  };

  template <typename T, typename U, typename WeakPtrImpl>
  inline WeakPtrImpl *weak_ptr_impl_cast(WeakPtrImpl *impl)
  {
    static_assert(std::same_as<typename T::WeakValueType, typename U::WeakValueType>,
                  "Invalid weak pointer cast");
    return impl;
  }

  template <typename T, typename U, typename WeakPtrImpl>
  inline WeakPtrImpl &weak_ptr_impl_cast(WeakPtrImpl &impl)
  {
    static_assert(std::same_as<typename T::WeakValueType, typename U::WeakValueType>,
                  "Invalid weak pointer cast");
    return impl;
  }

  template <typename T, typename WeakPtrImpl, typename PtrTraits>
  template <typename U>
  inline WeakPtr<T, WeakPtrImpl, PtrTraits>::WeakPtr(const WeakPtr<U, WeakPtrImpl, PtrTraits> &o)
      : m_impl(weak_ptr_impl_cast<T, U>(o.m_impl.get()))
#if KRYS_ENV(DEV)
        ,
        m_shouldEnableAssertions(o.m_shouldEnableAssertions)
#endif
  {
  }

  template <typename T, typename WeakPtrImpl, typename PtrTraits>
  template <typename U>
  inline WeakPtr<T, WeakPtrImpl, PtrTraits>::WeakPtr(WeakPtr<U, WeakPtrImpl, PtrTraits> &&o)
      : m_impl(adoptRef(weak_ptr_impl_cast<T, U>(o.m_impl.leakRef())))
#if KRYS_ENV(DEV)
        ,
        m_shouldEnableAssertions(o.m_shouldEnableAssertions)
#endif
  {
  }

  template <typename T, typename WeakPtrImpl, typename PtrTraits>
  template <typename U>
  inline WeakPtr<T, WeakPtrImpl, PtrTraits>::WeakPtr(const WeakRef<U, WeakPtrImpl> &o)
      : m_impl(&weak_ptr_impl_cast<T, U>(o.impl()))
#if KRYS_ENV(DEV)
        ,
        m_shouldEnableAssertions(o.enableWeakPtrThreadingAssertions()
                                 == EnableWeakPtrThreadingAssertions::Yes)
#endif
  {
  }

  template <typename T, typename WeakPtrImpl, typename PtrTraits>
  template <typename U>
  inline WeakPtr<T, WeakPtrImpl, PtrTraits>::WeakPtr(WeakRef<U, WeakPtrImpl> &&o)
      : m_impl(adoptRef(weak_ptr_impl_cast<T, U>(o.releaseImpl().leakRef())))
#if KRYS_ENV(DEV)
        ,
        m_shouldEnableAssertions(o.enableWeakPtrThreadingAssertions()
                                 == EnableWeakPtrThreadingAssertions::Yes)
#endif
  {
  }

  template <typename T, typename WeakPtrImpl, typename PtrTraits>
  template <typename U>
  inline WeakPtr<T, WeakPtrImpl, PtrTraits> &
    WeakPtr<T, WeakPtrImpl, PtrTraits>::operator=(const WeakPtr<U, WeakPtrImpl, PtrTraits> &o)
  {
    m_impl = weak_ptr_impl_cast<T, U>(o.m_impl.get());
#if KRYS_ENV(DEV)
    m_shouldEnableAssertions = o.m_shouldEnableAssertions;
#endif
    return *this;
  }

  template <typename T, typename WeakPtrImpl, typename PtrTraits>
  template <typename U>
  inline WeakPtr<T, WeakPtrImpl, PtrTraits> &
    WeakPtr<T, WeakPtrImpl, PtrTraits>::operator=(WeakPtr<U, WeakPtrImpl, PtrTraits> &&o)
  {
    m_impl = adoptRef(weak_ptr_impl_cast<T, U>(o.m_impl.leakRef()));
#if KRYS_ENV(DEV)
    m_shouldEnableAssertions = o.m_shouldEnableAssertions;
#endif
    return *this;
  }

  template <typename T, typename WeakPtrImpl, typename PtrTraits>
  template <typename U>
  inline WeakPtr<T, WeakPtrImpl, PtrTraits> &
    WeakPtr<T, WeakPtrImpl, PtrTraits>::operator=(const WeakRef<U, WeakPtrImpl> &o)
  {
    m_impl = &weak_ptr_impl_cast<T, U>(o.m_impl.get());
#if KRYS_ENV(DEV)
    m_shouldEnableAssertions = o.enableWeakPtrThreadingAssertions() == EnableWeakPtrThreadingAssertions::Yes;
#endif
    return *this;
  }

  template <typename T, typename WeakPtrImpl, typename PtrTraits>
  template <typename U>
  inline WeakPtr<T, WeakPtrImpl, PtrTraits> &
    WeakPtr<T, WeakPtrImpl, PtrTraits>::operator=(WeakRef<U, WeakPtrImpl> &&o)
  {
    m_impl = adoptRef(weak_ptr_impl_cast<T, U>(o.m_impl.leakRef()));
#if KRYS_ENV(DEV)
    m_shouldEnableAssertions = o.enableWeakPtrThreadingAssertions() == EnableWeakPtrThreadingAssertions::Yes;
#endif
    return *this;
  }

  template <typename T, typename WeakPtrImpl, typename PtrTraits>
  struct GetPtrHelper<WeakPtr<T, WeakPtrImpl, PtrTraits>>
  {
    using PtrType = T *;
    using UnderlyingType = T;
    static T *getPtr(const WeakPtr<T, WeakPtrImpl, PtrTraits> &p)
    {
      return const_cast<T *>(p.get());
    }
  };

  template <typename T, typename WeakPtrImpl, typename PtrTraits>
  struct IsSmartPtr<WeakPtr<T, WeakPtrImpl, PtrTraits>>
  {
    static constexpr bool value = true;
    static constexpr bool isNullable = true;
  };

  template <typename ExpectedType, typename ArgType, typename WeakPtrImpl, typename PtrTraits>
  inline bool is(WeakPtr<ArgType, WeakPtrImpl, PtrTraits> &source)
  {
    return is<ExpectedType>(source.get());
  }

  template <typename ExpectedType, typename ArgType, typename WeakPtrImpl, typename PtrTraits>
  inline bool is(const WeakPtr<ArgType, WeakPtrImpl, PtrTraits> &source)
  {
    return is<ExpectedType>(source.get());
  }

  template <typename Target, typename Source, typename WeakPtrImpl, typename PtrTraits>
  inline WeakPtr<match_constness_t<Source, Target>, WeakPtrImpl, PtrTraits>
    downcast(WeakPtr<Source, WeakPtrImpl, PtrTraits> source)
  {
    static_assert(!std::same_as<Source, Target>, "Unnecessary cast to same type");
    static_assert(std::derived_from<Target, Source>, "Should be a downcast");
    assert(!source || is<Target>(*source));
    return WeakPtr<match_constness_t<Source, Target>, WeakPtrImpl, PtrTraits> {
      unsafeRefPtrDowncast<match_constness_t<Source, Target>>(source.releaseImpl()),
      source.enableWeakPtrThreadingAssertions()};
  }

  template <typename Target, typename Source, typename WeakPtrImpl, typename PtrTraits>
  inline WeakPtr<match_constness_t<Source, Target>, WeakPtrImpl, PtrTraits>
    dynamicDowncast(WeakPtr<Source, WeakPtrImpl, PtrTraits> source)
  {
    static_assert(!std::same_as<Source, Target>, "Unnecessary cast to same type");
    static_assert(std::derived_from<Target, Source>, "Should be a downcast");
    if (!is<Target>(source))
      return nullptr;
    return WeakPtr<match_constness_t<Source, Target>, WeakPtrImpl, PtrTraits> {
      unsafeRefPtrDowncast<match_constness_t<Source, Target>, WeakPtrImpl>(source.releaseImpl()),
      source.enableWeakPtrThreadingAssertions()};
  }

  template <typename T, typename U, typename WeakPtrImpl, typename PtrTraits>
  inline bool operator==(const WeakPtr<T, WeakPtrImpl, PtrTraits> &a,
                         const WeakPtr<U, WeakPtrImpl, PtrTraits> &b)
  {
    return a.get() == b.get();
  }

  template <typename T, typename U, typename WeakPtrImpl, typename PtrTraits>
  inline bool operator==(const WeakPtr<T, WeakPtrImpl, PtrTraits> &a, U *b)
  {
    return a.get() == b;
  }

  template <class T, typename = std::enable_if_t<!IsSmartPtr<T>::value>>
  WeakPtr(const T *value, EnableWeakPtrThreadingAssertions = EnableWeakPtrThreadingAssertions::Yes)
    -> WeakPtr<T, typename T::WeakPtrImplType, RawPtrTraits<T>>;

  template <class T, typename = std::enable_if_t<!IsSmartPtr<T>::value && !std::is_pointer_v<T>>>
  WeakPtr(const T &value, EnableWeakPtrThreadingAssertions = EnableWeakPtrThreadingAssertions::Yes)
    -> WeakPtr<T, typename T::WeakPtrImplType, RawPtrTraits<T>>;

  template <class T, typename = std::enable_if_t<!IsSmartPtr<T>::value>>
  WeakPtr(const Ref<T> &value, EnableWeakPtrThreadingAssertions = EnableWeakPtrThreadingAssertions::Yes)
    -> WeakPtr<T, typename T::WeakPtrImplType, RawPtrTraits<T>>;

  template <class T, typename = std::enable_if_t<!IsSmartPtr<T>::value>>
  WeakPtr(const RefPtr<T> &value, EnableWeakPtrThreadingAssertions = EnableWeakPtrThreadingAssertions::Yes)
    -> WeakPtr<T, typename T::WeakPtrImplType, RawPtrTraits<T>>;

  template <typename T, typename PtrTraits = RawPtrTraits<SingleThreadWeakPtrImpl>>
  using SingleThreadWeakPtr = WeakPtr<T, SingleThreadWeakPtrImpl, PtrTraits>;
}
