#pragma once

#include "Krystal.Lib/Pointers/IntrusivePtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys
{
  template <typename T, typename Traits = RefCountedTraits>
  class Ref
  {
  private:
    RefPtr<T, Traits> _p;

  public:
    static constexpr Ref NoRef(T &p) noexcept
    {
      return Ref(RefPtr<T, Traits>::NoRef(&p));
    }

    static constexpr Ref WithRef(T &p) noexcept
    {
      return Ref(RefPtr<T, Traits>::WithRef(&p));
    }

    using element_type = T;

    explicit constexpr Ref(RefPtr<T, Traits> &&p) noexcept : _p(std::move(p))
    {
      assert(_p);
    }

    Ref() = delete;
    Ref(std::nullptr_t) = delete;

    Ref(const Ref &) noexcept = default;
    Ref(Ref &&) noexcept = default;
    Ref &operator=(const Ref &) noexcept = default;
    Ref &operator=(Ref &&) noexcept = default;

    template <typename U>
    requires(std::is_convertible_v<U *, T *>)
    Ref(Ref<U, Traits> &&other) noexcept : _p(std::move(other._p))
    {
      static_assert(std::is_base_of_v<T, U> || std::is_same_v<T, U>);
    }

    KRYS_NODISCARD constexpr T *get() const noexcept
    {
      return _p.get();
    }

    KRYS_NODISCARD constexpr T &operator*() const noexcept
    {
      return *_p;
    }

    KRYS_NODISCARD constexpr T *operator->() const noexcept
    {
      return _p.get();
    }

    KRYS_NODISCARD constexpr RefPtr<T, Traits> to_ptr() const noexcept
    {
      return _p;
    }

    friend bool operator==(const Ref &a, const Ref &b) noexcept
    {
      return a.get() == b.get();
    }

    friend bool operator==(const Ref&a, const RefPtr<T, Traits>& b) noexcept
    {
      return a.get() == b.get();
    }

    friend bool operator==(const RefPtr<T, Traits>& a, const Ref &b) noexcept
    {
      return a.get() == b.get();
    }
  };

  template <typename T, typename Traits = RefCountedTraits, typename... Args>
  requires(Constructible<T, Args...>)
  KRYS_NODISCARD constexpr inline Ref<T, Traits> CreateRef(Args &&...args)
  {
    T *p = new T(std::forward<Args>(args)...);
    return Ref<T, Traits>::NoRef(*p);
  }

  template <typename T, typename Traits = RefCountedTraits>
  KRYS_NODISCARD constexpr Ref<T, Traits> RefRetain(T &ptr) noexcept
  {
    return Ref<T, Traits>::WithRef(ptr);
  }

  template <typename T, typename Traits = RefCountedTraits>
  KRYS_NODISCARD constexpr Ref<T, Traits> RefAttach(T &ptr) noexcept
  {
    return Ref<T, Traits>::NoRef(ptr);
  }
}
