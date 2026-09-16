#pragma once

#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/StronglyTypedValue.hpp"
#include "Krystal.Core/Types/WeakPtrImpl.hpp"
#include "Krystal.Core/TypeTraits.hpp"
#include <cassert>

namespace krys::detail
{
  template <typename T, typename Impl, typename PtrTraits, IsNullable Nullable>
  class IntrusiveWeakPtr
  {
    template <typename, typename, typename, IsNullable>
    friend class IntrusiveWeakPtr;

    template <typename, typename, bool>
    friend class WeakPtrFactory;

  public:
    using type = T;
    using weak_pointer_impl = Impl;
    using pointer_traits = PtrTraits;
    constexpr static bool nullable = Nullable.Value;

  private:
    RefPtr<weak_pointer_impl, pointer_traits> _impl;

  public:
    KRYS_NODISCARD constexpr static IntrusiveWeakPtr WithRef(const T *ptr) noexcept
    {
      return IntrusiveWeakPtr(ptr);
    }

    KRYS_NODISCARD constexpr static IntrusiveWeakPtr WithRef(const T &ref) noexcept
    {
      return IntrusiveWeakPtr(ref);
    }

    KRYS_ALWAYS_INLINE constexpr IntrusiveWeakPtr() noexcept
    requires(nullable)
        : _impl(nullptr)
    {
    }

    KRYS_ALWAYS_INLINE constexpr IntrusiveWeakPtr(std::nullptr_t) noexcept
    requires(nullable)
        : _impl(nullptr)
    {
    }

    KRYS_ALWAYS_INLINE constexpr IntrusiveWeakPtr(const IntrusiveWeakPtr &o) noexcept : _impl(o._impl)
    {
    }

    template <typename X, typename Y, typename Z, IsNullable Nullable>
    KRYS_ALWAYS_INLINE constexpr IntrusiveWeakPtr(const IntrusiveWeakPtr<X, Y, Z, Nullable> &o) noexcept
        : _impl(o._impl)
    {
      if constexpr (!nullable)
      {
        krys_assert(_impl);
      }
    }

    KRYS_ALWAYS_INLINE constexpr IntrusiveWeakPtr(IntrusiveWeakPtr &&o) noexcept : _impl(krys::move(o._impl))
    {
    }

    template <typename X, typename Y, typename Z, IsNullable Nullable>
    KRYS_ALWAYS_INLINE constexpr IntrusiveWeakPtr(IntrusiveWeakPtr<X, Y, Z, Nullable> &&o) noexcept
        : _impl(krys::move(o._impl))
    {
      if constexpr (!nullable)
      {
        krys_assert(_impl);
      }
    }

    KRYS_ALWAYS_INLINE constexpr IntrusiveWeakPtr(const Ref<T> &object) noexcept
        : IntrusiveWeakPtr(object.get())
    {
    }

    KRYS_ALWAYS_INLINE constexpr IntrusiveWeakPtr(const RefPtr<T> &object) noexcept
        : IntrusiveWeakPtr(object.get())
    {
    }

    KRYS_ALWAYS_INLINE constexpr IntrusiveWeakPtr(const Ref<Impl> &impl) noexcept
        : _impl(ShareRefPtr<Impl>(impl.get()))
    {
    }

    KRYS_ALWAYS_INLINE constexpr IntrusiveWeakPtr(const RefPtr<Impl> &impl) noexcept
        : _impl(ShareRefPtr<Impl>(impl.get()))
    {
    }

    KRYS_ALWAYS_INLINE constexpr IntrusiveWeakPtr(Ref<Impl> &&impl) noexcept
        : _impl(AdoptRefPtr<Impl>(impl.release()))
    {
    }

    KRYS_ALWAYS_INLINE constexpr IntrusiveWeakPtr(RefPtr<Impl> &&impl) noexcept
        : _impl(AdoptRefPtr<Impl>(impl.release()))
    {
    }

    constexpr IntrusiveWeakPtr &operator=(std::nullptr_t) noexcept
    requires(nullable)
    {
      _impl = nullptr;
      return *this;
    }

    constexpr IntrusiveWeakPtr &operator=(const IntrusiveWeakPtr &o) noexcept
    {
      _impl = o._impl;
      return *this;
    }

