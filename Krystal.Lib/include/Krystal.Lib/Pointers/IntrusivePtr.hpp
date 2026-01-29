#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include <atomic>
#include <compare>
#include <format>
#include <memory>
#include <ostream>
#include <type_traits>

namespace Krys
{
  namespace detail
  {
    struct add_ref_detector
    {
      template <class Traits, class T>
      auto operator()(Traits *, T *p) noexcept(noexcept(Traits::AddRef(p))) -> decltype(Traits::AddRef(p));
    };

    struct sub_ref_detector
    {
      template <class Traits, class T>
      auto operator()(Traits *, T *p) noexcept(noexcept(Traits::SubRef(p))) -> decltype(Traits::SubRef(p));
    };

  }

  template <class Traits, class T>
  constexpr bool are_intrusive_shared_traits =
    std::is_nothrow_invocable_v<detail::add_ref_detector, Traits *, T *>
    && std::is_nothrow_invocable_v<detail::sub_ref_detector, Traits *, T *>;

  template <class T, class Traits>
  class KRYS_TRIVIAL_ABI IntrusivePtr
  {
  private:
    T *_ptr;

    static_assert(are_intrusive_shared_traits<Traits, T>, "Invalid Traits for type T");
    friend std::atomic<IntrusivePtr<T, Traits>>;
    friend std::out_ptr_t<IntrusivePtr<T, Traits>, T *>;
    friend std::inout_ptr_t<IntrusivePtr<T, Traits>, T *>;

  public:
    using pointer = T *;
    using element_type = T;
    using traits_type = Traits;

  private:
    class OutputParam
    {
      friend class IntrusivePtr<T, Traits>;

    public:
      constexpr operator T **() && noexcept
      {
        return _ptr;
      }

    private:
      constexpr OutputParam(IntrusivePtr<T, Traits> &owner) noexcept : _ptr(&owner._ptr)
      {
        owner.reset();
      }
      constexpr OutputParam(OutputParam &&src) noexcept = default;

      OutputParam(const OutputParam &) = delete;
      void operator=(const OutputParam &) = delete;
      void operator=(OutputParam &&) = delete;

    private:
      T **_ptr;
    };

    class InOutParam
    {
      friend class IntrusivePtr<T, Traits>;

    public:
      constexpr operator T **() && noexcept
      {
        return _ptr;
      }

    private:
      constexpr InOutParam(IntrusivePtr<T, Traits> &owner) noexcept : _ptr(&owner._ptr)
      {
      }
      constexpr InOutParam(InOutParam &&src) noexcept = default;

      InOutParam(const InOutParam &) = delete;
      void operator=(const InOutParam &) = delete;
      void operator=(InOutParam &&) = delete;

    private:
      T **_ptr;
    };

  public:
    static constexpr IntrusivePtr NoRef(T *p) noexcept
    {
      return IntrusivePtr(p);
    }

    static constexpr IntrusivePtr Ref(T *p) noexcept
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
      T *temp = this->_ptr;
      this->_ptr = src._ptr;
      this->DoAddRef(this->_ptr);
      this->DoSubRef(temp);
      return *this;
    }

    constexpr IntrusivePtr<T, Traits> &operator=(IntrusivePtr<T, Traits> &&src) noexcept
    {
      T *new_val = src.release();
      // this must come second so it is nullptr if src is us
      T *old_val = this->_ptr;
      this->_ptr = new_val;
      this->DoSubRef(old_val);
      return *this;
    }

    template <class Y, class YTraits, class = std::enable_if_t<std::is_convertible_v<Y *, T *>, void>>
    constexpr IntrusivePtr(const IntrusivePtr<Y, YTraits> &src) noexcept : _ptr(src.get())
    {
      this->DoAddRef(this->_ptr);
    }

    template <class Y, class = std::enable_if_t<std::is_convertible_v<Y *, T *>, void>>
    constexpr IntrusivePtr(IntrusivePtr<Y, Traits> &&src) noexcept : _ptr(src.release())
    {
    }

    template <class Y, class YTraits, class = std::enable_if_t<std::is_convertible_v<Y *, T *>, void>>
    constexpr IntrusivePtr(IntrusivePtr<Y, YTraits> &&src) noexcept : _ptr(src.get())
    {
      this->DoAddRef(this->_ptr);
      src.reset();
    }

    template <class Y, class YTraits, class = std::enable_if_t<std::is_convertible_v<Y *, T *>, void>>
    constexpr IntrusivePtr<T, Traits> &operator=(const IntrusivePtr<Y, YTraits> &src) noexcept
    {
      T *temp = this->_ptr;
      this->_ptr = src.get();
      this->DoAddRef(this->_ptr);
      this->DoSubRef(temp);
      return *this;
    }

    template <class Y, class = std::enable_if_t<std::is_convertible_v<Y *, T *>, void>>
    constexpr IntrusivePtr<T, Traits> &operator=(IntrusivePtr<Y, Traits> &&src) noexcept
    {
      this->DoSubRef(this->_ptr);
      this->_ptr = src.release();
      return *this;
    }

    template <class Y, class YTraits, class = std::enable_if_t<std::is_convertible_v<Y *, T *>, void>>
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

    constexpr T *get() const noexcept
    {
      return this->_ptr;
    }

    constexpr T *operator->() const noexcept
    {
      return this->_ptr;
    }

    template <class X = T>
    constexpr std::enable_if_t<SameType<X, T>, X &> operator*() const noexcept
    {
      return *this->_ptr;
    }

