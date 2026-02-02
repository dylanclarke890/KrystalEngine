#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include <atomic>
#include <compare>
#include <format>
#include <memory>
#include <ostream>
#include <type_traits>

namespace Krys::detail
{
  struct DetectAddRef
  {
    template <typename Traits, typename T>
    auto operator()(RawPtr<Traits>, RawPtr<T> p) noexcept(noexcept(Traits::AddRef(p)))
      -> decltype(Traits::AddRef(p));
  };

  struct DetectSubRef
  {
    template <typename Traits, typename T>
    auto operator()(RawPtr<Traits>, RawPtr<T> p) noexcept(noexcept(Traits::SubRef(p)))
      -> decltype(Traits::SubRef(p));
  };
}

namespace Krys
{
  template <typename Traits, typename T>
  concept AreIntrusiveSharedTraits = NoThrowInvocable<detail::DetectAddRef, RawPtr<Traits>, RawPtr<T>>
                                     && NoThrowInvocable<detail::DetectSubRef, RawPtr<Traits>, RawPtr<T>>;

  template <typename T, typename Traits>
  class KRYS_TRIVIAL_ABI IntrusivePtr
  {
    static_assert(AreIntrusiveSharedTraits<Traits, T>, "Invalid Traits for type T");

  private:
    RawPtr<T> _ptr;

    friend std::atomic<IntrusivePtr<T, Traits>>;
    friend std::out_ptr_t<IntrusivePtr<T, Traits>, T *>;
    friend std::inout_ptr_t<IntrusivePtr<T, Traits>, T *>;

  public:
    using pointer = RawPtr<T>;
    using element_type = T;
    using traits_type = Traits;

    KRYS_NODISCARD static constexpr IntrusivePtr NoRef(T *p) noexcept
    {
      return IntrusivePtr(p);
    }

    KRYS_NODISCARD static constexpr IntrusivePtr WithRef(T *p) noexcept
    {
      IntrusivePtr::DoAddRef(p);
      return IntrusivePtr(p);
    }

    constexpr IntrusivePtr() noexcept : _ptr(nullptr)
    {
    }

    constexpr IntrusivePtr(std::nullptr_t) noexcept : _ptr(nullptr)
    {
    }

    constexpr IntrusivePtr(const IntrusivePtr<T, Traits> &src) noexcept : _ptr(src._ptr)
    {
      this->DoAddRef(this->_ptr);
    }

    constexpr IntrusivePtr(IntrusivePtr<T, Traits> &&src) noexcept : _ptr(src.release())
    {
    }

    constexpr IntrusivePtr<T, Traits> &operator=(const IntrusivePtr<T, Traits> &src) noexcept
    {
      RawPtr<T> tmp = this->_ptr;
      this->_ptr = src._ptr;
      this->DoAddRef(this->_ptr);
      this->DoSubRef(tmp);
      return *this;
    }

    constexpr IntrusivePtr<T, Traits> &operator=(IntrusivePtr<T, Traits> &&src) noexcept
    {
      RawPtr<T> newValue = src.release();
      RawPtr<T> oldValue = this->_ptr; // this must come second so it is nullptr if src is us

      this->_ptr = newValue;
      this->DoSubRef(oldValue);

      return *this;
    }

    template <typename Y, typename YTraits>
    requires(ConvertibleTo<RawPtr<Y>, RawPtr<T>>)
    constexpr IntrusivePtr(const IntrusivePtr<Y, YTraits> &src) noexcept : _ptr(src.get())
    {
      this->DoAddRef(this->_ptr);
    }

    template <typename Y>
    requires(ConvertibleTo<RawPtr<Y>, RawPtr<T>>)
    constexpr IntrusivePtr(IntrusivePtr<Y, Traits> &&src) noexcept : _ptr(src.release())
    {
    }