    template <typename X, typename Y, typename Z, IsNullable Nullable>
    constexpr IntrusiveWeakPtr &operator=(const IntrusiveWeakPtr<X, Y, Z, Nullable> &o) noexcept
    {
      _impl = o._impl;
      return *this;
    }

    constexpr IntrusiveWeakPtr &operator=(IntrusiveWeakPtr &&o) noexcept
    {
      _impl = krys::move(o._impl);
      return *this;
    }

    template <typename X, typename Y, typename Z, IsNullable Nullable>
    constexpr IntrusiveWeakPtr &operator=(IntrusiveWeakPtr<X, Y, Z, Nullable> &&o) noexcept
    {
      _impl = krys::move(o._impl);
      return *this;
    }

    constexpr bool operator!() const noexcept
    {
      return !_impl || !*_impl;
    }

    constexpr explicit operator bool() const noexcept
    {
      return _impl && *_impl;
    }

    KRYS_ALWAYS_INLINE constexpr T &operator*() const noexcept KRYS_LIFETIME_BOUND
    {
      static_assert(SupportsRefPtr<T> || SupportsCheckedPtr<T>,
                    "Classes that offer weak pointers must also offer RefPtr or CheckedPtr");

      krys_assert(this->get());

      T *result = this->get();
      return *result;
    }

    KRYS_ALWAYS_INLINE constexpr T *operator->() const noexcept KRYS_LIFETIME_BOUND
    {
      static_assert(SupportsRefPtr<T> || SupportsCheckedPtr<T>,
                    "Classes that offer weak pointers must also offer RefPtr or CheckedPtr");

      T *result = this->get();
      krys_assert(result);
      return result;
    }

    template <typename X = T, typename TMember>
    requires(SameType<X, T>)
    constexpr TMember &operator->*(TMember X::*memptr) const noexcept
    {
      static_assert(SupportsRefPtr<T> || SupportsCheckedPtr<T>,
                    "Classes that offer weak pointers must also offer RefPtr or CheckedPtr");

      T *result = this->get();
      krys_assert(result);
      return result->*memptr;
    }

    KRYS_NODISCARD constexpr T *get() const noexcept
    {
      static_assert(SupportsRefPtr<T> || SupportsCheckedPtr<T>,
                    "Classes that offer weak pointers must also offer RefPtr or CheckedPtr");

      return _impl ? static_cast<T *>(_impl->template get<T>()) : nullptr;
    }

    KRYS_NODISCARD constexpr T *release() noexcept
    {
      static_assert(SupportsRefPtr<T> || SupportsCheckedPtr<T>,
                    "Classes that offer weak pointers must also offer RefPtr or CheckedPtr");

      return _impl.release();
    }

    constexpr void reset() noexcept
    {
      _impl = nullptr;
    }

    KRYS_NODISCARD constexpr RefPtr<T> lock() const noexcept
    {
      static_assert(SupportsRefPtr<T> || SupportsCheckedPtr<T>,
                    "Classes that offer weak pointers must also offer RefPtr or CheckedPtr");
      return _impl ? ShareRefPtr(_impl->template get<T>()) : nullptr;
    }

  private:
    explicit constexpr IntrusiveWeakPtr(const T *object) noexcept
        : _impl(ShareRefPtr<Impl>(object ? &object->WeakImpl() : nullptr))
    {
      krys_assert(!object || object == _impl->template get<T>());
    }

