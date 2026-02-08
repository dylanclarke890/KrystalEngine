#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Detection/Environment.hpp"
#include "Krystal.Lib/Pointers/RefCounted/CanMakeWeakPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/CompactRefPtrTuple.hpp"
#include "Krystal.Lib/Pointers/RefCounted/GetPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/TypeTraits.hpp"
#include "Krystal.Lib/Pointers/RefCounted/WeakPtrFactory.hpp"
#include "Krystal.Lib/Pointers/RefCounted/WeakPtrImpl.hpp"
#include "Krystal.Lib/Pointers/RefCounted/WeakRef.hpp"

namespace Krys
{
  template <typename T, typename WeakPtrImpl, typename PtrTraits>
  class WeakPtr
  {
  public:
    WeakPtr() = default;

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

    template <typename = enable_if_t<!IsSmartPtr<T>::value>>
    WeakPtr(const T *object,
            EnabledWeakPtrThreadAsserts shouldEnableAssertions = EnabledWeakPtrThreadAsserts::Yes)
        : _impl(object ? &object->WeakImpl() : nullptr)
#if KRYS_ENV(DEV)
          ,
          m_shouldEnableAssertions(shouldEnableAssertions == EnabledWeakPtrThreadAsserts::Yes)
#endif
    {
      (void)shouldEnableAssertions;
      assert(!object || object == _impl->template get<T>());
    }

    template <typename = std::enable_if_t<!IsSmartPtr<T>::value && !std::is_pointer_v<T>>>
    WeakPtr(const T &object,
            EnabledWeakPtrThreadAsserts shouldEnableAssertions = EnabledWeakPtrThreadAsserts::Yes)
        : _impl(&object.WeakImpl())
#if KRYS_ENV(DEV)
          ,
          m_shouldEnableAssertions(shouldEnableAssertions == EnabledWeakPtrThreadAsserts::Yes)
#endif
    {
      (void)shouldEnableAssertions;
      assert(&object == _impl->template get<T>());
    }

    template <typename = std::enable_if_t<!IsSmartPtr<T>::value>>
    WeakPtr(const Ref<T> &object,
            EnabledWeakPtrThreadAsserts shouldEnableAssertions = EnabledWeakPtrThreadAsserts::Yes)
        : WeakPtr(object.get(), shouldEnableAssertions)
    {
    }

    template <typename = std::enable_if_t<!IsSmartPtr<T>::value>>
    WeakPtr(const RefPtr<T> &object,
            EnabledWeakPtrThreadAsserts shouldEnableAssertions = EnabledWeakPtrThreadAsserts::Yes)
        : WeakPtr(object.get(), shouldEnableAssertions)
    {
    }

    template <typename OtherPtrTraits>
    explicit WeakPtr(RefPtr<WeakPtrImpl, OtherPtrTraits> impl) : _impl(Krys::Move(impl))
    {
    }

    WeakPtr(HashTableDeletedValueType) : _impl(HashTableDeletedValue)
    {
    }
    WeakPtr(HashTableEmptyValueType) : _impl(HashTableEmptyValue)
    {
    }

    bool isHashTableDeletedValue() const
    {
      return _impl.isHashTableDeletedValue();
    }
    bool isHashTableEmptyValue() const
    {
      return !_impl;
    }
    bool isWeakNullValue() const
    {
      return !*_impl;
    }

    T *ptrAllowingHashTableEmptyValue() const
    {
      static_assert(HasRefPtrMemberFunctions<T>::value || HasCheckedPtrMemberFunctions<T>::value,
                    "Classes that offer weak pointers must also offer RefPtr or CheckedPtr");

      return !_impl.isHashTableEmptyValue() ? static_cast<T *>(_impl->template get<T>()) : nullptr;
    }

    RefPtr<WeakPtrImpl, PtrTraits> releaseImpl()
    {
      return Krys::Move(_impl);
    }

    T *get() const
    {
      static_assert(HasRefPtrMemberFunctions<T>::value || HasCheckedPtrMemberFunctions<T>::value,
                    "Classes that offer weak pointers must also offer RefPtr or CheckedPtr");
      return _impl ? static_cast<T *>(_impl->template get<T>()) : nullptr;
    }

