#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Detection/Environment.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/CheckedPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/WeakPtrImpl.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

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
      static_assert(SupportsRefPtr<T> || SupportsCheckedPtr<T>,
                    "Classes that offer weak pointers must also offer RefPtr or CheckedPtr");

      auto *ptr = static_cast<RawPtr<T>>(_impl->template get<T>());
      assert(ptr);
      return ptr;
    }

    RawPtr<T> get() const noexcept
    {
      static_assert(SupportsRefPtr<T> || SupportsCheckedPtr<T>,
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

  template <typename TExpected, typename TArg, typename WeakPtrImpl>
  constexpr inline bool Is(const WeakRef<TArg, WeakPtrImpl> &source) noexcept
  {
    return Is<TExpected>(source.get());
  }
}
