#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Detection/Environment.hpp"
#include "Krystal.Lib/Pointers/RefCounted/RefCountedThreadSafe.hpp"
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
  private:
    Ref<WeakPtrImpl> _impl;
#if KRYS_ENV(DEV)
    bool _shouldEnableAssertions {true};
#endif

  public:
    WeakRef(const T &object,
            EnabledWeakPtrThreadAsserts shouldEnableAssertions = EnabledWeakPtrThreadAsserts::Yes) noexcept
    requires(!IsPointer<T>)
        : _impl(ShareRef(object.WeakImpl()))
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

    WeakPtrImpl &Impl() const noexcept
    {
      return *_impl.get();
    }

    Ref<WeakPtrImpl> ReleaseImpl() noexcept
    {
      return Krys::Move(_impl);
    }

    RawPtr<T> ptr() const noexcept
    {
      static_assert(HasRefPtrMemberFunctions<T>::value || HasCheckedPtrMemberFunctions<T>::value,
                    "Classes that offer weak pointers must also offer RefPtr or CheckedPtr");

      auto *ptr = static_cast<RawPtr<T>>(_impl->template get<T>());
      assert(ptr);
      return ptr;
    }

    T *get() const noexcept
    {
      static_assert(HasRefPtrMemberFunctions<T>::value || HasCheckedPtrMemberFunctions<T>::value,
                    "Classes that offer weak pointers must also offer RefPtr or CheckedPtr");

      return _impl == nullptr ? nullptr : static_cast<RawPtr<T>>(_impl->template get<T>());
    }

    operator T &() const noexcept
    {
      return *get();
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
  };

  template <class T>
  requires(!IsPointer<T>)
  WeakRef(const T &value, EnabledWeakPtrThreadAsserts = EnabledWeakPtrThreadAsserts::Yes)
    -> WeakRef<T, typename T::TWeakPtrImpl>;

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