    template <typename Y, typename YTraits>
    requires(ConvertibleTo<RawPtr<Y>, RawPtr<T>>)
    constexpr IntrusivePtr(IntrusivePtr<Y, YTraits> &&src) noexcept : _ptr(src.get())
    {
      this->DoAddRef(this->_ptr);
      src.reset();
    }

    template <typename Y, typename YTraits>
    requires(ConvertibleTo<RawPtr<Y>, RawPtr<T>>)
    constexpr IntrusivePtr<T, Traits> &operator=(const IntrusivePtr<Y, YTraits> &src) noexcept
    {
      RawPtr<T> temp = this->_ptr;
      this->_ptr = src.get();

      this->DoAddRef(this->_ptr);
      this->DoSubRef(temp);

      return *this;
    }

    template <typename Y>
    requires(ConvertibleTo<RawPtr<Y>, RawPtr<T>>)
    constexpr IntrusivePtr<T, Traits> &operator=(IntrusivePtr<Y, Traits> &&src) noexcept
    {
      this->DoSubRef(this->_ptr);
      this->_ptr = src.release();

      return *this;
    }

    template <typename Y, typename YTraits>
    requires(ConvertibleTo<RawPtr<Y>, RawPtr<T>>)
    constexpr IntrusivePtr<T, Traits> &operator=(IntrusivePtr<Y, YTraits> &&src) noexcept
    {
      this->DoSubRef(this->_ptr);
      this->_ptr = src.get();
      this->DoAddRef(this->_ptr);
      src.reset();

      return *this;
    }

    constexpr ~IntrusivePtr() noexcept
    {
      this->reset();
    }

    constexpr RawPtr<T> get() const noexcept
    {
      return this->_ptr;
    }

    constexpr RawPtr<T> operator->() const noexcept
    {
      return this->_ptr;
    }

    template <typename X = T>
    constexpr enable_if_t<SameType<X, T>, X &> operator*() const noexcept
    {
      return *this->_ptr;
    }

    template <typename M, typename X = T>
    constexpr enable_if_t<SameType<X, T>, M &> operator->*(M X::*memptr) const noexcept
    {
      return this->_ptr->*memptr;
    }

    constexpr explicit operator bool() const noexcept
    {
      return this->_ptr;
    }

    constexpr RawPtr<T> release() noexcept
    {
      return std::exchange(this->_ptr, nullptr);
    }

    KRYS_ALWAYS_INLINE constexpr void reset() noexcept // GCC refuses to inline this otherwise
    {
      this->DoSubRef(this->_ptr);
      this->_ptr = nullptr;
    }

    constexpr void swap(IntrusivePtr<T, Traits> &other) noexcept
    {
      RawPtr<T> temp = this->_ptr;
      this->_ptr = other._ptr;
      other._ptr = temp;
    }

    friend constexpr void swap(IntrusivePtr<T, Traits> &lhs, IntrusivePtr<T, Traits> &rhs) noexcept
    {
      lhs.swap(rhs);
    }

    template <typename Y, typename YTraits>
    friend constexpr bool operator==(const IntrusivePtr<T, Traits> &lhs,
                                     const IntrusivePtr<Y, YTraits> &rhs) noexcept
    {
      return lhs._ptr == rhs.get();
    }

    template <typename Y>
    friend constexpr bool operator==(const IntrusivePtr<T, Traits> &lhs, const Y *rhs) noexcept
    {
      return lhs._ptr == rhs;
    }

    template <typename Y>
    friend constexpr bool operator==(const Y *lhs, const IntrusivePtr<T, Traits> &rhs) noexcept
    {
      return lhs == rhs._ptr;
    }

    friend constexpr bool operator==(const IntrusivePtr<T, Traits> &lhs, std::nullptr_t) noexcept
    {
      return lhs._ptr == nullptr;
    }

    friend constexpr bool operator==(std::nullptr_t, const IntrusivePtr<T, Traits> &rhs) noexcept
    {
      return nullptr == rhs._ptr;
    }