    template <class M, class X = T>
    constexpr std::enable_if_t<SameType<X, T>, M &> operator->*(M X::*memptr) const noexcept
    {
      return this->_ptr->*memptr;
    }

    constexpr explicit operator bool() const noexcept
    {
      return this->_ptr;
    }

    constexpr OutputParam GetOutputParam() noexcept
    {
      return OutputParam(*this);
    }

    constexpr InOutParam GetInOutParam() noexcept
    {
      return InOutParam(*this);
    }

    constexpr T *release() noexcept
    {
      T *p = this->_ptr;
      this->_ptr = nullptr;
      return p;
    }

    KRYS_ALWAYS_INLINE constexpr void reset() noexcept // GCC refuses to inline this otherwise
    {
      this->DoSubRef(this->_ptr);
      this->_ptr = nullptr;
    }

    constexpr void swap(IntrusivePtr<T, Traits> &other) noexcept
    {
      T *temp = this->_ptr;
      this->_ptr = other._ptr;
      other._ptr = temp;
    }

    friend constexpr void swap(IntrusivePtr<T, Traits> &lhs, IntrusivePtr<T, Traits> &rhs) noexcept
    {
      lhs.swap(rhs);
    }

    template <class Y, class YTraits>
    friend constexpr bool operator==(const IntrusivePtr<T, Traits> &lhs,
                                     const IntrusivePtr<Y, YTraits> &rhs) noexcept
    {
      return lhs._ptr == rhs.get();
    }

    template <class Y>
    friend constexpr bool operator==(const IntrusivePtr<T, Traits> &lhs, const Y *rhs) noexcept
    {
      return lhs._ptr == rhs;
    }

    template <class Y>
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

    template <class Y, class YTraits>
    friend constexpr bool operator!=(const IntrusivePtr<T, Traits> &lhs,
                                     const IntrusivePtr<Y, YTraits> &rhs) noexcept
    {
      return !(lhs == rhs);
    }

    template <class Y>
    friend constexpr bool operator!=(const IntrusivePtr<T, Traits> &lhs, const Y *rhs) noexcept
    {
      return !(lhs == rhs);
    }

    template <class Y>
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

    template <class Char>
    friend std::basic_ostream<Char> &operator<<(std::basic_ostream<Char> &str,
                                                const IntrusivePtr<T, Traits> &ptr)
    {
      return str << ptr._ptr;
    }

    friend constexpr size_t hash_value(const IntrusivePtr<T, Traits> &ptr) noexcept
    {
      return std::hash<T *>()(ptr._ptr);
    }

  private:
    constexpr IntrusivePtr(T *ptr) noexcept : _ptr(ptr)
    {
    }

    static constexpr void DoAddRef(T *p) noexcept
    {
      if (p)
      {
        Traits::AddRef(p);
      }
    }

    static constexpr void DoSubRef(T *p) noexcept
    {
      if (p)
      {
        Traits::SubRef(p);
      }
    }
  };

  namespace detail
  {
    template <class T>
    std::false_type IsIntrusiveSharedPtrHelper(const T &);

    template <class T, class Traits>
    std::true_type IsIntrusiveSharedPtrHelper(const IntrusivePtr<T, Traits> &);
  }

  template <class T>
  using IsIntrusiveSharedPtr = decltype(detail::IsIntrusiveSharedPtrHelper(std::declval<T>()));

  template <class T>
  bool constexpr is_intrusive_shared_ptr_v = IsIntrusiveSharedPtr<T>::value;

  template <class Dest, class Src, class Traits>
  inline constexpr std::enable_if_t<is_intrusive_shared_ptr_v<Dest>, Dest>
    intrusive_const_cast(IntrusivePtr<Src, Traits> p) noexcept
  {
    return Dest::NoRef(const_cast<typename Dest::pointer>(p.release()));
  }

  template <class Dest, class Src, class Traits>
  inline constexpr std::enable_if_t<is_intrusive_shared_ptr_v<Dest>, Dest>
    intrusive_dynamic_cast(IntrusivePtr<Src, Traits> p) noexcept
  {
    auto res = dynamic_cast<typename Dest::pointer>(p.get());
    if (res)
    {
      p.release();
      return Dest::NoRef(res);
    }
    return Dest();
  }

  template <class Dest, class Src, class Traits>
  inline constexpr std::enable_if_t<is_intrusive_shared_ptr_v<Dest>, Dest>
    intrusive_static_cast(IntrusivePtr<Src, Traits> p) noexcept
  {
    return Dest::NoRef(static_cast<typename Dest::pointer>(p.release()));
  }
}

namespace std
{
  template <class Traits, class T>
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
      return value_type::Ref(ret);
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

  template <class T, class Traits>
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

  template <class T, class Traits>
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

  template <class T, class Traits, class CharT>
  struct formatter<::Krys::IntrusivePtr<T, Traits>, CharT> : public formatter<void *, CharT>
  {
    template <typename FormatContext>
    auto format(const ::Krys::IntrusivePtr<T, Traits> &ptr, FormatContext &ctx) const -> decltype(ctx.out())
    {
      return formatter<void *, CharT>::format(ptr.get(), ctx);
    }
  };

  template <class T, class Traits>
  struct hash<::Krys::IntrusivePtr<T, Traits>>
  {
    constexpr size_t operator()(const ::Krys::IntrusivePtr<T, Traits> &ptr) const noexcept
    {
      return hash_value(ptr);
    }
  };
}