    explicit constexpr IntrusiveWeakPtr(const T &object) noexcept
        : _impl(ShareRefPtr<Impl>(&object.WeakImpl()))
    {
      krys_assert(&object == _impl->template get<T>());
    }
  };

  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  constexpr bool operator==(const IntrusiveWeakPtr<T, PtrTraits, RefPolicy, Nullable> &lhs,
                            std::nullptr_t) noexcept
  {
    return lhs.get() == nullptr;
  }

  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  constexpr bool operator==(std::nullptr_t,
                            const IntrusiveWeakPtr<T, PtrTraits, RefPolicy, Nullable> &rhs) noexcept
  {
    return nullptr == rhs.get();
  }

  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable, typename U>
  requires(ConvertibleTo<U *, T *>)
  constexpr bool operator==(const IntrusiveWeakPtr<T, PtrTraits, RefPolicy, Nullable> &lhs, U *rhs) noexcept
  {
    return lhs.get() == rhs;
  }

  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable, typename U,
            typename UPtrTraits, typename URefPolicy, IsNullable ONullable>
  requires(ConvertibleTo<U *, T *>)
  constexpr bool operator==(const IntrusiveWeakPtr<T, PtrTraits, RefPolicy, Nullable> &lhs,
                            const IntrusiveWeakPtr<U, UPtrTraits, URefPolicy, ONullable> &rhs) noexcept
  {
    return lhs.get() == rhs.get();
  }
  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  constexpr bool operator!=(const IntrusiveWeakPtr<T, PtrTraits, RefPolicy, Nullable> &lhs,
                            std::nullptr_t) noexcept
  {
    return !(lhs == nullptr);
  }

  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  constexpr bool operator!=(std::nullptr_t,
                            const IntrusiveWeakPtr<T, PtrTraits, RefPolicy, Nullable> &rhs) noexcept
  {
    return !(nullptr == rhs);
  }

  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable, typename U>
  requires(ConvertibleTo<U *, T *>)
  constexpr bool operator!=(const IntrusiveWeakPtr<T, PtrTraits, RefPolicy, Nullable> &lhs, U *rhs) noexcept
  {
    return !(lhs == rhs);
  }

  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable, typename U,
            typename UPtrTraits, typename URefPolicy, IsNullable ONullable>
  requires(ConvertibleTo<U *, T *>)
  constexpr bool operator!=(const IntrusiveWeakPtr<T, PtrTraits, RefPolicy, Nullable> &lhs,
                            const IntrusiveWeakPtr<U, UPtrTraits, URefPolicy, ONullable> &rhs) noexcept
  {
    return !(lhs == rhs);
  }

  template <typename TExpected, typename T, typename WeakPtrImpl, typename PtrTraits, IsNullable Nullable>
  KRYS_NODISCARD constexpr bool
    Is(const IntrusiveWeakPtr<T, WeakPtrImpl, PtrTraits, Nullable> &source) noexcept
  {
    return Is<TExpected>(source.get());
  }
}

namespace krys
{
  template <typename T, typename Impl, typename PtrTraits, IsNullable Nullable>
  constexpr bool IsSmartPtr<::krys::detail::IntrusiveWeakPtr<T, Impl, PtrTraits, Nullable>> = true;

  template <typename T, typename Impl, typename PtrTraits, IsNullable Nullable>
  constexpr bool IsNullableSmartPtr<::krys::detail::IntrusiveWeakPtr<T, Impl, PtrTraits, Nullable>> =
    ::krys::detail::IntrusiveWeakPtr<T, Impl, PtrTraits, Nullable>::nullable;

  template <typename T, typename Impl = WeakPtrImpl, typename PtrTraits = RawPtrTraits<WeakPtrImpl>>
  using WeakPtr = ::krys::detail::IntrusiveWeakPtr<T, Impl, PtrTraits, IsNullable(true)>;

  template <typename T, typename Impl = WeakPtrImpl, typename PtrTraits = RawPtrTraits<WeakPtrImpl>>
  using WeakRef = ::krys::detail::IntrusiveWeakPtr<T, Impl, PtrTraits, IsNullable(false)>;

  template <typename T, typename Impl = WeakPtrImpl, typename PtrTraits = RawPtrTraits<WeakPtrImpl>>
  requires(!IsPointer<T>)
  KRYS_NODISCARD constexpr WeakPtr<T, Impl, PtrTraits> CreateWeakPtr(const T *object) noexcept
  {
    return WeakPtr<T, Impl, PtrTraits>::WithRef(object);
  }

  template <typename T, typename Impl = WeakPtrImpl, typename PtrTraits = RawPtrTraits<WeakPtrImpl>>
  requires(!IsPointer<T>)
  KRYS_NODISCARD constexpr WeakRef<T, Impl, PtrTraits> CreateWeakRef(const T &object) noexcept
  {
    return WeakRef<T, Impl, PtrTraits>::WithRef(object);
  }
}