    template <typename Y, typename YTraits>
    friend constexpr bool operator!=(const IntrusivePtr<T, Traits> &lhs,
                                     const IntrusivePtr<Y, YTraits> &rhs) noexcept
    {
      return !(lhs == rhs);
    }

    template <typename Y>
    friend constexpr bool operator!=(const IntrusivePtr<T, Traits> &lhs, const Y *rhs) noexcept
    {
      return !(lhs == rhs);
    }

    template <typename Y>
    friend constexpr bool operator!=(const Y *lhs, const IntrusivePtr<T, Traits> &rhs) noexcept
    {
      return !(lhs == rhs);
    }

    friend constexpr bool operator!=(const IntrusivePtr<T, Traits> &lhs, std::nullptr_t) noexcept
    {
      return !(lhs == nullptr);
    }

    friend constexpr bool operator!=(std::nullptr_t, const IntrusivePtr<T, Traits> &rhs) noexcept
    {
      return !(nullptr == rhs);
    }

    template <class Y, class YTraits>
    friend constexpr auto operator<=>(const IntrusivePtr<T, Traits> &lhs,
                                      const IntrusivePtr<Y, YTraits> &rhs) noexcept
    {
      return lhs._ptr <=> rhs.get();
    }

    template <class Y>
    friend constexpr auto operator<=>(const IntrusivePtr<T, Traits> &lhs, const Y *rhs) noexcept
    {
      return lhs._ptr <=> rhs;
    }

    template <class Y>
    friend constexpr auto operator<=>(const Y *lhs, const IntrusivePtr<T, Traits> &rhs) noexcept
    {
      return lhs <=> rhs._ptr;
    }

    template <typename Char>
    friend std::basic_ostream<Char> &operator<<(std::basic_ostream<Char> &str,
                                                const IntrusivePtr<T, Traits> &ptr)
    {
      return str << ptr._ptr;
    }

    friend constexpr size_t hash_value(const IntrusivePtr<T, Traits> &ptr) noexcept
    {
      return std::hash<RawPtr<T>>()(ptr._ptr);
    }

  private:
    constexpr IntrusivePtr(RawPtr<T> ptr) noexcept : _ptr(ptr)
    {
    }

    static constexpr void DoAddRef(RawPtr<T> p) noexcept
    {
      if (p != nullptr)
      {
        Traits::AddRef(p);
      }
    }

    static constexpr void DoSubRef(RawPtr<T> p) noexcept
    {
      if (p != nullptr)
      {
        Traits::SubRef(p);
      }
    }
  };
}

namespace Krys::detail
{
  template <typename T>
  std::false_type IsIntrusiveSharedPtrHelper(const T &);

  template <typename T, typename Traits>
  std::true_type IsIntrusiveSharedPtrHelper(const IntrusivePtr<T, Traits> &);
}

namespace Krys
{
  template <typename T>
  concept IsIntrusiveSharedPtr = decltype(detail::IsIntrusiveSharedPtrHelper(std::declval<T>()))::value;

  template <typename TDest, typename TSrc, typename Traits>
  inline constexpr enable_if_t<IsIntrusiveSharedPtr<TDest>, TDest>
    intrusive_const_cast(IntrusivePtr<TSrc, Traits> p) noexcept
  {
    return TDest::NoRef(const_cast<typename TDest::pointer>(p.release()));
  }

  template <typename TDest, typename TSrc, typename Traits>
  inline constexpr enable_if_t<IsIntrusiveSharedPtr<TDest>, TDest>
    intrusive_dynamic_cast(IntrusivePtr<TSrc, Traits> p) noexcept
  {
    if (auto res = dynamic_cast<typename TDest::pointer>(p.get()))
    {
      p.release();
      return TDest::NoRef(res);
    }
    return TDest();
  }

  template <typename TDest, typename TSrc, typename Traits>
  inline constexpr enable_if_t<IsIntrusiveSharedPtr<TDest>, TDest>
    intrusive_static_cast(IntrusivePtr<TSrc, Traits> p) noexcept
  {
    return TDest::NoRef(static_cast<typename TDest::pointer>(p.release()));
  }

