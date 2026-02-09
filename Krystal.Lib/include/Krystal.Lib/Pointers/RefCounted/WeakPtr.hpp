#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Detection/Environment.hpp"
#include "Krystal.Lib/Pointers/GetPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/CanMakeWeakPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/CompactRefPtrTuple.hpp"
#include "Krystal.Lib/Pointers/RefCounted/TypeTraits.hpp"
#include "Krystal.Lib/Pointers/RefCounted/WeakPtrFactory.hpp"
#include "Krystal.Lib/Pointers/RefCounted/WeakPtrImpl.hpp"
#include "Krystal.Lib/Pointers/RefCounted/WeakRef.hpp"

namespace Krys
{
  template <typename T, typename U, typename WeakPtrImpl>
  inline RawPtr<WeakPtrImpl> weak_ptr_impl_cast(RawPtr<WeakPtrImpl> impl) noexcept;

  template <typename T, typename U, typename WeakPtrImpl>
  inline WeakPtrImpl &weak_ptr_impl_cast(WeakPtrImpl &impl) noexcept;

  template <typename T, typename WeakPtrImpl, typename PtrTraits>
  class WeakPtr
  {
  public:
    WeakPtr() noexcept = default;

    WeakPtr(std::nullptr_t) noexcept
    {
    }

    template <typename U>
    WeakPtr(const WeakPtr<U, WeakPtrImpl, PtrTraits> &o) noexcept
        : _impl(weak_ptr_impl_cast<T, U>(o._impl.get()))
#if KRYS_ENV(DEV)
          ,
          _shouldEnableAssertions(o._shouldEnableAssertions)
#endif
    {
    }

    template <typename U>
    WeakPtr(WeakPtr<U, WeakPtrImpl, PtrTraits> &&o) noexcept
        : _impl(AdoptRef(weak_ptr_impl_cast<T, U>(o._impl.LeakRef())))
#if KRYS_ENV(DEV)
          ,
          _shouldEnableAssertions(o._shouldEnableAssertions)
#endif
    {
    }

    template <typename U>
    WeakPtr(const WeakRef<U, WeakPtrImpl> &o) noexcept
        : _impl(&weak_ptr_impl_cast<T, U>(o.Impl()))
#if KRYS_ENV(DEV)
          ,
          _shouldEnableAssertions(o.EnableThreadAsserts() == EnabledWeakPtrThreadAsserts::Yes)
#endif
    {
    }

    template <typename U>
    WeakPtr(WeakRef<U, WeakPtrImpl> &&o) noexcept
        : _impl(AdoptRef(weak_ptr_impl_cast<T, U>(o.ReleaseImpl().LeakRef())))
#if KRYS_ENV(DEV)
          ,
          _shouldEnableAssertions(o.EnableThreadAsserts() == EnabledWeakPtrThreadAsserts::Yes)
#endif
    {
    }

    template <typename = enable_if_t<!IsSmartPtr<T>::value>>
    WeakPtr(const T *object,
            EnabledWeakPtrThreadAsserts shouldEnableAssertions = EnabledWeakPtrThreadAsserts::Yes) noexcept
        : _impl(object ? &object->WeakImpl() : nullptr)
#if KRYS_ENV(DEV)
          ,
          _shouldEnableAssertions(shouldEnableAssertions == EnabledWeakPtrThreadAsserts::Yes)
#endif
    {
      (void)shouldEnableAssertions;
      assert(!object || object == _impl->template get<T>());
    }

    template <typename = enable_if_t<!IsSmartPtr<T>::value && !IsPointer<T>>>
    WeakPtr(const T &object,
            EnabledWeakPtrThreadAsserts shouldEnableAssertions = EnabledWeakPtrThreadAsserts::Yes) noexcept
        : _impl(&object.WeakImpl())
#if KRYS_ENV(DEV)
          ,
          _shouldEnableAssertions(shouldEnableAssertions == EnabledWeakPtrThreadAsserts::Yes)
#endif
    {
      (void)shouldEnableAssertions;
      assert(&object == _impl->template get<T>());
    }

    template <typename = enable_if_t<!IsSmartPtr<T>::value>>
    WeakPtr(const Ref<T> &object,
            EnabledWeakPtrThreadAsserts shouldEnableAssertions = EnabledWeakPtrThreadAsserts::Yes) noexcept
        : WeakPtr(object.get(), shouldEnableAssertions)
    {
    }

    template <typename = enable_if_t<!IsSmartPtr<T>::value>>
    WeakPtr(const RefPtr<T> &object,
            EnabledWeakPtrThreadAsserts shouldEnableAssertions = EnabledWeakPtrThreadAsserts::Yes) noexcept
        : WeakPtr(object.get(), shouldEnableAssertions)
    {
    }

