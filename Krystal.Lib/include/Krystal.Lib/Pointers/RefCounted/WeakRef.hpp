#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Detection/Environment.hpp"
#include "Krystal.Lib/Pointers/GetPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/ThreadSafeRefCounted.hpp"
#include "Krystal.Lib/Pointers/RefCounted/TypeTraits.hpp"
#include "Krystal.Lib/Pointers/RefCounted/WeakPtrImpl.hpp"

namespace Krys
{
  template <typename T, typename WeakPtrImpl, typename PtrTraits>
  class WeakPtr;

  enum class EnabledWeakPtrThreadAsserts : bool
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
            EnabledWeakPtrThreadAsserts shouldEnableAssertions = EnabledWeakPtrThreadAsserts::Yes) noexcept
    requires(!IsSmartPtr<T>::value && !IsPointer<T>)
        : _impl(object.WeakImpl())
#if KRYS_ENV(DEV)
          ,
          _shouldEnableAssertions(shouldEnableAssertions == EnabledWeakPtrThreadAsserts::Yes)
#endif
    {
      (void)shouldEnableAssertions;
    }

    explicit WeakRef(Ref<WeakPtrImpl> &&impl, EnabledWeakPtrThreadAsserts shouldEnableAssertions =
                                                EnabledWeakPtrThreadAsserts::Yes) noexcept
        : _impl(Krys::Move(impl))
#if KRYS_ENV(DEV)
          ,
          _shouldEnableAssertions(shouldEnableAssertions == EnabledWeakPtrThreadAsserts::Yes)
#endif
    {
      (void)shouldEnableAssertions;
    }

    WeakRef(HashTableDeletedValueType) noexcept : _impl(HashTableDeletedValue)
    {
    }

    WeakRef(HashTableEmptyValueType) noexcept : _impl(HashTableEmptyValue)
    {
    }

    bool IsHashTableDeletedValue() const noexcept
    {
      return _impl.IsHashTableDeletedValue();
    }

    bool IsHashTableEmptyValue() const noexcept
    {
      return _impl.IsHashTableEmptyValue();
    }

    WeakPtrImpl &Impl() const noexcept
    {
      return _impl;
    }

    Ref<WeakPtrImpl> ReleaseImpl() noexcept
    {
      return Krys::Move(_impl);
    }

    RawPtr<T> PtrAllowingHashTableEmptyValue() const noexcept
    {
      static_assert(HasRefPtrMemberFunctions<T>::value || HasCheckedPtrMemberFunctions<T>::value,
                    "Classes that offer weak pointers must also offer RefPtr or CheckedPtr");

      return !_impl.IsHashTableEmptyValue() ? static_cast<RawPtr<T>>(_impl->template get<T>()) : nullptr;
    }

    RawPtr<T> ptr() const noexcept
    {
      static_assert(HasRefPtrMemberFunctions<T>::value || HasCheckedPtrMemberFunctions<T>::value,
                    "Classes that offer weak pointers must also offer RefPtr or CheckedPtr");

      auto *ptr = static_cast<RawPtr<T>>(_impl->template get<T>());
      assert(ptr);
      return ptr;
    }

    T &get() const noexcept
    {
      static_assert(HasRefPtrMemberFunctions<T>::value || HasCheckedPtrMemberFunctions<T>::value,
                    "Classes that offer weak pointers must also offer RefPtr or CheckedPtr");

      auto *ptr = static_cast<RawPtr<T>>(_impl->template get<T>());
      assert(ptr);
      return *ptr;
    }

    operator T &() const noexcept
    {
      return get();
    }

    RawPtr<T> operator->() const noexcept
    {
      return ptr();
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
    Ref<WeakPtrImpl> _impl;
#if KRYS_ENV(DEV)
    bool _shouldEnableAssertions {true};
#endif
  };

  template <class T>
  requires(!IsSmartPtr<T>::value && !IsPointer<T>)
  WeakRef(const T &value, EnabledWeakPtrThreadAsserts = EnabledWeakPtrThreadAsserts::Yes)
    -> WeakRef<T, typename T::TWeakPtrImpl>;

  template <typename T, typename WeakPtrImpl>
  struct GetPtrHelper<WeakRef<T, WeakPtrImpl>>
  {
    using pointer_type = RawPtr<T>;
    using underlying_type = T;

    KRYS_NODISCARD static pointer_type GetPtr(const WeakRef<T, WeakPtrImpl> &p) noexcept
    {
      return const_cast<pointer_type>(p.ptr());
    }
  };

  template <typename T, typename WeakPtrImpl>
  struct IsSmartPtr<WeakRef<T, WeakPtrImpl>>
  {
    static constexpr bool value = true;
    static constexpr bool nullable = false;
  };

  template <typename T>
  using SingleThreadWeakRef = WeakRef<T, SingleThreadWeakPtrImpl>;

  template <typename ExpectedType, typename ArgType, typename WeakPtrImpl>
  inline bool Is(WeakRef<ArgType, WeakPtrImpl> &source) noexcept
  {
    return Is<ExpectedType>(source.get());
  }

  template <typename ExpectedType, typename ArgType, typename WeakPtrImpl>
  inline bool Is(const WeakRef<ArgType, WeakPtrImpl> &source) noexcept
  {
    return Is<ExpectedType>(source.get());
  }

  template <typename Target, typename Source, typename WeakPtrImpl>
  inline WeakRef<match_constness_t<Source, Target>, WeakPtrImpl>
    Downcast(WeakRef<Source, WeakPtrImpl> source) noexcept
  {
    static_assert(!SameType<Source, Target>, "Unnecessary cast to same type");
    static_assert(DerivedFrom<Target, Source>, "Should be a downcast");
    assert(Is<Target>(source));
    return WeakRef<match_constness_t<Source, Target>, WeakPtrImpl> {
      UnsafeRefDowncast<match_constness_t<Source, Target>>(source.ReleaseImpl()),
      source.EnableThreadAsserts()};
  }

  template <typename Target, typename Source, typename WeakPtrImpl>
  inline WeakPtr<match_constness_t<Source, Target>, WeakPtrImpl, RawPtrTraits<Target>>
    DynamicDowncast(WeakRef<Source, WeakPtrImpl> source) noexcept
  {
    static_assert(!SameType<Source, Target>, "Unnecessary cast to same type");
    static_assert(DerivedFrom<Target, Source>, "Should be a downcast");
    if (!Is<Target>(source))
    {
      return nullptr;
    }
    return WeakPtr<match_constness_t<Source, Target>, WeakPtrImpl, RawPtrTraits<Target>> {
      UnsafeRefDowncast<match_constness_t<Source, Target>>(source.ReleaseImpl()),
      source.EnableThreadAsserts()};
  }
}