  template <typename T, typename Traits = typename T::RefPtrTraits>
  using IntrusiveRefPtr = IntrusivePtr<T, Traits>;

  template <typename T>
  constexpr IntrusiveRefPtr<T> IntrusiveRefPtrRetain(RawPtr<T> ptr) noexcept
  {
    return IntrusiveRefPtr<T>::WithRef(ptr);
  }

  template <typename T>
  constexpr IntrusiveRefPtr<T> IntrusiveRefPtrAttach(RawPtr<T> ptr) noexcept
  {
    return IntrusiveRefPtr<T>::NoRef(ptr);
  }

  template <typename T, typename... Args>
  inline IntrusiveRefPtr<T> CreateIntrusiveRefPtr(Args &&...args)
  {
    return IntrusiveRefPtr<T>::NoRef(new T(std::forward<Args>(args)...));
  }

  template <typename T>
  inline IntrusiveRefPtr<typename T::weak_value_type> weak_cast(const IntrusiveRefPtr<T> &src)
  {
    return src->GetWeakPtr();
  }

  template <typename T>
  inline IntrusiveRefPtr<const typename T::weak_value_type> weak_cast(const IntrusiveRefPtr<const T> &src)
  {
    return src->GetWeakPtr();
  }

  template <typename T>
  inline IntrusiveRefPtr<typename T::strong_value_type> strong_cast(const IntrusiveRefPtr<T> &src) noexcept
  {
    return src->Lock();
  }

  template <typename T>
  inline IntrusiveRefPtr<const typename T::strong_value_type>
    strong_cast(const IntrusiveRefPtr<const T> &src) noexcept
  {
    return src->Lock();
  }
}

namespace std
{
  template <typename Traits, typename T>
  class atomic<::Krys::IntrusivePtr<T, Traits>>
  {
  public:
    using value_type = ::Krys::IntrusivePtr<T, Traits>;

  public:
    static constexpr bool is_always_lock_free = std::atomic<T *>::is_always_lock_free;

    constexpr atomic() noexcept = default;
    atomic(value_type desired) noexcept : _ptr(desired._ptr)
    {
      desired._ptr = nullptr;
    }

    atomic(const atomic &) = delete;
    void operator=(const atomic &) = delete;

    ~atomic() noexcept
    {
      value_type::DoSubRef(this->_ptr.load(memory_order_acquire));
    }

    void operator=(value_type desired) noexcept
    {
      this->store(std::move(desired));
    }

    operator value_type() const noexcept
    {
      return this->load();
    }

    value_type load(memory_order order = memory_order_seq_cst) const noexcept
    {
      T *ret = this->_ptr.load(order);
      return value_type::WithRef(ret);
    }

    void store(value_type desired, memory_order order = memory_order_seq_cst) noexcept
    {
      exchange(std::move(desired), order);
    }

    value_type exchange(value_type desired, memory_order order = memory_order_seq_cst) noexcept
    {
      T *ret = this->_ptr.exchange(desired._ptr, order);
      desired._ptr = nullptr;
      return value_type::NoRef(ret);
    }

    bool compare_exchange_strong(value_type &expected, value_type desired, memory_order success,
                                 memory_order failure) noexcept
    {
      T *saved_expected = expected._ptr;

      bool ret = this->_ptr.compare_exchange_strong(expected._ptr, desired._ptr, success, failure);
      return post_compare_exchange(ret, saved_expected, expected, desired);
    }
    bool compare_exchange_strong(value_type &expected, value_type desired,
                                 memory_order order = memory_order_seq_cst) noexcept
    {
      T *saved_expected = expected._ptr;

      bool ret = this->_ptr.compare_exchange_strong(expected._ptr, desired._ptr, order);
      return post_compare_exchange(ret, saved_expected, expected, desired);
    }