    template <typename OtherPtrTraits>
    explicit WeakPtr(RefPtr<WeakPtrImpl, OtherPtrTraits> impl) noexcept : _impl(Krys::Move(impl))
    {
    }

    WeakPtr(HashTableDeletedValueType) noexcept : _impl(HashTableDeletedValue)
    {
    }

    WeakPtr(HashTableEmptyValueType) noexcept : _impl(HashTableEmptyValue)
    {
    }

    KRYS_NODISCARD bool IsHashTableDeletedValue() const noexcept
    {
      return _impl.IsHashTableDeletedValue();
    }

    KRYS_NODISCARD bool IsHashTableEmptyValue() const noexcept
    {
      return !_impl;
    }

    KRYS_NODISCARD bool IsWeakNullValue() const noexcept
    {
      return !*_impl;
    }

    RawPtr<T> PtrAllowingHashTableEmptyValue() const noexcept
    {
      static_assert(HasRefPtrMemberFunctions<T>::value || HasCheckedPtrMemberFunctions<T>::value,
                    "Classes that offer weak pointers must also offer RefPtr or CheckedPtr");

      return !_impl.IsHashTableEmptyValue() ? static_cast<RawPtr<T>>(_impl->template get<T>()) : nullptr;
    }

    RefPtr<WeakPtrImpl, PtrTraits> ReleaseImpl() noexcept
    {
      return Krys::Move(_impl);
    }

    RawPtr<T> get() const noexcept
    {
      static_assert(HasRefPtrMemberFunctions<T>::value || HasCheckedPtrMemberFunctions<T>::value,
                    "Classes that offer weak pointers must also offer RefPtr or CheckedPtr");
      return _impl ? static_cast<RawPtr<T>>(_impl->template get<T>()) : nullptr;
    }

    WeakRef<T, WeakPtrImpl> ReleaseNonNull() noexcept
    {
      return WeakRef<T, WeakPtrImpl> {_impl.ReleaseNonNull(), EnableThreadAsserts()};
    }

    bool operator!() const noexcept
    {
      return !_impl || !*_impl;
    }
    explicit operator bool() const noexcept
    {
      return _impl && *_impl;
    }

    WeakPtr &operator=(std::nullptr_t) noexcept
    {
      _impl = nullptr;
      return *this;
    }
    template <typename U>
    WeakPtr &operator=(const WeakPtr<U, WeakPtrImpl, PtrTraits> &o) noexcept
    {
      _impl = weak_ptr_impl_cast<T, U>(o._impl.get());
#if KRYS_ENV(DEV)
      _shouldEnableAssertions = o._shouldEnableAssertions;
#endif
      return *this;
    }

    template <typename U>
    WeakPtr &operator=(WeakPtr<U, WeakPtrImpl, PtrTraits> &&o) noexcept
    {
      _impl = AdoptRef(weak_ptr_impl_cast<T, U>(o._impl.LeakRef()));
#if KRYS_ENV(DEV)
      _shouldEnableAssertions = o._shouldEnableAssertions;
#endif
      return *this;
    }

    template <typename U>
    WeakPtr &operator=(const WeakRef<U, WeakPtrImpl> &o) noexcept
    {
      _impl = &weak_ptr_impl_cast<T, U>(o._impl.get());
#if KRYS_ENV(DEV)
      _shouldEnableAssertions = o.EnableThreadAsserts() == EnabledWeakPtrThreadAsserts::Yes;
#endif
      return *this;
    }

    template <typename U>
    WeakPtr &operator=(WeakRef<U, WeakPtrImpl> &&o) noexcept
    {
      _impl = AdoptRef(weak_ptr_impl_cast<T, U>(o._impl.leakRef()));
#if KRYS_ENV(DEV)
      _shouldEnableAssertions = o.EnableThreadAsserts() == EnabledWeakPtrThreadAsserts::Yes;
#endif
      return *this;
    }

    RawPtr<T> operator->() const noexcept
    {
      static_assert(HasRefPtrMemberFunctions<T>::value || HasCheckedPtrMemberFunctions<T>::value,
                    "Classes that offer weak pointers must also offer RefPtr or CheckedPtr");

      auto *result = get();
      assert(result);
      return result;
    }

    T &operator*() const noexcept
    {
      static_assert(HasRefPtrMemberFunctions<T>::value || HasCheckedPtrMemberFunctions<T>::value,
                    "Classes that offer weak pointers must also offer RefPtr or CheckedPtr");

      auto *result = get();
      assert(result);
      return *result;
    }

    void clear() noexcept
    {
      _impl = nullptr;
    }