    WeakRef<T, WeakPtrImpl> releaseNonNull()
    {
      return WeakRef<T, WeakPtrImpl> {_impl.releaseNonNull(), enableWeakPtrThreadingAssertions()};
    }

    bool operator!() const
    {
      return !_impl || !*_impl;
    }
    explicit operator bool() const
    {
      return _impl && *_impl;
    }

    WeakPtr &operator=(std::nullptr_t)
    {
      _impl = nullptr;
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
      static_assert(HasRefPtrMemberFunctions<T>::value || HasCheckedPtrMemberFunctions<T>::value,
                    "Classes that offer weak pointers must also offer RefPtr or CheckedPtr");

      auto *result = get();
      assert(result);
      return result;
    }

    T &operator*() const
    {
      static_assert(HasRefPtrMemberFunctions<T>::value || HasCheckedPtrMemberFunctions<T>::value,
                    "Classes that offer weak pointers must also offer RefPtr or CheckedPtr");

      auto *result = get();
      assert(result);
      return *result;
    }

    void clear()
    {
      _impl = nullptr;
    }

    EnabledWeakPtrThreadAsserts enableWeakPtrThreadingAssertions() const
    {
#if KRYS_ENV(DEV)
      return m_shouldEnableAssertions ? EnabledWeakPtrThreadAsserts::Yes : EnabledWeakPtrThreadAsserts::No;
#else
      return EnabledWeakPtrThreadAsserts::No;
#endif
    }

  private:
    template <typename, typename, typename>
    friend class WeakPtr;
    template <typename, typename>
    friend class WeakPtrFactory;
    template <typename, typename>
    friend class WeakPtrFactoryWithBitField;

    explicit WeakPtr(Ref<WeakPtrImpl> &&ref, EnabledWeakPtrThreadAsserts shouldEnableAssertions)
        : _impl(Krys::Move(ref))
#if KRYS_ENV(DEV)
          ,
          m_shouldEnableAssertions(shouldEnableAssertions == EnabledWeakPtrThreadAsserts::Yes)
#endif
    {
      (void)shouldEnableAssertions;
    }

    RefPtr<WeakPtrImpl, PtrTraits> _impl;
#if KRYS_ENV(DEV)
    bool m_shouldEnableAssertions {true};
#endif
  };

  template <typename T, typename U, typename WeakPtrImpl>
  inline WeakPtrImpl *weak_ptr_impl_cast(WeakPtrImpl *impl)
  {
    static_assert(std::same_as<typename T::TWeakValue, typename U::TWeakValue>, "Invalid weak pointer cast");
    return impl;
  }

  template <typename T, typename U, typename WeakPtrImpl>
  inline WeakPtrImpl &weak_ptr_impl_cast(WeakPtrImpl &impl)
  {
    static_assert(std::same_as<typename T::TWeakValue, typename U::TWeakValue>, "Invalid weak pointer cast");
    return impl;
  }

  template <typename T, typename WeakPtrImpl, typename PtrTraits>
  template <typename U>
  inline WeakPtr<T, WeakPtrImpl, PtrTraits>::WeakPtr(const WeakPtr<U, WeakPtrImpl, PtrTraits> &o)
      : _impl(weak_ptr_impl_cast<T, U>(o._impl.get()))
#if KRYS_ENV(DEV)
        ,
        m_shouldEnableAssertions(o.m_shouldEnableAssertions)
#endif
  {
  }

  template <typename T, typename WeakPtrImpl, typename PtrTraits>
  template <typename U>
  inline WeakPtr<T, WeakPtrImpl, PtrTraits>::WeakPtr(WeakPtr<U, WeakPtrImpl, PtrTraits> &&o)
      : _impl(adoptRef(weak_ptr_impl_cast<T, U>(o._impl.leakRef())))
#if KRYS_ENV(DEV)
        ,
        m_shouldEnableAssertions(o.m_shouldEnableAssertions)
#endif
  {
  }