    bool compare_exchange_weak(value_type &expected, value_type desired, memory_order success,
                               memory_order failure) noexcept
    {
      T *saved_expected = expected._ptr;

      bool ret = this->_ptr.compare_exchange_weak(expected._ptr, desired._ptr, success, failure);
      return post_compare_exchange(ret, saved_expected, expected, desired);
    }
    bool compare_exchange_weak(value_type &expected, value_type desired,
                               memory_order order = memory_order_seq_cst) noexcept
    {
      T *saved_expected = expected._ptr;

      bool ret = this->_ptr.compare_exchange_weak(expected._ptr, desired._ptr, order);
      return post_compare_exchange(ret, saved_expected, expected, desired);
    }

    bool is_lock_free() const noexcept
    {
      return this->_ptr.is_lock_free();
    }

  private:
    static bool post_compare_exchange(bool exchange_result, T *saved_expected, value_type &expected,
                                      value_type &desired) noexcept
    {
      if (exchange_result)
      {
        // success: we are desired and expected is unchanged
        desired._ptr = nullptr;
        // saved_expected is equal to our original value which we need to SubRef
        value_type::DoSubRef(saved_expected);
      }
      else
      {
        // failure: expected is us and desired is unchanged.
        value_type::DoAddRef(expected._ptr);  // our value going out
        value_type::DoSubRef(saved_expected); // old expected
      }
      return exchange_result;
    }

  private:
    std::atomic<T *> _ptr = nullptr;
  };

  template <typename T, typename Traits>
  class out_ptr_t<::Krys::IntrusivePtr<T, Traits>, T *>
  {
  public:
    constexpr out_ptr_t(::Krys::IntrusivePtr<T, Traits> &owner) noexcept : _ptr(&owner._ptr)
    {
      owner.reset();
    }
    constexpr out_ptr_t(out_ptr_t &&src) noexcept = default;
    out_ptr_t(const out_ptr_t &) = delete;

    void operator=(const out_ptr_t &) = delete;
    void operator=(out_ptr_t &&) = delete;

    constexpr operator T **() const noexcept
    {
      return _ptr;
    }

    constexpr operator void **() const noexcept
    requires(!std::is_same_v<T *, void *>)
    {
      return reinterpret_cast<void **>(_ptr);
    }

  private:
    T **_ptr;
  };

  template <typename T, typename Traits>
  class inout_ptr_t<::Krys::IntrusivePtr<T, Traits>, T *>
  {
  public:
    constexpr inout_ptr_t(::Krys::IntrusivePtr<T, Traits> &owner) noexcept : _ptr(&owner._ptr)
    {
    }

    constexpr inout_ptr_t(inout_ptr_t &&src) noexcept = default;
    inout_ptr_t(const inout_ptr_t &) = delete;

    void operator=(const inout_ptr_t &) = delete;
    void operator=(inout_ptr_t &&) = delete;

    constexpr operator T **() const noexcept
    {
      return _ptr;
    }

    constexpr operator void **() const noexcept
    requires(!std::is_same_v<T *, void *>)
    {
      return reinterpret_cast<void **>(_ptr);
    }

  private:
    T **_ptr;
  };

  template <typename T, typename Traits, typename TChar>
  struct formatter<::Krys::IntrusivePtr<T, Traits>, TChar> : public formatter<void *, TChar>
  {
    template <typename FormatContext>
    auto format(const ::Krys::IntrusivePtr<T, Traits> &ptr, FormatContext &ctx) const -> decltype(ctx.out())
    {
      return formatter<void *, TChar>::format(ptr.get(), ctx);
    }
  };

  template <typename T, typename Traits>
  struct hash<::Krys::IntrusivePtr<T, Traits>>
  {
    constexpr size_t operator()(const ::Krys::IntrusivePtr<T, Traits> &ptr) const noexcept
    {
      return hash_value(ptr);
    }
  };
}
