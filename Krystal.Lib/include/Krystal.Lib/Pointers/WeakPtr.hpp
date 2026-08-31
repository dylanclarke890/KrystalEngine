#pragma once

#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Pointers/Concepts.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Pointers/WeakPtrImpl.hpp"
#include <cassert>

namespace Krys::detail
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
    KRYS_NODISCARD constexpr static IntrusiveWeakPtr WithRef(RawPtr<const T> ptr) noexcept
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
        assert(_impl);
      }
    }

    KRYS_ALWAYS_INLINE constexpr IntrusiveWeakPtr(IntrusiveWeakPtr &&o) noexcept : _impl(Krys::Move(o._impl))
    {
    }

    template <typename X, typename Y, typename Z, IsNullable Nullable>
    KRYS_ALWAYS_INLINE constexpr IntrusiveWeakPtr(IntrusiveWeakPtr<X, Y, Z, Nullable> &&o) noexcept
        : _impl(Krys::Move(o._impl))
    {
      if constexpr (!nullable)
      {
        assert(_impl);
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
      _impl = Krys::Move(o._impl);
      return *this;
    }

    template <typename X, typename Y, typename Z, IsNullable Nullable>
    constexpr IntrusiveWeakPtr &operator=(IntrusiveWeakPtr<X, Y, Z, Nullable> &&o) noexcept
    {
      _impl = Krys::Move(o._impl);
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

      assert(this->get());

      RawPtr<T> result = this->get();
      return *result;
    }

    KRYS_ALWAYS_INLINE constexpr RawPtr<T> operator->() const noexcept KRYS_LIFETIME_BOUND
    {
      static_assert(SupportsRefPtr<T> || SupportsCheckedPtr<T>,
                    "Classes that offer weak pointers must also offer RefPtr or CheckedPtr");

      RawPtr<T> result = this->get();
      assert(result);
      return result;
    }

    template <typename X = T, typename TMember>
    requires(SameType<X, T>)
    constexpr TMember &operator->*(TMember X::*memptr) const noexcept
    {
      static_assert(SupportsRefPtr<T> || SupportsCheckedPtr<T>,
                    "Classes that offer weak pointers must also offer RefPtr or CheckedPtr");

      RawPtr<T> result = this->get();
      assert(result);
      return result->*memptr;
    }

    KRYS_NODISCARD constexpr RawPtr<T> get() const noexcept
    {
      static_assert(SupportsRefPtr<T> || SupportsCheckedPtr<T>,
                    "Classes that offer weak pointers must also offer RefPtr or CheckedPtr");

      return _impl ? static_cast<RawPtr<T>>(_impl->template get<T>()) : nullptr;
    }

    KRYS_NODISCARD constexpr RawPtr<T> release() noexcept
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
      assert(!object || object == _impl->template get<T>());
    }

    explicit constexpr IntrusiveWeakPtr(const T &object) noexcept
        : _impl(ShareRefPtr<Impl>(&object.WeakImpl()))
    {
      assert(&object == _impl->template get<T>());
    }
  };

  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  constexpr inline bool operator==(const IntrusiveWeakPtr<T, PtrTraits, RefPolicy, Nullable> &lhs,
                                   std::nullptr_t) noexcept
  {
    return lhs.get() == nullptr;
  }

  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  constexpr inline bool operator==(std::nullptr_t,
                                   const IntrusiveWeakPtr<T, PtrTraits, RefPolicy, Nullable> &rhs) noexcept
  {
    return nullptr == rhs.get();
  }

  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable, typename U>
  requires(ConvertibleTo<RawPtr<U>, RawPtr<T>>)
  constexpr inline bool operator==(const IntrusiveWeakPtr<T, PtrTraits, RefPolicy, Nullable> &lhs,
                                   RawPtr<U> rhs) noexcept
  {
    return lhs.get() == rhs;
  }

  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable, typename U,
            typename UPtrTraits, typename URefPolicy, IsNullable ONullable>
  requires(ConvertibleTo<RawPtr<U>, RawPtr<T>>)
  constexpr inline bool operator==(const IntrusiveWeakPtr<T, PtrTraits, RefPolicy, Nullable> &lhs,
                                   const IntrusiveWeakPtr<U, UPtrTraits, URefPolicy, ONullable> &rhs) noexcept
  {
    return lhs.get() == rhs.get();
  }
  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  constexpr inline bool operator!=(const IntrusiveWeakPtr<T, PtrTraits, RefPolicy, Nullable> &lhs,
                                   std::nullptr_t) noexcept
  {
    return !(lhs == nullptr);
  }

  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  constexpr inline bool operator!=(std::nullptr_t,
                                   const IntrusiveWeakPtr<T, PtrTraits, RefPolicy, Nullable> &rhs) noexcept
  {
    return !(nullptr == rhs);
  }

  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable, typename U>
  requires(ConvertibleTo<RawPtr<U>, RawPtr<T>>)
  constexpr inline bool operator!=(const IntrusiveWeakPtr<T, PtrTraits, RefPolicy, Nullable> &lhs,
                                   RawPtr<U> rhs) noexcept
  {
    return !(lhs == rhs);
  }

  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable, typename U,
            typename UPtrTraits, typename URefPolicy, IsNullable ONullable>
  requires(ConvertibleTo<RawPtr<U>, RawPtr<T>>)
  constexpr inline bool operator!=(const IntrusiveWeakPtr<T, PtrTraits, RefPolicy, Nullable> &lhs,
                                   const IntrusiveWeakPtr<U, UPtrTraits, URefPolicy, ONullable> &rhs) noexcept
  {
    return !(lhs == rhs);
  }

  template <typename TExpected, typename T, typename WeakPtrImpl, typename PtrTraits, IsNullable Nullable>
  KRYS_NODISCARD constexpr inline bool
    Is(const IntrusiveWeakPtr<T, WeakPtrImpl, PtrTraits, Nullable> &source) noexcept
  {
    return Is<TExpected>(source.get());
  }
}

namespace Krys
{
  template <typename T, typename Impl, typename PtrTraits, IsNullable Nullable>
  constexpr bool IsSmartPtr<::Krys::detail::IntrusiveWeakPtr<T, Impl, PtrTraits, Nullable>> = true;

  template <typename T, typename Impl, typename PtrTraits, IsNullable Nullable>
  constexpr bool IsNullableSmartPtr<::Krys::detail::IntrusiveWeakPtr<T, Impl, PtrTraits, Nullable>> =
    ::Krys::detail::IntrusiveWeakPtr<T, Impl, PtrTraits, Nullable>::nullable;

  template <typename T, typename Impl = WeakPtrImpl, typename PtrTraits = RawPtrTraits<WeakPtrImpl>>
  using WeakPtr = ::Krys::detail::IntrusiveWeakPtr<T, Impl, PtrTraits, IsNullable(true)>;

  template <typename T, typename Impl = WeakPtrImpl, typename PtrTraits = RawPtrTraits<WeakPtrImpl>>
  using WeakRef = ::Krys::detail::IntrusiveWeakPtr<T, Impl, PtrTraits, IsNullable(false)>;

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