    EnabledWeakPtrThreadAsserts EnableThreadAsserts() const noexcept
    {
#if KRYS_ENV(DEV)
      return _shouldEnableAssertions ? EnabledWeakPtrThreadAsserts::Yes : EnabledWeakPtrThreadAsserts::No;
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

    explicit WeakPtr(Ref<WeakPtrImpl> &&ref, EnabledWeakPtrThreadAsserts shouldEnableAssertions) noexcept
        : _impl(Krys::Move(ref))
#if KRYS_ENV(DEV)
          ,
          _shouldEnableAssertions(shouldEnableAssertions == EnabledWeakPtrThreadAsserts::Yes)
#endif
    {
      (void)shouldEnableAssertions;
    }

    RefPtr<WeakPtrImpl, PtrTraits> _impl;
#if KRYS_ENV(DEV)
    bool _shouldEnableAssertions {true};
#endif
  };

  template <typename T, typename U, typename WeakPtrImpl>
  inline RawPtr<WeakPtrImpl> weak_ptr_impl_cast(RawPtr<WeakPtrImpl> impl) noexcept
  {
    static_assert(SameType<typename T::TWeakValue, typename U::TWeakValue>, "Invalid weak pointer cast");
    return impl;
  }

  template <typename T, typename U, typename WeakPtrImpl>
  inline WeakPtrImpl &weak_ptr_impl_cast(WeakPtrImpl &impl) noexcept
  {
    static_assert(SameType<typename T::TWeakValue, typename U::TWeakValue>, "Invalid weak pointer cast");
    return impl;
  }

  template <typename T, typename WeakPtrImpl, typename PtrTraits>
  struct GetPtrHelper<WeakPtr<T, WeakPtrImpl, PtrTraits>>
  {
    using pointer_type = RawPtr<T>;
    using underlying_type = T;

    KRYS_NODISCARD static pointer_type GetPtr(const WeakPtr<T, WeakPtrImpl, PtrTraits> &p) noexcept
    {
      return const_cast<pointer_type>(p.get());
    }
  };

  template <typename T, typename WeakPtrImpl, typename PtrTraits>
  struct IsSmartPtr<WeakPtr<T, WeakPtrImpl, PtrTraits>>
  {
    static constexpr bool value = true;
    static constexpr bool nullable = true;
  };

  template <typename TExpected, typename TArg, typename WeakPtrImpl, typename PtrTraits>
  inline bool Is(WeakPtr<TArg, WeakPtrImpl, PtrTraits> &source) noexcept
  {
    return Is<TExpected>(source.get());
  }

  template <typename TExpected, typename ArgType, typename WeakPtrImpl, typename PtrTraits>
  inline bool Is(const WeakPtr<ArgType, WeakPtrImpl, PtrTraits> &source) noexcept
  {
    return Is<TExpected>(source.get());
  }

  template <typename Target, typename Source, typename WeakPtrImpl, typename PtrTraits>
  inline WeakPtr<match_constness_t<Source, Target>, WeakPtrImpl, PtrTraits>
    Downcast(WeakPtr<Source, WeakPtrImpl, PtrTraits> source) noexcept
  {
    static_assert(!SameType<Source, Target>, "Unnecessary cast to same type");
    static_assert(DerivedFrom<Target, Source>, "Should be a downcast");
    assert(!source || Is<Target>(*source));
    return WeakPtr<match_constness_t<Source, Target>, WeakPtrImpl, PtrTraits> {
      UnsafeRefPtrDowncast<match_constness_t<Source, Target>>(source.ReleaseImpl()),
      source.EnableThreadAsserts()};
  }

  template <typename Target, typename Source, typename WeakPtrImpl, typename PtrTraits>
  inline WeakPtr<match_constness_t<Source, Target>, WeakPtrImpl, PtrTraits>
    DynamicDowncast(WeakPtr<Source, WeakPtrImpl, PtrTraits> source) noexcept
  {
    static_assert(!SameType<Source, Target>, "Unnecessary cast to same type");
    static_assert(DerivedFrom<Target, Source>, "Should be a downcast");
    if (!Is<Target>(source))
    {
      return nullptr;
    }
    return WeakPtr<match_constness_t<Source, Target>, WeakPtrImpl, PtrTraits> {
      UnsafeRefPtrDowncast<match_constness_t<Source, Target>, WeakPtrImpl>(source.ReleaseImpl()),
      source.EnableThreadAsserts()};
  }

  template <typename T, typename U, typename WeakPtrImpl, typename PtrTraits>
  inline bool operator==(const WeakPtr<T, WeakPtrImpl, PtrTraits> &a,
                         const WeakPtr<U, WeakPtrImpl, PtrTraits> &b) noexcept
  {
    return a.get() == b.get();
  }

  template <typename T, typename U, typename WeakPtrImpl, typename PtrTraits>
  inline bool operator==(const WeakPtr<T, WeakPtrImpl, PtrTraits> &a, RawPtr<U> b) noexcept
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
