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
      return Ref(RefPtr<T, Traits>(&p));
    }

    static constexpr Ref WithRef(T &p) noexcept
    {
      RefPtr<T, Traits>::DoAddRef(&p);
      return Ref(RefPtr<T, Traits>(&p));
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
  };

  template <typename T, typename Traits = RefCountedTraits, typename... Args>
  KRYS_NODISCARD constexpr inline Ref<T, Traits> CreateRef(Args &&...args)
  {
    return Ref<T, Traits>::NoRef(new T(std::forward<Args>(args)...));
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