  template <typename T, typename WeakPtrImpl, typename PtrTraits>
  template <typename U>
  inline WeakPtr<T, WeakPtrImpl, PtrTraits>::WeakPtr(const WeakRef<U, WeakPtrImpl> &o)
      : _impl(&weak_ptr_impl_cast<T, U>(o.Impl()))
#if KRYS_ENV(DEV)
        ,
        m_shouldEnableAssertions(o.enableWeakPtrThreadingAssertions() == EnabledWeakPtrThreadAsserts::Yes)
#endif
  {
  }

  template <typename T, typename WeakPtrImpl, typename PtrTraits>
  template <typename U>
  inline WeakPtr<T, WeakPtrImpl, PtrTraits>::WeakPtr(WeakRef<U, WeakPtrImpl> &&o)
      : _impl(adoptRef(weak_ptr_impl_cast<T, U>(o.releaseImpl().leakRef())))
#if KRYS_ENV(DEV)
        ,
        m_shouldEnableAssertions(o.enableWeakPtrThreadingAssertions() == EnabledWeakPtrThreadAsserts::Yes)
#endif
  {
  }

  template <typename T, typename WeakPtrImpl, typename PtrTraits>
  template <typename U>
  inline WeakPtr<T, WeakPtrImpl, PtrTraits> &
    WeakPtr<T, WeakPtrImpl, PtrTraits>::operator=(const WeakPtr<U, WeakPtrImpl, PtrTraits> &o)
  {
    _impl = weak_ptr_impl_cast<T, U>(o._impl.get());
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
    _impl = adoptRef(weak_ptr_impl_cast<T, U>(o._impl.leakRef()));
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
    _impl = &weak_ptr_impl_cast<T, U>(o._impl.get());
#if KRYS_ENV(DEV)
    m_shouldEnableAssertions = o.enableWeakPtrThreadingAssertions() == EnabledWeakPtrThreadAsserts::Yes;
#endif
    return *this;
  }

  template <typename T, typename WeakPtrImpl, typename PtrTraits>
  template <typename U>
  inline WeakPtr<T, WeakPtrImpl, PtrTraits> &
    WeakPtr<T, WeakPtrImpl, PtrTraits>::operator=(WeakRef<U, WeakPtrImpl> &&o)
  {
    _impl = adoptRef(weak_ptr_impl_cast<T, U>(o._impl.leakRef()));
#if KRYS_ENV(DEV)
    m_shouldEnableAssertions = o.enableWeakPtrThreadingAssertions() == EnabledWeakPtrThreadAsserts::Yes;
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
    static_assert(!SameType<Source, Target>, "Unnecessary cast to same type");
    static_assert(DerivedFrom<Target, Source>, "Should be a downcast");
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

  template <class T, typename = enable_if_t<!IsSmartPtr<T>::value>>
  WeakPtr(const T *value, EnabledWeakPtrThreadAsserts = EnabledWeakPtrThreadAsserts::Yes)
    -> WeakPtr<T, typename T::TWeakPtrImpl, RawPtrTraits<T>>;

  template <class T, typename = enable_if_t<!IsSmartPtr<T>::value && !IsPointer<T>>>
  WeakPtr(const T &value, EnabledWeakPtrThreadAsserts = EnabledWeakPtrThreadAsserts::Yes)
    -> WeakPtr<T, typename T::TWeakPtrImpl, RawPtrTraits<T>>;

  template <class T, typename = enable_if_t<!IsSmartPtr<T>::value>>
  WeakPtr(const Ref<T> &value, EnabledWeakPtrThreadAsserts = EnabledWeakPtrThreadAsserts::Yes)
    -> WeakPtr<T, typename T::TWeakPtrImpl, RawPtrTraits<T>>;

  template <class T, typename = enable_if_t<!IsSmartPtr<T>::value>>
  WeakPtr(const RefPtr<T> &value, EnabledWeakPtrThreadAsserts = EnabledWeakPtrThreadAsserts::Yes)
    -> WeakPtr<T, typename T::TWeakPtrImpl, RawPtrTraits<T>>;

  template <typename T, typename PtrTraits = RawPtrTraits<SingleThreadWeakPtrImpl>>
  using SingleThreadWeakPtr = WeakPtr<T, SingleThreadWeakPtrImpl, PtrTraits>;
}
