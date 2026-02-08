#pragma once

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
            EnabledWeakPtrThreadAsserts shouldEnableAssertions = EnabledWeakPtrThreadAsserts::Yes)
    requires(!IsSmartPtr<T>::value && !std::is_pointer_v<T>)
        : _impl(object.WeakImpl())
#if KRYS_ENV(DEV)
          ,
          m_shouldEnableAssertions(shouldEnableAssertions == EnabledWeakPtrThreadAsserts::Yes)
#endif
    {
      (void)shouldEnableAssertions;
    }

    explicit WeakRef(Ref<WeakPtrImpl> &&impl,
                     EnabledWeakPtrThreadAsserts shouldEnableAssertions = EnabledWeakPtrThreadAsserts::Yes)
        : _impl(Krys::Move(impl))
#if KRYS_ENV(DEV)
          ,
          m_shouldEnableAssertions(shouldEnableAssertions == EnabledWeakPtrThreadAsserts::Yes)
#endif
    {
      (void)shouldEnableAssertions;
    }

    WeakRef(HashTableDeletedValueType) : _impl(HashTableDeletedValue)
    {
    }
    WeakRef(HashTableEmptyValueType) : _impl(HashTableEmptyValue)
    {
    }

    bool IsHashTableDeletedValue() const
    {
      return _impl.IsHashTableDeletedValue();
    }
    bool IsHashTableEmptyValue() const
    {
      return _impl.IsHashTableEmptyValue();
    }

    WeakPtrImpl &Impl() const
    {
      return _impl;
    }
    Ref<WeakPtrImpl> releaseImpl()
    {
      return Krys::Move(_impl);
    }

    T *ptrAllowingHashTableEmptyValue() const
    {
      static_assert(HasRefPtrMemberFunctions<T>::value || HasCheckedPtrMemberFunctions<T>::value,
                    "Classes that offer weak pointers must also offer RefPtr or CheckedPtr");

      return !_impl.IsHashTableEmptyValue() ? static_cast<T *>(_impl->template get<T>()) : nullptr;
    }

    T *ptr() const
    {
      static_assert(HasRefPtrMemberFunctions<T>::value || HasCheckedPtrMemberFunctions<T>::value,
                    "Classes that offer weak pointers must also offer RefPtr or CheckedPtr");

      auto *ptr = static_cast<T *>(_impl->template get<T>());
      assert(ptr);
      return ptr;
    }

    T &get() const
    {
      static_assert(HasRefPtrMemberFunctions<T>::value || HasCheckedPtrMemberFunctions<T>::value,
                    "Classes that offer weak pointers must also offer RefPtr or CheckedPtr");

      auto *ptr = static_cast<T *>(_impl->template get<T>());
      assert(ptr);
      return *ptr;
    }

    operator T &() const
    {
      return get();
    }

    T *operator->() const
    {
      return ptr();
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
    Ref<WeakPtrImpl> _impl;
#if KRYS_ENV(DEV)
    bool m_shouldEnableAssertions {true};
#endif
  };

  template <class T>
  requires(!IsSmartPtr<T>::value && !std::is_pointer_v<T>)
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
